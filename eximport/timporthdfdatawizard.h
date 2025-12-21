#ifndef TIMPORTHDFDATAWIZARD_H
#define TIMPORTHDFDATAWIZARD_H

#include <QWizard>
#include <QSharedPointer>
#include <QString>
#include <qplaintextedit.h>

#include "thdfsession.h"

class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class THdfBrowserWidget;
class THdfSession;
class QLineEdit;
class QPushButton;

// -------------------- Page 1 --------------------

class ImportOpenFilePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ImportOpenFilePage(QWidget *parent = nullptr);

    bool validatePage() override;

private:
    void browseForFile();

private:
    QLineEdit *m_filePathEdit = nullptr;
    QPushButton *m_browseBtn = nullptr;
};


// -------------------- Page 2 --------------------

class SelectDatasetPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SelectDatasetPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    bool validatePage() override;

private:
    void rebuildFromSelection();
    bool queryDatasetInfo(const QString &path,
                          QString *typeTextOut,
                          int *elemBytesOut,
                          int *rankOut,
                          QVector<hsize_t> *dimsOut,
                          QString *whyNotOut) const;

    void updateRangeUiForRank(int rank);
    void clampRangeAndUpdateLabels();

    quint64 totalElements() const;

private:
    QLabel *m_fileNameLabel = nullptr;
    QLabel *m_filePathLabel = nullptr;

    THdfBrowserWidget *m_browser = nullptr;
    QLineEdit *m_targetEdit = nullptr;

    // Dataset info UI
    QLabel *m_typeLabel = nullptr;
    QLabel *m_elemBytesLabel = nullptr;
    QLabel *m_rankLabel = nullptr;
    QLabel *m_dimsLabel = nullptr;
    QLabel *m_totalElemsLabel = nullptr;
    QLabel *m_totalBytesLabel = nullptr;
    QLabel *m_warnLabel = nullptr;

    // Range UI
    QRadioButton *m_radioAll = nullptr;
    QRadioButton *m_radioRange = nullptr;

    QStackedWidget *m_rangeStack = nullptr; // 0 = rank1, 1 = rank2

    // Rank-1 range controls (elements)
    QSpinBox *m_start1 = nullptr;
    QSpinBox *m_count1 = nullptr;

    // Rank-2 hyperslab controls (rows/cols)
    QSpinBox *m_rowStart = nullptr;
    QSpinBox *m_rowCount = nullptr;
    QSpinBox *m_colStart = nullptr;
    QSpinBox *m_colCount = nullptr;

    QLabel *m_firstLabel = nullptr;
    QLabel *m_lastLabel = nullptr;
    QLabel *m_selElemsLabel = nullptr;
    QLabel *m_selBytesLabel = nullptr;
    QLabel *m_selShapeLabel = nullptr;

    // Current dataset cached (for clamping)
    QString m_curPath;
    QString m_curTypeText;
    int m_curElemBytes = 0;
    int m_curRank = 0;
    QVector<hsize_t> m_curDims; // rank 1 or 2

    bool m_curCompatible = false;
    bool m_complete = false;
};


// -------------------- Page 3 --------------------

class ImportExecutePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ImportExecutePage(QWidget *parent = nullptr);

    void initializePage() override;

private:
    void updateSummary();

private:
    QLabel *m_summaryLabel = nullptr;
    QPlainTextEdit *m_log = nullptr;
    bool m_ran = false;
};

// -------------------- Wizard --------------------

class TImportHDFDataWizard : public QWizard
{
    Q_OBJECT
public:
    explicit TImportHDFDataWizard(QWidget *parent = nullptr);

    THdfSession *hdfSession() const;
    QString filePath() const { return m_filePath; }

    bool openExistingSession(const QString &path);

    QString sourceDatasetPath() const { return m_datasetPath; }
    void setSourceDatasetPath(const QString &p) { m_datasetPath = p; }

    struct ImportRequest
    {
        QString datasetPath;
        bool all = true;

        int rank = 0;

        // rank-1 selection (elements)
        quint64 startElem = 0;
        quint64 elemCount = 0;

        // rank-2 selection (hyperslab rectangle)
        quint64 rowStart = 0;
        quint64 rowCount = 0;
        quint64 colStart = 0;
        quint64 colCount = 0;

        // Metadata (for Page 3 summary + read API selection)
        QString typeText;
        int elementBytes = 0;
        QVector<hsize_t> dims;
    };

    const ImportRequest &importRequest() const { return m_req; }
    void setImportRequest(const ImportRequest &r) { m_req = r; }

    const QByteArray &importedData() const { return m_importedData; }
    QString importedTypeText() const { return m_importedTypeText; }
    int importedElementBytes() const { return m_importedElementBytes; }
    const QVector<quint64> &importedDims() const { return m_importedDims; }
    bool importSucceeded() const { return m_importOk; }
    QString importLog() const { return m_importLog; }

    void clearImportResult()
    {
        m_importedData.clear();
        m_importedTypeText.clear();
        m_importedElementBytes = 0;
        m_importedDims.clear();
        m_importOk = false;
        m_importLog.clear();
    }

    void setImportResult(bool ok,
                         const QByteArray &data,
                         const QString &typeText,
                         int elementBytes,
                         const QVector<quint64> &dims,
                         const QString &log)
    {
        m_importOk = ok;
        m_importedData = data;
        m_importedTypeText = typeText;
        m_importedElementBytes = elementBytes;
        m_importedDims = dims;
        m_importLog = log;
    }

private:
    QSharedPointer<THdfSession> m_session;

    ImportOpenFilePage *m_page1 = nullptr;
    SelectDatasetPage *m_page2 = nullptr;
    ImportExecutePage *m_page3 = nullptr;

    ImportRequest m_req;
    QString m_filePath;    
    QString m_datasetPath;    
    QByteArray m_importedData;
    QString m_importedTypeText;    
    QVector<quint64> m_importedDims;    
    QString m_importLog;
    bool m_importOk = false;
    int m_importedElementBytes = 0;

};

#endif // TIMPORTHDFDATAWIZARD_H
