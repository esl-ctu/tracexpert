#include "tcommunicationlogwidget.h"

#include <QLayout>
#include <QPushButton>
#include <QTime>

TCommunicationLogWidget::TCommunicationLogWidget(QList<TSenderModel *> senderModels, QList<TReceiverModel *> receiverModels, QWidget * parent)
    : QWidget(parent), m_senderModels(senderModels), m_receiverModels(receiverModels)
{
    for (int i = 0; i < m_receiverModels.length(); i++) {
        connect(m_receiverModels[i], &TReceiverModel::readBusy, this, &TCommunicationLogWidget::receiveBusy);
        connect(m_receiverModels[i], &TReceiverModel::readFailed, this, &TCommunicationLogWidget::receiveFailed);
        connect(m_receiverModels[i], &TReceiverModel::dataRead, this, [=](QByteArray data){ dataReceived(data, m_receiverModels[i]); });
    }

    for (int i = 0; i < m_senderModels.length(); i++) {
        connect(m_senderModels[i], &TSenderModel::writeBusy, this, &TCommunicationLogWidget::sendBusy);
        connect(m_senderModels[i], &TSenderModel::writeFailed, this, &TCommunicationLogWidget::sendFailed);
        connect(m_senderModels[i], &TSenderModel::dataWritten, this, [=](QByteArray data){ dataSent(data, m_senderModels[i]); });
    }

    m_textEdit = new QPlainTextEdit;
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    m_textEdit->setFont(f);
    m_textEdit->setReadOnly(true);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_textEdit);

    QHBoxLayout * settingsLayout = new QHBoxLayout;

    m_format = new QComboBox;
    m_format->addItem("Show only hexadecimal values");
    m_format->addItem("Show human-readable string when possible");
    settingsLayout->addWidget(m_format);

    QPushButton * clearButton = new QPushButton;
    clearButton->setText("Clear");
    connect(clearButton, &QPushButton::clicked, m_textEdit, &QPlainTextEdit::clear);
    for (int i = 0; i < m_receiverModels.length(); i++)
        connect(clearButton, &QPushButton::clicked, m_receiverModels[i], &TReceiverModel::clearReceivedData);
    settingsLayout->addWidget(clearButton);

    layout->addLayout(settingsLayout);
    setLayout(layout);
}

QString TCommunicationLogWidget::byteArraytoHumanReadableString(const QByteArray & byteArray)
{
    static QRegularExpression nonAsciiRegExp("[^ -~]");
    bool isHumanReadable = !((QString)byteArray).contains(nonAsciiRegExp);

    if (byteArray.size() <= DISPLAY_DATA_LENGTH_LIMIT) {
        if(!isHumanReadable) {
            return byteArray.toHex(' ');
        }
        else {
            return "<i>\"" + QString(byteArray).toHtmlEscaped() + "\"</i>";
        }
    }
    else {
        if (!isHumanReadable) {
            return QString("%1 ... <i>skipping %2 bytes</i> ... %3")
            .arg(byteArray.first(5).toHex(' '))
                .arg(byteArray.length() - 10)
                .arg(byteArray.last(5).toHex(' '));
        }
        else {
            return QString("<i>\"%1\" ... skipping %2 bytes ... \"%3\"</i>")
            .arg(QString(byteArray.first(5)).toHtmlEscaped())
                .arg(byteArray.length() - 10)
                .arg(QString(byteArray.last(5)).toHtmlEscaped());
        }
    }
}

void TCommunicationLogWidget::receiveBusy()
{
    append("Failed to receive data: device busy");
}

void TCommunicationLogWidget::receiveFailed()
{
    append("Failed to receive data");
}

void TCommunicationLogWidget::dataReceived(QByteArray data, TReceiverModel * receiverModel)
{
    QString formattedData;
    qsizetype size;

    TProtocol protocol = m_protocols.contains(receiverModel) ? m_protocols[receiverModel] : TProtocol();

    if (protocol == TProtocol()) {
        formattedData = byteArraytoHumanReadableString(data);
        size = data.size();
    }
    else {
        TMessage matchedMessage = protocol.tryMatchResponse(data);

        if (matchedMessage.getName().isEmpty()) {
            qWarning("Received data could not be interpreted as any of the protocol's defined messages.");
            formattedData = byteArraytoHumanReadableString(data);
            size = data.size();
        }
        else {
            formattedData = matchedMessage.getPayloadSummary();
            size = -1;
        }
    }

    appendCommunication(Direction::Received, formattedData, size, receiverModel->name());
}

void TCommunicationLogWidget::sendBusy()
{
    append("Failed to send data: device busy");
}

void TCommunicationLogWidget::sendFailed()
{
    append("Failed to send data");
}

void TCommunicationLogWidget::dataSent(QByteArray data, TSenderModel * senderModel)
{
    QString formattedData;
    qsizetype size;

    TMessage sentMessage = m_sentMessages.contains(senderModel) ? m_sentMessages[senderModel] : TMessage();

    if (data == sentMessage.getData())
    {
        formattedData = sentMessage.getPayloadSummary();
        size = -1;
    }
    else {
        formattedData = byteArraytoHumanReadableString(data);
        size = data.size();
    }

    m_sentMessages[senderModel] = TMessage();

    appendCommunication(Direction::Sent, formattedData, size, senderModel->name());
}

void TCommunicationLogWidget::messageSent(TMessage message, TSenderModel * sender)
{
    m_sentMessages[sender] = message;
}

void TCommunicationLogWidget::protocolChanged(TProtocol protocol, TReceiverModel * receiver)
{
    m_protocols[receiver] = protocol;
}

void TCommunicationLogWidget::append(QString header, QString message, QColor color) {
    QString formattedTime = QTime::currentTime().toString("hh:mm:ss");

    QString formattedHeader;
    if (!header.isEmpty())
        formattedHeader = QString(" <b>%1</b>").arg(header);

    m_textEdit->appendHtml(QString("%1%2").arg(formattedTime, formattedHeader));

    if (message.isEmpty())
        return;

    QString formattedMessage = QString("<div style=\"color:%1\">%2</div>").arg(color.name(), message);
    m_textEdit->appendHtml(formattedMessage);
}

void TCommunicationLogWidget::appendCommunication(Direction direction, QString data, qsizetype size, QString origin) {
    QColor color = direction == Direction::Received ? QColorConstants::Red : QColorConstants::Blue;

    QString formattedDirection = direction == Direction::Received ? "Received" : "Sent";

    QString formattedSize;
    if (size >= 0)
        formattedSize = QString(" %1 B").arg(QString::number(size));

    QString formattedOrigin;
    if (!origin.isEmpty())
        formattedOrigin = QString(" (%1)").arg(origin);

    append(QString("%1%2%3").arg(formattedDirection, formattedSize, formattedOrigin), data, color);
}
