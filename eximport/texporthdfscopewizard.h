#ifndef TEXPORTHDFSCOPEWIZARD_H
#define TEXPORTHDFSCOPEWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QSharedPointer>
#include <QPlainTextEdit>
#include <QLabel>
#include <cstddef>

#include <tscope.h>

class QVBoxLayout;
class QListWidget;
class QStackedWidget;
class QTabWidget;

class THdfSession;
class THdfBrowserWidget;

// -------------------- Page 1 --------------------

class ExportFilePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ExportFilePage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;

private:
    QLineEdit *m_fileEdit = nullptr;
    QRadioButton *m_radioCurrentTrace = nullptr;
    QRadioButton *m_radioAllTraces    = nullptr;
    QVBoxLayout *m_channelsLayout = nullptr;
    QList<QCheckBox*> m_channelChecks;

};


// -------------------- Page 2 --------------------

class ChannelTargetsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ChannelTargetsPage(QWidget *parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;
    bool validatePage() override;

private:

    struct ChannelUi {
        QString alias;

        QWidget *root = nullptr;
        QTabWidget *tabs = nullptr;

        // Traces
        THdfBrowserWidget *tracesBrowser = nullptr;
        QLineEdit *tracesTarget = nullptr;

        // Metadata
        THdfBrowserWidget *metaBrowser = nullptr;
        QLineEdit *metaTarget = nullptr;
    };

    struct TFail {
        int channelIndex = -1;
        int tabIndex = 0; // 0 = Traces tab, 1 = Metadata group tab
        QString title;
        QString text;
    };

    void resetUi();
    bool isPathSyntaxValid(const QString &path, QString *why = nullptr) const;

    void refreshAllBrowsersAndTargets();
    bool validateOnce(TFail &fail);
    bool ensureDefaultExportStructure(QStringList &created, QStringList &repaired, QStringList &conflicts);

    QListWidget *m_channelList = nullptr;
    QStackedWidget *m_stack = nullptr;
    QVector<ChannelUi> m_channelsUi;
    QLabel *m_fileNameLabel = nullptr;
    QLabel *m_filePathLabel = nullptr;
    bool m_autoCreateOfferedThisVisit = false;

};

// -------------------- Page 3 --------------------

class ExportResultPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ExportResultPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override { return -1; } // no Next; this is the final page

private:
    QLabel *m_label = nullptr;
    QPlainTextEdit *m_text = nullptr;

};

// -------------------- Wizard --------------------

class TExportHDFScopeWizard : public QWizard
{
    Q_OBJECT
public:
    enum { Result_Paused = QDialog::Accepted + 1 };

    struct TChannelTarget {
        QString alias;
        QString tracesDataset;
        QString metadataGroup;
    };

    explicit TExportHDFScopeWizard(QWidget *parent = nullptr);
    ~TExportHDFScopeWizard() override;

    QSharedPointer<THdfSession> hdfSession() const { return m_hdfSession; }

    void setTraceFormat(std::size_t samplesPerTrace, TScope::TSampleType sampleType);
    std::size_t traceSamplesPerTrace() const { return m_traceSamplesPerTrace; }
    TScope::TSampleType traceSampleType() const { return m_traceSampleType; }

    void setTraceInfo(std::size_t total);
    std::size_t totalTraceCount()    const { return m_totalTraceCount; }

    void setChannels(const QStringList &channels);
    const QStringList &channels()    const { return m_channels; }

    void setInitialFileName(const QString &name);
    QString initialFileName()        const { return m_initialFileName; }

    QString resultLogText() const { return m_resultLogText; }
    QString outputFile() const;

    const QVector<TChannelTarget>& channelTargets() const { return m_channelTargets; }

    void setChannelTargets(const QVector<TChannelTarget> &targets) { m_channelTargets = targets; }
    int resultPageId() const { return m_resultPageId; }
    void requestPause();
    void showResultsPage(const QString & log);


protected:
    void done(int result) override;

private:
    QSharedPointer<THdfSession> m_hdfSession;

    std::size_t m_totalTraceCount = 0;
    QStringList m_channels;
    QString m_initialFileName;
    QVector<TChannelTarget> m_channelTargets;
    std::size_t m_traceSamplesPerTrace = 0;
    TScope::TSampleType m_traceSampleType = TScope::TSampleType::TReal32;
    QString m_resultLogText;
    int m_resultPageId = -1;
    bool m_pauseRequested = false;

};

#endif // TEXPORTHDFSCOPEWIZARD_H
