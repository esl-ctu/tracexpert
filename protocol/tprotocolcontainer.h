#ifndef TPROTOCOLCONTAINER_H
#define TPROTOCOLCONTAINER_H

#include <QAbstractTableModel>
#include "../projectunit/tprojectunitcontainer.h"

/*!
 * \brief The TProtocolContainer class represents a container for Protocols.
 *
 * The class represents a container for TProtocolModel objects.
 * It is a model for the Protocols view in the Project view.
 * It is also a model for the Protocols table view in the Protocol Manager.
 */
class TProtocolContainer : public TProjectUnitContainer {

public:
    explicit TProtocolContainer(TProjectModel * parent);
    QString name() const override;
};

#endif // TPROTOCOLCONTAINER_H
