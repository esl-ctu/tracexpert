#include "taddopenhdfwizard.h"

#include "thdfsession.h"
#include "thdfbrowserwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

// ---------------- PAGE 1 ----------------

AddOpenFilePage::AddOpenFilePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Add or open HDF5 file"));
    setSubTitle(tr("Create a new HDF5 file or open an existing one."));

    m_radioCreateNew = new QRadioButton(tr("Create new file"));
    m_radioOpenExisting = new QRadioButton(tr("Open existing file"));
    m_radioOpenExisting->setChecked(true);

    auto *modeBox = new QVBoxLayout;
    modeBox->addWidget(m_radioOpenExisting);
    modeBox->addWidget(m_radioCreateNew);

    auto *fileLabel = new QLabel(tr("HDF5 file:"));
    m_fileEdit = new QLineEdit;

    auto *browseBtn = new QPushButton(tr("Browse…"));

    auto *fileRow = new QHBoxLayout;
    fileRow->addWidget(m_fileEdit);
    fileRow->addWidget(browseBtn);

    auto *layout = new QVBoxLayout;
    layout->addLayout(modeBox);
    layout->addSpacing(8);
    layout->addWidget(fileLabel);
    layout->addLayout(fileRow);
    setLayout(layout);

    registerField("hdfFilePath*", m_fileEdit);
    registerField("hdfCreateNew", m_radioCreateNew);
    registerField("hdfOpenExisting", m_radioOpenExisting);

    connect(browseBtn, &QPushButton::clicked, this, [this] {
        const bool createNew = m_radioCreateNew && m_radioCreateNew->isChecked();

        QFileDialog dlg(this, tr("Select HDF5 file"));
        dlg.setNameFilter(tr("HDF5 files (*.h5 *.hdf5);;All files (*.*)"));
        dlg.setDefaultSuffix("h5");

        if (createNew) {
            dlg.setAcceptMode(QFileDialog::AcceptSave);
            dlg.setOption(QFileDialog::DontConfirmOverwrite, true);
        } else {
            dlg.setAcceptMode(QFileDialog::AcceptOpen);
            dlg.setFileMode(QFileDialog::ExistingFile);
        }

        if (dlg.exec() == QDialog::Accepted) {
            const auto files = dlg.selectedFiles();
            if (!files.isEmpty())
                m_fileEdit->setText(files.first());
        }
    });
}

void AddOpenFilePage::initializePage()
{

}

bool AddOpenFilePage::validatePage()
{
    auto *wiz = qobject_cast<TAddOpenHdfWizard*>(wizard());
    if (!wiz) {
        qCritical() << "[AddOpenFilePage] Wizard cast failed";
        return false;
    }

    QString path = m_fileEdit->text().trimmed();

    while (path.endsWith('.'))
        path.chop(1);

    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("No file selected"),
                             tr("Please choose an HDF5 file."));
        return false;
    }

    QFileInfo fi(path);

    // Append default suffix if none present
    if (fi.suffix().isEmpty()) {
        path += ".h5";
        m_fileEdit->setText(path);
        fi = QFileInfo(path);
    }

    // Close any previous session
    if (wiz->hdfSession() && wiz->hdfSession()->isOpen()) {
        qDebug() << "[AddOpenFilePage] Closing previous HDF session:" << wiz->hdfSession()->filePath();
        wiz->hdfSession()->close();
    }

    const bool createNew = field("hdfCreateNew").toBool();
    const QFileInfo fi2(path);

    if (createNew) {

        if (fi2.exists()) {
            QMessageBox::warning(this, tr("File already exists"),
                                 tr("The file already exists:\n%1\n\nChoose a different name or switch to 'Open existing file'.")
                                     .arg(path));
            return false;
        }

        qDebug() << "[HDF] Create new:" << path;
        if (!wiz->hdfSession()->createNew(path)) {
            qCritical() << "[HDF] Failed to create new HDF5 file:" << path;
            QMessageBox::warning(this, tr("Cannot create HDF5 file"),
                                 tr("Failed to create the HDF5 file:\n%1").arg(path));
            return false;
        }
    } else {

        if (!fi2.exists()) {
            QMessageBox::warning(this, tr("File does not exist"),
                                 tr("The selected file does not exist:\n%1").arg(path));
            return false;
        }

        qDebug() << "[HDF] Open existing:" << path;
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

HdfBrowsePage::HdfBrowsePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Browse HDF5 structure"));
    setSubTitle(tr("Create groups/datasets, remove items, and inspect dataset information."));

    m_browser = new THdfBrowserWidget(this);
    m_browser->setSelectionPolicy(THdfBrowserWidget::SelectionPolicy::GroupsAndDatasets);
    m_browser->setTitleText(tr("HDF5 structure"));

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
    layout->addWidget(m_browser, 1);
    setLayout(layout);

    setFinalPage(true);
}

void HdfBrowsePage::initializePage()
{
    auto *wiz = qobject_cast<TAddOpenHdfWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen()) {
        qCritical() << "[HdfBrowsePage] THdfSession missing or not open (Page 1 should open/create)";
        return;
    }

    const QString path = wiz->hdfSession()->filePath();
    const QFileInfo fi(path);

    wiz->setButtonText(QWizard::FinishButton, tr("Close"));

    m_fileNameLabel->setText(tr("<b>File:</b> %1").arg(fi.fileName()));
    m_filePathLabel->setText(tr("Path: %1").arg(QDir::toNativeSeparators(fi.absoluteFilePath())));

    m_browser->setSession(wiz->hdfSession().data());
    m_browser->refresh();
}

// ---------------- WIZARD ----------------

TAddOpenHdfWizard::TAddOpenHdfWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Add/Open HDF5 file"));

    m_hdfSession = QSharedPointer<THdfSession>::create(this);

    addPage(new AddOpenFilePage(this));
    addPage(new HdfBrowsePage(this));

    setWizardStyle(QWizard::ModernStyle);
}

TAddOpenHdfWizard::~TAddOpenHdfWizard()
{
    if (m_hdfSession && m_hdfSession->isOpen()) {
        qDebug() << "[TAddOpenHdfWizard] Destructor closing HDF5 session for" << m_hdfSession->filePath();
        m_hdfSession->close();
    }
}

QString TAddOpenHdfWizard::filePath() const
{
    return field("hdfFilePath").toString().trimmed();
}
