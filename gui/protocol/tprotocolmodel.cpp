#include "tprotocolmodel.h"
#include "tprotocolcontainer.h"

TProtocolModel::TProtocolModel(TProtocolContainer * parent) : QObject(parent), TProjectItem(parent->model(), parent) {
    m_typeName = "protocol";
}

TProtocolModel::TProtocolModel(TProtocol protocol, TProtocolContainer * parent) : QObject(parent), TProjectItem(parent->model(), parent), m_protocol(protocol) {
    m_typeName = "protocol";
}

const TProtocol & TProtocolModel::protocol() const {
    return m_protocol;
}

void TProtocolModel::setProtocol(const TProtocol & protocol) {
    m_protocol = protocol;
}

int TProtocolModel::childrenCount() const
{
    return 0;
}

TProjectItem * TProtocolModel::child(int row) const
{
    return nullptr;
}

QString TProtocolModel::name() const
{
    return m_protocol.getName();
}

TProjectItem::Status TProtocolModel::status() const
{
    return Status::None;
}


bool TProtocolModel::toBeSaved() const {
    return true; // TODO
}

QDomElement TProtocolModel::save(QDomDocument & document) const {
    QDomElement element = TProjectItem::save(document);

    element.setAttribute("name", m_protocol.getName());
    element.setAttribute("description", m_protocol.getDescription());
    element.setAttribute("messages", saveMessages(m_protocol.getMessages()));

    return element;
}

void TProtocolModel::load(QDomElement * element) {
    if (!element)
        return;

    if (element->tagName() != typeName())
        throw tr("Unexpected tag");

    QString name = element->attribute("name");

    if(name.isEmpty())
        throw tr("Protocol name is empty");

    QString description = element->attribute("description");

    QString messagesArray = element->attribute("messages");
    QList<TMessage> messages = loadMessages(messagesArray.toUtf8());

    m_protocol = TProtocol(name, description, messages);
}

QByteArray TProtocolModel::saveMessages(const QList<TMessage> & messages) const {
    QByteArray array;
    QDataStream stream(&array, QIODeviceBase::WriteOnly);
    stream << messages;
    return array.toBase64();
}

QList<TMessage> TProtocolModel::loadMessages(const QByteArray & array) const {
    QDataStream stream(QByteArray::fromBase64(array));
    QList<TMessage> messages;
    stream >> messages;
    return messages;
}
