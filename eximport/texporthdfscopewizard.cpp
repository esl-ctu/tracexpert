#include "texporthdfscopewizard.h"

#include "thdfsession.h"
#include "thdfbrowserwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFileInfo>
#include <QSplitter>
#include <QListWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include <QDebug>
#include <QInputDialog>
#include <QTimer>
#include <QDir>


static QString toTypeText(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TUInt8:  return "uint8";
    case TScope::TSampleType::TInt8:   return "int8";
    case TScope::TSampleType::TUInt16: return "uint16";
    case TScope::TSampleType::TInt16:  return "int16";
    case TScope::TSampleType::TUInt32: return "uint32";
    case TScope::TSampleType::TInt32:  return "int32";
    case TScope::TSampleType::TReal32: return "float32";
    case TScope::TSampleType::TReal64: return "float64";
    }
    return "float32";
}

static std::size_t sampleTypeBytes(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TUInt8:  return 1;
    case TScope::TSampleType::TInt8:   return 1;
    case TScope::TSampleType::TUInt16: return 2;
    case TScope::TSampleType::TInt16:  return 2;
    case TScope::TSampleType::TUInt32: return 4;
    case TScope::TSampleType::TInt32:  return 4;
    case TScope::TSampleType::TReal32: return 4;
    case TScope::TSampleType::TReal64: return 8;
    }
    return 4;
}

static H5T_class_t expectedTypeClass(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TReal32:
    case TScope::TSampleType::TReal64:
        return H5T_FLOAT;
    default:
        return H5T_INTEGER;
    }
}

struct ChunkProposal {
    qulonglong rows = 1;
    qulonglong cols = 1;
};

static ChunkProposal proposeTraceChunks(qulonglong samplesPerTrace, std::size_t elemBytes)
{
    ChunkProposal p;
    if (samplesPerTrace == 0 || elemBytes == 0) {
        p.rows = 1;
        p.cols = 1;
        return p;
    }

    const qulonglong targetBytes = 1024ULL * 1024ULL; // 1 MiB target
    const qulonglong bytesPerSample = static_cast<qulonglong>(elemBytes);

    qulonglong preferredCols = samplesPerTrace;
    qulonglong rowBytes = preferredCols * bytesPerSample;

    if (rowBytes > targetBytes) {
        qulonglong cappedCols = targetBytes / bytesPerSample;
        if (cappedCols < 1) cappedCols = 1;
        if (cappedCols > samplesPerTrace) cappedCols = samplesPerTrace;
        preferredCols = cappedCols;
        rowBytes = preferredCols * bytesPerSample;
    }

    qulonglong rows = targetBytes / rowBytes;
    if (rows < 1) rows = 1;
    if (rows > 1024) rows = 1024; // practical clamp

    p.rows = rows;
    p.cols = preferredCols;
    return p;
}

static hid_t sampleTypeToNativeH5Type(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TUInt8:  return H5T_NATIVE_UINT8;
    case TScope::TSampleType::TInt8:   return H5T_NATIVE_INT8;
    case TScope::TSampleType::TUInt16: return H5T_NATIVE_UINT16;
    case TScope::TSampleType::TInt16:  return H5T_NATIVE_INT16;
    case TScope::TSampleType::TUInt32: return H5T_NATIVE_UINT32;
    case TScope::TSampleType::TInt32:  return H5T_NATIVE_INT32;
    case TScope::TSampleType::TReal32: return H5T_NATIVE_FLOAT;
    case TScope::TSampleType::TReal64: return H5T_NATIVE_DOUBLE;
    }
    return H5T_NATIVE_FLOAT;
}

static bool isTotallyEmptyHdf(THdfSession *s)
{
    if (!s || !s->isOpen())
        return true; // treat as "empty" from UX standpoint

    return s->listChildren("/").isEmpty();
}

// ---------------- PAGE 1 ----------------

ExportFilePage::ExportFilePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Export destination and scope"));
    setSubTitle(tr("Choose output file, channels, and trace range."));

    auto *fileLabel = new QLabel(tr("Output file:"));
    m_fileEdit = new QLineEdit;

    auto *browseBtn = new QPushButton(tr("Browse…"));

    auto *fileRow = new QHBoxLayout;
    fileRow->addWidget(m_fileEdit);
    fileRow->addWidget(browseBtn);

    auto *channelsLabel = new QLabel(tr("Channels to export:"));
    m_channelsLayout = new QVBoxLayout;

    auto *tracesLabel = new QLabel(tr("Traces to export:"));
    m_radioCurrentTrace = new QRadioButton(tr("Currently displayed trace"));
    m_radioAllTraces    = new QRadioButton(tr("All measured traces"));
    m_radioAllTraces->setChecked(true);

    auto *traceLayout = new QVBoxLayout;
    traceLayout->addWidget(m_radioCurrentTrace);
    traceLayout->addWidget(m_radioAllTraces);

    auto *layout = new QVBoxLayout;
    layout->addWidget(fileLabel);
    layout->addLayout(fileRow);
    layout->addSpacing(8);

    layout->addWidget(channelsLabel);
    layout->addLayout(m_channelsLayout);
    layout->addSpacing(8);

    layout->addWidget(tracesLabel);
    layout->addLayout(traceLayout);

    setLayout(layout);

    registerField("outputFile*", m_fileEdit);
    registerField("exportCurrentTrace", m_radioCurrentTrace);
    registerField("exportAllTraces",    m_radioAllTraces);

    connect(browseBtn, &QPushButton::clicked, this, [this] {
        QFileDialog dlg(this, tr("Select HDF5 file"));
        dlg.setAcceptMode(QFileDialog::AcceptSave);
        dlg.setNameFilter(tr("HDF5 files (*.h5 *.hdf5);;All files (*.*)"));
        dlg.setDefaultSuffix("h5");
        dlg.setOption(QFileDialog::DontConfirmOverwrite, true);

        if (dlg.exec() == QDialog::Accepted) {
            const auto files = dlg.selectedFiles();
            if (!files.isEmpty())
                m_fileEdit->setText(files.first());
        }
    });
}

void ExportFilePage::initializePage()
{
    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    if (!wiz)
        return;

    if (m_fileEdit->text().isEmpty()) {
        const QString initial = wiz->initialFileName();
        if (!initial.isEmpty())
            m_fileEdit->setText(initial);
    }

    const QStringList &channels = wiz->channels();

    if (m_channelChecks.isEmpty()) {
        for (int i = 0; i < channels.size(); ++i) {
            auto *cb = new QCheckBox(channels.at(i), this);
            cb->setChecked(false);
            m_channelsLayout->addWidget(cb);
            m_channelChecks.append(cb);

            const QString fieldName = QStringLiteral("channel_%1").arg(i);
            registerField(fieldName, cb);
        }
    } else if (m_channelChecks.size() == channels.size()) {
        for (int i = 0; i < channels.size(); ++i)
            m_channelChecks[i]->setText(channels.at(i));
    }
}

bool ExportFilePage::validatePage()
{
    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    if (!wiz) {
        qCritical() << "[ExportFilePage] Wizard cast failed";
        return false;
    }

    QString path = m_fileEdit->text().trimmed();

    while (path.endsWith('.'))
        path.chop(1);

    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("No output file"),
                             tr("Please choose an output file."));
        return false;
    }

    QFileInfo fi(path);

    if (fi.suffix().isEmpty()) {
        path += ".h5";
        m_fileEdit->setText(path);
    }

    bool anyChecked = false;
    for (QCheckBox *cb : m_channelChecks) {
        if (cb->isChecked()) {
            anyChecked = true;
            break;
        }
    }
    if (!anyChecked) {
        QMessageBox::warning(this,
                             tr("No channels selected"),
                             tr("Please select at least one channel to export."));
        return false;
    }

    if (wiz->hdfSession()->isOpen()) {
        qDebug() << "[ExportFilePage] Closing previous HDF session:" << wiz->hdfSession()->filePath();
        wiz->hdfSession()->close();
    }

    const QFileInfo fi2(path);
    if (!fi2.exists()) {
        qDebug() << "[HDF] File does not exist -> create:" << path;
        if (!wiz->hdfSession()->createNew(path)) {
            qCritical() << "[HDF] Failed to create new HDF5 file:" << path;
            QMessageBox::warning(this, tr("Cannot create HDF5 file"),
                                 tr("Failed to create the HDF5 file:\n%1").arg(path));
            return false;
        }
    } else {
        qDebug() << "[HDF] File exists -> open:" << path;
        if (!wiz->hdfSession()->openExisting(path)) {
            qCritical() << "[HDF] Open failed; file may be corrupted/truncated/locked:" << path;
            QMessageBox::warning(this, tr("Cannot open HDF5 file"),
                                 tr("The selected file exists but cannot be opened as a valid HDF5 file.\n"
                                    "It may be corrupted or truncated.\n\n%1").arg(path));
            return false;
        }
    }

    qDebug() << "[HDF] Open OK:" << wiz->hdfSession()->filePath();
    return true;
}


// ---------------- PAGE 2 ----------------

ChannelTargetsPage::ChannelTargetsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Channel targets"));
    setSubTitle(tr("Browse the HDF5 structure, create groups/datasets, and select datasets for traces and metadata."));

    m_channelList = new QListWidget;
    m_channelList->setMinimumWidth(200);

    m_stack = new QStackedWidget;

    connect(m_channelList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < m_stack->count())
            m_stack->setCurrentIndex(row);

        if (row < 0 || row >= m_channelsUi.size())
            return;

        ChannelUi &ui = m_channelsUi[row];
        const int tab = (ui.tabs ? ui.tabs->currentIndex() : 0);

        if (tab == 0 && ui.tracesBrowser) ui.tracesBrowser->refresh();
        if (tab == 1 && ui.metaBrowser)   ui.metaBrowser->refresh();
    });

    auto *splitter = new QSplitter;
    splitter->addWidget(m_channelList);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setTextFormat(Qt::RichText);
    m_fileNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_filePathLabel = new QLabel(this);
    m_filePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_filePathLabel->setWordWrap(false);
    m_filePathLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *layout = new QVBoxLayout;
    layout->addWidget(m_fileNameLabel);
    layout->addWidget(m_filePathLabel);
    layout->addSpacing(6);
    layout->addWidget(splitter, 1);
    setLayout(layout);
}

bool ChannelTargetsPage::isPathSyntaxValid(const QString &path, QString *why) const
{
    const QString p = path.trimmed();
    if (p.isEmpty()) {
        if (why) *why = tr("Path cannot be empty.");
        return false;
    }
    if (!p.startsWith('/')) {
        if (why) *why = tr("Path must start with '/'.");
        return false;
    }
    static const QRegularExpression badChars(R"([\s\x00-\x1F])");
    if (p.contains(badChars)) {
        if (why) *why = tr("Path must not contain spaces or control characters.");
        return false;
    }
    return true;
}

void ChannelTargetsPage::resetUi()
{
    m_channelList->clear();

    while (m_stack->count() > 0) {
        QWidget *w = m_stack->widget(0);
        m_stack->removeWidget(w);
        w->deleteLater();
    }

    m_channelsUi.clear();
}

void ChannelTargetsPage::initializePage()
{
    resetUi();

    m_autoCreateOfferedThisVisit = false;

    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    if (!wiz) {
        qCritical() << "[ChannelTargetsPage] Wizard cast failed";
        return;
    }

    if (!wiz->hdfSession() || !wiz->hdfSession()->isOpen()) {
        qCritical() << "[ChannelTargetsPage] THdfSession missing or not open (Page 1 should open/create)";
        return;
    }

    wiz->setButtonText(QWizard::NextButton, tr("Append"));

    const QString path = wiz->hdfSession()->filePath();
    const QFileInfo fi(path);
    m_fileNameLabel->setText(tr("<b>File:</b> %1").arg(fi.fileName()));
    m_filePathLabel->setText(tr("Path: %1").arg(QDir::toNativeSeparators(fi.absoluteFilePath())));

    const QStringList all = wiz->channels();
    QStringList selected;
    for (int i = 0; i < all.size(); ++i) {
        const QString key = QString("channel_%1").arg(i);
        if (wiz->field(key).toBool())
            selected << all.at(i);
    }

    qDebug() << "[ChannelTargetsPage] Selected channels:" << selected;

    wiz->setChannelTargets({});

    for (int idx = 0; idx < selected.size(); ++idx) {
        ChannelUi ui;
        ui.alias = selected.at(idx);

        ui.root = new QWidget(this);
        auto *rootLayout = new QVBoxLayout(ui.root);

        ui.tabs = new QTabWidget(ui.root);

        // ---- Traces tab ----
        auto *tracesTab = new QWidget(ui.tabs);
        auto *tracesLayout = new QVBoxLayout(tracesTab);

        ui.tracesBrowser = new THdfBrowserWidget(tracesTab);
        ui.tracesBrowser->setSession(wiz->hdfSession().data());
        ui.tracesBrowser->setAutoPreselectSuggestedDataset(true);
        ui.tracesBrowser->setSelectionPolicy(THdfBrowserWidget::SelectionPolicy::DatasetsOnly);


        ui.tracesBrowser->setTitleText(
            tr("%1 destination dataset (to append)").arg(ui.alias)
            );

        const std::size_t samples = wiz->traceSamplesPerTrace();
        if (samples > 0) {
            const QString typeText = toTypeText(wiz->traceSampleType());
            const qulonglong samplesULL = static_cast<qulonglong>(samples);

            const ChunkProposal cp = proposeTraceChunks(samplesULL, sampleTypeBytes(wiz->traceSampleType()));

            THdfBrowserWidget::TDatasetTemplate tracesT;
            tracesT.suggestedName   = QString("ch%1/traces").arg(ui.alias);
            tracesT.rank            = 2;
            tracesT.initialDimsText = QString("0,%1").arg(QString::number(samplesULL));
            tracesT.maxDimsText     = QString("unlimited,%1").arg(QString::number(samplesULL));
            tracesT.typeText        = typeText;

            tracesT.lockRank = true;
            tracesT.lockInitialDims = true;
            tracesT.lockMaxDims = true;
            tracesT.lockType = true;

            tracesT.chunk2DRowBounded = true;
            tracesT.chunkRowsMin = 1;
            tracesT.chunkRowsMax = (cp.rows < 1 ? 1 : cp.rows);
            tracesT.chunkRowsDefault = tracesT.chunkRowsMax;
            tracesT.chunkColsFixed = (cp.cols < 1 ? 1 : cp.cols);

            tracesT.chunkDimsText = QString("%1,%2")
                                        .arg(QString::number(tracesT.chunkRowsDefault))
                                        .arg(QString::number(tracesT.chunkColsFixed));

            tracesT.deflateOn = false;

            ui.tracesBrowser->setNewDatasetTemplate(tracesT);

        } else {
            qCritical() << "[ChannelTargetsPage] samplesPerTrace is 0; traces template not applied";
        }

        auto *tracesTargetRow = new QHBoxLayout;
        auto *tracesTargetLabel = new QLabel(tr("Selected traces dataset:"));
        ui.tracesTarget = new QLineEdit;
        ui.tracesTarget->setReadOnly(true);
        tracesTargetRow->addWidget(tracesTargetLabel);
        tracesTargetRow->addWidget(ui.tracesTarget);

        auto *tracesBrowser = ui.tracesBrowser;
        auto *tracesTarget  = ui.tracesTarget;
        connect(tracesBrowser, &THdfBrowserWidget::selectionChanged, this,
                [tracesBrowser, tracesTarget](const QString &path, int /*type*/) {
                    if (tracesBrowser && tracesBrowser->selectedIsDataset())
                        tracesTarget->setText(path);
                });

        tracesLayout->addWidget(ui.tracesBrowser);
        tracesLayout->addLayout(tracesTargetRow);
        ui.tabs->addTab(tracesTab, tr("Traces"));

        // ---- Metadata tab ----
        auto *metaTab = new QWidget(ui.tabs);
        auto *metaLayout = new QVBoxLayout(metaTab);

        ui.metaBrowser = new THdfBrowserWidget(metaTab);
        ui.metaBrowser->setSession(wiz->hdfSession().data());
        ui.metaBrowser->setAutoPreselectPath(QString("ch%1/runs").arg(ui.alias));
        ui.metaBrowser->setAutoPreselectSuggestedDataset(true);
        ui.metaBrowser->setSelectionPolicy(THdfBrowserWidget::SelectionPolicy::GroupsOnly);


        ui.metaBrowser->setTitleText(
            tr("%1 metadata destination group").arg(ui.alias)
        );

        THdfBrowserWidget::TDatasetTemplate metaT;
        metaT.suggestedName = QString("");
        metaT.rank            = 1;
        metaT.lockRank = true;
        metaT.maxDimsText = "unlimited";
        metaT.lockMaxDims = true;
        metaT.typeText = "string";
        metaT.lockType = true;
        metaT.initialDimsText = "0";
        metaT.lockInitialDims = true;
        metaT.chunkDimsText = "256";
        metaT.deflateOn = false;

        ui.metaBrowser->setNewDatasetTemplate(metaT);

        auto *metaTargetRow = new QHBoxLayout;
        auto *metaTargetLabel = new QLabel(tr("Selected metadata group:"));
        ui.metaTarget = new QLineEdit;
        ui.metaTarget->setReadOnly(true);
        metaTargetRow->addWidget(metaTargetLabel);
        metaTargetRow->addWidget(ui.metaTarget);

        auto *metaBrowser = ui.metaBrowser;
        auto *metaTarget  = ui.metaTarget;
        connect(metaBrowser, &THdfBrowserWidget::selectionChanged, this,
                [metaBrowser, metaTarget](const QString &path, int /*type*/) {
                    if (!metaBrowser) return;
                    THdfSession *s = metaBrowser->session();
                    if (!s || !s->isOpen()) return;
                    if (s->isGroup(path))
                        metaTarget->setText(path);
                });


        auto *createMetaGroupBtn = new QPushButton(tr("Create metadata group…"));
        metaLayout->addWidget(createMetaGroupBtn);

        auto *metaBrowser2 = ui.metaBrowser;
        auto *metaTarget2  = ui.metaTarget;
        THdfSession *session = wiz->hdfSession().data();

        const QString channelAlias = ui.alias;

        connect(createMetaGroupBtn, &QPushButton::clicked, this,
                [this, metaBrowser2, metaTarget2, session, channelAlias]() {

                    if (!session || !session->isOpen() || !metaBrowser2) {
                        QMessageBox::warning(this, tr("HDF5"), tr("No open HDF5 file."));
                        return;
                    }

                    // Base = selected group, else parent of selected node, else "/"
                    QString base = "/";
                    const QString sel = THdfSession::normalizePath(metaBrowser2->selectedPath());
                    if (!sel.isEmpty() && session->pathExists(sel)) {
                        if (session->isGroup(sel)) base = sel;
                        else base = THdfSession::parentPath(sel);
                    }

                    bool ok = false;
                    const QString baseNorm = THdfSession::normalizePath(base);
                    const QString chRoot = THdfSession::normalizePath(QString("/ch%1").arg(channelAlias));
                    const bool baseInsideChannel =
                        (baseNorm == chRoot) || baseNorm.startsWith(chRoot + "/");

                    const QString defaultRel =
                        (baseNorm == "/" || !baseInsideChannel)
                            ? QString("ch%1/runs").arg(channelAlias)
                            : QString("runs");

                    const QString rel = QInputDialog::getText(
                                            metaBrowser2,
                                            tr("Create metadata group"),
                                            tr("Metadata group path (under %1):").arg(base),
                                            QLineEdit::Normal,
                                            defaultRel,
                                            &ok
                                            ).trimmed();

                    if (!ok || rel.isEmpty())
                        return;

                    if (rel.startsWith('/')) {
                        QMessageBox::warning(metaBrowser2, tr("Create metadata group"),
                                             tr("Please enter a relative path without a leading '/'."));
                        return;
                    }

                    QString newGroup;
                    if (base == "/") newGroup = THdfSession::normalizePath("/" + rel);
                    else             newGroup = THdfSession::normalizePath(base + "/" + rel);

                    if (session->pathExists(newGroup)) {

                        if (!session->isGroup(newGroup)) {
                            QMessageBox::warning(metaBrowser2, tr("Create metadata group"),
                                                 tr("A non-group object already exists at:\n%1").arg(newGroup));
                            return;
                        }

                        if (session->validateMetadataGroup(newGroup)) {
                            QMessageBox::warning(metaBrowser2, tr("Create metadata group"),
                                                 tr("A valid metadata group already exists:\n%1").arg(newGroup));
                            return;
                        }

                        if (!session->ensureMetadataGroup(newGroup)) {
                            QMessageBox::warning(metaBrowser2, tr("Create metadata group"),
                                                 tr("The group exists but could not be made into a valid metadata group:\n%1")
                                                     .arg(newGroup));
                            return;
                        }

                    } else {
                        if (!session->ensureMetadataGroup(newGroup)) {
                            QMessageBox::warning(metaBrowser2, tr("Create metadata group"),
                                                 tr("Failed to create a valid metadata group:\n%1").arg(newGroup));
                            return;
                        }
                    }

                    metaBrowser2->refresh();
                    metaBrowser2->selectPath(newGroup);
                    metaTarget2->setText(newGroup);
                });

        metaLayout->addWidget(ui.metaBrowser);
        metaLayout->addLayout(metaTargetRow);

        ui.tabs->addTab(metaTab, tr("Metadata group"));

        auto *tabs   = ui.tabs;
        auto *traces = ui.tracesBrowser;
        auto *meta   = ui.metaBrowser;

        connect(tabs, &QTabWidget::currentChanged, this,
                [traces, meta](int idx) {
                    if (idx == 0 && traces) traces->refresh();
                    if (idx == 1 && meta)   meta->refresh();
                });

        rootLayout->addWidget(ui.tabs);
        rootLayout->addStretch(1);

        m_channelList->addItem(ui.alias);
        m_stack->addWidget(ui.root);

        m_channelsUi.push_back(ui);
    }


    refreshAllBrowsersAndTargets();

    // Offer automatic creation when the file is totally empty.
    {
        auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
        THdfSession *s = (wiz && wiz->hdfSession()) ? wiz->hdfSession().data() : nullptr;

        if (!m_autoCreateOfferedThisVisit && isTotallyEmptyHdf(s)) {

            const auto ans = QMessageBox::question(
                this,
                tr("Automatic export structure creation"),
                tr("This HDF5 file is empty.\n\n"
                   "Create the required export structure now?\n"
                   "(Non-destructive: creates missing items, never overwrites.)"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
                );

            m_autoCreateOfferedThisVisit = true;

            if (ans == QMessageBox::Yes) {
                QStringList created, repaired, conflicts;
                ensureDefaultExportStructure(created, repaired, conflicts);

                QString report;
                if (!created.isEmpty())
                    report += tr("Created:\n  - %1\n\n").arg(created.join("\n  - "));
                if (!repaired.isEmpty())
                    report += tr("Repaired/Completed:\n  - %1\n\n").arg(repaired.join("\n  - "));
                if (!conflicts.isEmpty())
                    report += tr("Conflicts (not changed):\n  - %1\n\n").arg(conflicts.join("\n  - "));
                if (report.isEmpty())
                    report = tr("Nothing to create.");

                QMessageBox::information(this, tr("Automatic structure creation"), report.trimmed());

                refreshAllBrowsersAndTargets();
            }
        }
    }
}

void ChannelTargetsPage::refreshAllBrowsersAndTargets()
{
    for (int i = 0; i < m_channelsUi.size(); ++i) {
        ChannelUi &ui = m_channelsUi[i];

        if (ui.tracesBrowser) ui.tracesBrowser->refresh();
        if (ui.metaBrowser)   ui.metaBrowser->refresh();

        if (ui.tracesBrowser && ui.tracesBrowser->selectedIsDataset())
            ui.tracesTarget->setText(ui.tracesBrowser->selectedPath());

        if (ui.metaBrowser) {
            THdfSession *s = ui.metaBrowser->session();
            const QString p = ui.metaBrowser->selectedPath();
            if (s && s->isOpen() && s->isGroup(p))
                ui.metaTarget->setText(p);
        }
    }

    if (m_channelList->count() > 0)
        m_channelList->setCurrentRow(0);
}

bool ChannelTargetsPage::ensureDefaultExportStructure(QStringList &created, QStringList &repaired, QStringList &conflicts)
{
    created.clear();
    repaired.clear();
    conflicts.clear();

    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen())
        return false;

    THdfSession *s = wiz->hdfSession().data();

    const qulonglong samples = static_cast<qulonglong>(wiz->traceSamplesPerTrace());
    const H5T_class_t typeClass = expectedTypeClass(wiz->traceSampleType());
    const hid_t elementType = sampleTypeToNativeH5Type(wiz->traceSampleType());

    for (const ChannelUi &ui : m_channelsUi) {
        const QString tracesPath = THdfSession::normalizePath(QString("ch%1/traces").arg(ui.alias));
        const QString metaGroup  = THdfSession::normalizePath(QString("ch%1/runs").arg(ui.alias));

        const bool metaExisted = s->pathExists(metaGroup);
        const bool metaValidBefore = metaExisted ? s->validateMetadataGroup(metaGroup) : false;

        if (!s->ensureMetadataGroup(metaGroup)) {
            conflicts << tr("%1: cannot ensure valid metadata group at %2 (see log).")
            .arg(ui.alias, metaGroup);
        } else {
            if (!metaExisted) created << metaGroup;
            else if (!metaValidBefore) repaired << metaGroup;
        }

        if (!s->pathExists(tracesPath)) {
            if (samples == 0) {
                conflicts << tr("%1: cannot create traces dataset (samplesPerTrace = 0).").arg(ui.alias);
            } else {
                const ChunkProposal cp = proposeTraceChunks(samples, sampleTypeBytes(wiz->traceSampleType()));

                THdfSession::DatasetCreateParams p;
                p.path = tracesPath;
                p.elementType = elementType; // native type, no ownership
                p.takeOwnershipOfElementType = false;
                p.initialDims = { 0, static_cast<hsize_t>(samples) };
                p.maxDims     = { H5S_UNLIMITED, static_cast<hsize_t>(samples) };
                p.chunkDims   = {
                    static_cast<hsize_t>(qMax<qulonglong>(1, cp.rows)),
                    static_cast<hsize_t>(qMax<qulonglong>(1, cp.cols))
                };
                p.enableDeflate = false;
                p.deflateLevel = 4;

                if (!s->createDataset(p)) {
                    conflicts << tr("%1: failed to create traces dataset %2 (see log).")
                    .arg(ui.alias, tracesPath);
                } else {
                    created << tracesPath;
                }
            }
        } else {
            if (!s->isDataset(tracesPath)) {
                conflicts << tr("%1: %2 exists but is not a dataset.").arg(ui.alias, tracesPath);
            } else {
                const auto ti = s->datasetInfo(tracesPath);
                const bool ok =
                    ti.valid &&
                    ti.rank == 2 &&
                    ti.dims.size() == 2 &&
                    ti.maxDims.size() == 2 &&
                    ti.typeClass == typeClass &&
                    ti.maxDims[0] == H5S_UNLIMITED &&
                    (samples == 0 || ti.dims[1] == static_cast<hsize_t>(samples));

                if (!ok) {
                    conflicts << tr("%1: traces dataset exists but does not match required format: %2.")
                    .arg(ui.alias, tracesPath);
                }
            }
        }
    }

    return true;
}

bool ChannelTargetsPage::validateOnce(TFail &fail)
{
    fail = TFail{};

    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen()) {
        fail.title = tr("HDF5 error");
        fail.text  = tr("The HDF5 file is not open. Please go back and re-select the file.");
        return false;
    }

    THdfSession *s = wiz->hdfSession().data();

    const qulonglong samples = static_cast<qulonglong>(wiz->traceSamplesPerTrace());
    const H5T_class_t typeClass = expectedTypeClass(wiz->traceSampleType());

    QVector<TExportHDFScopeWizard::TChannelTarget> plan;
    plan.reserve(m_channelsUi.size());

    for (int i = 0; i < m_channelsUi.size(); ++i) {
        const ChannelUi &ui = m_channelsUi[i];

        const QString traces = THdfSession::normalizePath(ui.tracesTarget->text());
        const QString metaGroup = THdfSession::normalizePath(ui.metaTarget->text());

        // ---- Traces ----
        if (traces.isEmpty() || !s->isDataset(traces)) {
            fail.channelIndex = i;
            fail.tabIndex = 0;
            fail.title = tr("Traces dataset not selected");
            fail.text = tr("Channel %1:\nPlease select an existing traces dataset.\n"
                           "Use 'New dataset…' in the browser to create one first.")
                            .arg(ui.alias);
            return false;
        }

        const auto ti = s->datasetInfo(traces);
        if (!ti.valid || ti.rank != 2 || ti.dims.size() != 2 || ti.maxDims.size() != 2) {
            fail.channelIndex = i;
            fail.tabIndex = 0;
            fail.title = tr("Invalid traces dataset");
            fail.text = tr("Channel %1:\nSelected traces dataset has invalid shape.\n"
                           "Required: rank 2 dataset with dims [*, %2].")
                            .arg(ui.alias)
                            .arg(QString::number(samples));
            return false;
        }

        if (ti.typeClass != typeClass) {
            fail.channelIndex = i;
            fail.tabIndex = 0;
            fail.title = tr("Invalid traces dataset type");
            fail.text = tr("Channel %1:\nSelected traces dataset has wrong type class.\n"
                           "Required: %2.")
                            .arg(ui.alias)
                            .arg(typeClass == H5T_FLOAT ? "float" : "integer");
            return false;
        }

        if (ti.maxDims[0] != H5S_UNLIMITED) {
            fail.channelIndex = i;
            fail.tabIndex = 0;
            fail.title = tr("Invalid traces dataset");
            fail.text = tr("Channel %1:\nSelected traces dataset is not appendable.\n"
                           "Required: maxDims[0] = unlimited.")
                            .arg(ui.alias);
            return false;
        }

        if (samples > 0 && ti.dims[1] != static_cast<hsize_t>(samples)) {
            fail.channelIndex = i;
            fail.tabIndex = 0;
            fail.title = tr("Invalid traces dataset");
            fail.text = tr("Channel %1:\nSelected traces dataset has wrong trace length.\n"
                           "Required: dims[1] = %2.")
                            .arg(ui.alias)
                            .arg(QString::number(samples));
            return false;
        }

        // ---- Metadata ----
        if (metaGroup.isEmpty() || !s->isGroup(metaGroup)) {
            fail.channelIndex = i;
            fail.tabIndex = 1;
            fail.title = tr("Metadata group not selected");
            fail.text = tr("Channel %1:\nPlease select an existing metadata group.\n"
                           "Use 'Create metadata group…' to create one.")
                            .arg(ui.alias);
            return false;
        }

        if (!s->validateMetadataGroup(metaGroup)) {
            fail.channelIndex = i;
            fail.tabIndex = 1;
            fail.title = tr("Invalid metadata group");
            fail.text = tr("Channel %1:\nSelected metadata group is not valid.\n"
                           "See log output for details.")
                            .arg(ui.alias);
            return false;
        }

        TExportHDFScopeWizard::TChannelTarget t;
        t.alias = ui.alias;
        t.tracesDataset = traces;
        t.metadataGroup = metaGroup;
        plan.push_back(t);
    }

    wiz->setChannelTargets(plan);
    return true;
}


void ChannelTargetsPage::cleanupPage()
{
    resetUi();
    m_autoCreateOfferedThisVisit = false;
    QWizardPage::cleanupPage();
}

bool ChannelTargetsPage::validatePage()
{

    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    THdfSession *s = (wiz && wiz->hdfSession()) ? wiz->hdfSession().data() : nullptr;

    TFail fail;
    if (validateOnce(fail)) {
        wiz->requestPause();
        return false;
    }

    const bool empty = isTotallyEmptyHdf(s);

    if (fail.channelIndex >= 0 && fail.channelIndex < m_channelsUi.size()) {
        m_channelList->setCurrentRow(fail.channelIndex);
        if (m_channelsUi[fail.channelIndex].tabs)
            m_channelsUi[fail.channelIndex].tabs->setCurrentIndex(fail.tabIndex);
    }

    if (empty) {
        QMessageBox::warning(this,
                             fail.title.isEmpty() ? tr("Validation failed") : fail.title,
                             fail.text);
        //return false;
    }

    const auto ans = QMessageBox::question(
        this,
        tr("Repair export structure?"),
        fail.text + tr("\n\nCreate/repair the required export structure now?\n"
                       "(Non-destructive: creates missing items, never overwrites.)"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (ans == QMessageBox::Yes) {
        QStringList created, repaired, conflicts;
        ensureDefaultExportStructure(created, repaired, conflicts);

        QString report;
        if (!created.isEmpty())
            report += tr("Created:\n  - %1\n\n").arg(created.join("\n  - "));
        if (!repaired.isEmpty())
            report += tr("Repaired/Completed:\n  - %1\n\n").arg(repaired.join("\n  - "));
        if (!conflicts.isEmpty())
            report += tr("Conflicts (not changed):\n  - %1\n\n").arg(conflicts.join("\n  - "));
        if (report.isEmpty())
            report = tr("Nothing to create.");

        QMessageBox::information(this, tr("Repair result"), report.trimmed());

        refreshAllBrowsersAndTargets();

        return false;
    }

    return false;
}


// ---------------- PAGE 3 ----------------

ExportResultPage::ExportResultPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Export result"));
    setSubTitle(tr("Summary"));

    m_label = new QLabel(tr("Export finished. Watch for any errors in the log below."));
    m_label->setWordWrap(true);

    m_text = new QPlainTextEdit(this);
    m_text->setReadOnly(true);
    m_text->setLineWrapMode(QPlainTextEdit::NoWrap);

    auto *layout = new QVBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_text, 1);
    setLayout(layout);

    setFinalPage(true);
}

void ExportResultPage::initializePage()
{
    if (auto *w = wizard()) {
        if (auto *btn = w->button(QWizard::BackButton))
            btn->setEnabled(false);
        if (auto *btn = w->button(QWizard::CancelButton))
            btn->setVisible(false);
    }

    auto *wiz = qobject_cast<TExportHDFScopeWizard*>(wizard());
    const QString report = wiz ? wiz->resultLogText() : QString();

    m_text->setPlainText(report.isEmpty() ? tr("No report.") : report);
}


// ---------------- WIZARD ----------------

TExportHDFScopeWizard::TExportHDFScopeWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Export trace(s) to HDF5"));

    m_hdfSession = QSharedPointer<THdfSession>::create(this);

    addPage(new ExportFilePage(this));
    addPage(new ChannelTargetsPage(this));
    m_resultPageId = addPage(new ExportResultPage(this));

    setWizardStyle(QWizard::ModernStyle);
}

TExportHDFScopeWizard::~TExportHDFScopeWizard()
{
    if (m_hdfSession && m_hdfSession->isOpen()) {
        qDebug() << "[TExportHDFScopeWizard] Destructor closing HDF5 session for" << m_hdfSession->filePath();
        m_hdfSession->close();
    }
}

void TExportHDFScopeWizard::done(int result)
{
    const int finalResult = this->result();

    if (finalResult != Result_Paused) {
        // destructor will take care of closing
    } else {
        qDebug() << "[TExportHDFScopeWizard] Paused: keeping HDF5 session open for"
                 << (m_hdfSession ? m_hdfSession->filePath() : QString());
    }

    QWizard::done(finalResult);
}

QString TExportHDFScopeWizard::outputFile() const
{
    return field("outputFile").toString();
}

void TExportHDFScopeWizard::setTraceInfo(std::size_t total)
{
    m_totalTraceCount = total;
}

void TExportHDFScopeWizard::setChannels(const QStringList &channels)
{
    m_channels = channels;
}

void TExportHDFScopeWizard::setInitialFileName(const QString &name)
{
    m_initialFileName = name;
}

void TExportHDFScopeWizard::setTraceFormat(std::size_t samplesPerTrace,
                                           TScope::TSampleType sampleType)
{
    m_traceSamplesPerTrace = samplesPerTrace;
    m_traceSampleType = sampleType;
}

void TExportHDFScopeWizard::requestPause()
{
    if (m_pauseRequested)
        return;

    m_pauseRequested = true;

    // Defer closing by one event-loop tick to avoid re-entrancy from validatePage().
    QTimer::singleShot(0, this, [this]() {
        QDialog::done(Result_Paused);
    });
}

void TExportHDFScopeWizard::showResultsPage(const QString & log)
{
    m_resultLogText = log;
    setStartId(m_resultPageId);
    restart();
}
