#include "tscenariographicsview.h"

#include "qevent.h"

TScenarioGraphicsView::TScenarioGraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent) { }

void TScenarioGraphicsView::wheelEvent(QWheelEvent *event)
{
    const ViewportAnchor anchor = transformationAnchor();
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    int angle = event->angleDelta().y();

    qreal curScaleFactor = transform().m11();
    qreal scaleFactor = 1.0f;

    if (angle > 0 && curScaleFactor < MAX_SCALE) {
        scaleFactor = SCALE_DOWN;
    } else if (curScaleFactor > MIN_SCALE) {
        scaleFactor = SCALE_UP;
    }

    setTransformationAnchor(anchor);
    scale(scaleFactor, scaleFactor);

    emit scaleChangedUsingMouseWheel(scaleFactor);
}
