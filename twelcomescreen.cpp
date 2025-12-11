#include "twelcomescreen.h"

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QSizePolicy>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include "buildinfo.h"

TCenteredToolButton::TCenteredToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

QSize TCenteredToolButton::sizeHint() const
{

    QSize base = QToolButton::sizeHint();
    int side = qMax(base.width(), base.height());
    side = qMax(side, 72);
    return QSize(side, side);
}

void TCenteredToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);


    QStyleOptionToolButton panelOpt = opt;
    panelOpt.icon = QIcon();
    panelOpt.text.clear();
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, panelOpt);


    QRect contentRect =
        style()->subControlRect(QStyle::CC_ToolButton, &opt,
                                QStyle::SC_ToolButton, this);


    QIcon::Mode mode;
    if (!isEnabled())
        mode = QIcon::Disabled;
    else if (opt.state & QStyle::State_MouseOver)
        mode = QIcon::Active;
    else
        mode = QIcon::Normal;

    QIcon::State state = (opt.state & QStyle::State_On) ? QIcon::On : QIcon::Off;

    QPixmap pm;
    if (!opt.icon.isNull())
        pm = opt.icon.pixmap(opt.iconSize, mode, state);


    const QString text = opt.text;
    QFontMetrics fm(font());

    int iconHeight = pm.isNull() ? 0 : pm.height();
    int textHeight = text.isEmpty() ? 0 : fm.height();

    int spacing = (pm.isNull() || text.isEmpty()) ? 0 : 10;

    int totalHeight = iconHeight + spacing + textHeight;

    int y = contentRect.center().y() - totalHeight / 2;
    int cx = contentRect.center().x();

    if (!pm.isNull()) {
        QPoint iconPos(cx - pm.width() / 2, y);
        painter.drawPixmap(iconPos, pm);
        y += iconHeight + spacing;
    }

    if (!text.isEmpty()) {
        QRect textRect(contentRect.left(), y,
                       contentRect.width(), textHeight);
        painter.drawItemText(textRect,
                             Qt::AlignHCenter | Qt::AlignTop,
                             palette(),
                             isEnabled(),
                             text,
                             QPalette::ButtonText);
    }

    if (opt.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect frOpt;
        frOpt.QStyleOption::operator=(opt);
        frOpt.rect = contentRect.adjusted(2, 2, -2, -2);
        frOpt.state |= QStyle::State_KeyboardFocusChange;
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &frOpt, &painter, this);
    }
}

TWelcomeScreen::TWelcomeScreen(QWidget *parent)
    : QWidget(parent),
    m_card(new QFrame(this)),
    m_logoLabel(new QLabel(m_card)),
    m_titleLabel(new QLabel(m_card)),
    m_sloganLabel(new QLabel(m_card)),
    m_btnNewProject(new QToolButton(m_card)),
    m_btnOpenProject(new QToolButton(m_card)),
    m_btnSaveProject(new QToolButton(m_card)),
    m_btnCloseProject(new QToolButton(m_card)),
    m_btnAddDevice(new TCenteredToolButton(m_card)),
    m_btnProtocolManager(new TCenteredToolButton(m_card)),
    m_btnScenarioManager(new TCenteredToolButton(m_card))
{

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addStretch();

    auto *centerRow = new QHBoxLayout();
    centerRow->addStretch();
    centerRow->addWidget(m_card);
    centerRow->addStretch();
    rootLayout->addLayout(centerRow);

    rootLayout->addStretch();

    m_card->setFrameShape(QFrame::StyledPanel);
    m_card->setFrameShadow(QFrame::Raised);
    m_card->setObjectName("welcomeCard");
    m_card->setMinimumSize(400, 250);
    m_card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setMinimumSize(150, 100);

    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    QFont sloganFont;
    sloganFont.setPointSize(11);
    sloganFont.setItalic(true);
    m_sloganLabel->setFont(sloganFont);

    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_sloganLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    const QSize buttonSize(150, 32);
    for (QToolButton *btn : {
                             m_btnNewProject,
                             m_btnOpenProject,
                             m_btnSaveProject,
                             m_btnCloseProject
         })
    {
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setMinimumSize(buttonSize);

    }

    for (QToolButton *btn : {
                             static_cast<QToolButton*>(m_btnAddDevice),
                             static_cast<QToolButton*>(m_btnProtocolManager),
                             static_cast<QToolButton*>(m_btnScenarioManager) })
    {
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setFixedSize(128, 128);
        btn->setIconSize(QSize(64, 64));
    }


    auto *cardLayout = new QVBoxLayout(m_card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(16);


    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(16);


    auto *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(8);


    rightColumn->addWidget(m_titleLabel);
    rightColumn->addWidget(m_sloganLabel);


    auto *projectColumn = new QVBoxLayout();
    projectColumn->setSpacing(6);
    projectColumn->addWidget(m_btnNewProject);
    projectColumn->addWidget(m_btnOpenProject);
    projectColumn->addWidget(m_btnSaveProject);
    projectColumn->addWidget(m_btnCloseProject);

    rightColumn->addLayout(projectColumn);


    topRow->addWidget(m_logoLabel, 0, Qt::AlignTop);
    topRow->addLayout(rightColumn);

    cardLayout->addLayout(topRow);


    auto *toolsRow = new QHBoxLayout();
    toolsRow->setSpacing(8);
    toolsRow->addWidget(m_btnAddDevice);
    toolsRow->addWidget(m_btnProtocolManager);
    toolsRow->addWidget(m_btnScenarioManager);

    cardLayout->addSpacing(16);
    cardLayout->addLayout(toolsRow);

    m_logoLabel->setPixmap(QPixmap(":/icons/tracexpert256.png"));
    m_titleLabel->setText(QString("TraceXpert %1").arg(TRACEXPERT_VERSION));
    m_sloganLabel->setText("An integrated environment for side-channel data acquisition and analysis");

}

void TWelcomeScreen::setActions(QAction *newProject,
                               QAction *openProject,
                               QAction *saveProject,
                               QAction *closeProject,
                               QAction *openDevice,
                               QAction *protocolManager,
                               QAction *scenarioManager)
{
    if (newProject)
        m_btnNewProject->setDefaultAction(newProject);
    if (openProject)
        m_btnOpenProject->setDefaultAction(openProject);
    if (saveProject)
        m_btnSaveProject->setDefaultAction(saveProject);
    if (closeProject)
        m_btnCloseProject->setDefaultAction(closeProject);

    if (openDevice)
        m_btnAddDevice->setDefaultAction(openDevice);
    if (protocolManager)
        m_btnProtocolManager->setDefaultAction(protocolManager);
    if (scenarioManager)
        m_btnScenarioManager->setDefaultAction(scenarioManager);

}
