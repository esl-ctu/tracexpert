#ifndef TPROJECTUNITMODEL_H
#define TPROJECTUNITMODEL_H

#include "../project/tprojectitem.h"
#include "tprojectunitcontainer.h"
#include <QObject>

class TProjectUnitModel : public QObject, public TProjectItem {
    Q_OBJECT

public:
    TProjectUnitModel(const QString & typeName, TProjectUnitContainer * parent);
    TProjectUnitModel(const QString & typeName, TProjectUnit * item, TProjectUnitContainer * parent);

    ~TProjectUnitModel();

    void openEditor();

    TProjectUnit * unit() const;
    void setUnit(TProjectUnit * unit);

    // methods for TProjectItem - to be able to show Scenarios in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

    bool toBeSaved() const override;
    QDomElement save(QDomDocument & document) const override;
    void load(QDomElement * element);

signals:
    void editorRequested(TProjectUnitModel * projectUnitModel);

protected:
    TProjectUnit * m_unit;
};

#endif // TPROJECTUNITMODEL_H
