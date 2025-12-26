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
// Adam Švehla (initial author)

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
