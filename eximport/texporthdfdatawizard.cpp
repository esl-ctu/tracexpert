// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Petr Socha (initial author)

#include "texporthdfdatawizard.h"

#include "thdfsession.h"
#include "thdfbrowserwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <qcombobox.h>

static bool parseSelectedType(const QString& typeText,
                              H5T_class_t& outClass,
                              H5T_sign_t& outSign,
                              size_t& outSize)
{
    const QString t = typeText.trimmed().toLower();

    // floats
    if (t == "float32") { outClass = H5T_FLOAT; outSign = H5T_SGN_NONE; outSize = 4; return true; }
    if (t == "float64") { outClass = H5T_FLOAT; outSign = H5T_SGN_NONE; outSize = 8; return true; }

    // integers
    if (t == "uint8")  { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 1; return true; }
    if (t == "int8")   { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 1; return true; }
    if (t == "uint16") { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 2; return true; }
    if (t == "int16")  { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 2; return true; }
    if (t == "uint32") { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 4; return true; }
    if (t == "int32")  { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 4; return true; }

    return false;
}

static bool datasetMatchesType(const THdfSession* s, const QString& datasetPath, const QString& selectedTypeText)
{
    if (!s || !s->isOpen()) return false;

    H5T_class_t wantCls;
    H5T_sign_t  wantSign;
    size_t      wantSize = 0;

    if (!parseSelectedType(selectedTypeText, wantCls, wantSign, wantSize))
        return false;

    const QString p = THdfSession::normalizePath(datasetPath);
    if (!s->isDataset(p)) return false;

    hid_t dset = H5Dopen2(s->fileId(), p.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) return false;

    hid_t t = H5Dget_type(dset);
    if (t < 0) { H5Dclose(dset); return false; }

    const H5T_class_t gotCls = H5Tget_class(t);
    const size_t gotSize = H5Tget_size(t);

    bool ok = false;

    if (gotCls == wantCls && gotSize == wantSize) {
        if (wantCls == H5T_INTEGER) {
            const H5T_sign_t gotSign = H5Tget_sign(t);
            ok = (gotSign == wantSign);
        } else if (wantCls == H5T_FLOAT) {
            ok = true;
        }
    }

    H5Tclose(t);
    H5Dclose(dset);
    return ok;
}

static hid_t mapTypeTextToNativeHid(const QString& typeText)
{
    const QString t = typeText.trimmed().toLower();

    if (t == "uint8")   return H5T_NATIVE_UINT8;
    if (t == "int8")    return H5T_NATIVE_INT8;
    if (t == "uint16")  return H5T_NATIVE_UINT16;
    if (t == "int16")   return H5T_NATIVE_INT16;
    if (t == "uint32")  return H5T_NATIVE_UINT32;
    if (t == "int32")   return H5T_NATIVE_INT32;
    if (t == "float32") return H5T_NATIVE_FLOAT;
    if (t == "float64") return H5T_NATIVE_DOUBLE;

    return -1;
}


static QString absSuggestedPath(QString p)
{
    p = p.trimmed();
    if (p.isEmpty()) return "/export/data";
    if (!p.startsWith('/')) p.prepend('/');
    while (p.contains("//")) p.replace("//", "/");
    if (p.size() > 1 && p.endsWith('/')) p.chop(1);
    return p;
}

static bool parseDimsText(const QString &txt, int rank, QVector<hsize_t> &out, bool allowUnlimited)
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

static bool datasetIsUInt8(const THdfSession *s, const QString &datasetPath)
{
    if (!s || !s->isOpen()) return false;

    const QString p = THdfSession::normalizePath(datasetPath);
    if (!s->isDataset(p)) return false;

    hid_t dset = H5Dopen2(s->fileId(), p.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) return false;

    hid_t t = H5Dget_type(dset);
    if (t < 0) { H5Dclose(dset); return false; }

    const H5T_class_t cls = H5Tget_class(t);
    const size_t sz = H5Tget_size(t);

    bool ok = false;
    if (cls == H5T_INTEGER && sz == 1) {
        const H5T_sign_t sign = H5Tget_sign(t);
        ok = (sign == H5T_SGN_NONE); // unsigned 8-bit
    }

    H5Tclose(t);
    H5Dclose(dset);
    return ok;
}

static QString findFreeDatasetPath(THdfSession *s, const QString &baseAbs)
{
    if (!s) return baseAbs;

    QString candidate = baseAbs;
    if (!s->pathExists(candidate))
        return candidate;

    for (int i = 1; i <= 99; ++i) {
        const QString alt = baseAbs + QString("_%1").arg(i);
        if (!s->pathExists(alt))
            return alt;
    }
    return baseAbs;
}


// ==================== Page 1 ====================

ExportDataPage::ExportDataPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Data and file selection"));
    setSubTitle(tr("Select byte range, datatype, shaping, and destination HDF5 file."));

    auto *fileLabel = new QLabel(tr("Output file:"), this);
    m_filePathEdit = new QLineEdit(this);
    m_browseBtn = new QPushButton(tr("Browse..."), this);

    auto *fileRow = new QHBoxLayout;
    fileRow->addWidget(m_filePathEdit, 1);
    fileRow->addWidget(m_browseBtn);

    m_totalBytesLabel = new QLabel(tr("Input data size: 0 bytes"), this);

    auto *typeBox = new QGroupBox(tr("Datatype"), this);

    m_typeCombo = new QComboBox(typeBox);
    m_elementBytesValue = new QLabel(tr("1"), typeBox); // default uint8
    m_sizeValue       = new QLabel(tr("-"), typeBox); // slice elements (derived)
    m_alignWarnValue  = new QLabel(tr("-"), typeBox); // remainder warning (optional but recommended)
    m_alignWarnValue->setWordWrap(true);


    auto addType = [this](const QString &text, int bytes) {
        m_typeCombo->addItem(text, bytes); // userData = elementBytes
    };

    addType("uint8",   1);
    addType("int8",    1);
    addType("uint16",  2);
    addType("int16",   2);
    addType("uint32",  4);
    addType("int32",   4);
    addType("float32", 4);
    addType("float64", 8);

    m_typeCombo->setCurrentText("uint8");

    auto *typeForm = new QFormLayout;
    typeForm->addRow(tr("Dataset element type:"), m_typeCombo);
    typeForm->addRow(tr("Element bytes:"), m_elementBytesValue);
    typeForm->addRow(tr("Slice size (elements):"), m_sizeValue);
    typeForm->addRow(tr("Alignment:"), m_alignWarnValue);
    typeBox->setLayout(typeForm);

    auto *rangeBox = new QGroupBox(tr("Range"), this);

    m_radioAll = new QRadioButton(tr("All bytes"), rangeBox);
    m_radioRange = new QRadioButton(tr("Range (bytes)"), rangeBox);
    m_radioAll->setChecked(true);

    m_start = new QSpinBox(rangeBox); // byte start
    m_len   = new QSpinBox(rangeBox); // byte length
    m_start->setEnabled(false);
    m_len->setEnabled(false);

    m_firstIdxValue   = new QLabel(tr("-"), rangeBox); // first byte index
    m_lastIdxValue    = new QLabel(tr("-"), rangeBox); // last byte index
    m_sizeBytesValue  = new QLabel(tr("-"), rangeBox); // slice bytes

    auto *rangeForm = new QFormLayout;

    rangeForm->addRow(m_radioAll, new QWidget(rangeBox));

    auto *rangeRow = new QHBoxLayout;
    rangeRow->addWidget(new QLabel(tr("Start (bytes):"), rangeBox));
    rangeRow->addWidget(m_start);
    rangeRow->addSpacing(10);
    rangeRow->addWidget(new QLabel(tr("Length (bytes):"), rangeBox));
    rangeRow->addWidget(m_len);
    rangeRow->addStretch(1);

    auto *rangeW = new QWidget(rangeBox);
    rangeW->setLayout(rangeRow);

    rangeForm->addRow(m_radioRange, rangeW);

    rangeForm->addRow(tr("First exported byte index:"), m_firstIdxValue);
    rangeForm->addRow(tr("Last exported byte index:"),  m_lastIdxValue);
    rangeForm->addRow(tr("Slice size (bytes):"),              m_sizeBytesValue);

    rangeBox->setLayout(rangeForm);

    auto *shapeBox = new QGroupBox(tr("Shaping"), this);

    m_cols = new QSpinBox(shapeBox);
    m_cols->setMinimum(0);
    m_cols->setMaximum(0); // set later from derived slice elements
    m_cols->setSpecialValueText(tr("0 (1D)"));
    m_cols->setValue(0);

    m_rowsLabel  = new QLabel(tr("-"), shapeBox);
    m_rankLabel  = new QLabel(tr("-"), shapeBox);
    m_shapeLabel = new QLabel(tr("-"), shapeBox);

    auto *shapeForm = new QFormLayout;
    shapeForm->addRow(tr("Columns (elements/row):"), m_cols);
    shapeForm->addRow(tr("Rows:"),  m_rowsLabel);
    shapeForm->addRow(tr("Rank:"),  m_rankLabel);
    shapeForm->addRow(tr("Shape:"), m_shapeLabel);
    shapeBox->setLayout(shapeForm);

    auto *layout = new QVBoxLayout;
    layout->addWidget(m_totalBytesLabel);
    layout->addWidget(rangeBox);
    layout->addWidget(typeBox);
    layout->addWidget(shapeBox);
    layout->addSpacing(8);
    layout->addWidget(fileLabel);
    layout->addLayout(fileRow);
    layout->addStretch(1);
    setLayout(layout);

    registerField("exportFilePath*",    m_filePathEdit);
    registerField("exportAllBytes",     m_radioAll);
    registerField("exportStart",        m_start); // bytes
    registerField("exportLen",          m_len);   // bytes
    registerField("exportCols",         m_cols);  // elements per row
    registerField("exportTypeText",     m_typeCombo, "currentText", "currentTextChanged");
    registerField("exportElementBytes", m_elementBytesValue, "text");
    registerField("exportRows",         m_rowsLabel, "text");
    registerField("exportRank",         m_rankLabel, "text");

    connectSignals();
    recalcAndValidateLive();
}


void ExportDataPage::browseForFile()
{
    QFileDialog dlg(this, tr("Select HDF5 file"));
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("HDF5 files (*.h5 *.hdf5);;All files (*.*)"));
    dlg.setDefaultSuffix("h5");
    dlg.setOption(QFileDialog::DontConfirmOverwrite, true);

    if (dlg.exec() == QDialog::Accepted) {
        const auto files = dlg.selectedFiles();
        if (!files.isEmpty())
            m_filePathEdit->setText(files.first());
    }
}

void ExportDataPage::setTotalBytes(int totalBytes)
{
    m_totalBytes = qMax(0, totalBytes);

    m_totalBytesLabel->setText(tr("Input data size: %1 bytes").arg(m_totalBytes));

    // byte ranges
    m_start->setRange(0, qMax(0, m_totalBytes - 1));
    m_len->setRange(0, m_totalBytes);

    if (m_radioAll->isChecked()) {
        m_start->setValue(0);
        m_len->setValue(m_totalBytes);
    } else {
        const int s = qBound(0, m_start->value(), qMax(0, m_totalBytes - 1));
        m_start->setValue(s);
        m_len->setValue(qBound(0, m_len->value(), m_totalBytes - s));
    }

    // cols max depends on derived slice elements; recalc will set it
    recalcAndValidateLive();
}


void ExportDataPage::initializePage()
{
    recalcAndValidateLive();
}

bool ExportDataPage::validatePage()
{
    auto *wiz = qobject_cast<TExportHDFDataWizard*>(wizard());
    if (!wiz)
        return false;

    QString path = m_filePathEdit->text().trimmed();
    while (path.endsWith('.'))
        path.chop(1);

    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("No output file"),
                             tr("Please choose an output file."));
        return false;
    }

    // Append default suffix if none present
    QFileInfo fi(path);
    if (fi.suffix().isEmpty()) {
        path += ".h5";
        m_filePathEdit->setText(path);
    }

    // IMPORTANT: open/create session only after Page 1 validation succeeds
    if (!wiz->openOrCreateSession(path)) {
        QMessageBox::warning(this, tr("Cannot open/create HDF5 file"),
                             tr("Failed to open or create the HDF5 file:\n%1").arg(path));
        return false;
    }

    return true;
}

bool ExportDataPage::isComplete() const
{
    return m_complete;
}

void ExportDataPage::connectSignals()
{
    connect(m_browseBtn, &QPushButton::clicked, this, &ExportDataPage::browseForFile);

    connect(m_radioAll, &QRadioButton::toggled, this, [this] {
        const bool r = m_radioRange->isChecked();
        m_start->setEnabled(r);
        m_len->setEnabled(r);
        recalcAndValidateLive();
    });

    connect(m_radioRange, &QRadioButton::toggled, this, [this] {
        const bool r = m_radioRange->isChecked();
        m_start->setEnabled(r);
        m_len->setEnabled(r);
        recalcAndValidateLive();
    });

    connect(m_typeCombo, &QComboBox::currentTextChanged, this, [this] {
        m_elementBytes = m_typeCombo->currentData().toInt();
        if (m_elementBytes <= 0) m_elementBytes = 1;
        m_elementBytesValue->setText(QString::number(m_elementBytes));
        recalcAndValidateLive();
    });


    connect(m_start, qOverload<int>(&QSpinBox::valueChanged),
            this, &ExportDataPage::recalcAndValidateLive);
    connect(m_len, qOverload<int>(&QSpinBox::valueChanged),
            this, &ExportDataPage::recalcAndValidateLive);
    connect(m_cols, qOverload<int>(&QSpinBox::valueChanged),
            this, &ExportDataPage::recalcAndValidateLive);
}

void ExportDataPage::recalcAndValidateLive()
{

    m_elementBytes = m_typeCombo ? m_typeCombo->currentData().toInt() : 1;
    if (m_elementBytes <= 0) m_elementBytes = 1;
    if (m_elementBytesValue)
        m_elementBytesValue->setText(QString::number(m_elementBytes));

    const quint64 totalBytes = quint64(qMax(0, m_totalBytes));

    quint64 byteStart = 0;
    quint64 byteLen   = 0;

    if (m_radioAll && m_radioAll->isChecked()) {
        byteStart = 0;
        byteLen   = totalBytes;

        // keep spinboxes coherent even if disabled
        if (m_start && m_len) {
            QSignalBlocker bs(m_start);
            QSignalBlocker bl(m_len);
            m_start->setRange(0, int(qMax<quint64>(0, totalBytes ? totalBytes - 1 : 0)));
            m_len->setRange(0, int(totalBytes));
            m_start->setValue(0);
            m_len->setValue(int(totalBytes));
        }
    } else {
        // clamp to valid byte ranges
        const int maxStart = int(qMax<quint64>(0, totalBytes ? totalBytes - 1 : 0));
        const int s = qBound(0, m_start ? m_start->value() : 0, maxStart);

        if (m_start && s != m_start->value()) {
            QSignalBlocker b(m_start);
            m_start->setValue(s);
        }

        const int maxLen = int(totalBytes - quint64(s));
        const int l = qBound(0, m_len ? m_len->value() : 0, maxLen);

        if (m_len && l != m_len->value()) {
            QSignalBlocker b(m_len);
            m_len->setValue(l);
        }

        byteStart = quint64(s);
        byteLen   = quint64(l);
    }

    const quint64 eb = quint64(m_elementBytes);
    const quint64 sliceElements = (eb > 0) ? (byteLen / eb) : 0;
    const quint64 remainderBytes = (eb > 0) ? (byteLen % eb) : byteLen;

    if (m_totalBytesLabel) {
        m_totalBytesLabel->setText(tr("Input data size: %1 bytes").arg(m_totalBytes));
    }

    if (byteLen > 0) {
        if (m_firstIdxValue) m_firstIdxValue->setText(QString::number(byteStart));
        if (m_lastIdxValue)  m_lastIdxValue->setText(QString::number(byteStart + byteLen - 1));
        if (m_sizeBytesValue) m_sizeBytesValue->setText(QString::number(byteLen));
        if (m_sizeValue)      m_sizeValue->setText(remainderBytes == 0 ? QString::number(sliceElements) : tr("-"));
    } else {
        if (m_firstIdxValue)  m_firstIdxValue->setText(tr("-"));
        if (m_lastIdxValue)   m_lastIdxValue->setText(tr("-"));
        if (m_sizeBytesValue) m_sizeBytesValue->setText(tr("-"));
        if (m_sizeValue)      m_sizeValue->setText(tr("-"));
    }

    bool ok = true;

    if (byteLen == 0)
        ok = false;

    if (remainderBytes != 0) {
        ok = false;
        if (m_alignWarnValue) {
            m_alignWarnValue->setText(
                tr("%1 trailing byte(s): slice size (%2) is not divisible by element size (%3).")
                    .arg(QString::number(remainderBytes))
                    .arg(QString::number(byteLen))
                    .arg(QString::number(m_elementBytes))
                );
        }
    } else {
        if (m_alignWarnValue) {
            if (byteLen > 0)
                m_alignWarnValue->setText(tr("OK"));
            else
                m_alignWarnValue->setText(tr("-"));
        }
    }

    // shaping (cols in ELEMENTS per row)
    quint64 cols = 0;
    if (m_cols) {
        cols = quint64(m_cols->value());

        // Update max cols to current slice element count (keep current value if possible)
        const int maxCols = int(sliceElements);
        const int cur = m_cols->value();

        QSignalBlocker bc(m_cols);
        m_cols->setRange(0, qMax(0, maxCols));
        if (cur > maxCols) m_cols->setValue(maxCols);
    }

    if (ok && sliceElements > 0) {
        if (cols == 0) {
            // rank 1
            if (m_rowsLabel)  m_rowsLabel->setText(tr("-"));
            if (m_rankLabel)  m_rankLabel->setText(QStringLiteral("1"));
            if (m_shapeLabel) m_shapeLabel->setText(QString("(%1)").arg(QString::number(sliceElements)));
        } else {
            // rank 2 requires full rows
            if (sliceElements % cols == 0) {
                const quint64 rows = sliceElements / cols;
                if (m_rowsLabel)  m_rowsLabel->setText(QString::number(rows));
                if (m_rankLabel)  m_rankLabel->setText(QStringLiteral("2"));
                if (m_shapeLabel) m_shapeLabel->setText(QString("(%1 x %2)")
                                              .arg(QString::number(rows),
                                                   QString::number(cols)));
            } else {
                ok = false;
                if (m_rowsLabel)  m_rowsLabel->setText(tr("-"));
                if (m_rankLabel)  m_rankLabel->setText(tr("-"));
                if (m_shapeLabel) m_shapeLabel->setText(tr("Invalid: elements (%1) not divisible by columns (%2)")
                                              .arg(QString::number(sliceElements),
                                                   QString::number(cols)));
            }
        }
    } else {
        if (m_rowsLabel)  m_rowsLabel->setText(tr("-"));
        if (m_rankLabel)  m_rankLabel->setText(tr("-"));
        if (m_shapeLabel) m_shapeLabel->setText(ok ? tr("-") : tr("Invalid"));
    }

    m_complete = ok && (byteLen > 0) && (remainderBytes == 0) && (sliceElements > 0);
    emit completeChanged();
}


// ==================== Page 2 ====================

TargetDatasetPage::TargetDatasetPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Target dataset"));
    setSubTitle(tr("Browse the HDF5 structure and select the dataset to append to."));

    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setTextFormat(Qt::RichText);
    m_fileNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_filePathLabel = new QLabel(this);
    m_filePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_filePathLabel->setWordWrap(false);

    m_browser = new THdfBrowserWidget(this);
    m_browser->setSelectionPolicy(THdfBrowserWidget::SelectionPolicy::DatasetsOnly);
    m_browser->setAutoPreselectSuggestedDataset(true);
    m_browser->setTitleText(tr("Destination dataset (to append)"));

    auto *targetRow = new QHBoxLayout;
    targetRow->addWidget(new QLabel(tr("Selected dataset:"), this));
    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setReadOnly(true);
    targetRow->addWidget(m_targetEdit, 1);

    auto *layout = new QVBoxLayout;
    layout->addWidget(m_fileNameLabel);
    layout->addWidget(m_filePathLabel);
    layout->addSpacing(6);
    layout->addWidget(m_browser, 1);
    layout->addLayout(targetRow);
    setLayout(layout);

    connect(m_browser, &THdfBrowserWidget::selectionChanged,
            this, [this](const QString &p, int /*type*/) {
                m_targetEdit->setText(p);
                if (auto *wiz = qobject_cast<TExportHDFDataWizard*>(wizard()))
                    wiz->setTargetDatasetPath(p);
            });
}

void TargetDatasetPage::initializePage()
{

    auto *wiz = qobject_cast<TExportHDFDataWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen())
        return;

    const QFileInfo fi(wiz->hdfSession()->filePath());
    m_fileNameLabel->setText(tr("<b>File:</b> %1").arg(fi.fileName()));
    m_filePathLabel->setText(tr("Path: %1").arg(QDir::toNativeSeparators(fi.absoluteFilePath())));

    m_browser->setSession(wiz->hdfSession());
    rebuildTemplateFromPage1();
    m_browser->refresh();



}

void TargetDatasetPage::rebuildTemplateFromPage1()
{
    const int cols = field("exportCols").toInt(); // 0 => 1D
    const QString typeText = field("exportTypeText").toString().trimmed();
    const int elementBytes = field("exportElementBytes").toInt(); // stored as label text

    const int safeElementBytes = (elementBytes > 0 ? elementBytes : 1);

    THdfBrowserWidget::TDatasetTemplate t;
    t.suggestedName = "export/data";

    t.typeText = typeText.isEmpty() ? "uint8" : typeText;
    t.lockType = true;

    if (cols <= 0) {
        t.rank = 1;
        t.lockRank = true;
        t.initialDimsText = "0";
        t.maxDimsText = "unlimited";
        t.lockInitialDims = true;
        t.lockMaxDims = true;
    } else {
        t.rank = 2;
        t.lockRank = true;
        t.initialDimsText = QString("0,%1").arg(QString::number(cols));
        t.maxDimsText     = QString("unlimited,%1").arg(QString::number(cols));
        t.lockInitialDims = true;
        t.lockMaxDims = true;
    }

    const qulonglong targetBytes = 256ULL * 1024ULL;   // 256 KiB target
    const qulonglong maxBytes    = 1024ULL * 1024ULL;  // 1 MiB cap

    qulonglong targetElems = targetBytes / qulonglong(safeElementBytes);
    if (targetElems < 1ULL) targetElems = 1ULL;

    qulonglong maxElems = maxBytes / qulonglong(safeElementBytes);
    if (maxElems < 1ULL) maxElems = 1ULL;

    if (cols <= 0) {
        // rank-1: chunk is element count
        qulonglong chunk = targetElems;
        if (chunk < 1024ULL) chunk = 1024ULL;  // minimum 1024 elements (arbitrary but sane)
        if (chunk > maxElems) chunk = maxElems;
        t.chunkDimsText = QString::number(chunk);
    } else {
        // rank-2: (chunkRows, cols)
        const qulonglong ccols = static_cast<qulonglong>(cols);

        // choose chunkCols so chunkRows>=1 and chunk size stays under maxElems
        qulonglong chunkCols = ccols;
        if (chunkCols > maxElems) chunkCols = maxElems; // keep chunk at most maxElems elements wide
        if (chunkCols < 1ULL) chunkCols = 1ULL;

        // recompute chunkRows based on chunkCols
        qulonglong chunkRows = targetElems / chunkCols;
        if (chunkRows < 1ULL) chunkRows = 1ULL;

        t.chunkDimsText = QString("%1,%2").arg(chunkRows).arg(chunkCols);
    }

    m_browser->setNewDatasetTemplate(t);
}



bool TargetDatasetPage::validatePage()
{
    auto *wiz = qobject_cast<TExportHDFDataWizard*>(wizard());
    if (!wiz || !wiz->hdfSession() || !wiz->hdfSession()->isOpen()) {
        QMessageBox::warning(this, tr("HDF5 not open"),
                             tr("The HDF5 file is not open. Please go back and select a valid file."));
        return false;
    }

    THdfSession *s = wiz->hdfSession();

    const int cols = field("exportCols").toInt(); // 0 => 1D
    const QString typeText = field("exportTypeText").toString().trimmed();

    bool warnRowSplit = false;
    hsize_t warnChunkCols = 0;

    auto isCompatible = [&](const QString &path) -> bool {
        const QString p = THdfSession::normalizePath(path);
        if (!s->isDataset(p)) return false;

        if (!datasetMatchesType(s, p, typeText)) return false;

        const THdfSession::DatasetInfo info = s->datasetInfo(p);
        if (!info.valid) return false;

        if (info.maxDims.isEmpty() || info.maxDims[0] != H5S_UNLIMITED)
            return false;

        if (!info.chunked) return false;
        if (info.chunkDims.size() != info.rank) return false;

        if (info.chunkDims.size() != info.rank)
            return false;

        for (hsize_t cd : info.chunkDims) {
            if (cd == 0)
                return false;
        }

        if (cols <= 0) {
            if (info.rank != 1) return false;
            return true;
        }

        // 2D export requires rank-2 with fixed second dim == cols
        if (info.rank != 2) return false;
        if (info.dims.size() < 2 || info.maxDims.size() < 2) return false;

        const hsize_t wantCols = static_cast<hsize_t>(cols);
        if (info.dims[1] != wantCols || info.maxDims[1] != wantCols)
            return false;

        // For rank-2: allow chunkCols <= cols (row-splitting OK, but slower)
        if (info.chunkDims.size() < 2) return false;
        const hsize_t chunkCols = info.chunkDims[1];

        if (chunkCols > wantCols)
            return false;

        if (chunkCols < wantCols) {
            warnRowSplit = true;
            warnChunkCols = chunkCols;
        }

        return true;
    };


    const QString selected = m_browser->selectedPath();
    if (!selected.isEmpty() && isCompatible(selected)) {
        wiz->setTargetDatasetPath(THdfSession::normalizePath(selected));
        if (warnRowSplit) {
            QMessageBox::information(
                this,
                tr("Chunking note"),
                tr("The selected dataset splits rows across multiple chunks (chunk columns = %1, columns = %2).\n\n"
                   "This is valid, but appending may be significantly slower.\n"
                   "For best performance, use chunk columns = %2.")
                    .arg(QString::number(warnChunkCols))
                    .arg(QString::number(cols))
                );
        }
        return true;
    }    

    const QString msg =
        (selected.isEmpty()
             ? tr("No compatible dataset is selected.\n\nCreate a compatible dataset now?")
             : tr("The selected dataset is not compatible.\n\nCreate a compatible dataset now?"));

    if (QMessageBox::question(this, tr("Create dataset"), msg,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes) != QMessageBox::Yes) {
        return false;
    }

    const auto templ = m_browser->newDatasetTemplate();

    const QString typeTextLower = field("exportTypeText").toString().trimmed().toLower();
    if (templ.typeText.trimmed().toLower() != typeTextLower) {
        QMessageBox::warning(this, tr("Template mismatch"),
                             tr("Internal error: dataset template type must match selected type (%1).")
                                 .arg(typeTextLower));
        return false;
    }

    const int rank = qMax(1, templ.rank);    

    if (cols <= 0 && rank != 1) {
        QMessageBox::warning(this, tr("Template mismatch"),
                             tr("Internal error: dataset template rank must be 1 when columns == 0 (1D)."));
        return false;
    }
    if (cols > 0 && rank != 2) {
        QMessageBox::warning(this, tr("Template mismatch"),
                             tr("Internal error: dataset template rank must be 2 when columns > 0 (2D)."));
        return false;
    }

    QVector<hsize_t> initDims, maxDims, chunkDims;

    if (!parseDimsText(templ.initialDimsText, rank, initDims, /*allowUnlimited*/false)) {
        QMessageBox::warning(this, tr("Invalid template"),
                             tr("Cannot create dataset: invalid initial dims in template."));
        return false;
    }
    if (!parseDimsText(templ.maxDimsText, rank, maxDims, /*allowUnlimited*/true)) {
        QMessageBox::warning(this, tr("Invalid template"),
                             tr("Cannot create dataset: invalid max dims in template."));
        return false;
    }
    if (!parseDimsText(templ.chunkDimsText, rank, chunkDims, /*allowUnlimited*/false)) {
        QMessageBox::warning(this, tr("Invalid template"),
                             tr("Cannot create dataset: invalid chunk dims in template."));
        return false;
    }


    for (int i = 0; i < chunkDims.size(); ++i) {
        if (chunkDims[i] == 0) {
            QMessageBox::warning(this, tr("Invalid template"),
                                 tr("Cannot create dataset: chunk dimensions must be > 0."));
            return false;
        }
    }

    if (rank != initDims.size() || rank != maxDims.size() || rank != chunkDims.size()) {
        QMessageBox::warning(this, tr("Invalid template"),
                             tr("Cannot create dataset: template dimension lists are inconsistent."));
        return false;
    }

    if (maxDims[0] != H5S_UNLIMITED) {
        QMessageBox::warning(this, tr("Invalid template"),
                             tr("Cannot create dataset: first dimension must be unlimited."));
        return false;
    }

    if (cols > 0) {
        if (rank != 2 ||
            initDims[1] != static_cast<hsize_t>(cols) ||
            maxDims[1]  != static_cast<hsize_t>(cols)) {
            QMessageBox::warning(this, tr("Invalid template"),
                                 tr("Cannot create dataset: template does not match required column count."));
            return false;
        }

        // chunkCols must be <= cols (row-splitting allowed)
        if (chunkDims[1] > static_cast<hsize_t>(cols)) {
            QMessageBox::warning(this, tr("Invalid template"),
                                 tr("Cannot create dataset: chunk columns must be <= columns."));
            return false;
        }
    }

    const QString baseAbs = absSuggestedPath(templ.suggestedName.isEmpty() ? "export/data" : templ.suggestedName);
    const QString createPath = findFreeDatasetPath(s, baseAbs);

    THdfSession::DatasetCreateParams p;
    p.path = createPath;
    const hid_t elementType = mapTypeTextToNativeHid(typeText);
    if (elementType < 0) {
        QMessageBox::warning(this, tr("Invalid type"),
                             tr("Cannot create dataset: unsupported element type: %1").arg(typeText));
        return false;
    }
    p.elementType = elementType;
    p.initialDims = initDims;
    p.maxDims     = maxDims;
    p.chunkDims   = chunkDims;
    p.enableDeflate = templ.deflateOn;
    p.deflateLevel  = templ.deflateLevel;
    p.takeOwnershipOfElementType = false;

    if (!s->createDataset(p)) {
        QMessageBox::warning(this, tr("Create failed"),
                             tr("Failed to create dataset:\n%1").arg(createPath));
        return false;
    }

    // Refresh + select created dataset so the user can see it
    m_browser->refresh();
    m_browser->selectPath(createPath);
    m_targetEdit->setText(createPath);
    wiz->setTargetDatasetPath(createPath);

    QMessageBox::information(this, tr("Dataset created"),
                             tr("A compatible dataset was created and selected:\n\n%1\n\n"
                                "Please review it, then click Next again.")
                                 .arg(createPath));
    return false;
}


// ==================== Page 3 ====================

ExportExecutePage::ExportExecutePage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("Export results"));
    setFinalPage(true);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);

    m_log = new QPlainTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setMinimumHeight(200);
    m_log->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(m_summaryLabel);
    lay->addWidget(m_log);
}

void ExportExecutePage::initializePage()
{
    if (m_ran)
        return;
    m_ran = true;

    m_log->clear();

    auto* wiz = qobject_cast<TExportHDFDataWizard*>(wizard());
    if (!wiz) {
        m_log->appendPlainText(tr("Internal error: wizard not available."));
        return;
    }

    THdfSession* session = wiz->hdfSession();
    if (!session || !session->isOpen()) {
        m_log->appendPlainText(tr("No open HDF session."));
        return;
    }

    const QString dataset = wiz->targetDatasetPath();
    if (dataset.isEmpty()) {
        m_log->appendPlainText(tr("No dataset selected."));
        return;
    }

    const quint64 byteStart = field("exportStart").toULongLong();
    const quint64 byteLen   = field("exportLen").toULongLong();
    const quint64 cols      = field("exportCols").toULongLong(); // elements/row, 0 => 1D

    const QString typeText  = field("exportTypeText").toString().trimmed();
    const quint64 elementBytes = field("exportElementBytes").toULongLong();

    const quint64 sliceElements = (elementBytes > 0) ? (byteLen / elementBytes) : 0;
    const int rank = (cols == 0) ? 1 : 2;
    const quint64 rows = (rank == 2 && cols > 0) ? (sliceElements / cols) : 0;

    updateSummaryLabel();

    m_log->appendPlainText(tr("Starting export...\n"));
    m_log->appendPlainText(tr("Type: %1 (%2 bytes/element)").arg(typeText).arg(elementBytes));
    m_log->appendPlainText(tr("Slice: %1 bytes starting at %2 (%3 elements)")
                               .arg(QString::number(byteLen))
                               .arg(QString::number(byteStart))
                               .arg(QString::number(sliceElements)));

    if (rank == 2)
        m_log->appendPlainText(tr("Shape: (%1 x %2)").arg(QString::number(rows), QString::number(cols)));
    else
        m_log->appendPlainText(tr("Shape: (%1)").arg(QString::number(sliceElements)));

    QString sessionLog;

    const bool ok = session->appendRawSlice(
        dataset,
        QByteArrayView(wiz->data()),
        byteStart,
        byteLen,
        cols,
        typeText,
        &sessionLog
        );


    if (!sessionLog.isEmpty())
        m_log->appendPlainText(sessionLog.trimmed());

    m_log->appendPlainText(ok ? tr("\nExport completed successfully.")
                              : tr("\nExport failed."));
}


void ExportExecutePage::updateSummaryLabel()
{
    auto* wiz = qobject_cast<TExportHDFDataWizard*>(wizard());
    if (!wiz) {
        m_summaryLabel->setText(tr("-"));
        return;
    }

    const QString filePath = field("exportFilePath").toString();
    const QString dataset  = wiz->targetDatasetPath();

    const quint64 byteStart = field("exportStart").toULongLong();
    const quint64 byteLen   = field("exportLen").toULongLong();
    const quint64 cols      = field("exportCols").toULongLong(); // elements/row, 0 => 1D

    const QString typeText  = field("exportTypeText").toString().trimmed();
    const quint64 elementBytes = field("exportElementBytes").toULongLong();

    const quint64 sliceElements = (elementBytes > 0) ? (byteLen / elementBytes) : 0;
    const int rank = (cols > 0) ? 2 : 1;
    const quint64 rows = (rank == 2 && cols > 0) ? (sliceElements / cols) : 0;

    const QString shape =
        (rank == 1)
            ? QString("(%1)").arg(QString::number(sliceElements))
            : QString("(%1 x %2)").arg(QString::number(rows), QString::number(cols));

    const QString appended =
        (rank == 1)
            ? tr("Appended elements: %1").arg(QString::number(sliceElements))
            : tr("Appended rows: %1").arg(QString::number(rows));

    const QString lastByteIdx =
        (byteLen > 0) ? QString::number(byteStart + byteLen - 1) : tr("-");

    m_summaryLabel->setText(
        tr("<b>File:</b> %1<br>"
           "<b>Dataset:</b> %2<br>"
           "<b>Type:</b> %3 (%4 bytes/element)<br>"
           "<b>Selected bytes:</b> %5 ... %6 (%7 bytes)<br>"
           "<b>Slice elements:</b> %8<br>"
           "<b>Shape:</b> rank-%9 %10<br>"
           "<b>%11</b>")
            .arg(filePath)
            .arg(dataset)
            .arg(typeText.isEmpty() ? tr("-") : typeText)
            .arg(QString::number(elementBytes))
            .arg(QString::number(byteStart))
            .arg(lastByteIdx)
            .arg(QString::number(byteLen))
            .arg(QString::number(sliceElements))
            .arg(QString::number(rank))
            .arg(shape)
            .arg(appended)
        );
}




// ==================== Wizard ====================

TExportHDFDataWizard::TExportHDFDataWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Export data"));
    setWizardStyle(QWizard::ModernStyle);

    m_session = QSharedPointer<THdfSession>::create();

    m_page1 = new ExportDataPage(this);
    m_page2 = new TargetDatasetPage(this);
    m_page3 = new ExportExecutePage(this);

    addPage(m_page1);
    addPage(m_page2);
    addPage(m_page3);


}

void TExportHDFDataWizard::setData(const QByteArray &data)
{
    m_data = data;
    if (m_page1)
        m_page1->setTotalBytes(m_data.size());
}

bool TExportHDFDataWizard::openOrCreateSession(const QString &path)
{
    if (!m_session)
        m_session = QSharedPointer<THdfSession>::create();

    if (m_session->isOpen())
        m_session->close();

    const QFileInfo fi(path);
    if (!fi.exists()) {
        return m_session->createNew(path);
    }
    return m_session->openExisting(path);
}
