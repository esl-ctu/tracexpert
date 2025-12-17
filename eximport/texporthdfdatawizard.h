#ifndef TEXPORTHDFDATAWIZARD_H
#define TEXPORTHDFDATAWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QByteArray>
#include <QSharedPointer>
#include <QPlainTextEdit>

class QLabel;
class QRadioButton;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QComboBox;

class THdfSession;
class THdfBrowserWidget;

// -------------------- Page 1 --------------------

class ExportDataPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ExportDataPage(QWidget *parent = nullptr);

    void setTotalBytes(int totalBytes);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;

private:
    void connectSignals();
    void recalcAndValidateLive();
    void browseForFile();

private:
    // File selection
    QLineEdit   *m_filePathEdit = nullptr;
    QPushButton *m_browseBtn = nullptr;

    // Data info
    int   m_totalBytes = 0;
    bool  m_complete = false;
    QLabel *m_totalBytesLabel = nullptr;

    // Range selection
    QRadioButton *m_radioAll = nullptr;
    QRadioButton *m_radioRange = nullptr;
    QSpinBox *m_start = nullptr;
    QSpinBox *m_len = nullptr;

    QLabel *m_firstIdxValue = nullptr;
    QLabel *m_lastIdxValue = nullptr;
    QLabel *m_sizeValue = nullptr;

    // Shaping
    QSpinBox *m_cols = nullptr;

    // Derived/working labels
    QLabel *m_rowsLabel = nullptr;
    QLabel *m_rankLabel = nullptr;
    QLabel *m_shapeLabel = nullptr;

    // Datatype selection
    QComboBox *m_typeCombo = nullptr;
    QLabel *m_elementBytesValue = nullptr;
    int m_elementBytes = 1;

    QLabel *m_sizeBytesValue = nullptr;
    QLabel *m_alignWarnValue = nullptr;

};


// -------------------- Page 2 --------------------

class TargetDatasetPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit TargetDatasetPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;


private:
    void rebuildTemplateFromPage1();

private:
    THdfBrowserWidget *m_browser = nullptr;
    QLabel *m_fileNameLabel = nullptr;
    QLabel *m_filePathLabel = nullptr;
    QLineEdit *m_targetEdit = nullptr;

};


// -------------------- Page 3 --------------------

class ExportExecutePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ExportExecutePage(QWidget* parent = nullptr);

    void initializePage() override;

private:
    void updateSummaryLabel();

private:
    QLabel* m_summaryLabel = nullptr;
    QPlainTextEdit* m_log = nullptr;
    bool m_ran = false;
};

// -------------------- Wizard --------------------

class TExportHDFDataWizard : public QWizard
{
    Q_OBJECT
public:
    explicit TExportHDFDataWizard(QWidget *parent = nullptr);
    ~TExportHDFDataWizard() override = default;

    THdfSession *hdfSession() const { return m_session.data(); }

    void setData(const QByteArray &data);
    const QByteArray& data() const { return m_data; }

    bool openOrCreateSession(const QString &path);

    void setTargetDatasetPath(const QString &path) { m_targetDatasetPath = path; }
    QString targetDatasetPath() const { return m_targetDatasetPath; }


private:
    ExportDataPage *m_page1 = nullptr;
    TargetDatasetPage *m_page2 = nullptr;
    ExportExecutePage* m_page3 = nullptr;

    QSharedPointer<THdfSession> m_session;
    QByteArray m_data;
    QString m_targetDatasetPath;

};

#endif // TEXPORTHDFDATAWIZARD_H
