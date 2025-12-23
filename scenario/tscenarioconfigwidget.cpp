#include "tscenarioconfigwidget.h"

#include <QCheckBox>
#include <QHeaderView>

TScenarioConfigParamWidget::TScenarioConfigParamWidget(const TConfigParam & param, QWidget * parent, bool readOnly)
    : TConfigParamWidget(param, parent, readOnly) {

    setColumnCount(4);

    headerItem()->setText(3, tr("Dyn."));
    headerItem()->setToolTip(3, tr("If selected, an extra data input is added to this block; "
                                   "by passing values to it, the value of the respective parameter can be changed dynamically."));

    header()->setSectionResizeMode(3, QHeaderView::Fixed);
}

void TScenarioConfigParamWidget::setDynamicParamNames(QStringList allowedDynamicParamNames, QStringList selectedDynamicParamNames)
{
    m_allowedDynamicParamNames = allowedDynamicParamNames;
    m_selectedDynamicParamNames = selectedDynamicParamNames;
}

void TScenarioConfigParamWidget::addCheckBox(const TConfigParam & param, QTreeWidgetItem * parent, bool forceAllowed)
{
    qsizetype paramNameIndex = m_allowedDynamicParamNames.indexOf(QRegularExpression("^" + param.getName() + "\\*?$"));
    bool isAllowed = paramNameIndex > -1;

    if((isAllowed || forceAllowed) && param.getType() != TConfigParam::TType::TDummy) {
        bool isSelected = m_selectedDynamicParamNames.contains(param.getName());

        QCheckBox * checkBox = new QCheckBox(this);
        checkBox->setCheckState(isSelected ? Qt::Checked : Qt::Unchecked);
        checkBox->setDisabled(param.isReadonly() || isReadOnly());
        setItemWidget(parent, 3, checkBox);

        connect(checkBox, &QCheckBox::stateChanged, this, [this, param](int state) {
            if(state == Qt::Checked) {
                m_selectedDynamicParamNames.append(param.getName());
            }
            else {
                m_selectedDynamicParamNames.removeAll(param.getName());
            }
        });
    }

    bool forceAllowSubParams = isAllowed && m_allowedDynamicParamNames[paramNameIndex].endsWith('*');
    const QList<TConfigParam> & subParams = param.getSubParams();

    for (int i = 0; i < subParams.size(); i++) {
        const TConfigParam & subParam = subParams[i];
        addCheckBox(subParam, parent->child(i), forceAllowed || forceAllowSubParams);
    }
}

void TScenarioConfigParamWidget::setParam(const TConfigParam & param)
{
    TConfigParamWidget::setParam(param);
    addCheckBox(param, topLevelItem(0));
}

QStringList TScenarioConfigParamWidget::getSelectedDynamicParamNames() const {
    return m_selectedDynamicParamNames;
}
