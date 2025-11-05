#ifndef TANALSTREAMSENDER_H
#define TANALSTREAMSENDER_H

#include "../../common/sender/tsender.h"

#include "tanaldevice.h"

class TAnalStreamSender : public TSender
{
    Q_OBJECT

public:
    explicit TAnalStreamSender(TAnalOutputStream * stream, QObject * parent = nullptr);

    QString name() override;
    QString info() override;

protected:
    size_t writeData(const uint8_t * buffer, size_t len) override;

private:
    TAnalOutputStream * m_stream;
};

#endif // TANALSTREAMSENDER_H
