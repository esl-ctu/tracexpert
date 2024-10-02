#ifndef TIODEVICERECEIVER_H
#define TIODEVICERECEIVER_H

#include "treceiver.h"

#include "tiodevice.h"

class TIODeviceReceiver : public TReceiver
{
    Q_OBJECT

public:
    explicit TIODeviceReceiver(TIODevice * IODevice, QObject * parent = nullptr);

protected:
    size_t readData(uint8_t * buffer, size_t len) override;

private:
    TIODevice * m_IODevice;
};

#endif // TIODEVICERECEIVER_H
