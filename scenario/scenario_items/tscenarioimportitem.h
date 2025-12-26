// COPYRIGHT HEADER BEGIN
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
// Petr Socha (initial author)
// COPYRIGHT HEADER END

#ifndef TSCENARIOIMPORTITEM_H
#define TSCENARIOIMPORTITEM_H

#include <QString>
#include <QByteArray>
#include <QByteArrayView>
#include <QSharedPointer>
#include <QHash>
#include <QDataStream>
#include <limits>

#include "../tscenarioitem.h"
#include "../tscenarioitemport.h"
#include "../../eximport/thdfsession.h"

/*!
 * \brief TScenarioImportItem reads bytes from an existing HDF5 dataset using THdfSession::readDataset().
 *
 * Design:
 * - lengthIn specifies how many BYTES to output (exactly).
 * - HDF reads happen in WHOLE ELEMENTS; we keep a byte cache to satisfy arbitrary byte lengths.
 * - Read cursor is tracked in ELEMENTS (flat index). Rank-2 is row-major linearization.
 * - Shape rules:
 *    cols = 0 -> require rank-1
 *    cols >=1 -> require rank-2 and dims[1]==cols and maxDims[1]==cols
 * - On insufficient remaining data -> runtime error (visible immediately).
 */
class TScenarioImportItem : public TScenarioItem
{
public:
    TItemClass itemClass() const override
    {
        return TItemClass::TScenarioImportItem;
    }

    TScenarioImportItem()
        : TScenarioItem(tr("Import data"), tr("Read bytes from an existing HDF5 dataset."))
    {
        addFlowInputPort("flowIn");

        addDataInputPort("lengthIn", "length", tr("Number of bytes to read, as integer."), "[unsigned long long]");
        addDataOutputPort("dataOut", "data", tr("Byte array with read data."), "[byte array]");

        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        // Parameters
        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

        m_params.addSubParam(TConfigParam("Block name", "Import data",
                                          TConfigParam::TType::TString,
                                          tr("Display name of the block."), false));

        m_params.addSubParam(TConfigParam("HDF file path", "",
                                          TConfigParam::TType::TFileName,
                                          tr("Path to the HDF5 file to read from."), false));

        m_params.addSubParam(TConfigParam("Dataset path", "/export/data",
                                          TConfigParam::TType::TString,
                                          tr("Absolute dataset path inside the HDF5 file."), false));

        TConfigParam typeParam("Datatype", "uint8",
                               TConfigParam::TType::TEnum,
                               tr("Dataset element datatype (must match the existing dataset exactly)."), false);
        typeParam.addEnumValue("uint8");
        typeParam.addEnumValue("int8");
        typeParam.addEnumValue("uint16");
        typeParam.addEnumValue("int16");
        typeParam.addEnumValue("uint32");
        typeParam.addEnumValue("int32");
        typeParam.addEnumValue("float32");
        typeParam.addEnumValue("float64");
        m_params.addSubParam(typeParam);

        m_params.addSubParam(TConfigParam("Columns", "0",
                                          TConfigParam::TType::TULongLong,
                                          tr("cols=0 => rank-1 dataset. cols>=1 => rank-2 dataset with fixed second dim == cols."), false));

        // Optional: you can remove this param if you want strictly sequential always-from-start.
        m_params.addSubParam(TConfigParam("Reset cursor on prepare", "true",
                                          TConfigParam::TType::TBool,
                                          tr("If true, cursor resets to start when scenario starts (prepare)."), false));

        m_params.addSubParam(TConfigParam("Initial byte offset", "0",
                                          TConfigParam::TType::TULongLong,
                                          tr("Starting position in BYTES from the beginning of the dataset (flat, row-major for rank-2). Applied when cursor is reset/initialized."), false));

        m_session = QSharedPointer<THdfSession>::create();
        resetRuntimeState();

        setState(TState::TError, tr("Block requires configuration."));
        updateAppearanceFromParams();
    }

    TScenarioImportItem(const TScenarioImportItem &x)
        : TScenarioItem(x)
    {
        m_session = QSharedPointer<THdfSession>::create();
        resetRuntimeState();
        updateAppearanceFromParams();
    }

    TScenarioItem * copy() const override
    {
        return new TScenarioImportItem(*this);
    }

    const QString getIconResourcePath() const override
    {
        return ":/icons/import.png"; // adjust if you have a resource
    }

    bool validateParamsStructure(TConfigParam params) override
    {
        bool ok = false;

        params.getSubParamByName("Block name", &ok); if (!ok) return false;
        params.getSubParamByName("HDF file path", &ok); if (!ok) return false;
        params.getSubParamByName("Dataset path", &ok); if (!ok) return false;
        params.getSubParamByName("Datatype", &ok); if (!ok) return false;
        params.getSubParamByName("Columns", &ok); if (!ok) return false;
        params.getSubParamByName("Reset cursor on prepare", &ok); if (!ok) return false;
        params.getSubParamByName("Initial byte offset", &ok); if (!ok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override
    {
        if (!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the params."));
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        updateAppearanceFromParams();

        // Validate now so user sees issues before running
        validateAndMaybeOpenSession(/*setStateOnItem*/true);

        return m_params;
    }

    bool shouldUpdateParams(TConfigParam) override { return false; }
    void updateParams(bool = true) override {}

    bool prepare() override
    {
        resetState(true);

        // DO reset derived state; DON'T reset cursor/cache unconditionally
        resetDerivedState();

        if (!validateAndMaybeOpenSession(/*setStateOnItem*/true))
            return false;

        const QString keyNow = cursorContextKey();
        if (m_cursorInitialized && !m_cursorContextKey.isEmpty() && keyNow != m_cursorContextKey) {
            // Params changed in a way that makes cursor meaningless => force reset behavior
            m_cursorInitialized = false;
            m_cache.clear();
            m_warnedOffsetIgnored = false;
            m_warnedOffsetMisaligned = false;
            // m_nextElem will be set by initCursorFromOffset() below
        }
        m_cursorContextKey = keyNow;

        const bool willReset = (!m_cursorInitialized || resetCursorOnPrepare());

        // WARNING: offset will be ignored if cursor stays as-is
        if (!willReset && initialOffsetBytes() != 0) {
            if (!m_warnedOffsetIgnored) {
                setState(TState::TRuntimeWarning,
                         tr("Initial byte offset is set (%1), but cursor is not reset (Reset cursor on prepare=false). Offset is ignored.")
                             .arg(QString::number(initialOffsetBytes())));
                log(tr("Initial byte offset is set (%1), but cursor is not reset (Reset cursor on prepare=false). Offset is ignored.").arg(QString::number(initialOffsetBytes())),
                    TLogLevel::TWarning);
                m_warnedOffsetIgnored = true;
            }
        }

        if (willReset) {
            if (!initCursorFromOffset())
                return false;
        }

        return true;
    }


    bool cleanup() override
    {
        // If the scenario ends with cached bytes, it's not necessarily an error for import.
        // (We may have prefetched extra bytes to satisfy element reads.)
        // So we just close session.
        if (m_session && m_session->isOpen())
            m_session->close();
        return true;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(
        const QHash<TScenarioItemPort *, QByteArray> &inputData) override
    {
        if (!ensurePreparedForExecute())
            return {};

        quint64 wantBytes = 0;
        if (!decodeLength(inputData.value(getItemPortByName("lengthIn")), wantBytes)) {
            setState(TState::TRuntimeError, tr("Invalid lengthIn value."));
            log(tr("Invalid lengthIn value."),
                TLogLevel::TWarning);
            return {};
        }

        if (wantBytes == 0) {
            setState(TState::TRuntimeWarning, tr("Requested to read 0 bytes."));
            log(tr("Requested to read 0 bytes."),
                TLogLevel::TWarning);
            return {};
        }

        QByteArray out;
        if (!readBytes(wantBytes, out)) {
            // state already set
            return {};
        }

        QHash<TScenarioItemPort*, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), out);
        return outputData;
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> &inputData) override
    {
        if (!ensurePreparedForExecute()) {
            emit executionFinished();
            return;
        }

        quint64 wantBytes = 0;
        if (!decodeLength(inputData.value(getItemPortByName("lengthIn")), wantBytes)) {
            setState(TState::TRuntimeError, tr("Invalid lengthIn value."));
            log(tr("Invalid lengthIn value."),
                TLogLevel::TError);
            emit executionFinished();
            return;
        }

        if (wantBytes == 0) {
            setState(TState::TRuntimeWarning, tr("Requested to read 0 bytes."));
            log(tr("Requested to read 0 bytes."),
                TLogLevel::TWarning);
            emit executionFinished();
            return;
        }

        QByteArray out;
        if (!readBytes(wantBytes, out)) {
            emit executionFinished();
            return;
        }

        QHash<TScenarioItemPort*, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), out);
        emit executionFinished(outputData);
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override
    {
        return (m_state == TState::TRuntimeError)
        ? getItemPortByName("flowOutError")
        : getItemPortByName("flowOut");
    }

private:
    // ---- param helpers ----
    QString hdfPath() const
    {
        auto *p = m_params.getSubParamByName("HDF file path");
        return p ? p->getValue().trimmed() : QString();
    }

    QString datasetPath() const
    {
        auto *p = m_params.getSubParamByName("Dataset path");
        return p ? p->getValue().trimmed() : QString();
    }

    QString typeText() const
    {
        auto *p = m_params.getSubParamByName("Datatype");
        return p ? p->getValue().trimmed() : QString();
    }

    quint64 colsParam() const
    {
        bool ok = false;
        auto *p = m_params.getSubParamByName("Columns");
        const quint64 v = p ? p->getValue().toULongLong(&ok) : 0ULL;
        return ok ? v : 0ULL;
    }

    bool resetCursorOnPrepare() const
    {
        auto *p = m_params.getSubParamByName("Reset cursor on prepare");
        return p ? (p->getValue().trimmed().toLower() == "true") : true;
    }

    quint64 initialOffsetBytes() const
    {
        bool ok = false;
        auto *p = m_params.getSubParamByName("Initial byte offset");
        const quint64 v = p ? p->getValue().toULongLong(&ok) : 0ULL;
        return ok ? v : 0ULL;
    }


    static quint64 elementBytesForTypeText(const QString &t)
    {
        const QString s = t.trimmed().toLower();
        if (s == "uint8" || s == "int8") return 1;
        if (s == "uint16" || s == "int16") return 2;
        if (s == "uint32" || s == "int32" || s == "float32") return 4;
        if (s == "float64") return 8;
        return 0;
    }

    void updateAppearanceFromParams()
    {
        const QString newTitle = m_params.getSubParamByName("Block name")->getValue();
        if (m_title != newTitle) {
            m_title = newTitle;
            emit appearanceChanged();
        }

        const QString ds = datasetPath();
        m_subtitle = ds.isEmpty() ? tr("no dataset") : ds;
        emit appearanceChanged();
    }

    void resetDerivedState()
    {
        m_elementBytes = 0;
        m_colsEff = 0;
        m_rank = 0;
        m_preparedOnce = false;
    }

    void resetRuntimeState() {
        m_preparedOnce = false;
        m_cursorInitialized = false;
        m_warnedOffsetIgnored = false;
        m_warnedOffsetMisaligned = false;
        m_elementBytes = 0;
        m_colsEff = 0;
        m_rank = 0;
        m_nextElem = 0;
        m_cache.clear();
    }

    bool ensurePreparedForExecute()
    {
        if (m_preparedOnce)
            return true;

        if (!prepare())
            return false;

        m_preparedOnce = true;
        return true;
    }

    static bool decodeLength(const QByteArray &raw, quint64 &outLen)
    {
        outLen = 0;
        if (raw.isEmpty())
            return false;

        QDataStream ds(raw);
        ds.setByteOrder(QDataStream::LittleEndian);

        // lengthIn is described as "[unsigned long long]"
        // In Qt, quint64 matches that expectation.
        ds >> outLen;
        return (ds.status() == QDataStream::Ok);
    }

    bool initCursorFromOffset()
    {
        if (m_elementBytes == 0) {
            setState(TState::TRuntimeError, tr("Internal error: element size is 0."));
            log(tr("Internal error: element size is 0."),
                TLogLevel::TError);
            return false;
        }

        const quint64 offBytes = initialOffsetBytes();

        // Compute element cursor + in-element remainder
        const quint64 elemStart = offBytes / m_elementBytes;
        const quint64 remBytes  = offBytes % m_elementBytes;

        if (remBytes != 0) {
            if (!m_warnedOffsetMisaligned) {
                setState(TState::TRuntimeWarning,
                         tr("Initial byte offset (%1) is not aligned to element size (%2). Import will start mid-element.")
                             .arg(QString::number(offBytes))
                             .arg(QString::number(m_elementBytes)));
                log(tr("Initial byte offset (%1) is not aligned to element size (%2). Import will start mid-element.").arg(QString::number(offBytes)).arg(QString::number(m_elementBytes)),
                    TLogLevel::TWarning);
                m_warnedOffsetMisaligned = true;
            }
        }

        // Validate offset is not past end (byte-precise check)
        quint64 totalElems = 0;
        if (!currentTotalElements(totalElems)) {
            setState(TState::TRuntimeError, tr("Failed to query dataset size."));
            log(tr("Failed to query dataset size."),
                TLogLevel::TError);
            return false;
        }

        // total bytes available in dataset
        // (overflow-safe because totalElems is quint64 and m_elementBytes small, but still guard)
        if (totalElems > 0 && m_elementBytes > (std::numeric_limits<quint64>::max() / totalElems)) {
            setState(TState::TRuntimeError, tr("Dataset size overflow."));
            log(tr("Dataset size overflow."),
                TLogLevel::TError);
            return false;
        }
        const quint64 totalBytes = totalElems * m_elementBytes;

        if (offBytes > totalBytes) {
            setState(TState::TRuntimeError,
                     tr("Initial byte offset (%1) is past end of dataset (%2 bytes).")
                         .arg(QString::number(offBytes))
                         .arg(QString::number(totalBytes)));
            log(tr("Initial byte offset (%1) is past end of dataset (%2 bytes).").arg(QString::number(offBytes)).arg(QString::number(totalBytes)),
                TLogLevel::TError);
            return false;
        }

        // Set cursor and clear cache
        m_nextElem = elemStart;
        m_cache.clear();

        if (remBytes == 0) {
            m_cursorInitialized = true;
            return true;
        }

        // Need to start mid-element: read 1 element, drop remBytes, keep remainder in cache
        if (m_nextElem >= totalElems) {
            // offset points exactly at end-of-dataset but with remainder can't happen due to remBytes!=0
            setState(TState::TRuntimeError, tr("Initial offset points past available data."));
            log(tr("Initial offset points past available data."),
                TLogLevel::TError);
            return false;
        }

        QByteArray oneElem;
        if (!readLinearElements(m_nextElem, /*elemCount*/ 1, oneElem)) {
            // state set inside
            return false;
        }

        // Advance cursor because we consumed that element from dataset
        m_nextElem += 1;

        // Drop remBytes from the front, keep the rest
        if (remBytes > quint64(oneElem.size())) {
            setState(TState::TRuntimeError, tr("Internal error: element read smaller than element size."));
            log(tr("Internal error: element read smaller than element size."),
                TLogLevel::TError);
            return false;
        }

        oneElem.remove(0, int(remBytes));
        m_cache.append(oneElem);

        m_cursorInitialized = true;
        return true;
    }


    // Validate dataset + open session. Also sets m_elementBytes/m_colsEff/m_rank.
    bool validateAndMaybeOpenSession(bool setStateOnItem)
    {
        m_params.resetState(true);

        const QString file = hdfPath();
        const QString dset = datasetPath();
        const QString wantType = typeText();

        if (file.isEmpty()) {
            m_params.getSubParamByName("HDF file path")->setState(TConfigParam::TState::TError, tr("HDF file path is empty."));
            if (setStateOnItem) setState(TState::TError, tr("HDF file path is empty."));
            return false;
        }

        if (dset.isEmpty() || !dset.startsWith('/')) {
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Dataset path must be absolute (start with '/')."));
            if (setStateOnItem) setState(TState::TError, tr("Dataset path must be absolute."));
            return false;
        }

        m_elementBytes = elementBytesForTypeText(wantType);
        if (m_elementBytes == 0) {
            m_params.getSubParamByName("Datatype")->setState(TConfigParam::TState::TError, tr("Unsupported datatype: %1").arg(wantType));
            if (setStateOnItem) setState(TState::TError, tr("Unsupported datatype."));
            return false;
        }

        m_colsEff = colsParam(); // raw user value (0 => rank-1, >=1 => rank-2)
        m_rank = (m_colsEff == 0) ? 1 : 2;

        if (!m_session)
            m_session = QSharedPointer<THdfSession>::create();

        if (m_session->isOpen())
            m_session->close();

        if (!m_session->openExisting(file)) {
            m_params.getSubParamByName("HDF file path")->setState(TConfigParam::TState::TError, tr("Failed to open the HDF5 file."));
            if (setStateOnItem) setState(TState::TError, tr("Failed to open HDF5 file."));
            return false;
        }

        const QString pNorm = THdfSession::normalizePath(dset);

        if (!m_session->isDataset(pNorm)) {
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Target path is not a dataset."));
            if (setStateOnItem) setState(TState::TError, tr("Target path is not a dataset."));
            return false;
        }

        const auto info = m_session->datasetInfo(pNorm);
        if (!info.valid) {
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Failed to query dataset info."));
            if (setStateOnItem) setState(TState::TError, tr("Failed to query dataset info."));
            return false;
        }

        // Shape rules EXACTLY as requested
        if (m_colsEff == 0) {
            if (info.rank != 1 || info.dims.size() < 1 || info.maxDims.size() < 1) {
                m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Columns=0 requires a rank-1 dataset."));
                if (setStateOnItem) setState(TState::TError, tr("Rank mismatch (need rank-1)."));
                return false;
            }
        } else {
            if (info.rank != 2 || info.dims.size() < 2 || info.maxDims.size() < 2) {
                m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError,
                                                                     tr("Columns=%1 requires a rank-2 dataset.").arg(QString::number(m_colsEff)));
                if (setStateOnItem) setState(TState::TError, tr("Rank mismatch (need rank-2)."));
                return false;
            }
            if (quint64(info.dims[1]) != m_colsEff) {
                m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError,
                                                                     tr("Dataset column count (%1) does not match Columns (%2).")
                                                                         .arg(QString::number(quint64(info.dims[1])))
                                                                         .arg(QString::number(m_colsEff)));
                if (setStateOnItem) setState(TState::TError, tr("Dataset column mismatch."));
                return false;
            }
            if (quint64(info.maxDims[1]) != m_colsEff) {
                m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError,
                                                                     tr("Dataset max column count (%1) must equal Columns (%2).")
                                                                         .arg(QString::number(quint64(info.maxDims[1])))
                                                                         .arg(QString::number(m_colsEff)));
                if (setStateOnItem) setState(TState::TError, tr("Dataset max column mismatch."));
                return false;
            }
        }

        if (!m_session->datasetMatchesTypeText(pNorm, wantType)) {
            const QString got = m_session->datasetTypeText(pNorm);
            m_params.getSubParamByName("Datatype")->setState(TConfigParam::TState::TError,
                                                             tr("Datatype mismatch: dataset is %1, user declares %2.")
                                                                 .arg(got.isEmpty() ? tr("(unknown)") : got)
                                                                 .arg(wantType));
            if (setStateOnItem) setState(TState::TError, tr("Datatype mismatch."));
            return false;
        }

        m_params.setState(TConfigParam::TState::TOk);
        if (setStateOnItem) resetState();

        return true;
    }

    // Returns total elements currently available in the dataset (re-queried each call).
    bool currentTotalElements(quint64 &outTotalElems) const
    {
        outTotalElems = 0;

        if (!m_session || !m_session->isOpen())
            return false;

        const QString pNorm = THdfSession::normalizePath(datasetPath());
        const auto info = m_session->datasetInfo(pNorm);
        if (!info.valid || info.dims.isEmpty())
            return false;

        if (m_colsEff == 0) {
            outTotalElems = quint64(info.dims[0]);
            return true;
        }

        // rank-2
        if (info.dims.size() < 2)
            return false;

        const quint64 rows = quint64(info.dims[0]);
        const quint64 cols = quint64(info.dims[1]);
        if (cols == 0)
            return false;

        // overflow safe: rows*cols
        if (rows > 0 && cols > (std::numeric_limits<quint64>::max() / rows))
            return false;

        outTotalElems = rows * cols;
        return true;
    }

    // Read enough elements to fill cache up to at least wantBytes total cached.
    bool ensureCacheHas(quint64 wantBytes)
    {
        if (m_cache.size() >= 0 && quint64(m_cache.size()) >= wantBytes)
            return true;

        if (m_elementBytes == 0) {
            setState(TState::TRuntimeError, tr("Internal error: element size is 0."));
            log(tr("Internal error: element size is 0."),
                TLogLevel::TError);
            return false;
        }

        quint64 totalElems = 0;
        if (!currentTotalElements(totalElems)) {
            setState(TState::TRuntimeError, tr("Failed to query dataset size."));
            log(tr("Failed to query dataset size."),
                TLogLevel::TError);
            return false;
        }

        // Remaining elements from cursor
        if (m_nextElem > totalElems) {
            setState(TState::TRuntimeError, tr("Read cursor is past end of dataset."));
            log(tr("Read cursor is past end of dataset."),
                TLogLevel::TError);
            return false;
        }

        const quint64 remainElems = totalElems - m_nextElem;
        if (remainElems > 0 && m_elementBytes > (std::numeric_limits<quint64>::max() / remainElems)) {
            setState(TState::TRuntimeError, tr("Remaining dataset size overflow."));
            log(tr("Remaining dataset size overflow."),
                TLogLevel::TError);
            return false;
        }
        const quint64 remainBytes = remainElems * m_elementBytes;

        const quint64 cachedBytes = quint64(m_cache.size());
        const quint64 needBytes = (wantBytes > cachedBytes) ? (wantBytes - cachedBytes) : 0;

        if (needBytes == 0)
            return true;

        if (needBytes > remainBytes) {
            setState(TState::TRuntimeError,
                     tr("Not enough data in dataset to satisfy request. Need %1 bytes, have %2 bytes remaining.")
                         .arg(QString::number(needBytes))
                         .arg(QString::number(remainBytes)));
            log(tr("Not enough data in dataset to satisfy request. Need %1 bytes, have %2 bytes remaining.").arg(QString::number(needBytes)).arg(QString::number(remainBytes)),
                TLogLevel::TError);
            return false;
        }

        // Read ceil(needBytes / elementBytes) elements
        quint64 needElems = needBytes / m_elementBytes;
        if ((needBytes % m_elementBytes) != 0)
            needElems += 1;

        // Clamp to remaining (should already fit due to check above)
        if (needElems > remainElems)
            needElems = remainElems;

        QByteArray readOut;
        if (!readLinearElements(m_nextElem, needElems, readOut)) {
            // state set inside
            return false;
        }

        if ((quint64(readOut.size()) % m_elementBytes) != 0) {
            setState(TState::TRuntimeError, tr("Internal error: read size is not aligned to element size."));
            log(tr("Internal error: read size is not aligned to element size."), TLogLevel::TError);
            return false;
        }

        // Advance cursor by elements actually read
        const quint64 readElems = quint64(readOut.size()) / m_elementBytes;
        m_nextElem += readElems;

        m_cache.append(readOut);
        return true;
    }

    // Reads exactly wantBytes into out (byte-precise) using cache + element reads.
    bool readBytes(quint64 wantBytes, QByteArray &out)
    {
        out.clear();

        if (!ensureCacheHas(wantBytes))
            return false;

        if (wantBytes > quint64(std::numeric_limits<int>::max())) {
            setState(TState::TRuntimeError, tr("Requested read too large."));
            log(tr("Requested read too large."),
                TLogLevel::TError);
            return false;
        }

        out = m_cache.left(int(wantBytes));
        m_cache.remove(0, int(wantBytes));

        setState(TState::TRuntimeInfo,
                 tr("Read %1 bytes. Cursor=%2 elems. Cache=%3 bytes.")
                     .arg(QString::number(wantBytes))
                     .arg(QString::number(m_nextElem))
                     .arg(QString::number(m_cache.size())));
        //log(tr("Read %1 bytes. Cursor=%2 elems. Cache=%3 bytes.").arg(QString::number(wantBytes)).arg(QString::number(m_nextElem)).arg(QString::number(m_cache.size())),
        //    TLogLevel::TInfo);

        return true;
    }

    // Reads elemCount elements starting at flatElemStart (row-major for rank-2).
    bool readLinearElements(quint64 flatElemStart, quint64 elemCount, QByteArray &out)
    {
        out.clear();

        if (!m_session || !m_session->isOpen()) {
            setState(TState::TRuntimeError, tr("No open HDF session."));
            log(tr("No open HDF session."),
                TLogLevel::TError);
            return false;
        }

        if (elemCount == 0)
            return true;

        const QString pNorm = THdfSession::normalizePath(datasetPath());

        if (m_colsEff == 0) {
            // rank-1 direct contiguous read
            QVector<quint64> dims;
            QString logOut;
            if (!m_session->readDataset(pNorm, flatElemStart, elemCount, out, &dims, &logOut)) {
                setState(TState::TRuntimeError, logOut.isEmpty() ? tr("Read failed.") : logOut);
                log(logOut.isEmpty() ? tr("Read failed.") : logOut,
                    TLogLevel::TError);
                return false;
            }
            return true;
        }

        // rank-2: loop by rows/partials because THdfSession::readDataset rank-2 is rectangular only.
        const quint64 cols = m_colsEff;

        quint64 remaining = elemCount;
        quint64 cur = flatElemStart;

        while (remaining > 0) {
            const quint64 row = cur / cols;
            const quint64 col = cur % cols;

            // how many elements can we read contiguously in this row from 'col'?
            quint64 take = cols - col;
            if (take > remaining)
                take = remaining;

            QByteArray chunk;
            QVector<quint64> dims;
            QString logOut;

            if (!m_session->readDataset(pNorm,
                                        /*rowStart*/ row,
                                        /*rowCount*/ 1,
                                        /*colStart*/ col,
                                        /*colCount*/ take,
                                        chunk,
                                        &dims,
                                        &logOut)) {
                setState(TState::TRuntimeError, logOut.isEmpty() ? tr("Read failed.") : logOut);
                log(logOut.isEmpty() ? tr("Read failed.") : logOut,
                    TLogLevel::TError);
                return false;
            }

            out.append(chunk);

            cur += take;
            remaining -= take;
        }

        return true;
    }

    QString cursorContextKey() const
    {
        // Normalize what matters for interpreting m_nextElem + cache
        return QString("%1|%2|%3|%4")
            .arg(hdfPath().trimmed())
            .arg(THdfSession::normalizePath(datasetPath()))
            .arg(typeText().trimmed().toLower())
            .arg(QString::number(colsParam()));
    }


private:
    QSharedPointer<THdfSession> m_session;

    // Derived from params / dataset
    quint64 m_elementBytes = 0;
    quint64 m_colsEff = 0; // raw cols param: 0 => rank-1, >=1 => rank-2
    int     m_rank = 0;

    // Read cursor + cache
    quint64  m_nextElem = 0; // next element index to read (flat)
    QByteArray m_cache;

    bool m_preparedOnce = false;
    bool m_cursorInitialized = false;
    bool m_warnedOffsetIgnored = false;
    bool m_warnedOffsetMisaligned = false;

    QString m_cursorContextKey;

};

#endif // TSCENARIOIMPORTITEM_H
