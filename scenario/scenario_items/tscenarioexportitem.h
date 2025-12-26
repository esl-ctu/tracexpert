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

#ifndef TSCENARIOEXPORTITEM_H
#define TSCENARIOEXPORTITEM_H

#include <QString>
#include <QByteArray>
#include <QByteArrayView>
#include <QSharedPointer>
#include <QHash>
#include <QDataStream>
#include <limits>

#include "../tscenarioitem.h"
#include "../tscenarioitemport.h"
#include "../../eximport/thdfsession.h" // adjust include path to your project

/*!
 * \brief TScenarioExportItem represents a Scenario block that appends incoming byte payload
 *        into an existing HDF5 dataset using THdfSession::appendRawSlice().
 *
 * Design:
 * - Input data can arrive in arbitrary sizes (even less than a full element or row).
 * - Writes to HDF are always full rows (rank-2) or full elements (rank-1 treated as cols=1).
 * - Cache holds bytes until at least one full row is available.
 * - Flush threshold is chosen automatically from dataset chunking (chunkRows * multiplier).
 */
class TScenarioExportItem : public TScenarioItem
{
public:
    TItemClass itemClass() const override
    {
        return TItemClass::TScenarioExportItem;
    }

    TScenarioExportItem()
        : TScenarioItem(tr("Export data"), tr("Append data into an existing HDF5 dataset."))
    {
        addFlowInputPort("flowIn");
        addDataInputPort("dataIn", "data", tr("Byte array payload to append."), "[byte array]");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

        m_params.addSubParam(TConfigParam("Block name", "Export data",
                                          TConfigParam::TType::TString,
                                          tr("Display name of the block."), false));

        m_params.addSubParam(TConfigParam("HDF file path", "",
                                          TConfigParam::TType::TFileName,
                                          tr("Path to the HDF5 file to append into."), false));

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

        m_params.addSubParam(TConfigParam("Chunk flush multiplier", "1",
                                          TConfigParam::TType::TUInt,
                                          tr("Flush threshold in full rows: chunkRows * multiplier."), false));

        m_session = QSharedPointer<THdfSession>::create();
        resetRuntimeState();
        setState(TState::TError, tr("Block requires configuration."));
        updateAppearanceFromParams();
    }

    TScenarioExportItem(const TScenarioExportItem &x)
        : TScenarioItem(x)
    {
        m_session = QSharedPointer<THdfSession>::create();

        resetRuntimeState();
        updateAppearanceFromParams();
    }

    TScenarioItem * copy() const override
    {
        return new TScenarioExportItem(*this);
    }

    const QString getIconResourcePath() const override
    {
        return ":/icons/export.png";
    }

    bool validateParamsStructure(TConfigParam params) override
    {
        bool ok = false;

        params.getSubParamByName("Block name", &ok);
        if (!ok) return false;

        params.getSubParamByName("HDF file path", &ok);
        if (!ok) return false;

        params.getSubParamByName("Dataset path", &ok);
        if (!ok) return false;

        params.getSubParamByName("Datatype", &ok);
        if (!ok) return false;

        params.getSubParamByName("Columns", &ok);
        if (!ok) return false;

        params.getSubParamByName("Chunk flush multiplier", &ok);
        if (!ok) return false;

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

        validateAndMaybeOpenSession(/*setStateOnItem*/true);

        return m_params;
    }

    bool shouldUpdateParams(TConfigParam /*newParams*/) override
    {
        return false;
    }

    void updateParams(bool /*paramValuesChanged*/ = true) override
    {

    }

    bool prepare() override
    {
        resetState(true);
        resetRuntimeState();

        if (!validateAndMaybeOpenSession(/*setStateOnItem*/true))
            return false;

        computeFlushPolicyFromDataset();

        return true;
    }

    bool cleanup() override
    {
        const bool flushedOk = flushIfPossible(/*forceFullRows*/true);

        if (m_session && m_session->isOpen())
            m_session->close();

        if (!flushedOk)  // flush failed -> state already set
            return false;

        if (!m_cache.isEmpty()) {
            setState(TState::TRuntimeError,
                     tr("Export finished with %1 cached bytes that do not form a full row; data were not written.")
                         .arg(QString::number(m_cache.size())));
            log(tr("Export finished with %1 cached bytes that do not form a full row; data were not written.").arg(QString::number(m_cache.size())),
                TLogLevel::TError);
            return false;
        }
        return true;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(
        const QHash<TScenarioItemPort *, QByteArray> &inputData) override
    {
        if (!ensurePreparedForExecute()) {
            return {};
        }

        const QByteArray payload = inputData.value(getItemPortByName("dataIn"));
        if (payload.isEmpty()) {
            setState(TState::TRuntimeWarning, tr("Requested to export 0 bytes."));
            return {};
        }

        if (!appendIntoCacheAndFlush(payload)) {
            // Error state already set.
            return {};
        }

        return {};
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> &inputData) override
    {
        if (!ensurePreparedForExecute()) {
            emit executionFinished();
            return;
        }

        const QByteArray payload = inputData.value(getItemPortByName("dataIn"));
        if (payload.isEmpty()) {
            setState(TState::TRuntimeWarning, tr("Requested to export 0 bytes."));
            emit executionFinished();
            return;
        }

        const bool ok = appendIntoCacheAndFlush(payload);
        Q_UNUSED(ok);
        emit executionFinished(QHash<TScenarioItemPort *, QByteArray>());
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override
    {
        return (m_state == TState::TRuntimeError)
        ? getItemPortByName("flowOutError")
        : getItemPortByName("flowOut");
    }

private:

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
        const quint64 v = m_params.getSubParamByName("Columns")->getValue().toULongLong(&ok);
        return ok ? v : 0ULL;
    }

    quint32 flushMultParam() const
    {
        bool ok = false;
        const quint32 v = m_params.getSubParamByName("Chunk flush multiplier")->getValue().toUInt(&ok);
        return ok ? v : 1U;
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

    void resetRuntimeState()
    {
        m_preparedOnce = false;

        m_cache.clear();
        m_cachedBytesTotal = 0;

        m_elementBytes = 0;
        m_colsEff = 0;
        m_rowBytes = 0;

        m_flushRowsThreshold = 0;
    }

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

        const quint64 cols = colsParam();      // RAW user value, no normalization
        m_colsEff = cols;                      // keep raw (name kept to avoid wider refactor)

        const bool rank1Mode = (m_colsEff == 0);
        if (rank1Mode) {
            m_rowBytes = m_elementBytes;       // one element == one "row" in rank-1 mode
        } else {
            m_rowBytes = m_colsEff * m_elementBytes; // one row = cols elements
        }

        if (m_rowBytes == 0) {
            m_params.setState(TConfigParam::TState::TError, tr("Invalid row size."));
            if (setStateOnItem) setState(TState::TError, tr("Invalid row size."));
            return false;
        }

        if (!m_session)
            m_session = QSharedPointer<THdfSession>::create();

        if (m_session->isOpen())
            m_session->close();

        if (!m_session->openExisting(file)) {
            m_params.setState(TConfigParam::TState::TError, tr("Failed to open the HDF5 file."));
            if (setStateOnItem) setState(TState::TError, tr("Failed to open HDF5 file."));
            return false;
        }

        // Validate dataset existence + compatibility (strict).
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

        // Must be extendible + chunked
        if (info.maxDims.isEmpty() || info.maxDims[0] != H5S_UNLIMITED) {
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Dataset is not unlimited along the first dimension."));
            if (setStateOnItem) setState(TState::TError, tr("Dataset not appendable (dim0 not unlimited)."));
            return false;
        }
        if (!info.chunked || info.chunkDims.isEmpty()) {
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError, tr("Dataset is not chunked (cannot append)."));
            if (setStateOnItem) setState(TState::TError, tr("Dataset not chunked (cannot append)."));
            return false;
        }

        // Rank compatibility:
        // cols==0  => rank-1
        // cols>=1  => rank-2, dims[1]==cols, maxDims[1]==cols
        if (m_colsEff == 0) {
            if (info.rank != 1 || info.dims.size() < 1 || info.maxDims.size() < 1) {
                m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError,
                                  tr("Columns=0 requires a rank-1 dataset."));
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
            m_params.getSubParamByName("Dataset path")->setState(TConfigParam::TState::TError,
                              tr("Datatype mismatch: dataset is %1, user declares %2.")
                                  .arg(got.isEmpty() ? tr("(unknown)") : got)
                                  .arg(wantType));
            if (setStateOnItem) setState(TState::TError, tr("Datatype mismatch."));
            return false;
        }

        m_params.setState(TConfigParam::TState::TOk);
        if (setStateOnItem) resetState();

        if (!m_cache.isEmpty()) {
            m_params.setState(TConfigParam::TState::TWarning,
                              tr("Parameters changed while cache is non-empty. Consider resetting the scenario."));
        }

        return true;
    }

    void computeFlushPolicyFromDataset()
    {
        const auto info = m_session->datasetInfo(THdfSession::normalizePath(datasetPath()));
        quint64 chunkRows = 0;

        if (info.valid && info.chunked && !info.chunkDims.isEmpty()) {
            chunkRows = quint64(info.chunkDims[0]);
        }

        if (chunkRows == 0) {
            // Fallback: 256 KiB target, but expressed in FULL ROWS.
            const quint64 targetBytes = 256ULL * 1024ULL;
            chunkRows = qMax<quint64>(1, targetBytes / qMax<quint64>(1, m_rowBytes));
        }

        const quint64 mult = qMax<quint64>(1, flushMultParam());
        m_flushRowsThreshold = qMax<quint64>(1, chunkRows * mult);
    }

    bool ensurePreparedForExecute()
    {

        if (m_preparedOnce)
            return true;

        if (!prepare()) {
            return false;
        }

        m_preparedOnce = true;
        return true;
    }

    bool appendIntoCacheAndFlush(const QByteArray &incoming)
    {
        m_cache.append(incoming);
        m_cachedBytesTotal += quint64(incoming.size());

        return flushIfPossible(/*forceFullRows*/false);
    }

    bool flushIfPossible(bool forceFullRows)
    {
        if (!m_session || !m_session->isOpen()) {
            setState(TState::TRuntimeError, tr("No open HDF session."));
            log(tr("No open HDF session."),
                TLogLevel::TError);
            return false;
        }

        if (m_rowBytes == 0) {
            setState(TState::TRuntimeError, tr("Invalid row size."));
            log(tr("Invalid row size."),
                TLogLevel::TError);
            return false;
        }

        const quint64 cached = quint64(m_cache.size());
        const quint64 fullRowsAvailable = cached / m_rowBytes;

        if (fullRowsAvailable == 0) {
            // nothing to flush yet
            return true;
        }

        quint64 rowsToFlush = 0;
        if (forceFullRows) {
            rowsToFlush = fullRowsAvailable;
        } else {
            if (fullRowsAvailable < m_flushRowsThreshold)
                return true;
            rowsToFlush = (fullRowsAvailable / m_flushRowsThreshold) * m_flushRowsThreshold; // multiple of threshold
            if (rowsToFlush == 0)
                rowsToFlush = fullRowsAvailable;
        }

        const quint64 bytesToFlush = rowsToFlush * m_rowBytes;
        if (bytesToFlush == 0)
            return true;

        QString logOut;
        const bool ok = m_session->appendRawSlice(
            THdfSession::normalizePath(datasetPath()),
            QByteArrayView(m_cache),
            /*startByte*/ 0,
            /*byteCount*/ bytesToFlush,
            /*cols*/ m_colsEff,
            /*typeText*/ typeText(),
            &logOut
            );

        if (!ok) {
            setState(TState::TRuntimeError,
                     logOut.isEmpty() ? tr("Append failed.") : logOut);
            log(logOut.isEmpty() ? tr("Append failed.") : logOut,
                TLogLevel::TError);
            return false;
        }

        if (bytesToFlush > quint64(std::numeric_limits<int>::max())) {
            setState(TState::TRuntimeError,
                     tr("Internal error: flush chunk too large for QByteArray::remove()."));
            log(tr("Internal error: flush chunk too large for QByteArray::remove()."),
                TLogLevel::TError);
            return false;
        }
        m_cache.remove(0, int(bytesToFlush));

        setState(TState::TRuntimeInfo,
                 tr("Flushed %1 bytes (%2 rows). Cache: %3 bytes remaining.")
                     .arg(QString::number(bytesToFlush))
                     .arg(QString::number(rowsToFlush))
                     .arg(QString::number(m_cache.size())));

        return true;
    }

private:
    QSharedPointer<THdfSession> m_session;
    QByteArray m_cache;
    quint64 m_cachedBytesTotal = 0;
    quint64 m_elementBytes = 0;
    quint64 m_colsEff = 0;
    quint64 m_rowBytes = 0;
    quint64 m_flushRowsThreshold = 0;
    bool m_preparedOnce = false;
};

#endif // TSCENARIOEXPORTITEM_H
