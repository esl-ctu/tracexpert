#include "tscenario.h"
#include "tscenariomodel.h"
#include "scenario/tscenariocontainer.h"

TScenarioModel::TScenarioModel(TScenarioContainer * parent) :
    QObject(parent),
    TProjectItem(parent->model(), parent),
    m_scenario(nullptr)
{
    m_typeName = "scenario";
}

TScenarioModel::TScenarioModel(TScenario * scenario, TScenarioContainer * parent)
    : QObject(parent), TProjectItem(parent->model(), parent), m_scenario(scenario)
{
    m_typeName = "scenario";
}

TScenarioModel::~TScenarioModel() {
    delete m_scenario;
}

TScenario * TScenarioModel::scenario() const {
    return m_scenario;
}

void TScenarioModel::setScenario(TScenario * scenario) {
    delete m_scenario;
    m_scenario = scenario;
}

int TScenarioModel::childrenCount() const
{
    return 0;
}

TProjectItem * TScenarioModel::child(int row) const
{
    return nullptr;
}

QString TScenarioModel::name() const
{
    return m_scenario->getName();
}

TProjectItem::Status TScenarioModel::status() const
{
    return Status::None;
}


bool TScenarioModel::toBeSaved() const {
    return true;
}

QDomElement TScenarioModel::save(QDomDocument & document) const {
    QDomElement element = TProjectItem::save(document);

    element.setAttribute("name", m_scenario->getName());
    element.setAttribute("description", m_scenario->getDescription());

    QByteArray array;
    QDataStream stream(&array, QIODeviceBase::WriteOnly);
    stream << *m_scenario;
    element.setAttribute("data", array.toBase64());

    return element;
}

void TScenarioModel::load(QDomElement * element) {
    if (!element)
        return;

    if (element->tagName() != typeName())
        throw tr("Unexpected tag");

    QString name = element->attribute("name");

    if(name.isEmpty())
        throw tr("Protocol name is empty");

    QString dataArray = element->attribute("data");
    QDataStream stream(QByteArray::fromBase64(dataArray.toUtf8()));

    delete m_scenario;

    m_scenario = new TScenario();
    stream >> *m_scenario;
}
