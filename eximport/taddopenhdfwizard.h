#ifndef TADDOPENHDFWIZARD_H
#define TADDOPENHDFWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLineEdit>
#include <QRadioButton>
#include <QSharedPointer>
#include <QLabel>

class QCheckBox;

class THdfSession;
class THdfBrowserWidget;

// -------------------- Page 1 --------------------

class AddOpenFilePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit AddOpenFilePage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;

private:
    QRadioButton *m_radioCreateNew  = nullptr;
    QRadioButton *m_radioOpenExisting = nullptr;
    QLineEdit *m_fileEdit = nullptr;
};

// -------------------- Page 2 --------------------

class HdfBrowsePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit HdfBrowsePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override { return -1; } // final page

private:
    THdfBrowserWidget *m_browser = nullptr;
    QLabel *m_fileNameLabel = nullptr;
    QLabel *m_filePathLabel = nullptr;
};

// -------------------- Wizard --------------------

class TAddOpenHdfWizard : public QWizard
{
    Q_OBJECT
public:
    explicit TAddOpenHdfWizard(QWidget *parent = nullptr);
    ~TAddOpenHdfWizard() override;

    QSharedPointer<THdfSession> hdfSession() const { return m_hdfSession; }

    QString filePath() const;

private:
    QSharedPointer<THdfSession> m_hdfSession;
};

#endif // TADDOPENHDFWIZARD_H
