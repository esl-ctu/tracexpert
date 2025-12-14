#include "tprojectunit.h"
#include "../scenario/tscenario.h"
#include "../protocol/tprotocol.h"

TProjectUnit * TProjectUnit::instantiate(const QString & typeName) {
    if(typeName == "scenario") return new TScenario();
    if(typeName == "protocol") return new TProtocol();

    qCritical() << "Failed to instatiate TProjectUnit: unknown type: " << typeName;
    return nullptr;
}

TProjectUnit * TProjectUnit::deserialize(const QString & typeName, QDataStream &in) {
    TProjectUnit * unit = instantiate(typeName);

    if (!unit) {
        qWarning() << "Cannot deserialize unknown TProjectUnit type: " << typeName;
        return nullptr;
    }

    unit->deserialize(in);
    return unit;
}
