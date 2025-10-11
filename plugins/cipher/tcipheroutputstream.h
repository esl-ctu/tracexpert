#ifndef TCIPHEROUTPUTSTREAM_H
#define TCIPHEROUTPUTSTREAM_H

#include "tanaldevice.h"

class TCipherOutputStream : public TAnalOutputStream
{
public:
    explicit TCipherOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData);

    /// AnalStream name
    QString getName() const override;
    /// AnalStream info
    QString getInfo() const override;

    size_t writeData(const uint8_t * buffer, size_t len) override;

private:
    QString m_name;
    QString m_info;
    std::function<size_t(const uint8_t *, size_t)> m_writeData;
};

#endif // TCIPHEROUTPUTSTREAM_H

