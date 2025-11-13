#ifndef TRECEIVERWIDGET_H
#define TRECEIVERWIDGET_H

#include <QLineEdit>
#include <QComboBox>

#include "../../protocol/tprotocolcontainer.h"
#include "../receiver/treceivermodel.h"

class TReceiverWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TReceiverWidget(TReceiverModel * receiverModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

signals:
    void protocolSelected(TProtocol protocol, TReceiverModel * receiver);

public slots:
    void setAutoreceive(bool enabled);
    void receiveBytes();

    void receiveFile(QString fileName);

    void updateDisplayedProtocols();

private slots:
    void protocolChanged(int index);

private:
    TReceiverModel * m_receiverModel;

    TProtocolContainer * m_protocolContainer;

    QLineEdit * m_bytesEdit;
    QComboBox * m_protocolComboBox;
};

#endif // TRECEIVERWIDGET_H
