#ifndef TPROTOCOLMANAGER_H
#define TPROTOCOLMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTableView>

#include "../projectunit/tprojectunitwidget.h"
#include "tprotocolcontainer.h"
#include "tprotocolmodel.h"

class TProtocolWidget : public TProjectUnitWidget {
    Q_OBJECT
public:
    explicit TProtocolWidget(TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
};

#endif // TPROTOCOLMANAGER_H
