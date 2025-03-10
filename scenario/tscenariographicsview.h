#ifndef TSCENARIOGRAPHICSVIEW_H
#define TSCENARIOGRAPHICSVIEW_H

#include <QGraphicsView>

/*!
 * \brief The TScenarioGraphicsView class represents a graphics view for a scenario.
 *
 * The class represents a graphics view for a scenario.
 * It is needed for implementing zoom by mouse wheel in the scenario scene.
 *
 */
class TScenarioGraphicsView : public QGraphicsView {

    Q_OBJECT

public:
    TScenarioGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

signals:
    void scaleChangedUsingMouseWheel(qreal scale);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    const qreal MIN_SCALE = 0.2f;
    const qreal MAX_SCALE = 2.0f;
    const qreal SCALE_UP = 0.9f;
    const qreal SCALE_DOWN = 1.1f;

};

#endif // TSCENARIOGRAPHICSVIEW_H
