#ifndef TIODEVICEWIDGET_H
#define TIODEVICEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>

#include "tiodevicemodel.h"
#include "tconfigparamwidget.h"

class TIODeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIODeviceWidget(TIODeviceModel * deviceModel, QWidget * parent = nullptr);

public slots:
    bool applyPostInitParam();

    void setAutoreceive(bool enabled);
    void receiveBytes();
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data);

    void sendBytes();
    void sendBusy();
    void sendFailed();
    //void selectSendMessageValidator();

private:
    TIODeviceModel * m_deviceModel;

    TConfigParamWidget * m_paramWidget;
    QLineEdit * m_receiveBytesEdit;
    QLineEdit * m_sendMessageEdit;
    QPlainTextEdit * m_communicationLogTextEdit;
};

#endif // TIODEVICEWIDGET_H
