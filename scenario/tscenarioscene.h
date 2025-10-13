#ifndef TSCENARIOSCENE_H
#define TSCENARIOSCENE_H

// #include "scenario/tscenarioblock.h"

#include <QMenu>
#include <QGraphicsScene>

#include "tscenario.h"

class TScenarioGraphicalItem;
class TScenarioGraphicalItemPort;

/*!
 * \brief The TScenarioScene class represents a scene for editing a scenario.
 *
 * The class represents a scene for editing a scenario.
 *
 * It is a container for graphical items representing scenario blocks and connections between them.
 * It includes methods for adding, removing and moving items and connections.
 *
 * The scenario "data" is loaded into the scene using the loadScenario method and
 * can the be manipulated through the GUI and saved using the scenario or copyScenario method.
 *
 */
class TScenarioScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum TScenarioPointerTool { MousePointer, MouseDrag, InsertLine, InsertItem };

    explicit TScenarioScene(TProjectModel * projectModel, QObject *parent = nullptr);
    ~TScenarioScene();

    void loadScenario(const TScenario * scenario);

    // returns copy of TScenario in scene, callers responsibility to delete!
    TScenario * copyScenario();

    // returns actual TScenario in scene, callers responsibility to not mess it up
    TScenario * scenario();

    void removeItem(QGraphicsItem * item);
    void removeSelectedItems();

    void sendSelectedToBack();
    void bringSelectedToFront();

    TScenarioScene::TScenarioPointerTool pointerTool();

public slots:
    void setPointerTool(TScenarioPointerTool mode);
    void setInsertItemMode(TScenarioGraphicalItem * insertedBlockInstance);

signals:
    void itemInserted(TScenarioGraphicalItem * item);
    void itemInsertCanceled();
    void pointerToolChanged(TScenarioPointerTool tool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    const QColor FLOW_LINE_COLOR = QColor::fromString("#99cfe0");
    const QColor DATA_LINE_COLOR = QColor::fromString("#da9a85");
    const QColor CONN_LINE_COLOR = QColor::fromString("#9a85da");

    TScenarioPointerTool m_pointerTool;

    TScenario * m_scenario;
    TProjectModel * m_projectModel;

    void cancelInsertMode();
    TScenarioGraphicalItem * m_insertedBlockInstance = nullptr;

    // methods for line (aka possible connection) validity evaluation
    bool isLineValidArrow() const;
    QPen evaluateLinePen() const;

    TScenarioGraphicalItemPort * findLineStartPort(const QPointF & startPoint);
    TScenarioGraphicalItemPort * findLineEndPort(const QPointF & endPoint);

    QGraphicsLineItem * m_tmpLine;
    TScenarioGraphicalItemPort * m_tmpLineStartPort;
    TScenarioGraphicalItemPort * m_tmpLineEndPort;
};

#endif // TSCENARIOSCENE_H
