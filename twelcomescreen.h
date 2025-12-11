#ifndef TWELCOMESCREEN_H
#define TWELCOMESCREEN_H

#include <QWidget>
#include <QFrame>
#include <QLabel>

#include <QToolButton>

class TCenteredToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit TCenteredToolButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};


class TWelcomeScreen : public QWidget
{
    Q_OBJECT
public:
    explicit TWelcomeScreen(QWidget *parent = nullptr);

    void setActions(QAction *newProject,
                    QAction *openProject,
                    QAction *saveProject,
                    QAction *closeProject,
                    QAction *openDevice,
                    QAction *protocolManager,
                    QAction *scenarioManager);

private:
    QFrame      *m_card;

    QLabel      *m_logoLabel;
    QLabel      *m_titleLabel;
    QLabel      *m_sloganLabel;

    QToolButton *m_btnNewProject;
    QToolButton *m_btnOpenProject;
    QToolButton *m_btnSaveProject;
    QToolButton *m_btnCloseProject;

    TCenteredToolButton *m_btnAddDevice;
    TCenteredToolButton *m_btnProtocolManager;
    TCenteredToolButton *m_btnScenarioManager;
};

#endif // TWELCOMESCREEN_H
