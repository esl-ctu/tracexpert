#include "timporthdfdatawizard.h"
#include "thdfsession.h"
#include "thdfbrowserwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QGridLayout>
#include <QDir>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QSignalBlocker>
#include <limits>


// ==================== Page 1 ====================

ImportOpenFilePage::ImportOpenFilePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Open HDF5 file"));
    setSubTitle(tr("Select an existing HDF5 file to import data from."));

    auto *fileLabel = new QLabel(tr("Input file:"), this);

    m_filePathEdit = new QLineEdit(this);
    m_browseBtn = new QPushButton(tr("Browse..."), this);

    auto *fileRow = new QHBoxLayout;
    fileRow->addWidget(m_filePathEdit, 1);
    fileRow->addWidget(m_browseBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(fileLabel);
    layout->addLayout(fileRow);
    layout->addStretch(1);
    setLayout(layout);

    registerField("importFilePath*", m_filePathEdit);

    connect(m_browseBtn, &QPushButton::clicked,
            this, &ImportOpenFilePage::browseForFile);
}

void ImportOpenFilePage::browseForFile()
{
    QFileDialog dlg(this, tr("Select HDF5 file"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilter(tr("HDF5 files (*.h5 *.hdf5);;All files (*.*)"));

    if (dlg.exec() == QDialog::Accepted) {
        const auto files = dlg.selectedFiles();
        if (!files.isEmpty())
            m_filePathEdit->setText(files.first());
    }
}

bool ImportOpenFilePage::validatePage()
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    if (!wiz)
        return false;

    QString path = m_filePathEdit->text().trimmed();
    while (path.endsWith('.'))
        path.chop(1);

    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("No input file"),
                             tr("Please choose an input HDF5 file."));
        return false;
    }

    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
        QMessageBox::warning(this, tr("File does not exist"),
                             tr("The selected file does not exist:\n%1").arg(path));
        return false;
    }

    if (!wiz->openExistingSession(path)) {
        QMessageBox::warning(this, tr("Cannot open HDF5 file"),
                             tr("Failed to open the HDF5 file:\n%1").arg(path));
        return false;
    }

    return true;
}


// ==================== Page 2 ====================

SelectDatasetPage::SelectDatasetPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Source dataset"));
    setSubTitle(tr("Select a dataset and choose which elements to import."));

    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setTextFormat(Qt::RichText);
    m_fileNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_filePathLabel = new QLabel(this);
    m_filePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_filePathLabel->setWordWrap(false);

    m_browser = new THdfBrowserWidget(this);
    m_browser->setSelectionPolicy(THdfBrowserWidget::SelectionPolicy::DatasetsOnly);
    m_browser->setAutoPreselectSuggestedDataset(false);
    m_browser->setTitleText(tr("Source dataset (to import)"));

    auto *targetRow = new QHBoxLayout;
    targetRow->addWidget(new QLabel(tr("Selected dataset:"), this));
    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setReadOnly(true);
    targetRow->addWidget(m_targetEdit, 1);

    auto *infoBox = new QGroupBox(tr("Dataset info"), this);
    m_typeLabel = new QLabel("-", infoBox);
    m_elemBytesLabel = new QLabel("-", infoBox);
    m_rankLabel = new QLabel("-", infoBox);
    m_dimsLabel = new QLabel("-", infoBox);
    m_totalElemsLabel = new QLabel("-", infoBox);
    m_totalBytesLabel = new QLabel("-", infoBox);
    m_warnLabel = new QLabel("-", infoBox);
    m_warnLabel->setWordWrap(true);

    auto *infoForm = new QFormLayout;
    infoForm->addRow(tr("Type:"), m_typeLabel);
    infoForm->addRow(tr("Element bytes:"), m_elemBytesLabel);
    infoForm->addRow(tr("Rank:"), m_rankLabel);
    infoForm->addRow(tr("Dims:"), m_dimsLabel);
    infoForm->addRow(tr("Total elements:"), m_totalElemsLabel);
    infoForm->addRow(tr("Total bytes:"), m_totalBytesLabel);
    infoForm->addRow(tr("Status:"), m_warnLabel);
    infoBox->setLayout(infoForm);

    auto *rangeBox = new QGroupBox(tr("Range"), this);
    m_radioAll = new QRadioButton(tr("All elements"), rangeBox);
    m_radioRange = new QRadioButton(tr("Range"), rangeBox);
    m_radioAll->setChecked(true);

    auto *rank1W = new QWidget(rangeBox);
    m_start1 = new QSpinBox(rank1W);
    m_count1 = new QSpinBox(rank1W);
    m_start1->setMinimum(0);
    m_count1->setMinimum(0);

    auto *rank1Row = new QHBoxLayout(rank1W);
    rank1Row->addWidget(new QLabel(tr("Start element:"), rank1W));
    rank1Row->addWidget(m_start1);
    rank1Row->addSpacing(12);
    rank1Row->addWidget(new QLabel(tr("Element count:"), rank1W));
    rank1Row->addWidget(m_count1);
    rank1Row->addStretch(1);
    rank1W->setLayout(rank1Row);

    auto *rank2W = new QWidget(rangeBox);

    m_rowStart = new QSpinBox(rank2W);
    m_rowCount = new QSpinBox(rank2W);
    m_colStart = new QSpinBox(rank2W);
    m_colCount = new QSpinBox(rank2W);

    m_rowStart->setMinimum(0);
    m_rowCount->setMinimum(0);
    m_colStart->setMinimum(0);
    m_colCount->setMinimum(0);

    auto *rank2Grid = new QGridLayout(rank2W);
    rank2Grid->setContentsMargins(0, 0, 0, 0);
    rank2Grid->setHorizontalSpacing(10);
    rank2Grid->setVerticalSpacing(6);

    rank2Grid->addWidget(new QLabel(tr("Row start:"), rank2W), 0, 0);
    rank2Grid->addWidget(m_rowStart, 0, 1);
    rank2Grid->addWidget(new QLabel(tr("Col start:"), rank2W), 0, 2);
    rank2Grid->addWidget(m_colStart, 0, 3);

    rank2Grid->addWidget(new QLabel(tr("Row count:"), rank2W), 1, 0);
    rank2Grid->addWidget(m_rowCount, 1, 1);
    rank2Grid->addWidget(new QLabel(tr("Col count:"), rank2W), 1, 2);
    rank2Grid->addWidget(m_colCount, 1, 3);

    rank2Grid->setColumnStretch(1, 1);
    rank2Grid->setColumnStretch(3, 1);

    rank2W->setLayout(rank2Grid);

    m_rangeStack = new QStackedWidget(rangeBox);
    m_rangeStack->addWidget(rank1W);
    m_rangeStack->addWidget(rank2W);

    m_firstLabel = new QLabel("-", rangeBox);
    m_lastLabel = new QLabel("-", rangeBox);
    m_selElemsLabel = new QLabel("-", rangeBox);
    m_selBytesLabel = new QLabel("-", rangeBox);
    m_selShapeLabel = new QLabel("-", rangeBox);

    auto *summaryW = new QWidget(rangeBox);

    auto *sumGrid = new QGridLayout(summaryW);
    sumGrid->setContentsMargins(0, 0, 0, 0);
    sumGrid->setHorizontalSpacing(12);
    sumGrid->setVerticalSpacing(4);

    sumGrid->addWidget(new QLabel(tr("First:"), summaryW), 0, 0);
    sumGrid->addWidget(m_firstLabel, 0, 1);
    sumGrid->addWidget(new QLabel(tr("Last:"), summaryW), 0, 2);
    sumGrid->addWidget(m_lastLabel, 0, 3);
    sumGrid->addWidget(new QLabel(tr("Shape:"), summaryW), 0, 4);
    sumGrid->addWidget(m_selShapeLabel, 0, 5);

    sumGrid->addWidget(new QLabel(tr("Elements:"), summaryW), 1, 0);
    sumGrid->addWidget(m_selElemsLabel, 1, 1);
    sumGrid->addWidget(new QLabel(tr("Bytes:"), summaryW), 1, 2);
    sumGrid->addWidget(m_selBytesLabel, 1, 3);

    sumGrid->setColumnStretch(1, 1);
    sumGrid->setColumnStretch(3, 1);
    sumGrid->setColumnStretch(5, 1);

    summaryW->setLayout(sumGrid);

    auto *radioRow = new QWidget(rangeBox);
    auto *radioLay = new QHBoxLayout(radioRow);
    radioLay->setContentsMargins(0, 0, 0, 0);
    radioLay->addWidget(m_radioAll);
    radioLay->addSpacing(12);
    radioLay->addWidget(m_radioRange);
    radioLay->addStretch(1);
    radioRow->setLayout(radioLay);

    auto *rangeForm = new QFormLayout;
    rangeForm->addRow(radioRow);
    rangeForm->addRow(m_rangeStack);
    rangeForm->addRow(summaryW);
    rangeBox->setLayout(rangeForm);

    auto *leftCol = new QVBoxLayout;
    leftCol->addWidget(m_browser, 1);
    leftCol->addLayout(targetRow);

    auto *rightCol = new QVBoxLayout;
    rightCol->addWidget(infoBox);
    rightCol->addStretch(1);

    auto *midRow = new QHBoxLayout;
    midRow->addLayout(leftCol, 2);
    midRow->addSpacing(10);
    midRow->addLayout(rightCol, 1);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_fileNameLabel);
    layout->addWidget(m_filePathLabel);
    layout->addSpacing(6);
    layout->addLayout(midRow, 1);
    layout->addWidget(rangeBox);
    setLayout(layout);

    registerField("importAll", m_radioAll);
    registerField("importRank", m_rankLabel, "text");
    registerField("importElementBytes", m_elemBytesLabel, "text");

    registerField("importStartElem", m_start1);
    registerField("importElemCount", m_count1);

    registerField("importRowStart", m_rowStart);
    registerField("importRowCount", m_rowCount);
    registerField("importColStart", m_colStart);
    registerField("importColCount", m_colCount);

    connect(m_browser, &THdfBrowserWidget::selectionChanged,
            this, [this](const QString &p, int /*type*/) {
                m_targetEdit->setText(p);
                m_curPath = p;
                rebuildFromSelection();
            });

    auto recalcFn = [this]() {
        updateRangeUiForRank(m_curRank);
        clampRangeAndUpdateLabels();
    };

    connect(m_radioAll, &QRadioButton::toggled, this, recalcFn);
    connect(m_radioRange, &QRadioButton::toggled, this, recalcFn);

    connect(m_start1, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);
    connect(m_count1, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);

    connect(m_rowStart, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);
    connect(m_rowCount, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);
    connect(m_colStart, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);
    connect(m_colCount, qOverload<int>(&QSpinBox::valueChanged), this, recalcFn);

    updateRangeUiForRank(0);
    rebuildFromSelection();
}

void SelectDatasetPage::initializePage()
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen())
        return;

    const QFileInfo fi(wiz->hdfSession()->filePath());
    m_fileNameLabel->setText(tr("<b>File:</b> %1").arg(fi.fileName()));
    m_filePathLabel->setText(tr("Path: %1").arg(QDir::toNativeSeparators(fi.absoluteFilePath())));

    m_browser->setSession(wiz->hdfSession());
    m_browser->refresh();

    m_curPath = m_browser->selectedPath();
    m_targetEdit->setText(m_curPath);
    rebuildFromSelection();
}

bool SelectDatasetPage::isComplete() const
{
    return m_complete;
}

bool SelectDatasetPage::validatePage()
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen()) {
        QMessageBox::warning(this, tr("HDF5 not open"),
                             tr("The HDF5 file is not open. Please go back and select a valid file."));
        return false;
    }

    const QString selected = m_browser->selectedPath();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("No dataset selected"),
                             tr("Please select a dataset to import."));
        return false;
    }

    if (!m_curCompatible) {
        QMessageBox::warning(this, tr("Unsupported dataset"),
                             tr("Please select a supported dataset (rank 1 or 2, supported type)."));
        return false;
    }

    updateRangeUiForRank(m_curRank);
    clampRangeAndUpdateLabels();

    TImportHDFDataWizard::ImportRequest req;
    req.datasetPath = THdfSession::normalizePath(selected);
    req.all = m_radioAll->isChecked();
    req.rank = m_curRank;

    req.typeText = m_curTypeText;
    req.elementBytes = m_curElemBytes;
    req.dims = m_curDims;

    if (req.rank == 1) {
        req.startElem = static_cast<quint64>(m_start1->value());
        req.elemCount = static_cast<quint64>(m_count1->value());

        if (req.elemCount == 0) {
            QMessageBox::warning(this, tr("Invalid range"),
                                 tr("Element count must be greater than 0."));
            return false;
        }
    } else if (req.rank == 2) {
        req.rowStart = static_cast<quint64>(m_rowStart->value());
        req.rowCount = static_cast<quint64>(m_rowCount->value());
        req.colStart = static_cast<quint64>(m_colStart->value());
        req.colCount = static_cast<quint64>(m_colCount->value());

        if (req.rowCount == 0 || req.colCount == 0) {
            QMessageBox::warning(this, tr("Invalid range"),
                                 tr("Row count and column count must be greater than 0."));
            return false;
        }
    } else {
        QMessageBox::warning(this, tr("Unsupported dataset"),
                             tr("Only rank 1 and rank 2 datasets are supported."));
        return false;
    }

    wiz->setSourceDatasetPath(req.datasetPath);
    wiz->setImportRequest(req);

    return true;
}


void SelectDatasetPage::rebuildFromSelection()
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    THdfSession *s = wiz ? wiz->hdfSession() : nullptr;

    m_curCompatible = false;
    m_curTypeText.clear();
    m_curElemBytes = 0;
    m_curRank = 0;
    m_curDims.clear();

    QString why;
    if (!s || !s->isOpen() || m_curPath.isEmpty()) {
        m_typeLabel->setText("-");
        m_elemBytesLabel->setText("-");
        m_rankLabel->setText("-");
        m_dimsLabel->setText("-");
        m_totalElemsLabel->setText("-");
        m_totalBytesLabel->setText("-");
        m_warnLabel->setText(tr("No dataset selected."));
        updateRangeUiForRank(0);
        clampRangeAndUpdateLabels();
        return;
    }

    QString typeText;
    int elemBytes = 0;
    int rank = 0;
    QVector<hsize_t> dims;

    if (!queryDatasetInfo(m_curPath, &typeText, &elemBytes, &rank, &dims, &why)) {
        m_typeLabel->setText("-");
        m_elemBytesLabel->setText("-");
        m_rankLabel->setText("-");
        m_dimsLabel->setText("-");
        m_totalElemsLabel->setText("-");
        m_totalBytesLabel->setText("-");
        m_warnLabel->setText(why.isEmpty() ? tr("Selected dataset is not supported.") : why);
        updateRangeUiForRank(0);
        clampRangeAndUpdateLabels();
        return;
    }

    m_curCompatible = true;
    m_curTypeText = typeText;
    m_curElemBytes = elemBytes;
    m_curRank = rank;
    m_curDims = dims;

    m_typeLabel->setText(typeText);
    m_elemBytesLabel->setText(QString::number(elemBytes));
    m_rankLabel->setText(QString::number(rank));

    if (rank == 1) {
        m_dimsLabel->setText(QString("(%1)").arg(QString::number(static_cast<qulonglong>(dims[0]))));
    } else {
        m_dimsLabel->setText(QString("(%1 x %2)")
                                 .arg(QString::number(static_cast<qulonglong>(dims[0])),
                                      QString::number(static_cast<qulonglong>(dims[1]))));
    }

    const quint64 elems = totalElements();
    const quint64 bytes = elems * static_cast<quint64>(elemBytes);

    m_totalElemsLabel->setText(QString::number(static_cast<qulonglong>(elems)));
    m_totalBytesLabel->setText(QString::number(static_cast<qulonglong>(bytes)));
    m_warnLabel->setText(tr("OK"));

    updateRangeUiForRank(rank);
    clampRangeAndUpdateLabels();
}

void SelectDatasetPage::updateRangeUiForRank(int rank)
{
    if (rank == 1) {
        m_rangeStack->setCurrentIndex(0);
        m_start1->setEnabled(m_radioRange->isChecked());
        m_count1->setEnabled(m_radioRange->isChecked());

        m_rowStart->setEnabled(false);
        m_rowCount->setEnabled(false);
        m_colStart->setEnabled(false);
        m_colCount->setEnabled(false);
    } else if (rank == 2) {
        m_rangeStack->setCurrentIndex(1);
        m_rowStart->setEnabled(m_radioRange->isChecked());
        m_rowCount->setEnabled(m_radioRange->isChecked());
        m_colStart->setEnabled(m_radioRange->isChecked());
        m_colCount->setEnabled(m_radioRange->isChecked());

        m_start1->setEnabled(false);
        m_count1->setEnabled(false);
    } else {
        m_rangeStack->setCurrentIndex(0);
        m_start1->setEnabled(false);
        m_count1->setEnabled(false);
        m_rowStart->setEnabled(false);
        m_rowCount->setEnabled(false);
        m_colStart->setEnabled(false);
        m_colCount->setEnabled(false);
    }
}

quint64 SelectDatasetPage::totalElements() const
{
    if (!m_curCompatible)
        return 0;

    if (m_curRank == 1 && m_curDims.size() >= 1)
        return static_cast<quint64>(m_curDims[0]);

    if (m_curRank == 2 && m_curDims.size() >= 2)
        return static_cast<quint64>(m_curDims[0]) * static_cast<quint64>(m_curDims[1]);

    return 0;
}

void SelectDatasetPage::clampRangeAndUpdateLabels()
{
    const quint64 elemsTotal = totalElements();
    const int elemBytes = m_curElemBytes;

    bool ok = m_curCompatible && elemsTotal > 0;

    if (!ok) {
        m_firstLabel->setText("-");
        m_lastLabel->setText("-");
        m_selElemsLabel->setText("0");
        m_selBytesLabel->setText("0");
        m_selShapeLabel->setText("-");
        if (m_complete != false) { m_complete = false; emit completeChanged(); }
        return;
    }

    const bool all = m_radioAll->isChecked();

    if (all) {
        if (m_curRank == 1) {
            QSignalBlocker bs(m_start1);
            QSignalBlocker bc(m_count1);
            m_start1->setMaximum(static_cast<int>(qMax<quint64>(0, elemsTotal - 1)));
            m_count1->setMaximum(static_cast<int>(elemsTotal));
            m_start1->setValue(0);
            m_count1->setValue(static_cast<int>(elemsTotal));
        } else if (m_curRank == 2 && m_curDims.size() >= 2) {
            const quint64 rows = static_cast<quint64>(m_curDims[0]);
            const quint64 cols = static_cast<quint64>(m_curDims[1]);

            QSignalBlocker br0(m_rowStart);
            QSignalBlocker brc(m_rowCount);
            QSignalBlocker bc0(m_colStart);
            QSignalBlocker bcc(m_colCount);

            m_rowStart->setMaximum(static_cast<int>(qMax<quint64>(0, rows - 1)));
            m_rowCount->setMaximum(static_cast<int>(rows));
            m_colStart->setMaximum(static_cast<int>(qMax<quint64>(0, cols - 1)));
            m_colCount->setMaximum(static_cast<int>(cols));

            m_rowStart->setValue(0);
            m_rowCount->setValue(static_cast<int>(rows));
            m_colStart->setValue(0);
            m_colCount->setValue(static_cast<int>(cols));
        }

        if (m_curRank == 1) {
            m_firstLabel->setText(QString::number(0));
            m_lastLabel->setText(QString::number(static_cast<qulonglong>(elemsTotal - 1)));
            m_selShapeLabel->setText(QString("(%1)").arg(QString::number(static_cast<qulonglong>(elemsTotal))));
        } else {
            const quint64 rows = static_cast<quint64>(m_curDims[0]);
            const quint64 cols = static_cast<quint64>(m_curDims[1]);
            m_firstLabel->setText(QString("(0,0)"));
            m_lastLabel->setText(QString("(%1,%2)")
                                     .arg(QString::number(static_cast<qulonglong>(rows - 1)),
                                          QString::number(static_cast<qulonglong>(cols - 1))));
            m_selShapeLabel->setText(QString("(%1 x %2)")
                                         .arg(QString::number(static_cast<qulonglong>(rows)),
                                              QString::number(static_cast<qulonglong>(cols))));
        }

        const quint64 selElems = elemsTotal;
        const quint64 selBytes = selElems * static_cast<quint64>(elemBytes);
        m_selElemsLabel->setText(QString::number(static_cast<qulonglong>(selElems)));
        m_selBytesLabel->setText(QString::number(static_cast<qulonglong>(selBytes)));

        ok = (selElems > 0);
    } else {
        if (m_curRank == 1) {
            const quint64 maxStart = (elemsTotal > 0) ? (elemsTotal - 1) : 0;
            {
                QSignalBlocker bs(m_start1);
                m_start1->setMaximum(static_cast<int>(maxStart));
            }
            const quint64 start = static_cast<quint64>(m_start1->value());
            const quint64 maxCount = (start <= elemsTotal) ? (elemsTotal - start) : 0;
            {
                QSignalBlocker bc(m_count1);
                m_count1->setMaximum(static_cast<int>(maxCount));
                if (static_cast<quint64>(m_count1->value()) > maxCount)
                    m_count1->setValue(static_cast<int>(maxCount));
            }
            const quint64 count = static_cast<quint64>(m_count1->value());

            m_firstLabel->setText(count > 0 ? QString::number(static_cast<qulonglong>(start)) : "-");
            m_lastLabel->setText(count > 0 ? QString::number(static_cast<qulonglong>(start + count - 1)) : "-");
            m_selShapeLabel->setText(count > 0 ? QString("(%1)").arg(QString::number(static_cast<qulonglong>(count))) : "-");

            const quint64 selBytes = count * static_cast<quint64>(elemBytes);
            m_selElemsLabel->setText(QString::number(static_cast<qulonglong>(count)));
            m_selBytesLabel->setText(QString::number(static_cast<qulonglong>(selBytes)));

            ok = (count > 0);
        } else if (m_curRank == 2 && m_curDims.size() >= 2) {
            const quint64 rows = static_cast<quint64>(m_curDims[0]);
            const quint64 cols = static_cast<quint64>(m_curDims[1]);

            {
                QSignalBlocker br0(m_rowStart);
                m_rowStart->setMaximum(static_cast<int>((rows > 0) ? (rows - 1) : 0));
            }
            const quint64 r0 = static_cast<quint64>(m_rowStart->value());
            const quint64 maxRCount = (r0 <= rows) ? (rows - r0) : 0;
            {
                QSignalBlocker brc(m_rowCount);
                m_rowCount->setMaximum(static_cast<int>(maxRCount));
                if (static_cast<quint64>(m_rowCount->value()) > maxRCount)
                    m_rowCount->setValue(static_cast<int>(maxRCount));
            }
            const quint64 rCount = static_cast<quint64>(m_rowCount->value());

            {
                QSignalBlocker bc0(m_colStart);
                m_colStart->setMaximum(static_cast<int>((cols > 0) ? (cols - 1) : 0));
            }
            const quint64 c0 = static_cast<quint64>(m_colStart->value());
            const quint64 maxCCount = (c0 <= cols) ? (cols - c0) : 0;
            {
                QSignalBlocker bcc(m_colCount);
                m_colCount->setMaximum(static_cast<int>(maxCCount));
                if (static_cast<quint64>(m_colCount->value()) > maxCCount)
                    m_colCount->setValue(static_cast<int>(maxCCount));
            }
            const quint64 cCount = static_cast<quint64>(m_colCount->value());

            const bool rectOk = (rCount > 0 && cCount > 0);

            m_firstLabel->setText(rectOk ? QString("(%1,%2)")
                                               .arg(QString::number(static_cast<qulonglong>(r0)),
                                                    QString::number(static_cast<qulonglong>(c0)))
                                         : "-");

            m_lastLabel->setText(rectOk ? QString("(%1,%2)")
                                              .arg(QString::number(static_cast<qulonglong>(r0 + rCount - 1)),
                                                   QString::number(static_cast<qulonglong>(c0 + cCount - 1)))
                                        : "-");

            m_selShapeLabel->setText(rectOk ? QString("(%1 x %2)")
                                                  .arg(QString::number(static_cast<qulonglong>(rCount)),
                                                       QString::number(static_cast<qulonglong>(cCount)))
                                            : "-");

            const quint64 selElems = rCount * cCount;
            const quint64 selBytes = selElems * static_cast<quint64>(elemBytes);

            m_selElemsLabel->setText(QString::number(static_cast<qulonglong>(selElems)));
            m_selBytesLabel->setText(QString::number(static_cast<qulonglong>(selBytes)));

            ok = rectOk;
        } else {
            ok = false;
        }
    }

    if (m_complete != ok) {
        m_complete = ok;
        emit completeChanged();
    }
}

bool SelectDatasetPage::queryDatasetInfo(const QString &path,
                                         QString *typeTextOut,
                                         int *elemBytesOut,
                                         int *rankOut,
                                         QVector<hsize_t> *dimsOut,
                                         QString *whyNotOut) const
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    THdfSession *s = wiz ? wiz->hdfSession() : nullptr;
    if (!s || !s->isOpen()) {
        if (whyNotOut) *whyNotOut = tr("No open session.");
        return false;
    }

    const QString p = THdfSession::normalizePath(path);
    if (!s->isDataset(p)) {
        if (whyNotOut) *whyNotOut = tr("Selection is not a dataset.");
        return false;
    }

    const THdfSession::DatasetInfo info = s->datasetInfo(p);
    if (!info.valid) {
        if (whyNotOut) *whyNotOut = tr("Cannot query dataset info.");
        return false;
    }

    if (info.rank != 1 && info.rank != 2) {
        if (whyNotOut) *whyNotOut = tr("Only rank 1 and rank 2 datasets are supported.");
        return false;
    }

    if (info.dims.size() < info.rank) {
        if (whyNotOut) *whyNotOut = tr("Dataset dims are invalid.");
        return false;
    }

    hid_t dset = H5Dopen2(s->fileId(), p.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        if (whyNotOut) *whyNotOut = tr("Cannot open dataset.");
        return false;
    }

    hid_t t = H5Dget_type(dset);
    if (t < 0) {
        H5Dclose(dset);
        if (whyNotOut) *whyNotOut = tr("Cannot read dataset type.");
        return false;
    }

    const H5T_class_t cls = H5Tget_class(t);
    const size_t sz = H5Tget_size(t);

    QString typeText;
    int elemBytes = static_cast<int>(sz);
    bool okType = false;

    if (cls == H5T_INTEGER) {
        const H5T_sign_t sign = H5Tget_sign(t);
        const bool isUnsigned = (sign == H5T_SGN_NONE);

        if (sz == 1 || sz == 2 || sz == 4 || sz == 8) {
            typeText = QString("%1%2")
            .arg(isUnsigned ? "uint" : "int")
                .arg(QString::number(static_cast<int>(sz * 8)));
            okType = true;
        }
    } else if (cls == H5T_FLOAT) {
        if (sz == 4 || sz == 8) {
            typeText = QString("float%1").arg(QString::number(static_cast<int>(sz * 8)));
            okType = true;
        }
    }

    H5Tclose(t);
    H5Dclose(dset);

    if (!okType) {
        if (whyNotOut) *whyNotOut = tr("Dataset type is not supported.");
        return false;
    }

    if (typeTextOut) *typeTextOut = typeText;
    if (elemBytesOut) *elemBytesOut = elemBytes;
    if (rankOut) *rankOut = info.rank;

    if (dimsOut) {
        dimsOut->clear();
        for (int i = 0; i < info.rank; ++i)
            dimsOut->push_back(info.dims[i]);
    }

    return true;
}


// ==================== Page 3 ====================

ImportExecutePage::ImportExecutePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Import results"));
    setFinalPage(true);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);

    m_log = new QPlainTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setMinimumHeight(220);
    m_log->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(m_summaryLabel);
    lay->addWidget(m_log);
}

void ImportExecutePage::initializePage()
{
    if (m_ran)
        return;
    m_ran = true;

    m_log->clear();

    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    if (!wiz) {
        m_log->appendPlainText("Internal error: wizard not available.");
        return;
    }

    wiz->clearImportResult();

    THdfSession *session = wiz->hdfSession();
    if (!session || !session->isOpen()) {
        m_log->appendPlainText("No open HDF session.");
        updateSummary();
        return;
    }

    const TImportHDFDataWizard::ImportRequest req = wiz->importRequest();

    updateSummary();
    m_log->appendPlainText("Starting import...\n");

    quint64 selElems = 0;
    if (req.rank == 1) {
        selElems = req.all ? static_cast<quint64>(req.dims.value(0)) : req.elemCount;
    } else if (req.rank == 2) {
        if (req.all) {
            selElems = static_cast<quint64>(req.dims.value(0)) * static_cast<quint64>(req.dims.value(1));
        } else {
            selElems = req.rowCount * req.colCount;
        }
    }

    QVector<quint64> resultDims;

    if (req.rank == 1) {
        const quint64 n = req.all ? static_cast<quint64>(req.dims.value(0))
                                  : req.elemCount;
        resultDims.push_back(n);
    } else if (req.rank == 2) {
        const quint64 r = req.all ? static_cast<quint64>(req.dims.value(0))
                                  : req.rowCount;
        const quint64 c = req.all ? static_cast<quint64>(req.dims.value(1))
                                  : req.colCount;
        resultDims.push_back(r);
        resultDims.push_back(c);
    }

    QString opLog;

    const quint64 selBytes64 = selElems * static_cast<quint64>(qMax(0, req.elementBytes));
    if (selBytes64 > static_cast<quint64>(std::numeric_limits<int>::max())) {
        const QString msg = QString("Selection is too large to fit into QByteArray (%1 bytes).").arg(QString::number(static_cast<qulonglong>(selBytes64)));
        m_log->appendPlainText(msg);
        wiz->setImportResult(false, QByteArray(), req.typeText, req.elementBytes, resultDims, opLog);
        return;
    }

    QByteArray out;

    bool ok = false;

    QVector<quint64> readDims;

    if (req.rank == 1) {
        // Rank-1: either all elements or [startElem, elemCount]
        const quint64 start = req.all ? 0ULL : req.startElem;
        const quint64 count = req.all ? static_cast<quint64>(req.dims.value(0)) : req.elemCount;

        ok = session->readDataset(req.datasetPath,
                                  start,
                                  count,
                                  out,
                                  &readDims,
                                  &opLog);

    } else if (req.rank == 2) {
        const quint64 rowStart = req.all ? 0ULL : req.rowStart;
        const quint64 rowCount = req.all ? static_cast<quint64>(req.dims.value(0)) : req.rowCount;
        const quint64 colStart = req.all ? 0ULL : req.colStart;
        const quint64 colCount = req.all ? static_cast<quint64>(req.dims.value(1)) : req.colCount;

        ok = session->readDataset(req.datasetPath,
                                  rowStart,
                                  rowCount,
                                  colStart,
                                  colCount,
                                  out,
                                  &readDims,
                                  &opLog);

    } else {
        opLog = "Unsupported rank.";
        ok = false;
    }

    if (!opLog.isEmpty())
        m_log->appendPlainText(opLog.trimmed());

    const QVector<quint64> finalDims = !readDims.isEmpty() ? readDims : resultDims;

    if (ok) {
        m_log->appendPlainText("\nImport completed successfully.");
        wiz->setImportResult(true, out, req.typeText, req.elementBytes, finalDims , opLog);
    } else {
        m_log->appendPlainText("\nImport failed.");
        wiz->setImportResult(false, QByteArray(), req.typeText, req.elementBytes, finalDims , opLog);
    }
}

void ImportExecutePage::updateSummary()
{
    auto *wiz = qobject_cast<TImportHDFDataWizard*>(wizard());
    if (!wiz) {
        m_summaryLabel->setText("-");
        return;
    }

    const QString filePath = field("importFilePath").toString();
    const TImportHDFDataWizard::ImportRequest req = wiz->importRequest();

    QString selectionText;
    QString shapeText;

    if (req.rank == 1) {
        const quint64 n = static_cast<quint64>(req.dims.value(0));
        if (req.all) {
            selectionText = QString("All elements (0 .. %1)").arg(QString::number(static_cast<qulonglong>(n ? (n - 1) : 0)));
            shapeText = QString("(%1)").arg(QString::number(static_cast<qulonglong>(n)));
        } else {
            const quint64 first = req.startElem;
            const quint64 last = (req.elemCount > 0) ? (req.startElem + req.elemCount - 1) : 0;
            selectionText = QString("Elements %1 .. %2 (%3)")
                                .arg(QString::number(static_cast<qulonglong>(first)),
                                     QString::number(static_cast<qulonglong>(last)),
                                     QString::number(static_cast<qulonglong>(req.elemCount)));
            shapeText = QString("(%1)").arg(QString::number(static_cast<qulonglong>(req.elemCount)));
        }
    } else if (req.rank == 2) {
        const quint64 rows = static_cast<quint64>(req.dims.value(0));
        const quint64 cols = static_cast<quint64>(req.dims.value(1));
        if (req.all) {
            selectionText = "All elements";
            shapeText = QString("(%1 x %2)")
                            .arg(QString::number(static_cast<qulonglong>(rows)),
                                 QString::number(static_cast<qulonglong>(cols)));
        } else {
            const quint64 r0 = req.rowStart;
            const quint64 c0 = req.colStart;
            const quint64 r1 = (req.rowCount > 0) ? (req.rowStart + req.rowCount - 1) : 0;
            const quint64 c1 = (req.colCount > 0) ? (req.colStart + req.colCount - 1) : 0;
            selectionText = QString("Rows %1 .. %2, Cols %3 .. %4")
                                .arg(QString::number(static_cast<qulonglong>(r0)),
                                     QString::number(static_cast<qulonglong>(r1)),
                                     QString::number(static_cast<qulonglong>(c0)),
                                     QString::number(static_cast<qulonglong>(c1)));
            shapeText = QString("(%1 x %2)")
                            .arg(QString::number(static_cast<qulonglong>(req.rowCount)),
                                 QString::number(static_cast<qulonglong>(req.colCount)));
        }
    } else {
        selectionText = "Unsupported rank";
        shapeText = "-";
    }

    const QString dimsText =
        (req.rank == 1)
            ? QString("(%1)").arg(QString::number(static_cast<qulonglong>(req.dims.value(0))))
            : QString("(%1 x %2)")
                  .arg(QString::number(static_cast<qulonglong>(req.dims.value(0))),
                       QString::number(static_cast<qulonglong>(req.dims.value(1))));

    m_summaryLabel->setText(
        QString("<b>File:</b> %1<br>"
                "<b>Dataset:</b> %2<br>"
                "<b>Type:</b> %3 (%4 bytes)<br>"
                "<b>Dataset dims:</b> %5<br>"
                "<b>Selection:</b> %6<br>"
                "<b>Selected shape:</b> %7")
            .arg(filePath.toHtmlEscaped())
            .arg(req.datasetPath.toHtmlEscaped())
            .arg(req.typeText.toHtmlEscaped())
            .arg(QString::number(req.elementBytes))
            .arg(dimsText.toHtmlEscaped())
            .arg(selectionText.toHtmlEscaped())
            .arg(shapeText.toHtmlEscaped())
        );
}



// ==================== Wizard ====================

TImportHDFDataWizard::TImportHDFDataWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Import data"));

    m_session = QSharedPointer<THdfSession>::create();

    m_page1 = new ImportOpenFilePage(this);
    m_page2 = new SelectDatasetPage(this);
    m_page3 = new ImportExecutePage(this);
    addPage(m_page1);
    addPage(m_page2);
    addPage(m_page3);

}

THdfSession *TImportHDFDataWizard::hdfSession() const
{
    return m_session.data();
}

bool TImportHDFDataWizard::openExistingSession(const QString &path)
{
    if (!m_session)
        m_session = QSharedPointer<THdfSession>::create();

    if (m_session->isOpen())
        m_session->close();

    m_filePath.clear();

    if (!m_session->openExisting(path))
        return false;

    m_filePath = path;
    return true;
}
