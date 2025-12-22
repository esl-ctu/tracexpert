#include "thdfbrowserwidget.h"
#include "thdfsession.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QStackedWidget>
#include <QAbstractSpinBox>

static QString joinPath(const QString &base, const QString &name)
{
    QString b = THdfSession::normalizePath(base);
    if (b == "/") return "/" + name;
    return b + "/" + name;
}

class TNewDatasetDialog : public QDialog
{
public:
    explicit TNewDatasetDialog(QWidget *parent=nullptr) : QDialog(parent)
    {
        setWindowTitle(tr("New dataset"));

        m_name = new QLineEdit;

        m_rank = new QSpinBox;
        m_rank->setRange(1, 8);
        m_rank->setValue(2);

        m_dims = new QLineEdit;
        m_dims->setPlaceholderText("0,1024");

        m_maxDims = new QLineEdit;
        m_maxDims->setPlaceholderText("unlimited,1024");

        m_chunkStack = new QStackedWidget;

        m_chunksText = new QLineEdit;
        m_chunksText->setPlaceholderText("64,1024");
        m_chunkStack->addWidget(m_chunksText);

        QWidget *chunk2d = new QWidget;
        auto *chunk2dLayout = new QHBoxLayout(chunk2d);
        chunk2dLayout->setContentsMargins(0, 0, 0, 0);

        m_chunkRows = new QSpinBox;
        m_chunkRows->setRange(1, 1024);
        m_chunkRows->setValue(1);
        m_chunkRows->setButtonSymbols(QAbstractSpinBox::NoButtons);

        m_chunkColsLabel = new QLabel;
        m_chunkColsLabel->setText("x ?");
        m_chunkColsLabel->setMinimumWidth(80);

        chunk2dLayout->addWidget(m_chunkRows);
        chunk2dLayout->addWidget(m_chunkColsLabel);
        chunk2dLayout->addStretch(1);

        m_chunkStack->addWidget(chunk2d);

        m_type = new QComboBox;
        m_type->addItem("uint8");
        m_type->addItem("int8");
        m_type->addItem("uint16");
        m_type->addItem("int16");
        m_type->addItem("uint32");
        m_type->addItem("int32");
        m_type->addItem("float32");
        m_type->addItem("float64");
        m_type->addItem("string");

        m_deflate = new QComboBox;
        m_deflate->addItem("off");
        m_deflate->addItem("on");

        m_deflateLevel = new QSpinBox;
        m_deflateLevel->setRange(0, 9);
        m_deflateLevel->setValue(4);

        auto *form = new QFormLayout;
        m_nameLabel = new QLabel(tr("Name:"));
        form->addRow(m_nameLabel, m_name);
        form->addRow(tr("Rank:"), m_rank);
        form->addRow(tr("Initial dims:"), m_dims);
        form->addRow(tr("Max dims:"), m_maxDims);
        form->addRow(tr("Chunk dims:"), m_chunkStack);
        form->addRow(tr("Type:"), m_type);
        form->addRow(tr("Deflate:"), m_deflate);
        form->addRow(tr("Deflate level:"), m_deflateLevel);

        auto *ok = new QPushButton(tr("Create"));
        auto *cancel = new QPushButton(tr("Cancel"));
        connect(ok, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

        auto *btns = new QHBoxLayout;
        btns->addStretch(1);
        btns->addWidget(ok);
        btns->addWidget(cancel);

        auto *layout = new QVBoxLayout;
        layout->addLayout(form);
        layout->addLayout(btns);
        setLayout(layout);
    }

    void applyTemplate(const THdfBrowserWidget::TDatasetTemplate &t)
    {

        if (!t.suggestedName.isEmpty())
            m_name->setText(t.suggestedName);

        m_rank->setValue(qMax(1, t.rank));
        m_rank->setEnabled(!t.lockRank);

        if (!t.initialDimsText.isEmpty()) m_dims->setText(t.initialDimsText);
        m_dims->setReadOnly(t.lockInitialDims);

        if (!t.maxDimsText.isEmpty())     m_maxDims->setText(t.maxDimsText);
        m_maxDims->setReadOnly(t.lockMaxDims);

        if (!t.typeText.isEmpty()) {
            const int typeIdx = m_type->findText(t.typeText);
            if (typeIdx >= 0) m_type->setCurrentIndex(typeIdx);
        }
        m_type->setEnabled(!t.lockType);

        const bool useBounded2D =
            (t.rank == 2) &&
            t.chunk2DRowBounded &&
            (t.chunkColsFixed > 0) &&
            (t.chunkRowsMax >= t.chunkRowsMin) &&
            (t.chunkRowsMax >= 1);

        if (useBounded2D) {
            m_chunkStack->setCurrentIndex(1);

            const int minR = static_cast<int>(qBound<qulonglong>(1, t.chunkRowsMin, 2147483647ULL));
            const int maxR = static_cast<int>(qBound<qulonglong>(1, t.chunkRowsMax, 2147483647ULL));
            m_chunkRows->setRange(minR, maxR);

            const qulonglong defR = (t.chunkRowsDefault > 0) ? t.chunkRowsDefault : t.chunkRowsMax;
            const int defRi = static_cast<int>(qBound<qulonglong>(
                static_cast<qulonglong>(minR),
                defR,
                static_cast<qulonglong>(maxR)));
            m_chunkRows->setValue(defRi);

            m_chunkColsFixed = t.chunkColsFixed;
            m_chunkColsLabel->setText(QString("x %1").arg(QString::number(m_chunkColsFixed)));

            m_chunksText->setText(QString("%1,%2")
                                      .arg(QString::number(static_cast<qulonglong>(m_chunkRows->value())))
                                      .arg(QString::number(m_chunkColsFixed)));
        } else {
            m_chunkStack->setCurrentIndex(0);

            if (!t.chunkDimsText.isEmpty())
                m_chunksText->setText(t.chunkDimsText);

            m_chunkColsFixed = 0;
            m_chunkColsLabel->setText("x ?");
        }

        m_deflate->setCurrentText(t.deflateOn ? "on" : "off");
        m_deflateLevel->setValue(qBound(0, t.deflateLevel, 9));
    }

    void setBasePathHint(const QString &base)
    {
        if (!m_nameLabel) return;
        m_nameLabel->setText(tr("Name (under to %1):").arg(base));
    }

    void setNameText(const QString &t) { m_name->setText(t); }

    QString name() const { return m_name->text().trimmed(); }
    int rank() const { return m_rank->value(); }
    QString dimsText() const { return m_dims->text().trimmed(); }
    QString maxDimsText() const { return m_maxDims->text().trimmed(); }

    QString chunksText() const
    {
        if (m_chunkStack->currentIndex() == 1 && m_chunkColsFixed > 0) {
            const qulonglong rows = static_cast<qulonglong>(m_chunkRows->value());
            return QString("%1,%2").arg(QString::number(rows), QString::number(m_chunkColsFixed));
        }
        return m_chunksText->text().trimmed();
    }

    QString typeText() const { return m_type->currentText(); }
    bool deflateOn() const { return m_deflate->currentText() == "on"; }
    int deflateLevel() const { return m_deflateLevel->value(); }

private:
    QLabel *m_nameLabel = nullptr;
    QLineEdit *m_name = nullptr;
    QSpinBox *m_rank = nullptr;
    QLineEdit *m_dims = nullptr;
    QLineEdit *m_maxDims = nullptr;

    QStackedWidget *m_chunkStack = nullptr;
    QLineEdit *m_chunksText = nullptr;

    QSpinBox *m_chunkRows = nullptr;
    QLabel *m_chunkColsLabel = nullptr;
    qulonglong m_chunkColsFixed = 0;

    QComboBox *m_type = nullptr;
    QComboBox *m_deflate = nullptr;
    QSpinBox *m_deflateLevel = nullptr;
};

static bool parseDims(const QString &txt, int rank, QVector<hsize_t> &out, bool allowUnlimited)
{
    out.clear();
    const QStringList parts = txt.split(',', Qt::SkipEmptyParts);
    if (parts.size() != rank) return false;

    for (const QString &p : parts) {
        const QString s = p.trimmed().toLower();
        if (allowUnlimited && (s == "unlimited" || s == "inf" || s == "*")) {
            out.push_back(H5S_UNLIMITED);
            continue;
        }
        bool ok = false;
        const qulonglong v = s.toULongLong(&ok);
        if (!ok) return false;
        out.push_back(static_cast<hsize_t>(v));
    }
    return true;
}

static hid_t typeFromText(const QString &t)
{
    if (t == "string")  return H5I_INVALID_HID;

    if (t == "uint8")   return H5T_NATIVE_UINT8;
    if (t == "int8")    return H5T_NATIVE_INT8;
    if (t == "uint16")  return H5T_NATIVE_UINT16;
    if (t == "int16")   return H5T_NATIVE_INT16;
    if (t == "uint32")  return H5T_NATIVE_UINT32;
    if (t == "int32")   return H5T_NATIVE_INT32;
    if (t == "float32") return H5T_NATIVE_FLOAT;
    if (t == "float64") return H5T_NATIVE_DOUBLE;
    return H5I_INVALID_HID;
}

THdfBrowserWidget::THdfBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    m_title = new QLabel(tr("HDF5 structure"));
    m_tree = new QTreeView;
    m_tree->setHeaderHidden(true);

    m_model = new QStandardItemModel(this);
    m_tree->setModel(m_model);
    m_tree->setMinimumHeight(200);

    m_refreshBtn = new QPushButton(tr("Refresh"));
    m_newGroupBtn = new QPushButton(tr("New group…"));
    m_newDatasetBtn = new QPushButton(tr("New dataset…"));
    m_removeBtn = new QPushButton(tr("Remove…"));
    m_removeBtn->setEnabled(false);

    auto *top = new QHBoxLayout;
    top->addWidget(m_title);
    top->addStretch(1);
    top->addWidget(m_refreshBtn);
    top->addWidget(m_newGroupBtn);
    top->addWidget(m_newDatasetBtn);
    top->addWidget(m_removeBtn);

    m_infoTitle = new QLabel(tr("Selection info"));
    m_infoText = new QTextEdit;
    m_infoText->setReadOnly(true);
    m_infoText->setMinimumHeight(120);
    m_infoText->setPlaceholderText(tr("Select a group or dataset to see details."));

    auto *layout = new QVBoxLayout;
    layout->addLayout(top);
    layout->addWidget(m_tree);
    layout->addWidget(m_infoTitle);
    layout->addWidget(m_infoText);
    setLayout(layout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &THdfBrowserWidget::refresh);
    connect(m_newGroupBtn, &QPushButton::clicked, this, &THdfBrowserWidget::onNewGroup);
    connect(m_newDatasetBtn, &QPushButton::clicked, this, &THdfBrowserWidget::onNewDataset);
    connect(m_removeBtn, &QPushButton::clicked, this, &THdfBrowserWidget::onRemove);

    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]{ onSelectionChanged(); });
}

void THdfBrowserWidget::setSession(THdfSession *session)
{
    m_session = session;
    refresh();
}

void THdfBrowserWidget::refresh()
{
    const QString prev = m_selectedPath;
    buildModel();

    auto isAcceptable = [&](const QString &p) -> bool {
        if (p.isEmpty() || !m_session || !m_session->isOpen() || !m_session->pathExists(p))
            return false;

        switch (m_selectionPolicy) {
        case SelectionPolicy::GroupsAndDatasets:
            return m_session->isGroup(p) || m_session->isDataset(p);
        case SelectionPolicy::GroupsOnly:
            return m_session->isGroup(p);
        case SelectionPolicy::DatasetsOnly:
            return m_session->isDataset(p);
        }
        return false;
    };

    // 1) Try to restore previous selection (if any)
    if (!prev.isEmpty()) {
        const QModelIndex prevIdx = findIndexByPath(prev);
        if (prevIdx.isValid()) {
            m_tree->setCurrentIndex(prevIdx);
            m_tree->scrollTo(prevIdx);
        }
    }

    // 2) If nothing valid selected, optionally preselect an explicit path (group or dataset)
    if (!m_autoPreselectPath.trimmed().isEmpty()) {
        const QModelIndex cur = m_tree->currentIndex();
        const QString curPath = itemPath(cur);

        const bool curAcceptable = isAcceptable(curPath);

        if (!curAcceptable) {
            const QString abs = toAbsolutePath(m_autoPreselectPath);
            const QModelIndex pick = findIndexByPath(abs);
            if (pick.isValid()) {
                m_tree->setCurrentIndex(pick);
                m_tree->scrollTo(pick);
            }
        }
    }

    // 3) If nothing valid selected, optionally preselect by suggested name/path
    if (m_autoPreselectSuggested) {
        const QModelIndex cur = m_tree->currentIndex();
        const QString curPath = itemPath(cur);

        const bool curAcceptable = isAcceptable(curPath);

        if (!curAcceptable) {
            const QString suggestedRaw = m_newDatasetTemplate.suggestedName.trimmed();
            if (!suggestedRaw.isEmpty()) {

                QModelIndex pick;

                if (suggestedRaw.contains('/')) {
                    const QString abs = toAbsolutePath(suggestedRaw);   // "chA/traces" -> "/chA/traces"
                    pick = findIndexByPath(abs);

                } else {
                    pick = findFirstDatasetByLeafName(suggestedRaw);
                }

                if (pick.isValid()) {
                    m_tree->setCurrentIndex(pick);
                    m_tree->scrollTo(pick);
                }
            }
        }
    }

    onSelectionChanged();
}

QString THdfBrowserWidget::selectedPath() const
{
    return m_selectedPath;
}

bool THdfBrowserWidget::selectedIsDataset() const
{
    return m_selectedIsDataset;
}

bool THdfBrowserWidget::selectionAllowed(int nodeTypeInt) const
{
    const auto t = static_cast<THdfSession::NodeType>(nodeTypeInt);
    switch (m_selectionPolicy) {
    case SelectionPolicy::GroupsAndDatasets:
        return (t == THdfSession::NodeType::Group || t == THdfSession::NodeType::Dataset);
    case SelectionPolicy::DatasetsOnly:
        return (t == THdfSession::NodeType::Dataset);
    case SelectionPolicy::GroupsOnly:
        return (t == THdfSession::NodeType::Group);
    }
    return true;
}

void THdfBrowserWidget::buildModel()
{
    m_model->clear();

    auto *root = new QStandardItem("/");
    root->setData(QString("/"), Qt::UserRole + 1);
    m_model->appendRow(root);

    if (!m_session || !m_session->isOpen()) {
        root->appendRow(new QStandardItem(tr("(no open file)")));
        m_tree->expandAll();
        return;
    }

    addGroupRecursive("/", m_model, root);
    m_tree->expandToDepth(1);
}

void THdfBrowserWidget::addGroupRecursive(const QString &groupPath, QStandardItemModel * /*model*/, QStandardItem *parentItem)
{
    if (!m_session) return;

    const QStringList children = m_session->listChildren(groupPath);
    for (const QString &name : children) {
        const QString childPath = THdfSession::normalizePath(joinPath(groupPath, name));
        const auto t = m_session->nodeType(childPath);

        QString label = name;
        if (t == THdfSession::NodeType::Dataset)
            label = QString("[D] %1").arg(name);

        auto *item = new QStandardItem(label);
        item->setData(childPath, Qt::UserRole + 1);
        parentItem->appendRow(item);

        if (t == THdfSession::NodeType::Group) {
            addGroupRecursive(childPath, m_model, item);
        }
    }
}

QString THdfBrowserWidget::itemPath(const QModelIndex &idx) const
{
    return idx.isValid() ? idx.data(Qt::UserRole + 1).toString() : QString();
}

void THdfBrowserWidget::onSelectionChanged()
{
    m_selectedPath.clear();
    m_selectedIsDataset = false;

    if (!m_session || !m_session->isOpen()) {
        updateInfoPanel();
        return;
    }

    const QModelIndex idx = m_tree->currentIndex();
    const QString p = itemPath(idx);
    if (!p.isEmpty()) {
        m_selectedPath = p;
        m_selectedIsDataset = m_session->isDataset(p);

        const int nt = static_cast<int>(m_session->nodeType(p));
        if (selectionAllowed(nt))
            emit selectionChanged(m_selectedPath, nt);
    }

    if (m_removeBtn) {
        const bool canRemove =
            (m_session && m_session->isOpen() &&
             !m_selectedPath.isEmpty() &&
             THdfSession::normalizePath(m_selectedPath) != "/");
        m_removeBtn->setEnabled(canRemove);
    }

    updateInfoPanel();
}

static QString dimsToString(const QVector<hsize_t> &v)
{
    QStringList parts;
    parts.reserve(v.size());
    for (hsize_t x : v)
        parts << QString::number(static_cast<qulonglong>(x));
    return parts.join(", ");
}

static QString maxDimsToString(const QVector<hsize_t> &v)
{
    QStringList parts;
    parts.reserve(v.size());
    for (hsize_t x : v) {
        if (x == H5S_UNLIMITED) parts << "unlimited";
        else parts << QString::number(static_cast<qulonglong>(x));
    }
    return parts.join(", ");
}

static QString typeClassToString(H5T_class_t c)
{
    switch (c) {
    case H5T_INTEGER:  return "integer";
    case H5T_FLOAT:    return "float";
    case H5T_STRING:   return "string";
    case H5T_COMPOUND: return "compound";
    default:           return "other";
    }
}

void THdfBrowserWidget::updateInfoPanel()
{
    if (!m_infoText) return;

    if (!m_session || !m_session->isOpen()) {
        m_infoText->setPlainText(tr("No open HDF5 file."));
        return;
    }

    if (m_selectedPath.isEmpty()) {
        m_infoText->setPlainText(tr("No selection."));
        return;
    }

    const auto t = m_session->nodeType(m_selectedPath);

    if (t == THdfSession::NodeType::Group) {
        m_infoText->setPlainText(tr("Group:\n%1").arg(m_selectedPath));
        return;
    }

    if (t != THdfSession::NodeType::Dataset) {
        m_infoText->setPlainText(tr("Path:\n%1\n\n(Not a dataset.)").arg(m_selectedPath));
        return;
    }

    const auto info = m_session->datasetInfo(m_selectedPath);
    if (!info.valid) {
        m_infoText->setPlainText(tr("Dataset:\n%1\n\nFailed to read dataset info.").arg(m_selectedPath));
        return;
    }

    const quint64 elemBytes   = m_session->datasetElementBytes(m_selectedPath);
    const quint64 elemCount   = m_session->datasetElementCount(m_selectedPath);
    const quint64 storageByte = m_session->datasetStorageBytes(m_selectedPath);

    quint64 rows = 0;
    if (!info.dims.isEmpty())
        rows = static_cast<quint64>(info.dims[0]);

    QString txt;
    txt += tr("Dataset:\n%1\n\n").arg(m_selectedPath);
    txt += tr("Rank: %1\n").arg(info.rank);
    txt += tr("Dims: %1\n").arg(dimsToString(info.dims));
    txt += tr("Max dims: %1\n").arg(maxDimsToString(info.maxDims));
    txt += tr("Type class: %1\n").arg(typeClassToString(info.typeClass));

    if (info.chunked)
        txt += tr("Chunk dims: %1\n").arg(dimsToString(info.chunkDims));
    else
        txt += tr("Chunk dims: (not chunked)\n");

    txt += "\n";
    txt += tr("Elements: %1\n").arg(QString::number(elemCount));
    if (elemBytes > 0)
        txt += tr("Element size: %1 B\n").arg(QString::number(elemBytes));
    txt += tr("Allocated storage: %1 B\n").arg(QString::number(storageByte));

    if (rows > 0)
        txt += tr("First-dim count: %1\n").arg(QString::number(rows));

    m_infoText->setPlainText(txt);
}

void THdfBrowserWidget::onNewGroup()
{
    if (!m_session || !m_session->isOpen()) {
        QMessageBox::warning(this, tr("HDF5"), tr("No open HDF5 file."));
        return;
    }

    const QString base = m_selectedPath.isEmpty() ? "/" :
                             (m_session->isGroup(m_selectedPath) ? m_selectedPath : THdfSession::parentPath(m_selectedPath));

    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("New group"),
                                               tr("Group name (under %1):").arg(base),
                                               QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || name.isEmpty())
        return;

    const QString newPath = THdfSession::normalizePath(joinPath(base, name));
    if (m_session->pathExists(newPath)) {
        QMessageBox::warning(this, tr("New group"), tr("Path already exists:\n%1").arg(newPath));
        return;
    }

    if (!m_session->createGroup(newPath)) {
        QMessageBox::warning(this, tr("New group"), tr("Failed to create group:\n%1").arg(newPath));
        return;
    }

    emit groupCreated(newPath);
    refresh();
}

void THdfBrowserWidget::onNewDataset()
{

    if (!m_session || !m_session->isOpen()) {
        QMessageBox::warning(this, tr("HDF5"), tr("No open HDF5 file."));
        return;
    }

    const QString base = m_selectedPath.isEmpty() ? "/" :
                             (m_session->isGroup(m_selectedPath) ? m_selectedPath : THdfSession::parentPath(m_selectedPath));

    TNewDatasetDialog dlg(this);
    dlg.applyTemplate(m_newDatasetTemplate);
    dlg.setBasePathHint(base);

    // If template suggests a full-ish path like "chA/traces", but base is "/chA",
    // default to a relative name like "traces" (or "sub/traces").
    {
        const QString baseNorm = THdfSession::normalizePath(base);
        const QString suggestedRaw = m_newDatasetTemplate.suggestedName.trimmed();

        if (!suggestedRaw.isEmpty() && suggestedRaw.contains('/')) {

            const QString absSuggested = THdfSession::normalizePath(toAbsolutePath(suggestedRaw));

            QString defaultRel = suggestedRaw;

            if (baseNorm != "/" && absSuggested.startsWith(baseNorm + "/")) {
                // make it relative to base
                defaultRel = absSuggested.mid(baseNorm.size() + 1);
            } else if (baseNorm != "/") {
                // base is not root but suggested is elsewhere -> use leaf as a safe default
                defaultRel = leafNameFromPath(absSuggested);
            } else {
                // base is "/", keep "chA/traces" style (without leading '/')
                defaultRel = absSuggested.startsWith('/') ? absSuggested.mid(1) : absSuggested;
            }

            dlg.setNameText(defaultRel);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString name = dlg.name();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("New dataset"), tr("Dataset name cannot be empty."));
        return;
    }

    if (name.startsWith('/')) {
        QMessageBox::warning(this, tr("New dataset"),
                             tr("Please enter a relative path without a leading '/'."));
        return;
    }

    const int rank = dlg.rank();

    QVector<hsize_t> dims, maxDims, chunks;
    if (!parseDims(dlg.dimsText(), rank, dims, false)) {
        QMessageBox::warning(this, tr("New dataset"),
                             tr("Invalid initial dims.\n\n"
                                "Rank = %1\n"
                                "Initial dims must contain %1 comma-separated positive integer(s).\n"
                                "Entered: '%2'")
                                 .arg(rank)
                                 .arg(dlg.dimsText()));
        return;
    }
    if (!parseDims(dlg.maxDimsText(), rank, maxDims, true)) {
        QMessageBox::warning(this, tr("New dataset"),
                             tr("Invalid max dims.\n\n"
                                "Rank = %1\n"
                                "Max dims must contain %1 comma-separated value(s),\n"
                                "each being a positive integer or the keyword 'unlimited'.\n"
                                "Entered: '%2'")
                                 .arg(rank)
                                 .arg(dlg.maxDimsText()));
        return;
    }
    if (!parseDims(dlg.chunksText(), rank, chunks, false)) {
        QMessageBox::warning(this, tr("New dataset"),
                             tr("Invalid chunk dims.\n\n"
                                "Rank = %1\n"
                                "Chunk dims must contain %1 comma-separated integer(s).\n"
                                "Entered: '%2'")
                                 .arg(rank)
                                 .arg(dlg.chunksText()));

        return;
    }

    hid_t et = typeFromText(dlg.typeText());
    bool takeOwnership = false;

    if (dlg.typeText() == "string") {
        // Variable-length UTF-8 string type
        et = H5Tcopy(H5T_C_S1);
        if (et < 0) {
            QMessageBox::warning(this, tr("New dataset"), tr("Failed to create string datatype."));
            return;
        }
        if (H5Tset_size(et, H5T_VARIABLE) < 0 ||
            H5Tset_cset(et, H5T_CSET_UTF8) < 0 ||
            H5Tset_strpad(et, H5T_STR_NULLTERM) < 0) {
            H5Tclose(et);
            QMessageBox::warning(this, tr("New dataset"), tr("Failed to configure string datatype."));
            return;
        }
        takeOwnership = true;
    } else if (et < 0) {
        QMessageBox::warning(this, tr("New dataset"), tr("Invalid element type."));
        return;
    }


    THdfSession::DatasetCreateParams p;
    p.path = THdfSession::normalizePath(joinPath(base, name));
    p.elementType = et;
    p.initialDims = dims;
    p.maxDims = maxDims;
    p.chunkDims = chunks;
    p.enableDeflate = dlg.deflateOn();
    p.deflateLevel = dlg.deflateLevel();
    p.takeOwnershipOfElementType = takeOwnership;

    if (!m_session->createDataset(p)) {
        QMessageBox::warning(this, tr("New dataset"), tr("Failed to create dataset:\n%1").arg(p.path));
        return;
    }

    emit datasetCreated(p.path);
    refresh();
    selectPath(p.path);
}

void THdfBrowserWidget::setTitleText(const QString &text)
{
    if (m_title)
        m_title->setText(text);
}

void THdfBrowserWidget::onRemove()
{
    if (!m_session || !m_session->isOpen()) {
        QMessageBox::warning(this, tr("HDF5"), tr("No open HDF5 file."));
        return;
    }

    const QString p = THdfSession::normalizePath(m_selectedPath);
    if (p.isEmpty() || p == "/") {
        QMessageBox::warning(this, tr("Remove"), tr("Nothing removable is selected."));
        return;
    }

    const auto t = m_session->nodeType(p);
    if (t != THdfSession::NodeType::Group && t != THdfSession::NodeType::Dataset) {
        QMessageBox::warning(this, tr("Remove"), tr("Selected item is not a group or dataset."));
        return;
    }

    // Non-empty group safety (no recursive delete for now)
    if (t == THdfSession::NodeType::Group) {
        if (!m_session->isGroupEmpty(p)) {
            QMessageBox::warning(this, tr("Remove group"),
                                 tr("Group is not empty:\n%1\n\nRemove its contents first.")
                                     .arg(p));
            return;
        }
    }

    const QString what = (t == THdfSession::NodeType::Dataset) ? tr("dataset") : tr("group");

    const auto ans = QMessageBox::question(
        this,
        tr("Remove %1").arg(what),
        tr("Really remove this %1?\n\n%2").arg(what, p),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (ans != QMessageBox::Yes)
        return;

    if (!m_session->removeLink(p)) {
        QMessageBox::warning(this, tr("Remove"),
                             tr("Failed to remove:\n%1").arg(p));
        return;
    }

    refresh();
}

QModelIndex THdfBrowserWidget::findIndexByPath(const QString &absPath) const
{
    if (!m_model) return {};

    const QString p = THdfSession::normalizePath(absPath);
    if (p.isEmpty()) return {};

    const int role = Qt::UserRole + 1;

    // Start from root item index (0,0)
    const QModelIndex start = m_model->index(0, 0);
    if (!start.isValid()) return {};

    const auto hits = m_model->match(start, role, p, 1, Qt::MatchRecursive);
    return hits.isEmpty() ? QModelIndex() : hits.first();
}

QString THdfBrowserWidget::leafNameFromPath(const QString &absPath)
{
    const QString p = THdfSession::normalizePath(absPath);
    if (p.isEmpty() || p == "/") return QString();
    const int k = p.lastIndexOf('/');
    return (k >= 0) ? p.mid(k + 1) : p;
}

QModelIndex THdfBrowserWidget::findFirstDatasetByLeafName(const QString &leafName) const
{
    if (!m_model || !m_session || !m_session->isOpen())
        return QModelIndex();

    const QString wanted = leafName.trimmed();
    if (wanted.isEmpty())
        return QModelIndex();

    const int role = Qt::UserRole + 1;

    // Depth-first search over the model
    std::function<QModelIndex(const QModelIndex&)> dfs = [&](const QModelIndex &parent) -> QModelIndex {
        const int rows = m_model->rowCount(parent);
        for (int r = 0; r < rows; ++r) {
            const QModelIndex idx = m_model->index(r, 0, parent);
            if (!idx.isValid())
                continue;

            const QString path = idx.data(role).toString();
            if (!path.isEmpty()) {
                if (leafNameFromPath(path) == wanted && m_session->isDataset(path)) {
                    return idx;
                }
            }

            // Recurse into children
            if (m_model->rowCount(idx) > 0) {
                const QModelIndex hit = dfs(idx);
                if (hit.isValid())
                    return hit;
            }
        }
        return QModelIndex();
    };

    // Start from the root item (row 0, col 0)
    const QModelIndex root = m_model->index(0, 0);
    if (!root.isValid())
        return QModelIndex();

    return dfs(root);
}

QString THdfBrowserWidget::toAbsolutePath(const QString &maybeRelative)
{
    QString s = maybeRelative.trimmed();
    if (s.isEmpty()) return s;
    if (!s.startsWith('/'))
        s.prepend('/');
    return THdfSession::normalizePath(s);
}

bool THdfBrowserWidget::selectPath(const QString &absPath)
{
    if (!m_model || !m_tree) return false;

    const QString p = THdfSession::normalizePath(absPath);
    if (p.isEmpty()) return false;

    const int role = Qt::UserRole + 1;

    const QModelIndex start = m_model->index(0, 0);
    if (!start.isValid()) return false;

    const auto hits = m_model->match(start, role, p, 1, Qt::MatchRecursive);
    if (hits.isEmpty()) return false;

    m_tree->setCurrentIndex(hits.first());
    m_tree->scrollTo(hits.first());
    onSelectionChanged();
    return true;
}
