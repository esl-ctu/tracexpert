#ifndef TANALSTREAMSENDERMODEL_H
#define TANALSTREAMSENDERMODEL_H

#include "../../common/sender/tsendermodel.h"

#include "tanalstreamsender.h"

class TAnalStreamSenderModel : public TSenderModel
{
public:
    explicit TAnalStreamSenderModel(TAnalStreamSender * sender, QObject * parent = nullptr);

    QString name();
    QString info();

private:
    TAnalStreamSender * m_analSender;
};

#endif // TANALSTREAMSENDERMODEL_H
