#ifndef TANALSTREAMRECEIVER_H
#define TANALSTREAMRECEIVER_H

#include "../../common/receiver/treceiver.h"

#include "tanaldevice.h"

class TAnalStreamReceiver : public TReceiver
{
    Q_OBJECT

public:
    explicit TAnalStreamReceiver(TAnalInputStream * stream, QObject * parent = nullptr);

    QString name() override;
    QString info() override;

protected:
    size_t readData(uint8_t * buffer, size_t len) override;
    std::optional<size_t> availableBytes() override;

private:
    TAnalInputStream * m_stream;
};

#endif // TANALSTREAMRECEIVER_H
