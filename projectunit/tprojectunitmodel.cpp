#include "tprojectunitmodel.h"
#include "tprojectunit.h"


TProjectUnitModel::TProjectUnitModel(const QString & typeName, TProjectUnitContainer * parent) :
    TProjectItem(parent->model(), parent),
    m_unit(nullptr)
{
    m_typeName = typeName;
}

TProjectUnitModel::TProjectUnitModel(const QString & typeName, TProjectUnit * unit, TProjectUnitContainer * parent)
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
