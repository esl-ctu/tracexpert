#ifndef TSCENARIOCONFIGWIDGET_H
#define TSCENARIOCONFIGWIDGET_H

#include "widgets/tconfigparamwidget.h"

class TScenarioConfigParamWidget : public TConfigParamWidget
{
    Q_OBJECT

public:
    explicit TScenarioConfigParamWidget(const TConfigParam & param, QWidget * parent = nullptr, bool readOnly = false);

    void setDynamicParamNames(QStringList allowedDynamicParamNames, QStringList selectedDynamicParamNames);
    QStringList getSelectedDynamicParamNames() const;

    void setParam(const TConfigParam & param);

private:
    void addCheckBox(const TConfigParam & param, QTreeWidgetItem * parent, bool forceAllowed = false);

    QStringList m_allowedDynamicParamNames;
    QStringList m_selectedDynamicParamNames;
};

#endif // TSCENARIOCONFIGWIDGET_H
