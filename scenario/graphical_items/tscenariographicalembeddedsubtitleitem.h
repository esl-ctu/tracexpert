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

#ifndef TSCENARIOGRAPHICALEMBEDDEDSUBTITLEITEM_H
#define TSCENARIOGRAPHICALEMBEDDEDSUBTITLEITEM_H

#include <qpainter.h>
#include "../tscenariographicalitem.h"

/*!
 *
 */
class TScenarioGraphicalEmbeddedSubtitleItem : public TScenarioGraphicalItem
{

public:
    TScenarioGraphicalEmbeddedSubtitleItem(
        TScenarioItem * scenarioItem,
        QGraphicsItem * parent = nullptr
        ) : TScenarioGraphicalItem(scenarioItem, parent) { }

protected:
    void updateBlockAppearance() override {

        int textWidth = 0;
        if(m_titleText) {
            QString elidedTitleText = QFontMetrics(m_titleText->font()).elidedText(m_scenarioItem->getTitle(), Qt::ElideRight, 250);
            m_titleText->setText(elidedTitleText);
            textWidth = m_titleText->boundingRect().width();
        }

        if(m_subtitleText) {
            int subtitleWidth = fmax(130, textWidth);
            QString elidedSubtitleText = QFontMetrics(m_subtitleText->font()).elidedText(m_scenarioItem->getSubtitle(), Qt::ElideRight, subtitleWidth);
            m_subtitleText->setText(elidedSubtitleText);
        }

        int width = fmax(150, textWidth + 20);
        int topOffset = 0;

        QPainterPath path;
        if(m_titleText && m_subtitleText) {
            if(!m_scenarioItem->getTitle().isEmpty()) {
                m_titleText->setPos(10, 5 + topOffset);
                path.moveTo(0,      24 + topOffset);
                path.lineTo(width,  24 + topOffset);
                path.moveTo(0,      26 + topOffset);
                path.lineTo(width,  26 + topOffset);
                topOffset += 26;
            }

            if(!m_scenarioItem->getSubtitle().isEmpty()) {
                m_subtitleText->setPos(10, 5 + topOffset);
                //path.moveTo(0,      24 + topOffset);
                //path.lineTo(width,  24 + topOffset);
                //topOffset += 24;
            }
        }

        topOffset += 20;

        int leftConnectorCount = 0;
        int rightConnectorCount = 0;
        for(TScenarioGraphicalItemPort * itemPort : m_graphicalItemPorts) {
            if(itemPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort) {
                itemPort->setPos(QPointF(0, topOffset + VERTICAL_PORT_OFFSET*leftConnectorCount));
                leftConnectorCount++;
            }
            else {
                itemPort->setPos(QPointF(width, topOffset + VERTICAL_PORT_OFFSET*rightConnectorCount));
                rightConnectorCount++;
            }
        }

        topOffset += 20;

        // redraw the polygon itself
        int height = fmax(100, topOffset + VERTICAL_PORT_OFFSET*fmax(leftConnectorCount, rightConnectorCount));

        if(m_editableIcon) {
            m_editableIcon->setPos(width - 20, height - 20);
        }

        path.addRoundedRect(0, 0, width, height, 10, 10);
        setPath(path);
    }
};

#endif // TSCENARIOGRAPHICALEMBEDDEDSUBTITLEITEM_H
