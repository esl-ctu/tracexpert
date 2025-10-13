#ifndef TPREDICTINPUTSTREAM_H
#define TPREDICTINPUTSTREAM_H

#include "tanaldevice.h"

class TPredictInputStream : public TAnalInputStream
{
public:
    explicit TPredictInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData);

    /// AnalStream name
    QString getName() const override;
    /// AnalStream info
    QString getInfo() const override;

    size_t readData(uint8_t * buffer, size_t len) override;

private:
    QString m_name;
    QString m_info;
    std::function<size_t(uint8_t *, size_t)> m_readData;
};

#endif // TPREDICTINPUTSTREAM_H

