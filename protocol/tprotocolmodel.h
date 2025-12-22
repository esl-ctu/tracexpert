#ifndef TPROTOCOLMODEL_H
#define TPROTOCOLMODEL_H

#include "../projectunit/tprojectunitmodel.h"

class TProtocol;
class TProtocolContainer;

/*!
 * \brief The TProtocolModel class represents a model for a Protocol.
 *
 * The class represents a model for a Protocol.
 * It is a model for the Protocol view in the Project view.
 * It is also a model for the Protocol table view in the Protocol Manager.
 *
 */
class TProtocolModel : public TProjectUnitModel {

public:
    TProtocolModel(TProtocolContainer * parent, TProtocol * protocol = nullptr);

    TProtocol * protocol() const;
};

#endif // TPROTOCOLMODEL_H
