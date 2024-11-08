#ifndef TSCENARIOCOUNTERITEM_H
#define TSCENARIOCOUNTERITEM_H

#include <QIODevice>
#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioLoopItem class represents a block that represents a loop.
 *
 * The class represents a block that represents a loop.
 * It is a block with one input flow port and two output flow ports. The output flow ports are "done" and "repeat".
 * The block directs the flow based on the number of iterations left - "repeat" is activated when there are more iterations left, "done" when there are none.
 * The number of iterations can be set in the configuration.
 *
 */
class TScenarioLoopItem : public TScenarioItem {

public:
    enum { TItemClass = 50 };
    int itemClass() const override { return TItemClass; }

    TScenarioLoopItem() : TScenarioItem(tr("Loop"), tr("This block represents a loop.")) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOutDone", "done");
        addFlowOutputPort("flowOutRepeat", "repeat");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Loop", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Number of iterations", "3", TConfigParam::TType::TULongLong, tr("Number of times the repeat flow output will be activated."), false));
    }

    TScenarioItem * copy() const override {
        return new TScenarioLoopItem(*this);
    }

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Number of iterations", &iok);
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

        return m_params;
    }

    bool prepare() override {
        bool ok;
        m_totalIterations = m_params.getSubParamByName("Number of iterations")->getValue().toULongLong(&ok);
        m_numIterationsLeft = m_totalIterations + 1;
        return ok;
    }

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        if(m_numIterationsLeft == 0) {
            m_numIterationsLeft = m_totalIterations;
        }
        else {
            m_numIterationsLeft--;
        }

        if(m_numIterationsLeft > 0) {
            log(QString(tr("Starting iteration #%1")).arg(m_totalIterations - m_numIterationsLeft + 1));
        }
        else {
            log(tr("Loop done"));
        }

        return QHash<TScenarioItemPort *, QByteArray>();
    }
    
    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_numIterationsLeft == 0 ? this->getItemPortByName("flowOutDone") : this->getItemPortByName("flowOutRepeat");
    }

private:    
    uint64_t m_totalIterations;
    uint64_t m_numIterationsLeft;

};


#endif // TSCENARIOCOUNTERITEM_H
