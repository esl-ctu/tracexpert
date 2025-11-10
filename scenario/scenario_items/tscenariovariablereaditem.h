#ifndef TSCENARIOVARIABLEREADITEM_H
#define TSCENARIOVARIABLEREADITEM_H

#include <QIODevice>
#include "../tscenarioitem.h"

/*!
 * \brief ...
 *
 */
class TScenarioVariableReadItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioVariableReadItem;
    }

    TScenarioVariableReadItem() : TScenarioItem(tr("Variable: read"), tr("This block allows reading of a variable.")) {
        addDataOutputPort("dataOut");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Variable: read", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Variable name", "", TConfigParam::TType::TString, tr("Variable name."), false));

        m_subtitle = "missing variable name";
        setState(TState::TError, tr("Missing variable name."));
    }

    TScenarioItem * copy() const override {
        return new TScenarioVariableReadItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/variable.png";
    }

    QString variableName() {
        return m_params.getSubParamByName("Variable name")->getValue();
    }

    bool prepare() override {
        if(getState() == TState::TError) {
            return false;
        }

        return true;
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Variable name", &iok);
        if(!iok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the pre-init params."));
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        if(m_title != params.getSubParamByName("Block name")->getValue()) {
            m_title = params.getSubParamByName("Block name")->getValue();
            emit appearanceChanged();
        }


        if(m_params.getSubParamByName("Variable name")->getValue().isEmpty()) {
            m_params.getSubParamByName("Variable name")->setState(TConfigParam::TState::TError, "Varaible name cannot be empty.");
        }

        if(m_subtitle != params.getSubParamByName("Variable name")->getValue()) {
            m_subtitle = params.getSubParamByName("Variable name")->getValue();
            emit appearanceChanged();
        }

        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            resetState();
        }

        return m_params;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        // purposefully left blank
        return QHash<TScenarioItemPort *, QByteArray>();
    }
};

#endif // TSCENARIOVARIABLEREADITEM_H
