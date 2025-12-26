// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contributors to this file:
// Tomáš Přeučil (initial author)

#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, targetType type_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    m_name = name_in;
    sn = sn_in;
    plugin = plugin_in;
    m_createdManually = createdManually_in;
    scopeParent = NULL;
    cwId = NO_CW_ID;
    type = type_in;
}

void TnewaeDevice::preparePreInitParams(){
    if (type == TARGET_NORMAL){
        m_preInitParams = TConfigParam("NewAE target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "Nothing to set here");
        return;
    }

    if (type == TARGET_CW305) {
        m_preInitParams = TConfigParam("NewAE CW305 target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "");
        m_preInitParams.addSubParam(TConfigParam("Beta version! If this causes a Cadmium II leak, I'm not responsible!", "", TConfigParam::TType::TDummy, ""));
        m_preInitParams.addSubParam(TConfigParam("Bitstream", "", TConfigParam::TType::TFileName, ""));
    }
    else if (type == TARGET_CW310){
        m_preInitParams = TConfigParam("NewAE CW310 target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "");
        m_preInitParams.addSubParam(TConfigParam("Pre-alpha version! Never tested! Use at your own risk, I do NOT own a CW310!", "", TConfigParam::TType::TDummy, ""));
    }

    m_preInitParams.addSubParam(TConfigParam("To bind a scope to this target, it needs to be autodetected at the same time s the target.\nManual config is not supported.", "", TConfigParam::TType::TDummy, ""));
    auto scopeEnum1 = TConfigParam("Bind to scope", QString(""), TConfigParam::TType::TEnum, "");
    scopeEnum1.addEnumValue("None");
    QList<TScope *> scopes = plugin->getScopes();
    for (int i = 0; i < scopes.length(); ++i){
        TnewaeScope * currentScope = (TnewaeScope *) scopes.at(i);
        QString scopeSn = currentScope->getInfo();
        scopeEnum1.addEnumValue(scopeSn);
    }
    m_preInitParams.addSubParam(scopeEnum1);

}


TnewaeDevice::~TnewaeDevice(){
    if(m_initialized)
        TnewaeDevice::deInit();
}

uint8_t TnewaeDevice::getId(){
    return cwId;
}

QString TnewaeDevice::getDeviceSn(){
    return sn;
}

QString TnewaeDevice::getName() const{
    return m_name;
}

QString TnewaeDevice::getInfo() const{
    return m_info;
}

TConfigParam TnewaeDevice::getPreInitParams() const{
    return m_preInitParams;
}

TConfigParam TnewaeDevice::setPreInitParams(TConfigParam params){
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

TConfigParam TnewaeDevice::updatePostInitParams(TConfigParam paramsIn, bool write /*= false*/) const {
    auto topPrm = paramsIn;
    topPrm.resetState();
    auto prms = topPrm.getSubParams();
    for (auto it = prms.begin(); it != prms.end(); ++it) {
        bool ok, ok2;
        QString out = "";;
        size_t len = 0;
        QString prmName = (*it).getName();

        //Check if the param is local to the plugin and does not need to be written to the device
        if (prmName == "FPGA read address" || prmName == "FPGA write address" || prmName == "Function to use when reading from the FPGA"){
            topPrm.getSubParamByName(prmName)->setValue((*it).getValue());
            continue;
        }

        QList<QString> args;
        if ((*it).getType() == TConfigParam::TType::TDummy && (*it).getHint() != WRITE_ONLY_STRING) { //process subparam, (*it).getHint() == WRITE_ONLY_STRING would be a fucntion!
            auto subPrms = (*it).getSubParams();
            for (auto itt = subPrms.begin(); itt != subPrms.end(); ++itt) {
                args.clear();
                out = "";
                len = 0;
                bool ok3, ok4;
                QString subPrmName = (*itt).getName();
                if ((*itt).getHint() == READ_ONLY_STRING) { // a function on a subobject
                    if (write)
                        continue;
                    QList<QString> par;
                    ok4 = plugin->runPythonFunctionOnAnObjectAndGetStringOutput(cwId, prmName, subPrmName, 0, par, len, out, true);
                } else if ((*itt).getHint() == WRITE_ONLY_STRING) { // function that sets stuff
                    if (!write)
                        continue;

                    bool run = (*itt).getSubParamByName("Run?")->getValue().toLower() == "true";
                    if (!run)
                        continue;

                    auto prmArgs = (*itt).getSubParams();
                    for (auto arg = prmArgs.begin(); arg != prmArgs.end(); ++arg) {
                        if ((*arg).getName() == "Run?" || (*arg).getName() == "Result" )
                            continue;

                        topPrm.getSubParamByName((*it).getName())->getSubParamByName((*itt).getName())->getSubParamByName((*arg).getName())->setValue((*arg).getValue());
                        args.append((*arg).getValue());
                    }

                    ok4 = plugin->runPythonFunctionOnAnObjectAndGetStringOutput(cwId, prmName, subPrmName, args.length(), args, len, out, true);

                    topPrm.getSubParamByName((*it).getName())->getSubParamByName((*itt).getName())->getSubParamByName("Run?")->setValue("false");
                    topPrm.getSubParamByName((*it).getName())->getSubParamByName((*itt).getName())->getSubParamByName("Result")->setValue(out);

                    if (!ok) {
                        topPrm.setState(TConfigParam::TState::TWarning, "Cannot run a function on a subobject.");
                        qDebug("%s", ("Error running a function " + subPrmName + " on object " + prmName + "res: " + out).toLocal8Bit().constData());
                    }

                } else { // normal property
                    if (write) {
                        QString val = (*itt).getValue();
                        if (!((*itt).isReadonly())) //do not write readonly params
                            ok4 = plugin->setPythonSubparameter(cwId, prmName, subPrmName, val, out, true);
                    } else {
                        ok4 = plugin->getPythonSubparameter(cwId, prmName, subPrmName, out, true);
                    }
                }
                topPrm.getSubParamByName(prmName, &ok)->getSubParamByName(subPrmName, &ok2)->setValue(out.toLower(), &ok3);
                if (!ok || !ok2 || !ok3 || !ok4) {
                    topPrm.setState(TConfigParam::TState::TWarning, "Cannot read/write some params.");
                    qDebug("%s", ("Error reading/writing subparam " + subPrmName + " of param " + prmName).toLocal8Bit().constData());
                }
            }
        } else if ((*it).getHint() == READ_ONLY_STRING) { //get a param that needs a function call with no args
            if (write)
                continue;

            bool ok3 = plugin->runPythonFunctionAndGetStringOutput(cwId, prmName, 0, args, len, out, true);
            topPrm.getSubParamByName(prmName, &ok)->setValue(out.toLower(), &ok2);
            if (!ok || !ok2 || !ok3) {
                topPrm.setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                qDebug("%s", ("Error reading param " + prmName + " res(1): " + out).toLocal8Bit().constData());
            }
        } else if ((*it).getHint() == WRITE_ONLY_STRING) { // function that sets stuff
            if (!write)
                continue;
            bool run = (*it).getSubParamByName("Run?")->getValue().toLower() == "true";
            if (!run)
                continue;

            auto prmArgs = (*it).getSubParams();
            for (auto arg = prmArgs.begin(); arg != prmArgs.end(); ++arg) {
                if ((*arg).getName() == "Run?" || (*arg).getName() == "Result" )
                    continue;

                topPrm.getSubParamByName((*it).getName())->getSubParamByName((*arg).getName())->setValue((*arg).getValue());
                args.append((*arg).getValue());
            }

            ok = plugin->runPythonFunctionAndGetStringOutput(cwId, prmName, args.length(), args, len, out, true);

            topPrm.getSubParamByName((*it).getName())->getSubParamByName("Run?")->setValue("false");
            topPrm.getSubParamByName((*it).getName())->getSubParamByName("Result")->setValue(out);

            if (!ok) {
                topPrm.setState(TConfigParam::TState::TWarning, "Cannot run a function.");
                qDebug("%s", ("Error running a function " + prmName + " res: " + out).toLocal8Bit().constData());
            }
        } else { //normal param (python object property)
            if (write) {
                QString val = (*it).getValue();
                if (!((*it).isReadonly())) //do not write readonly params
                    plugin->setPythonParameter(cwId, prmName, val, out, true);
            } else {
                plugin->getPythonParameter(cwId, prmName, out, true);
            }
            topPrm.getSubParamByName(prmName, &ok)->setValue(out.toLower(), &ok2);
            if (!ok || !ok2) {
                if (write) {
                    topPrm.setState(TConfigParam::TState::TWarning, "Cannot write some params.");
                    qDebug("%s", ("Error writing param " + prmName + " res(2): " + out).toLocal8Bit().constData());
                } else {
                    topPrm.setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                    qDebug("%s", ("Error reading param " + prmName + " res(2): " + out).toLocal8Bit().constData());
                }
            }
        }
    }

    return topPrm;
}

void TnewaeDevice::init(bool *ok/* = nullptr*/){
    QList<TScope *> scopes = plugin->getScopes();
    TnewaeScope * matchingScope = NULL;

    if (type != TARGET_NORMAL && m_preInitParams.getSubParamByName("Bind to scope")->getValue() == "None"){
        cwId = plugin->addDummyScope();
    } else {
        QString snToCheck = type == TARGET_NORMAL ? sn : m_preInitParams.getSubParamByName("Bind to scope")->getValue();

        for (int i = 0; i < scopes.length(); ++i){
            TnewaeScope * currentScope = (TnewaeScope *) scopes.at(i);

            QString scopeSn = currentScope->getSn();
            if (scopeSn == snToCheck) {
                matchingScope = currentScope;
                break;
            }
        }

        if (matchingScope) {
            scopeParent = matchingScope;
            cwId = scopeParent->getId();
            if(!scopeParent->isInitialized()) {
                qWarning("Matching scope was found but was not initialized yet. Please initialize the scope first!");
                if (ok != nullptr) {
                    //TODO okýnko
                    *ok = false;
                    return;
                }
            }
        } else {
            qWarning("Matching scope was not found. Please set up and initialize the scope first!");
            if (ok != nullptr) {
                *ok = false;
                //TODO okýnko
                return;
            }
        }
    }

    QString toSend;
    QList<QString> params;
    //params.append(QString::number(cwId));
    if (type == TARGET_CW305) {
        params.append(m_preInitParams.getSubParamByName("Bitstream")->getValue());
        plugin->packageDataForPython(cwId, "T305-SETUP", 1, params, toSend);
    } else if (type == TARGET_CW305) {
        plugin->packageDataForPython(cwId, "T310-SETUP", 0, params, toSend);
    } else {
        plugin->packageDataForPython(cwId, "T-SETUP", 0, params, toSend);
    }
    bool succ = plugin->writeToPython(cwId, toSend, true);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        qCritical("Error setting target up in Python (1)");
    }

    succ = plugin->waitForPythonTargetDone(cwId);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        qCritical("Error setting target up in Python (2)");
        return;
    }

    m_postInitParams = _createPostInitParams();
    m_postInitParams = updatePostInitParams(m_postInitParams);

    if(ok != nullptr) *ok = true;
    m_initialized = true;


}

//IMPORTANT: Do not edit the hints that say "alwaysRunFunc" (READ_ONLY_STRING) or "writeOnlyFunc" (WRITE_ONLY_STRING)!!!
TConfigParam TnewaeDevice::_createPostInitParams(){
    TConfigParam postInitParams = TConfigParam("NewAE target " + m_name + " post-init config", "", TConfigParam::TType::TDummy, "");
    if (type == TARGET_NORMAL) {
        postInitParams.addSubParam(TConfigParam("baud", "38400", TConfigParam::TType::TInt, "Baudrate for the target"));
    } else if (type == TARGET_CW305) {
        postInitParams.addSubParam(TConfigParam("INITB_state", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("get_fpga_buildtime", "", TConfigParam::TType::TString, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("is_done", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("is_programmed", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("core_type", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(TConfigParam("crypt_rev", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(TConfigParam("crypt_type", "", TConfigParam::TType::TString, "", true));

        //function helpers
        auto fpgaReadAddres = TConfigParam("FPGA read address", "0", TConfigParam::TType::TULongLong, "Used in the fpga_read function/stream");
        auto fpgaWriteAddres = TConfigParam("FPGA write address", "0", TConfigParam::TType::TULongLong, "Used in the fpga_write function/stream");
        auto readUse = TConfigParam("Function to use when reading from the FPGA", "fpga_read", TConfigParam::TType::TEnum, "");
        readUse.addEnumValue("fpga_read");
        readUse.addEnumValue("readOutput");
        postInitParams.addSubParam(fpgaReadAddres);
        postInitParams.addSubParam(fpgaWriteAddres);
        postInitParams.addSubParam(readUse);

        //functions
        auto br = TConfigParam("batchRun", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        br.addSubParam(TConfigParam("batchsize", "1024", TConfigParam::TType::TInt, ""));
        br.addSubParam(TConfigParam("random_key", "True", TConfigParam::TType::TBool, ""));
        br.addSubParam(TConfigParam("random_pt", "True", TConfigParam::TType::TBool, ""));
        br.addSubParam(TConfigParam("seed", "0", TConfigParam::TType::TInt, "")); //todo does this work? Default is none
        br.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        br.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(br);

        auto cek = TConfigParam("checkEncryptionKey", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        cek.addSubParam(TConfigParam("key", "", TConfigParam::TType::TString, ""));
        cek.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        cek.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(cek);

        auto go = TConfigParam("go", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        go.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        go.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(go);

        auto lek = TConfigParam("loadEncryptionKey", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        lek.addSubParam(TConfigParam("key", "", TConfigParam::TType::TString, ""));
        lek.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        lek.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(lek);

        auto li = TConfigParam("loadInput", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        li.addSubParam(TConfigParam("inputtext", "", TConfigParam::TType::TString, ""));
        li.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        li.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(li);

        auto sk = TConfigParam("loadEncryptionKey", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        sk.addSubParam(TConfigParam("key", "", TConfigParam::TType::TString, ""));
        sk.addSubParam(TConfigParam("timeout", "250", TConfigParam::TType::TInt, ""));
        sk.addSubParam(TConfigParam("always_send", QString("false"), TConfigParam::TType::TBool, ""));
        sk.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        sk.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(sk);

        auto ro = TConfigParam("readOutput", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        ro.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        ro.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(ro);

        //pll
        auto pll = TConfigParam("pll", "", TConfigParam::TType::TDummy, "");

        auto cmd = TConfigParam("calcMulDiv", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        cmd.addSubParam(TConfigParam("freqdesired", "1000000", TConfigParam::TType::TInt, ""));
        cmd.addSubParam(TConfigParam("freqsource", "0", TConfigParam::TType::TInt, ""));
        cmd.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        cmd.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(cmd);

        auto c9i = TConfigParam("cdce906init", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        c9i.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        c9i.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(c9i);

        auto c9r = TConfigParam("cdce906init", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        c9r.addSubParam(TConfigParam("addr", "0", TConfigParam::TType::TInt, ""));
        c9r.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        c9r.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(c9r);

        auto c9so = TConfigParam("cdce906setoutput", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        c9so.addSubParam(TConfigParam("outpin", "0", TConfigParam::TType::TInt, ""));
        c9so.addSubParam(TConfigParam("divsource", "", TConfigParam::TType::TString, ""));
        c9so.addSubParam(TConfigParam("slewrate", "+0nS", TConfigParam::TType::TString, ""));
        c9so.addSubParam(TConfigParam("enabled", "true", TConfigParam::TType::TBool, ""));
        c9so.addSubParam(TConfigParam("inverted", "false", TConfigParam::TType::TBool, ""));
        c9so.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        c9so.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(c9so);

        auto c9w = TConfigParam("cdce906write", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        c9w.addSubParam(TConfigParam("addr", "0", TConfigParam::TType::TInt, ""));
        c9w.addSubParam(TConfigParam("data", "", TConfigParam::TType::TString, ""));
        c9w.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        c9w.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(c9w);

        auto otp = TConfigParam("outnumToPin", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        otp.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        otp.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        otp.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(otp);

        auto ouo = TConfigParam("outputUpdateOutputs", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        ouo.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        ouo.addSubParam(TConfigParam("pllsrc_new", "None", TConfigParam::TType::TString, ""));
        ouo.addSubParam(TConfigParam("pllenabled_new", "None", TConfigParam::TType::TString, ""));
        ouo.addSubParam(TConfigParam("pllslewrate_new", "None", TConfigParam::TType::TString, ""));
        ouo.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        ouo.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(ouo);

        pll.addSubParam(TConfigParam("pll_enable_get", "", TConfigParam::TType::TString, READ_ONLY_STRING, true));

        auto pes = TConfigParam("pll_enable_set", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pes.addSubParam(TConfigParam("enabled", "", TConfigParam::TType::TBool, ""));
        pes.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pes.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pes);

        auto pes2 = TConfigParam("pll_outenable_get", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pes2.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pes2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pes2.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pes);

        auto pos = TConfigParam("pll_outenable_set", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pos.addSubParam(TConfigParam("enabled", "", TConfigParam::TType::TBool, ""));
        pos.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pos.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pos.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pos);

        auto pog = TConfigParam("pll_outfreq_get", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pog.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pog.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pog.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pog);

        auto pos2 = TConfigParam("pll_outfreq_set", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pos2.addSubParam(TConfigParam("freq", "1000000", TConfigParam::TType::TInt, ""));
        pos2.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pos2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pos2.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pos2);

        auto pog2 = TConfigParam("pll_outslew_get", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pog2.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pog2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pog2.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pog2);

        auto pos3 = TConfigParam("pll_outslew_set", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pos3.addSubParam(TConfigParam("slew", "", TConfigParam::TType::TString, ""));
        pos3.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pos3.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pos3.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pos3);

        auto pog3 = TConfigParam("pll_outsource_get", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pog3.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pog3.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pog3.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pog3);

        auto pos4 = TConfigParam("pll_outsource_set", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pos3.addSubParam(TConfigParam("source", "", TConfigParam::TType::TString, ""));
        pos3.addSubParam(TConfigParam("outnum", "0", TConfigParam::TType::TInt, ""));
        pos3.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pos3.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pos3);

        auto pw = TConfigParam("pll_writedefaults", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pw.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pw.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pw);

        auto pllread = TConfigParam("pllread", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pllread.addSubParam(TConfigParam("pllnum", "0", TConfigParam::TType::TInt, ""));
        pllread.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pllread.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pllread);

        auto pllwrite = TConfigParam("pllwrite", "", TConfigParam::TType::TDummy, WRITE_ONLY_STRING);
        pllwrite.addSubParam(TConfigParam("pllnum", "0", TConfigParam::TType::TInt, ""));
        pllwrite.addSubParam(TConfigParam("N", "0", TConfigParam::TType::TInt, ""));
        pllwrite.addSubParam(TConfigParam("M", "0", TConfigParam::TType::TInt, ""));
        pllwrite.addSubParam(TConfigParam("outdiv", "1", TConfigParam::TType::TInt, ""));
        pllwrite.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
        pllwrite.addSubParam(TConfigParam("Result", "", TConfigParam::TType::TString, "", true));
        pll.addSubParam(pllwrite);

        postInitParams.addSubParam(pll);

        //TODO: SAM3U IO Control, SPI Program - is it needed?

    } else if (type == TARGET_CW310) {
        //lol, give me 150k CZK to buy a CW310 and I'll fix this
    }
    //postInitParams.addSubParam(TConfigParam("simpleserial_last_sent", "", TConfigParam::TType::TString, "The last raw string read by a simpleserial_read* command"));
    //postInitParams.addSubParam(TConfigParam("simpleserial_last_read", "", TConfigParam::TType::TString, "The last raw string written via simpleserial_write"));

    return postInitParams;

}

bool TnewaeDevice::_validatePostInitParamsStructure(TConfigParam & params){
    bool ok = true;

    if (type == TARGET_NORMAL) {
        bool ok2;

        int val = params.getSubParamByName("baud")->getValue().toInt(&ok2);

        if (!(val >= 500 && val <= 2000000))
            ok = false;

        if (!ok || !ok2)
            params.getSubParamByName("baud")->setState(TConfigParam::TState::TWarning);
    }

    return ok;
}

void TnewaeDevice::deInit(bool *ok/* = nullptr*/){
    m_postInitParams = updatePostInitParams(m_postInitParams);

    bool succ = true;
    if (m_initialized){
        m_initialized = false;

        QString toSend;
        QList<QString> params;
        plugin->packageDataForPython(cwId, "T-DEINI", 0, params, toSend);
        succ = plugin->writeToPython(cwId, toSend, true);
        succ &= plugin->waitForPythonTargetDone(cwId);
    }

    if(ok != nullptr) *ok = succ;
}

TConfigParam TnewaeDevice::getPostInitParams() const{
    if(!m_initialized){
        //qWarning("Device not initalized! (get)");
        return m_postInitParams;
    }

    TConfigParam params = m_postInitParams;
    return updatePostInitParams(params);
}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){
    if(!m_initialized){
        qWarning("Device not initalized! (set)");
        return m_postInitParams;
    }

    bool ok = true;
    ok = _validatePostInitParamsStructure(params);
    m_postInitParams.resetState();
    if (ok) {
        m_postInitParams = updatePostInitParams(params, true);
        m_postInitParams = updatePostInitParams(m_postInitParams);
    } else {
        m_postInitParams = params;
        qWarning("Post init params vadiation for target not successful, nothing was stored");
    }
    return m_postInitParams;
}

size_t TnewaeDevice::writeData(const uint8_t * buffer, size_t len){
    if (!m_initialized)
        return 0;

    if (len == 0)
        return 0;

    bool ok;
    QString out;
    size_t lenOut;

    if (type == TARGET_NORMAL) {
        ok = plugin->runPythonFunctionWithBinaryDataAsOneArgumentAndGetStringOutput(cwId, "write", (char*) buffer, len, lenOut, out, true);
    } else {
        QString addr = m_postInitParams.getSubParamByName("FPGA write address")->getValue();
        QByteArray packed;
        packed.append(addr.toLocal8Bit());              // ASCII address
        packed.append(fieldSeparator);
        packed.append(QByteArray::fromRawData((char*) buffer, len));
        ok = plugin->runPythonFunctionWithBinaryDataAsOneArgumentAndGetStringOutput(cwId, "fpga_write", packed.data(), packed.size(), lenOut, out, true);
    }

    if (!ok)
        return 0;
    return len;
}

void TnewaeDevice::performHardwareRead() {
    QByteArray tmp;
    tmp.resize(4096); // read in chunks of up to 4 KiB per call

    size_t size = 0;
    bool ok = false;

    if (type == TARGET_NORMAL) {
        ok = plugin->readFromTarget(cwId, &size, reinterpret_cast<uint8_t*>(tmp.data()), tmp.size(), "read");
    } else {
        QString func = m_postInitParams.getSubParamByName("Function to use when reading from the FPGA")->getValue();

        if (func == "fpga_read") {
            QString addrStr = m_postInitParams.getSubParamByName("FPGA read address")->getValue();
            unsigned long long addr = addrStr.toULongLong();

            ok = plugin->readFromTarget(cwId, &size, reinterpret_cast<uint8_t*>(tmp.data()), tmp.size(), func, addr);
        } else {
            ok = plugin->readFromTarget(cwId, &size, reinterpret_cast<uint8_t*>(tmp.data()), tmp.size(), func);
        }
    }

    if (ok && size > 0)
        m_readBuffer.append(tmp.constData(), size);
}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){
    if (!m_initialized)
        return 0;
    QMutexLocker locker(&m_readMutex);

    if (!m_lastReadTimer.isValid())
        m_lastReadTimer.start();

    if (m_lastReadTimer.elapsed() >= TIMER_READ_INTERVAL) {
        m_lastReadTimer.restart();
        performHardwareRead();
    }

    const size_t toCopy = std::min<size_t>(len, m_readBuffer.size());
    if (toCopy == 0)
        return 0;

    std::memcpy(buffer, m_readBuffer.constData(), toCopy);
    m_readBuffer.remove(0, toCopy);

    return toCopy;
}

std::optional<size_t> TnewaeDevice::availableBytes(){
    return std::nullopt;
}
