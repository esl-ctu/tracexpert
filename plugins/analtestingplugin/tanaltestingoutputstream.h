#ifndef TANALTESTINGOUTPUTSTREAM_H
#define TANALTESTINGOUTPUTSTREAM_H

#include "tanaldevice.h"

class TAnalTestingOutputStream : public TAnalOutputStream
{
public:
    explicit TAnalTestingOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData);

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

#endif // TANALTESTINGOUTPUTSTREAM_H
