#ifndef TIODEVICEWIDGET_H
#define TIODEVICEWIDGET_H

#include <QWidget>
#include <QLineEdit>

#include "tiodevicemodel.h"
#include "tconfigparamwidget.h"

class TIODeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIODeviceWidget(TIODeviceModel * deviceModel, QWidget * parent = nullptr);

public slots:
    bool applyPostInitParam();
    void receiveBytes();
    void receiveBusy();
    //void selectSendMessageValidator();

private:
    TIODeviceModel * m_deviceModel;

    TConfigParamWidget * m_paramWidget;
    QLineEdit * m_receiveBytesEdit;
};

#endif // TIODEVICEWIDGET_H
