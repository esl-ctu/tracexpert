#ifndef TIODEVICESENDER_H
#define TIODEVICESENDER_H

#include "tsender.h"

#include "tiodevice.h"

class TIODeviceSender : public TSender
{
    Q_OBJECT

public:
    explicit TIODeviceSender(TIODevice * IODevice, QObject * parent = nullptr);

protected:
    size_t writeData(const uint8_t * buffer, size_t len) override;

private:
    TIODevice * m_IODevice;
};

#endif // TIODEVICESENDER_H
