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
// Vojtěch Miškovský

#include "tprojectunitmodel.h"
#include "tprojectunit.h"
#include "../scenario/tscenario.h"
#include "../protocol/tprotocol.h"


TProjectUnitModel::TProjectUnitModel(const QString & typeName, TProjectUnitContainer * parent, TProjectUnit * unit)
    : TProjectItem(parent->model(), parent), m_unit(unit)
{
    m_typeName = typeName;
}

TProjectUnitModel::~TProjectUnitModel() {
    delete m_unit;
}

void TProjectUnitModel::openEditor() {
    emit editorRequested(this);
}

TProjectUnit * TProjectUnitModel::unit() const {
    return m_unit;
}

void TProjectUnitModel::setUnit(TProjectUnit * unit) {
    delete m_unit;
    m_unit = unit;
}

int TProjectUnitModel::childrenCount() const {
    return 0;
}

TProjectItem * TProjectUnitModel::child(int row) const {
    return nullptr;
}

QString TProjectUnitModel::name() const {
    return m_unit->name();
}

TProjectItem::Status TProjectUnitModel::status() const {
    return Status::None;
}

bool TProjectUnitModel::toBeSaved() const {
    return true;
}

QDomElement TProjectUnitModel::save(QDomDocument & document) const {
    QDomElement element = TProjectItem::save(document);

    element.setAttribute("name", m_unit->name());
    element.setAttribute("description", m_unit->description());

    QByteArray array;
    QDataStream stream(&array, QIODeviceBase::WriteOnly);
    m_unit->serialize(stream);
    element.setAttribute("data", array.toBase64());

    return element;
}

void TProjectUnitModel::load(QDomElement * element) {
    if (!element)
        return;

    if (element->tagName() != typeName())
        throw tr("Unexpected tag");

    QString name = element->attribute("name");

    if(name.isEmpty())
        throw tr("Project unit's name is empty");

    QString description = element->attribute("description");

    QString dataArray = element->attribute("data");
    QDataStream stream(QByteArray::fromBase64(dataArray.toUtf8()));

    TProjectUnit * unit = TProjectUnit::deserialize(typeName(), stream);

    if(!unit)
        throw tr("Failed to load project unit");

    setUnit(unit);

    // set name and description to reflect the respective attributes
    // rather than the data attribute
    m_unit->setName(name);
    m_unit->setDescription(description);
}

TProjectUnitModel * TProjectUnitModel::instantiate(const QString & typeName, TProjectUnitContainer * parent, TProjectUnit * unit)
{
    if(typeName == "scenario") {
        TScenarioContainer * scenarioContainer = dynamic_cast<TScenarioContainer *>(parent);
        if (!scenarioContainer)
            qWarning("Attempted to create scenario model in non-scenario container");

        TScenario * scenario = dynamic_cast<TScenario *>(unit);
        if (unit && !scenario)
            qWarning("Attempted to create scenario model from non-scenario unit");

        return new TScenarioModel(scenarioContainer, scenario);
    }
    if(typeName == "protocol") {
        TProtocolContainer * protocolContainer = dynamic_cast<TProtocolContainer *>(parent);
        if (!protocolContainer)
            qWarning("Attempted to create protocol model in non-protocol container");

        TProtocol * protocol = dynamic_cast<TProtocol *>(unit);
        if (unit && !protocol)
            qWarning("Attempted to create protocol model from non-protocol unit");

        return new TProtocolModel(protocolContainer, protocol);
    }

    qCritical() << "Failed to instatiate TProjectUnit: unknown type: " << typeName;
    return nullptr;
}
