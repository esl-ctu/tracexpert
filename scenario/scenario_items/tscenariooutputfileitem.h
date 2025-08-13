#ifndef TSCENARIOOUTPUTFILEITEM_H
#define TSCENARIOOUTPUTFILEITEM_H

#include <QString>
#include <QList>
#include <QDataStream>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioOutputFileItem class represents a block that creates untracked output files.
 *
 * The class represents a block that creates untracked output files.
 *
 */
class TScenarioOutputFileItem : public TScenarioItem {

public:
    enum { TItemClass = 90 };
    int itemClass() const override { return TItemClass; }

    TScenarioOutputFileItem() : TScenarioItem(tr("Write to: untracked file"), tr("This block writes data into files not tracked by TraceXpert.")) {
        addFlowInputPort("flowIn");
        addDataInputPort("dataIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Write to untracked file", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Path to folder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), TConfigParam::TType::TDirectoryName, tr("Folder where files will be saved to."), false));
        m_params.addSubParam(TConfigParam("File name", "output.txt", TConfigParam::TType::TString, tr("Name of file to be generated."), false));

        TConfigParam modeParam("Write mode", "Create new file on each input", TConfigParam::TType::TEnum, tr("Which way should the data be written to files."), false);
        modeParam.addEnumValue("Create new file on each input");
        modeParam.addEnumValue("Append to single file on each input");
        modeParam.addEnumValue("Override single file on each input");
        m_params.addSubParam(modeParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioOutputFileItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/file.png";
    }

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Path to folder", &iok);
        if(!iok) return false;

        params.getSubParamByName("File name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Write mode", &iok);
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
        m_counter = 0;
        return true;
    }

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        QString directory = m_params.getSubParamByName("Path to folder")->getValue();
        QString filename = m_params.getSubParamByName("File name")->getValue();

        if(m_params.getSubParamByName("Write mode")->getValue() == "Create new file on each input") {
            QFileInfo fileInfo(filename);
            filename = QString("%1%2.%3").arg(fileInfo.completeBaseName()).arg(m_counter).arg(fileInfo.suffix());
        }

        QFile file(QString("%1/%2").arg(directory, filename));

        bool fileOpened;
        if(m_params.getSubParamByName("Write mode")->getValue() == "Override single file on each input") {
            fileOpened = file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        }
        else{
            fileOpened = file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        }

        if (!fileOpened) {
            setState(TState::TRuntimeError, tr("Could not open file for writing."));
            return QHash<TScenarioItemPort *, QByteArray>();
        }

        QTextStream stream(&file);
        stream << dataInputValues.value(getItemPortByName("dataIn"));
        file.close();

        m_counter++;

        return QHash<TScenarioItemPort *, QByteArray>();
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_state == TState::TRuntimeError ? this->getItemPortByName("flowOutError") : this->getItemPortByName("flowOut");
    }

protected:
    uint m_counter;
};

#endif // TSCENARIOOUTPUTFILEITEM_H
