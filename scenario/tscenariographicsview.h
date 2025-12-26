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
