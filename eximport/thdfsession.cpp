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

#include "thdfsession.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>
#include <limits>

static bool parseSelectedType_native(const QString& typeText,
                                     H5T_class_t& outClass,
                                     H5T_sign_t& outSign,
                                     size_t& outSize,
                                     hid_t& outNativeHid)
{
    const QString t = typeText.trimmed().toLower();

    if (t == "float32") { outClass = H5T_FLOAT;   outSign = H5T_SGN_NONE; outSize = 4; outNativeHid = H5T_NATIVE_FLOAT;  return true; }
    if (t == "float64") { outClass = H5T_FLOAT;   outSign = H5T_SGN_NONE; outSize = 8; outNativeHid = H5T_NATIVE_DOUBLE; return true; }

    if (t == "uint8")   { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 1; outNativeHid = H5T_NATIVE_UINT8;  return true; }
    if (t == "int8")    { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 1; outNativeHid = H5T_NATIVE_INT8;   return true; }
    if (t == "uint16")  { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 2; outNativeHid = H5T_NATIVE_UINT16; return true; }
    if (t == "int16")   { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 2; outNativeHid = H5T_NATIVE_INT16;  return true; }
    if (t == "uint32")  { outClass = H5T_INTEGER; outSign = H5T_SGN_NONE; outSize = 4; outNativeHid = H5T_NATIVE_UINT32; return true; }
    if (t == "int32")   { outClass = H5T_INTEGER; outSign = H5T_SGN_2;    outSize = 4; outNativeHid = H5T_NATIVE_INT32;  return true; }

    return false;
}

static bool u64_to_hsize(quint64 v, hsize_t& out)
{
    if (v > static_cast<quint64>(std::numeric_limits<hsize_t>::max()))
        return false;
    out = static_cast<hsize_t>(v);
    return true;
}

static std::size_t sampleTypeBytes(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TUInt8:  return 1;
    case TScope::TSampleType::TInt8:   return 1;
    case TScope::TSampleType::TUInt16: return 2;
    case TScope::TSampleType::TInt16:  return 2;
    case TScope::TSampleType::TUInt32: return 4;
    case TScope::TSampleType::TInt32:  return 4;
    case TScope::TSampleType::TReal32: return 4;
    case TScope::TSampleType::TReal64: return 8;
    }
    return 0;
}

static hid_t sampleTypeToNativeH5Type(TScope::TSampleType t)
{
    switch (t) {
    case TScope::TSampleType::TUInt8:  return H5T_NATIVE_UINT8;
    case TScope::TSampleType::TInt8:   return H5T_NATIVE_INT8;
    case TScope::TSampleType::TUInt16: return H5T_NATIVE_UINT16;
    case TScope::TSampleType::TInt16:  return H5T_NATIVE_INT16;
    case TScope::TSampleType::TUInt32: return H5T_NATIVE_UINT32;
    case TScope::TSampleType::TInt32:  return H5T_NATIVE_INT32;
    case TScope::TSampleType::TReal32: return H5T_NATIVE_FLOAT;
    case TScope::TSampleType::TReal64: return H5T_NATIVE_DOUBLE;
    }
    return H5I_INVALID_HID;
}

static bool datasetTypeMatches(hid_t dset, TScope::TSampleType expected)
{
    hid_t t = H5Dget_type(dset);
    if (t < 0)
        return false;

    const H5T_class_t cls = H5Tget_class(t);
    const size_t sz = H5Tget_size(t);

    bool ok = false;
    if (expected == TScope::TSampleType::TReal32 || expected == TScope::TSampleType::TReal64) {
        ok = (cls == H5T_FLOAT) &&
             ((expected == TScope::TSampleType::TReal32 && sz == 4) ||
              (expected == TScope::TSampleType::TReal64 && sz == 8));
    } else {
        if (cls == H5T_INTEGER) {
            const H5T_sign_t sgn = H5Tget_sign(t);
            const bool wantUnsigned =
                (expected == TScope::TSampleType::TUInt8 ||
                 expected == TScope::TSampleType::TUInt16 ||
                 expected == TScope::TSampleType::TUInt32);

            const size_t wantSz =
                (expected == TScope::TSampleType::TUInt8 || expected == TScope::TSampleType::TInt8) ? 1 :
                    (expected == TScope::TSampleType::TUInt16 || expected == TScope::TSampleType::TInt16) ? 2 : 4;

            ok = (sz == wantSz) &&
                 ((wantUnsigned && sgn == H5T_SGN_NONE) || (!wantUnsigned && sgn == H5T_SGN_2));
        }
    }

    H5Tclose(t);
    return ok;
}


THdfSession::THdfSession(QObject *parent)
    : QObject(parent)
{
}

THdfSession::~THdfSession()
{
    close();
}

bool THdfSession::isOpen() const
{
    return m_fileId >= 0;
}

QString THdfSession::filePath() const
{
    return m_filePath;
}

hid_t THdfSession::fileId() const
{
    return m_fileId;
}

void THdfSession::close()
{
    if (m_fileId >= 0) {
        H5Fclose(m_fileId);
        m_fileId = H5I_INVALID_HID;
        m_filePath.clear();
    }
}

bool THdfSession::isLikelyHdf5BySignature(const QString &path)
{
    const QByteArray p = QFileInfo(path).absoluteFilePath().toUtf8();
    const htri_t r = H5Fis_hdf5(p.constData());
    return (r > 0);
}

bool THdfSession::openExisting(const QString &path)
{
    close();

    const QFileInfo fi(path);
    if (!fi.exists()) {
        qCritical() << "[THdfSession] File does not exist:" << path;
        return false;
    }

    if (!isLikelyHdf5BySignature(fi.absoluteFilePath())) {
        qCritical() << "[THdfSession] File is not recognized as HDF5 by signature:" << path;
        return false;
    }

    const QByteArray p = fi.absoluteFilePath().toUtf8();

    hid_t id = H5Fopen(p.constData(), H5F_ACC_RDWR, H5P_DEFAULT);
    if (id < 0) {
        qCritical() << "[THdfSession] Failed to open HDF5 file (corrupt/truncated/locked?):" << path;
        return false;
    }

    m_fileId = id;
    m_filePath = fi.absoluteFilePath();
    return true;
}

bool THdfSession::createNew(const QString &path)
{
    close();

    const QFileInfo fi(path);
    if (fi.exists()) {
        qCritical() << "[THdfSession] Cannot create, file already exists:" << path;
        return false;
    }

    const QByteArray p = fi.absoluteFilePath().toUtf8();
    hid_t id = H5Fcreate(p.constData(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    if (id < 0) {
        qCritical() << "[THdfSession] Failed to create HDF5 file:" << path;
        return false;
    }

    m_fileId = id;
    m_filePath = fi.absoluteFilePath();

    if (H5Fflush(m_fileId, H5F_SCOPE_GLOBAL) < 0) {
        qCritical() << "[THdfSession] Warning: H5Fflush failed after create:" << m_filePath;
    }

    return true;
}

// ---------------- Path helpers ----------------

QString THdfSession::normalizePath(QString p)
{
    p = p.trimmed();
    if (p.isEmpty())
        return p;

    if (!p.startsWith('/'))
        p.prepend('/');

    while (p.contains("//"))
        p.replace("//", "/");

    if (p.size() > 1 && p.endsWith('/'))
        p.chop(1);

    return p;
}

QString THdfSession::parentPath(const QString &p)
{
    const QString n = normalizePath(p);
    if (n.isEmpty() || n == "/")
        return "/";

    const int lastSlash = n.lastIndexOf('/');
    if (lastSlash <= 0)
        return "/";

    return n.left(lastSlash);
}

THdfSession::NodeType THdfSession::nodeType(const QString &path) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] nodeType called but file is not open";
        return NodeType::Error;
    }

    const QString p = normalizePath(path);
    if (p.isEmpty()) {
        return NodeType::Missing;
    }

    const QByteArray pb = p.toUtf8();

    htri_t lexists = -1;
    H5E_BEGIN_TRY {
        lexists = H5Lexists(m_fileId, pb.constData(), H5P_DEFAULT);
    } H5E_END_TRY;

    if (lexists <= 0)
        return NodeType::Missing;

    H5O_info2_t info{};
    const herr_t ok = H5Oget_info_by_name3(
        m_fileId,
        pb.constData(),
        &info,
        H5O_INFO_BASIC,
        H5P_DEFAULT
        );

    if (ok < 0) {
        qCritical() << "[THdfSession] H5Oget_info_by_name3 failed for:" << p;
        return NodeType::Error;
    }

    switch (info.type) {
    case H5O_TYPE_GROUP:   return NodeType::Group;
    case H5O_TYPE_DATASET: return NodeType::Dataset;
    default:               return NodeType::Other;
    }
}

bool THdfSession::pathExists(const QString &path) const
{
    const NodeType t = nodeType(path);
    return (t != NodeType::Missing && t != NodeType::Error);
}

bool THdfSession::isGroup(const QString &path) const
{
    return nodeType(path) == NodeType::Group;
}

bool THdfSession::isDataset(const QString &path) const
{
    return nodeType(path) == NodeType::Dataset;
}

static herr_t iter_names(hid_t /*group*/, const char *name, const H5L_info_t* /*info*/, void *op_data)
{
    auto *out = static_cast<QStringList*>(op_data);
    out->append(QString::fromUtf8(name));
    return 0;
}

QStringList THdfSession::listChildren(const QString &groupPath) const
{
    QStringList out;

    if (!isOpen()) {
        qCritical() << "[THdfSession] listChildren called but file is not open";
        return out;
    }

    const QString gp = normalizePath(groupPath);
    if (!isGroup(gp)) {
        qCritical() << "[THdfSession] listChildren requires group path, got:" << gp;
        return out;
    }

    const QByteArray gb = gp.toUtf8();
    hid_t g = H5Gopen2(m_fileId, gb.constData(), H5P_DEFAULT);
    if (g < 0) {
        qCritical() << "[THdfSession] H5Gopen2 failed for:" << gp;
        return out;
    }

    hsize_t idx = 0;
    const herr_t r = H5Literate(g, H5_INDEX_NAME, H5_ITER_INC, &idx, &iter_names, &out);
    if (r < 0)
        qCritical() << "[THdfSession] H5Literate failed for:" << gp;

    H5Gclose(g);
    return out;
}

QStringList THdfSession::listGroups(const QString &groupPath) const
{
    QStringList groups;
    const QString gp = normalizePath(groupPath);

    for (const QString &name : listChildren(gp)) {
        const QString child = normalizePath(gp + "/" + name);
        if (isGroup(child))
            groups << name;
    }
    return groups;
}

QStringList THdfSession::listDatasets(const QString &groupPath) const
{
    QStringList datasets;
    const QString gp = normalizePath(groupPath);

    for (const QString &name : listChildren(gp)) {
        const QString child = normalizePath(gp + "/" + name);
        if (isDataset(child))
            datasets << name;
    }
    return datasets;
}

bool THdfSession::ensureGroup(const QString &groupPath) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] ensureGroup called but file is not open";
        return false;
    }

    const QString gp = normalizePath(groupPath);
    if (gp.isEmpty() || gp == "/")
        return true;

    QString current;
    const QStringList parts = gp.split('/', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        current += "/" + part;

        const NodeType t = nodeType(current);
        if (t == NodeType::Group)
            continue;

        if (t == NodeType::Dataset || t == NodeType::Other) {
            qCritical() << "[THdfSession] ensureGroup conflict, non-group exists at:" << current;
            return false;
        }
        if (t == NodeType::Error)
            return false;

        const QByteArray cb = current.toUtf8();
        hid_t g = H5Gcreate2(m_fileId, cb.constData(),
                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (g < 0) {
            qCritical() << "[THdfSession] Failed to create group:" << current;
            return false;
        }
        H5Gclose(g);
    }

    if (H5Fflush(m_fileId, H5F_SCOPE_GLOBAL) < 0) {
        qCritical() << "[THdfSession] Warning: H5Fflush failed after ensureGroup";
    }

    return true;
}

bool THdfSession::createGroup(const QString &groupPath) const
{
    return ensureGroup(groupPath);
}

THdfSession::DatasetInfo THdfSession::datasetInfo(const QString &datasetPath) const
{
    DatasetInfo out;

    if (!isOpen())
        return out;

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm))
        return out;

    const QByteArray p = pNorm.toUtf8();
    hid_t dset = H5Dopen2(m_fileId, p.constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] H5Dopen2 failed for:" << pNorm;
        return out;
    }

    hid_t space = H5Dget_space(dset);
    hid_t type  = H5Dget_type(dset);
    if (space < 0 || type < 0) {
        qCritical() << "[THdfSession] Failed to get space/type for:" << pNorm;
        if (type >= 0) H5Tclose(type);
        if (space >= 0) H5Sclose(space);
        H5Dclose(dset);
        return out;
    }

    const int rank = H5Sget_simple_extent_ndims(space);
    if (rank < 0) {
        qCritical() << "[THdfSession] Failed to get rank for:" << pNorm;
        H5Tclose(type);
        H5Sclose(space);
        H5Dclose(dset);
        return out;
    }

    out.rank = rank;
    out.dims.resize(rank);
    out.maxDims.resize(rank);
    if (H5Sget_simple_extent_dims(space, out.dims.data(), out.maxDims.data()) < 0) {
        qCritical() << "[THdfSession] Failed to get dims for:" << pNorm;
        H5Tclose(type);
        H5Sclose(space);
        H5Dclose(dset);
        return out;
    }

    out.typeClass = H5Tget_class(type);

    hid_t dcpl = H5Dget_create_plist(dset);
    if (dcpl >= 0) {
        const H5D_layout_t layout = H5Pget_layout(dcpl);
        if (layout == H5D_CHUNKED) {
            out.chunked = true;
            out.chunkDims.resize(rank);
            if (H5Pget_chunk(dcpl, rank, out.chunkDims.data()) < 0) {
                out.chunked = false;
                out.chunkDims.clear();
            }
        }
        H5Pclose(dcpl);
    }

    out.valid = true;

    H5Tclose(type);
    H5Sclose(space);
    H5Dclose(dset);
    return out;
}

bool THdfSession::canAppendAlongFirstDim(const QString &datasetPath,
                                         int expectedRank,
                                         H5T_class_t expectedTypeClass) const
{
    const DatasetInfo info = datasetInfo(datasetPath);
    if (!info.valid)
        return false;
    if (info.rank != expectedRank)
        return false;
    if (info.typeClass != expectedTypeClass)
        return false;
    return (!info.maxDims.isEmpty() && info.maxDims[0] == H5S_UNLIMITED);
}

bool THdfSession::createDataset(const DatasetCreateParams &p) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] createDataset called but file not open";
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    const QString path = normalizePath(p.path);

    if (p.elementType < 0) {
        qCritical() << "[THdfSession] createDataset invalid elementType for:" << path;
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }
    if (p.initialDims.isEmpty() ||
        p.initialDims.size() != p.maxDims.size() ||
        p.initialDims.size() != p.chunkDims.size()) {
        qCritical() << "[THdfSession] createDataset dims mismatch for:" << path;
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    if (!ensureGroup(parentPath(path))){
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    if (pathExists(path)) {
        qCritical() << "[THdfSession] createDataset path already exists:" << path;
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    hid_t space = H5Screate_simple(p.initialDims.size(), p.initialDims.data(), p.maxDims.data());
    if (space < 0) {
        qCritical() << "[THdfSession] H5Screate_simple failed for:" << path;
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    if (dcpl < 0) {
        qCritical() << "[THdfSession] H5Pcreate failed for:" << path;
        H5Sclose(space);
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    if (H5Pset_chunk(dcpl, p.chunkDims.size(), p.chunkDims.data()) < 0) {
        qCritical() << "[THdfSession] H5Pset_chunk failed for:" << path;
        H5Pclose(dcpl);
        H5Sclose(space);
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    if (p.enableDeflate) {
        const unsigned lvl = static_cast<unsigned>(qBound(0, p.deflateLevel, 9));
        if (H5Pset_deflate(dcpl, lvl) < 0) {
            qCritical() << "[THdfSession] H5Pset_deflate failed for:" << path;
            H5Pclose(dcpl);
            H5Sclose(space);
            if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
            return false;
        }
    }

    hid_t dset = H5Dcreate2(m_fileId,
                            path.toUtf8().constData(),
                            p.elementType,
                            space,
                            H5P_DEFAULT,
                            dcpl,
                            H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (dset < 0) {
        qCritical() << "[THdfSession] H5Dcreate2 failed for:" << path;
        if (p.takeOwnershipOfElementType && p.elementType >= 0) H5Tclose(p.elementType);
        return false;
    }

    H5Dclose(dset);

    if (p.takeOwnershipOfElementType && p.elementType >= 0) {
        H5Tclose(p.elementType);
    }

    if (H5Fflush(m_fileId, H5F_SCOPE_GLOBAL) < 0) {
        qCritical() << "[THdfSession] Warning: H5Fflush failed after createDataset:" << path;
    }

    qDebug() << "[THdfSession] Created dataset:" << path;
    return true;
}

quint64 THdfSession::datasetStorageBytes(const QString &datasetPath) const
{
    if (!isOpen()) return 0;

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm)) return 0;

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] H5Dopen2 failed for storageBytes:" << pNorm;
        return 0;
    }

    const hsize_t sz = H5Dget_storage_size(dset);
    H5Dclose(dset);
    return static_cast<quint64>(sz);
}

quint64 THdfSession::datasetElementBytes(const QString &datasetPath) const
{
    if (!isOpen()) return 0;

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm)) return 0;

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] H5Dopen2 failed for elementBytes:" << pNorm;
        return 0;
    }

    hid_t type = H5Dget_type(dset);
    if (type < 0) {
        qCritical() << "[THdfSession] H5Dget_type failed for elementBytes:" << pNorm;
        H5Dclose(dset);
        return 0;
    }

    const size_t sz = H5Tget_size(type);
    H5Tclose(type);
    H5Dclose(dset);
    return static_cast<quint64>(sz);
}

quint64 THdfSession::datasetElementCount(const QString &datasetPath) const
{
    const DatasetInfo info = datasetInfo(datasetPath);
    if (!info.valid) return 0;

    quint64 count = 1;
    for (hsize_t d : info.dims) {
        count *= static_cast<quint64>(d);
    }
    return count;
}

bool THdfSession::isGroupEmpty(const QString &groupPath) const
{
    if (!isOpen())
        return false;

    const QString gp = normalizePath(groupPath);
    if (!isGroup(gp))
        return false;

    const QByteArray gb = gp.toUtf8();
    hid_t g = H5Gopen2(m_fileId, gb.constData(), H5P_DEFAULT);
    if (g < 0) {
        qCritical() << "[THdfSession] isGroupEmpty: H5Gopen2 failed for:" << gp;
        return false;
    }

    H5G_info_t info{};
    const herr_t r = H5Gget_info(g, &info);
    H5Gclose(g);

    if (r < 0) {
        qCritical() << "[THdfSession] isGroupEmpty: H5Gget_info failed for:" << gp;
        return false;
    }

    return info.nlinks == 0;
}


bool THdfSession::removeLink(const QString &path) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] removeLink called but file is not open";
        return false;
    }

    const QString p = normalizePath(path);
    if (p.isEmpty() || p == "/") {
        qCritical() << "[THdfSession] removeLink refusing to delete root/empty path";
        return false;
    }

    const NodeType t = nodeType(p);
    if (t == NodeType::Missing) {
        qCritical() << "[THdfSession] removeLink: path missing:" << p;
        return false;
    }
    if (t == NodeType::Error) {
        qCritical() << "[THdfSession] removeLink: nodeType error:" << p;
        return false;
    }

    if (t == NodeType::Group) {
        if (!isGroupEmpty(p)) {
            qCritical() << "[THdfSession] removeLink: group not empty:" << p;
            return false;
        }
    } else if (t != NodeType::Dataset) {
        qCritical() << "[THdfSession] removeLink: refusing to delete non group/dataset:" << p;
        return false;
    }

    const QByteArray pb = p.toUtf8();
    if (H5Ldelete(m_fileId, pb.constData(), H5P_DEFAULT) < 0) {
        qCritical() << "[THdfSession] removeLink: H5Ldelete failed for:" << p;
        return false;
    }

    if (H5Fflush(m_fileId, H5F_SCOPE_GLOBAL) < 0) {
        qCritical() << "[THdfSession] Warning: H5Fflush failed after removeLink:" << p;
    }

    return true;
}

static bool datasetIsUInt64(const THdfSession *self, const QString &path)
{
    if (!self || !self->isOpen()) return false;

    hid_t dset = H5Dopen2(self->fileId(), path.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) return false;

    hid_t t = H5Dget_type(dset);
    if (t < 0) { H5Dclose(dset); return false; }

    const H5T_class_t cls = H5Tget_class(t);
    const size_t sz = H5Tget_size(t);
    const H5T_sign_t sgn = (cls == H5T_INTEGER) ? H5Tget_sign(t) : H5T_SGN_ERROR;

    H5Tclose(t);
    H5Dclose(dset);

    return (cls == H5T_INTEGER && sz == 8 && sgn == H5T_SGN_NONE);
}

static bool datasetIsString(const THdfSession *self, const QString &path)
{
    if (!self || !self->isOpen()) return false;

    hid_t dset = H5Dopen2(self->fileId(), path.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) return false;

    hid_t t = H5Dget_type(dset);
    if (t < 0) { H5Dclose(dset); return false; }

    const H5T_class_t cls = H5Tget_class(t);

    H5Tclose(t);
    H5Dclose(dset);

    return (cls == H5T_STRING);
}

static bool ensure1DUnlimitedDatasetUInt64(const THdfSession *self, const QString &path)
{
    if (!self || !self->isOpen()) {
        qCritical() << "[THdfSession] ensure1DUnlimitedDatasetUInt64: file not open";
        return false;
    }

    if (self->pathExists(path)) {
        const auto info = self->datasetInfo(path);
        if (!info.valid || info.rank != 1 || info.maxDims.size() != 1 || info.maxDims[0] != H5S_UNLIMITED) {
            qCritical() << "[THdfSession] Metadata schema invalid:" << path
                        << "(must be rank-1, maxDims[0]=unlimited)";
            return false;
        }
        if (!datasetIsUInt64(self, path)) {
            qCritical() << "[THdfSession] Metadata schema invalid:" << path
                        << "(must be uint64)";
            return false;
        }
        return true;
    }

    THdfSession::DatasetCreateParams p;
    p.path = path;
    p.elementType = H5T_NATIVE_UINT64;
    p.initialDims = {0};
    p.maxDims = {H5S_UNLIMITED};
    p.chunkDims = {64};
    p.enableDeflate = false;

    if (!self->createDataset(p)) {
        qCritical() << "[THdfSession] Failed to create required metadata dataset:" << path;
        return false;
    }
    return true;
}

static bool ensure1DUnlimitedDatasetString(const THdfSession *self, const QString &path)
{
    if (!self || !self->isOpen()) {
        qCritical() << "[THdfSession] ensure1DUnlimitedDatasetString: file not open";
        return false;
    }

    if (self->pathExists(path)) {
        const auto info = self->datasetInfo(path);
        if (!info.valid || info.rank != 1 || info.maxDims.size() != 1 || info.maxDims[0] != H5S_UNLIMITED) {
            qCritical() << "[THdfSession] Metadata schema invalid:" << path
                        << "(must be rank-1, maxDims[0]=unlimited)";
            return false;
        }
        if (!datasetIsString(self, path)) {
            qCritical() << "[THdfSession] Metadata schema invalid:" << path
                        << "(must be string)";
            return false;
        }
        return true;
    }

    hid_t et = H5Tcopy(H5T_C_S1);
    if (et < 0) {
        qCritical() << "[THdfSession] H5Tcopy failed while creating string dataset:" << path;
        return false;
    }

    if (H5Tset_size(et, H5T_VARIABLE) < 0 ||
        H5Tset_cset(et, H5T_CSET_UTF8) < 0 ||
        H5Tset_strpad(et, H5T_STR_NULLTERM) < 0) {
        qCritical() << "[THdfSession] Failed to configure vlen UTF-8 string type for:" << path;
        H5Tclose(et);
        return false;
    }

    THdfSession::DatasetCreateParams p;
    p.path = path;
    p.elementType = et;
    p.takeOwnershipOfElementType = true;
    p.initialDims = {0};
    p.maxDims = {H5S_UNLIMITED};
    p.chunkDims = {64};          // not critical now
    p.enableDeflate = false;

    if (!self->createDataset(p)) {
        qCritical() << "[THdfSession] Failed to create required metadata dataset:" << path;
        return false;
    }
    return true;
}

bool THdfSession::ensureMetadataGroup(const QString &groupPath) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] ensureMetadataGroup: file not open";
        return false;
    }

    const QString gp = normalizePath(groupPath);
    if (gp.isEmpty() || gp == "/") {
        qCritical() << "[THdfSession] ensureMetadataGroup: invalid group path:" << gp;
        return false;
    }

    if (!pathExists(gp)) {
        if (!ensureGroup(gp)) {
            qCritical() << "[THdfSession] ensureMetadataGroup: failed to create group:" << gp;
            return false;
        }
    }

    if (!isGroup(gp)) {
        qCritical() << "[THdfSession] ensureMetadataGroup: path is not a group:" << gp;
        return false;
    }

    const QString pFirst = normalizePath(gp + "/first_trace");
    const QString pCount = normalizePath(gp + "/trace_count");
    const QString pTime  = normalizePath(gp + "/timestamp");
    const QString pSet   = normalizePath(gp + "/settings");

    if (!ensure1DUnlimitedDatasetUInt64(this, pFirst)) return false;
    if (!ensure1DUnlimitedDatasetUInt64(this, pCount)) return false;
    if (!ensure1DUnlimitedDatasetString(this, pTime))  return false;
    if (!ensure1DUnlimitedDatasetString(this, pSet))   return false;

    return validateMetadataGroup(gp);
}

bool THdfSession::validateMetadataGroup(const QString &groupPath) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] validateMetadataGroup: file not open";
        return false;
    }

    const QString gp = normalizePath(groupPath);
    if (!isGroup(gp)) {
        qCritical() << "[THdfSession] validateMetadataGroup: not a group:" << gp;
        return false;
    }

    const QString pFirst = normalizePath(gp + "/first_trace");
    const QString pCount = normalizePath(gp + "/trace_count");
    const QString pTime  = normalizePath(gp + "/timestamp");
    const QString pSet   = normalizePath(gp + "/settings");

    const auto iFirst = datasetInfo(pFirst);
    const auto iCount = datasetInfo(pCount);
    const auto iTime  = datasetInfo(pTime);
    const auto iSet   = datasetInfo(pSet);

    auto require1DUnlimited = [&](const QString &p, const DatasetInfo &i) -> bool {
        if (!i.valid || i.rank != 1 || i.maxDims.size() != 1 || i.maxDims[0] != H5S_UNLIMITED) {
            qCritical() << "[THdfSession] Metadata schema invalid:" << p
                        << "(must be rank-1, maxDims[0]=unlimited)";
            return false;
        }
        return true;
    };

    if (!require1DUnlimited(pFirst, iFirst)) return false;
    if (!require1DUnlimited(pCount, iCount)) return false;
    if (!require1DUnlimited(pTime,  iTime))  return false;
    if (!require1DUnlimited(pSet,   iSet))   return false;

    if (!datasetIsUInt64(this, pFirst)) { qCritical() << "[THdfSession] Metadata schema invalid:" << pFirst << "(must be uint64)"; return false; }
    if (!datasetIsUInt64(this, pCount)) { qCritical() << "[THdfSession] Metadata schema invalid:" << pCount << "(must be uint64)"; return false; }
    if (!datasetIsString(this, pTime))  { qCritical() << "[THdfSession] Metadata schema invalid:" << pTime  << "(must be string)"; return false; }
    if (!datasetIsString(this, pSet))   { qCritical() << "[THdfSession] Metadata schema invalid:" << pSet   << "(must be string)"; return false; }

    const hsize_t n0 = iFirst.dims.isEmpty() ? 0 : iFirst.dims[0];
    const hsize_t n1 = iCount.dims.isEmpty() ? 0 : iCount.dims[0];
    const hsize_t n2 = iTime.dims.isEmpty()  ? 0 : iTime.dims[0];
    const hsize_t n3 = iSet.dims.isEmpty()   ? 0 : iSet.dims[0];

    if (!(n0 == n1 && n0 == n2 && n0 == n3)) {
        qCritical() << "[THdfSession] Metadata schema invalid:" << gp
                    << "parallel lengths mismatch:"
                    << "first_trace=" << (qulonglong)n0
                    << "trace_count=" << (qulonglong)n1
                    << "timestamp=" << (qulonglong)n2
                    << "settings=" << (qulonglong)n3;
        return false;
    }

    return true;
}

bool THdfSession::beginAppendTraces(const QString &datasetPath, hsize_t samplesPerTrace, TScope::TSampleType expectedType, hsize_t rowsToAppend, TraceAppendHandle &out) const
{
    endAppendTraces(out);

    if (!isOpen()) {
        qCritical() << "[THdfSession] beginAppendTraces: file not open";
        return false;
    }

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm)) {
        qCritical() << "[THdfSession] beginAppendTraces: not a dataset:" << pNorm;
        return false;
    }

    if (samplesPerTrace == 0) {
        qCritical() << "[THdfSession] beginAppendTraces: samplesPerTrace == 0 for:" << pNorm;
        return false;
    }
    if (rowsToAppend == 0) {
        qCritical() << "[THdfSession] beginAppendTraces: rowsToAppend == 0 for:" << pNorm;
        return false;
    }

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Dopen2 failed for:" << pNorm;
        return false;
    }

    hid_t fileSpace = H5Dget_space(dset);
    if (fileSpace < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Dget_space failed for:" << pNorm;
        H5Dclose(dset);
        return false;
    }

    const int rank = H5Sget_simple_extent_ndims(fileSpace);
    if (rank != 2) {
        qCritical() << "[THdfSession] beginAppendTraces: dataset rank is" << rank
                    << "expected 2 for:" << pNorm;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    hsize_t curDims[2]{0,0};
    hsize_t maxDims[2]{0,0};
    if (H5Sget_simple_extent_dims(fileSpace, curDims, maxDims) < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Sget_simple_extent_dims failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    if (maxDims[0] != H5S_UNLIMITED) {
        qCritical() << "[THdfSession] beginAppendTraces: dim0 not unlimited for:" << pNorm;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    if (curDims[1] != samplesPerTrace) {
        qCritical() << "[THdfSession] beginAppendTraces: columns mismatch for:" << pNorm
                    << "got" << (qulonglong)curDims[1]
                    << "expected" << (qulonglong)samplesPerTrace;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    if (!datasetTypeMatches(dset, expectedType)) {
        qCritical() << "[THdfSession] beginAppendTraces: dataset element type does not match expected type for:"
                    << pNorm << "expectedType=" << int(expectedType);
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    // Reserve space: extend by rowsToAppend up-front.
    const hsize_t oldRows = curDims[0];
    const hsize_t newRows = oldRows + rowsToAppend;

    hsize_t newDims[2]{ newRows, samplesPerTrace };
    if (H5Dset_extent(dset, newDims) < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Dset_extent failed for:" << pNorm
                    << "oldRows" << (qulonglong)oldRows
                    << "append" << (qulonglong)rowsToAppend;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    // Refresh dataspace after extend
    H5Sclose(fileSpace);
    fileSpace = H5Dget_space(dset);
    if (fileSpace < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Dget_space after extend failed for:" << pNorm;
        H5Dclose(dset);
        return false;
    }

    // One-row memspace [1, cols]
    hsize_t memDims[2]{ 1, samplesPerTrace };
    hid_t memSpace = H5Screate_simple(2, memDims, nullptr);
    if (memSpace < 0) {
        qCritical() << "[THdfSession] beginAppendTraces: H5Screate_simple(mem) failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Dclose(dset);
        return false;
    }

    out.datasetPath = pNorm;
    out.dset = dset;
    out.fileSpace = fileSpace;
    out.memSpace = memSpace;
    out.cols = samplesPerTrace;
    out.nextRow = oldRows;
    out.endRowExclusive = newRows;
    out.sampleType = expectedType;
    out.active = true;

    return true;
}

bool THdfSession::appendTraceRow(TraceAppendHandle &h, const void *data, TScope::TSampleType providedType, std::size_t bytes) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] appendTraceRow: file not open";
        return false;
    }
    if (!h.active || h.dset < 0 || h.fileSpace < 0 || h.memSpace < 0) {
        qCritical() << "[THdfSession] appendTraceRow: handle not active for:" << h.datasetPath;
        return false;
    }
    if (!data) {
        qCritical() << "[THdfSession] appendTraceRow: null data for:" << h.datasetPath;
        return false;
    }
    if (h.nextRow >= h.endRowExclusive) {
        qCritical() << "[THdfSession] appendTraceRow: write past reserved rows for:" << h.datasetPath
                    << "nextRow" << (qulonglong)h.nextRow
                    << "end" << (qulonglong)h.endRowExclusive;
        return false;
    }

    if (providedType != h.sampleType) {
        qCritical() << "[THdfSession] appendTraceRow: type mismatch for:" << h.datasetPath
                    << "provided" << int(providedType)
                    << "expected" << int(h.sampleType);
        return false;
    }

    const std::size_t elemBytes = sampleTypeBytes(providedType);
    if (elemBytes == 0) {
        qCritical() << "[THdfSession] appendTraceRow: unknown elemBytes for type" << int(providedType)
        << "dataset" << h.datasetPath;
        return false;
    }

    const std::size_t expectedBytes = (std::size_t)h.cols * elemBytes;
    if (bytes != expectedBytes) {
        qCritical() << "[THdfSession] appendTraceRow: byte count mismatch for:" << h.datasetPath
                    << "got" << (qulonglong)bytes
                    << "expected" << (qulonglong)expectedBytes
                    << "cols" << (qulonglong)h.cols
                    << "elemBytes" << (qulonglong)elemBytes;
        return false;
    }

    // Select hyperslab in file space: [row, 0] size [1, cols]
    hsize_t start[2]{ h.nextRow, 0 };
    hsize_t count[2]{ 1, h.cols };

    if (H5Sselect_hyperslab(h.fileSpace, H5S_SELECT_SET, start, nullptr, count, nullptr) < 0) {
        qCritical() << "[THdfSession] appendTraceRow: H5Sselect_hyperslab failed for:" << h.datasetPath
                    << "row" << (qulonglong)h.nextRow;
        return false;
    }

    const hid_t memType = sampleTypeToNativeH5Type(h.sampleType);
    if (memType < 0) {
        qCritical() << "[THdfSession] appendTraceRow: invalid memType for type" << int(h.sampleType)
        << "dataset" << h.datasetPath;
        return false;
    }

    if (H5Dwrite(h.dset, memType, h.memSpace, h.fileSpace, H5P_DEFAULT, data) < 0) {
        qCritical() << "[THdfSession] appendTraceRow: H5Dwrite failed for:" << h.datasetPath
                    << "row" << (qulonglong)h.nextRow;
        return false;
    }

    h.nextRow++;
    return true;
}

void THdfSession::endAppendTraces(TraceAppendHandle &h) const
{
    if (h.memSpace >= 0) {
        H5Sclose(h.memSpace);
        h.memSpace = H5I_INVALID_HID;
    }
    if (h.fileSpace >= 0) {
        H5Sclose(h.fileSpace);
        h.fileSpace = H5I_INVALID_HID;
    }
    if (h.dset >= 0) {
        H5Dclose(h.dset);
        h.dset = H5I_INVALID_HID;
    }

    h.datasetPath.clear();
    h.cols = 0;
    h.nextRow = 0;
    h.endRowExclusive = 0;
    h.sampleType = TScope::TSampleType::TReal32;
    h.active = false;
}

bool THdfSession::appendTracesMetadataRecord(const QString &groupPath, quint64 first_trace, quint64 trace_count, const QString &timestamp, const QString &settings) const
{
    if (!isOpen()) {
        qCritical() << "[THdfSession] appendMetadataRecord: file not open";
        return false;
    }

    const QString gp = normalizePath(groupPath);
    if (!validateMetadataGroup(gp)) {
        qCritical() << "[THdfSession] appendMetadataRecord: invalid metadata group:" << gp;
        return false;
    }

    const QString pFirst = normalizePath(gp + "/first_trace");
    const QString pCount = normalizePath(gp + "/trace_count");
    const QString pTime  = normalizePath(gp + "/timestamp");
    const QString pSet   = normalizePath(gp + "/settings");

    // Determine append index (validateMetadataGroup guarantees all 4 lengths equal). :contentReference[oaicite:3]{index=3}
    const auto iFirst = datasetInfo(pFirst);
    if (!iFirst.valid || iFirst.rank != 1 || iFirst.dims.size() != 1) {
        qCritical() << "[THdfSession] appendMetadataRecord: cannot read dims for:" << pFirst;
        return false;
    }
    const hsize_t idx = iFirst.dims[0];

    auto openD = [&](const QString &p) -> hid_t {
        hid_t d = H5Dopen2(m_fileId, p.toUtf8().constData(), H5P_DEFAULT);
        if (d < 0)
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dopen2 failed for:" << p;
        return d;
    };

    hid_t dFirst = openD(pFirst);
    hid_t dCount = openD(pCount);
    hid_t dTime  = openD(pTime);
    hid_t dSet   = openD(pSet);

    if (dFirst < 0 || dCount < 0 || dTime < 0 || dSet < 0) {
        if (dFirst >= 0) H5Dclose(dFirst);
        if (dCount >= 0) H5Dclose(dCount);
        if (dTime  >= 0) H5Dclose(dTime);
        if (dSet   >= 0) H5Dclose(dSet);
        return false;
    }

    // Extend all 4 datasets first so they remain aligned even if a later write fails.
    const hsize_t newLen[1]{ idx + 1 };
    if (H5Dset_extent(dFirst, newLen) < 0 ||
        H5Dset_extent(dCount, newLen) < 0 ||
        H5Dset_extent(dTime,  newLen) < 0 ||
        H5Dset_extent(dSet,   newLen) < 0) {
        qCritical() << "[THdfSession] appendMetadataRecord: H5Dset_extent failed for group:" << gp;
        H5Dclose(dFirst); H5Dclose(dCount); H5Dclose(dTime); H5Dclose(dSet);
        return false;
    }

    // Common 1-element selection at idx
    const hsize_t start[1]{ idx };
    const hsize_t count[1]{ 1 };

    hid_t memSpace = H5Screate_simple(1, count, nullptr);
    if (memSpace < 0) {
        qCritical() << "[THdfSession] appendMetadataRecord: H5Screate_simple failed";
        H5Dclose(dFirst); H5Dclose(dCount); H5Dclose(dTime); H5Dclose(dSet);
        return false;
    }

    auto writeUInt64At = [&](hid_t dset, quint64 value, const char *label) -> bool {
        hid_t fileSpace = H5Dget_space(dset);
        if (fileSpace < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dget_space failed for" << label;
            return false;
        }
        const herr_t s = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, count, nullptr);
        if (s < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: hyperslab select failed for" << label;
            H5Sclose(fileSpace);
            return false;
        }
        const herr_t w = H5Dwrite(dset, H5T_NATIVE_UINT64, memSpace, fileSpace, H5P_DEFAULT, &value);
        H5Sclose(fileSpace);
        if (w < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dwrite failed for" << label;
            return false;
        }
        return true;
    };

    auto writeVlenStringAt = [&](hid_t dset, const QString &s, const char *label) -> bool {
        // Dataset was created as vlen UTF-8 string in ensureMetadataGroup().
        hid_t memType = H5Dget_type(dset);
        if (memType < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dget_type failed for" << label;
            return false;
        }

        hid_t fileSpace = H5Dget_space(dset);
        if (fileSpace < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dget_space failed for" << label;
            H5Tclose(memType);
            return false;
        }

        const herr_t sel = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, count, nullptr);
        if (sel < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: hyperslab select failed for" << label;
            H5Sclose(fileSpace);
            H5Tclose(memType);
            return false;
        }

        const QByteArray utf8 = s.toUtf8();
        const char *one = utf8.constData();          // one vlen string element
        const herr_t w = H5Dwrite(dset, memType, memSpace, fileSpace, H5P_DEFAULT, &one);

        H5Sclose(fileSpace);
        H5Tclose(memType);

        if (w < 0) {
            qCritical() << "[THdfSession] appendMetadataRecord: H5Dwrite failed for" << label;
            return false;
        }
        return true;
    };

    const bool ok =
        writeUInt64At(dFirst, first_trace, "first_trace") &&
        writeUInt64At(dCount, trace_count, "trace_count") &&
        writeVlenStringAt(dTime, timestamp, "timestamp") &&
        writeVlenStringAt(dSet, settings, "settings");

    H5Sclose(memSpace);
    H5Dclose(dFirst);
    H5Dclose(dCount);
    H5Dclose(dTime);
    H5Dclose(dSet);

    return ok;
}

bool THdfSession::readDataset(const QString &datasetPath, quint64 startElem, quint64 elemCount, QByteArray &out, QVector<quint64> *outDims, QString *logOut) const
{
    out.clear();
    if (outDims) outDims->clear();
    if (logOut) logOut->clear();

    if (!isOpen()) {
        qCritical() << "[THdfSession] readDataset(rank1): file not open";
        return false;
    }

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm)) {
        qCritical() << "[THdfSession] readDataset(rank1): not a dataset:" << pNorm;
        return false;
    }

    if (elemCount == 0) {
        qCritical() << "[THdfSession] readDataset(rank1): elemCount is 0 for:" << pNorm;
        return false;
    }

    const DatasetInfo info = datasetInfo(pNorm);
    if (!info.valid) {
        qCritical() << "[THdfSession] readDataset(rank1): datasetInfo invalid for:" << pNorm;
        return false;
    }
    if (info.rank != 1) {
        qCritical() << "[THdfSession] readDataset(rank1): expected rank 1, got" << info.rank << "for:" << pNorm;
        return false;
    }
    if (info.dims.size() < 1) {
        qCritical() << "[THdfSession] readDataset(rank1): dims invalid for:" << pNorm;
        return false;
    }

    const quint64 dim0 = static_cast<quint64>(info.dims[0]);
    if (startElem >= dim0) {
        qCritical() << "[THdfSession] readDataset(rank1): startElem out of range for:" << pNorm
                    << "start=" << startElem << "dim0=" << dim0;
        return false;
    }
    if (elemCount > (dim0 - startElem)) {
        qCritical() << "[THdfSession] readDataset(rank1): selection exceeds dataset for:" << pNorm
                    << "start=" << startElem << "count=" << elemCount << "dim0=" << dim0;
        return false;
    }

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Dopen2 failed for:" << pNorm;
        return false;
    }

    hid_t memType = H5Dget_type(dset);
    if (memType < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Dget_type failed for:" << pNorm;
        H5Dclose(dset);
        return false;
    }

    const H5T_class_t cls = H5Tget_class(memType);
    const size_t elemBytes = H5Tget_size(memType);

    // Keep this consistent with what you consider "creatable" in the browser:
    bool supported = false;
    if (cls == H5T_INTEGER) {
        supported = (elemBytes == 1 || elemBytes == 2 || elemBytes == 4 || elemBytes == 8);
    } else if (cls == H5T_FLOAT) {
        supported = (elemBytes == 4 || elemBytes == 8);
    }

    if (!supported || elemBytes == 0) {
        qCritical() << "[THdfSession] readDataset(rank1): unsupported element type for:" << pNorm
                    << "class=" << cls << "bytes=" << static_cast<qulonglong>(elemBytes);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    // QByteArray size must fit into int.
    const quint64 totalBytes64 = elemCount * static_cast<quint64>(elemBytes);
    if (totalBytes64 > static_cast<quint64>(std::numeric_limits<int>::max())) {
        qCritical() << "[THdfSession] readDataset(rank1): selection too large for QByteArray for:" << pNorm
                    << "bytes=" << static_cast<qulonglong>(totalBytes64);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    // HDF uses hsize_t.
    if (startElem > static_cast<quint64>(std::numeric_limits<hsize_t>::max()) ||
        elemCount  > static_cast<quint64>(std::numeric_limits<hsize_t>::max())) {
        qCritical() << "[THdfSession] readDataset(rank1): selection exceeds hsize_t range for:" << pNorm;
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    const hsize_t start[1]{ static_cast<hsize_t>(startElem) };
    const hsize_t count[1]{ static_cast<hsize_t>(elemCount) };

    hid_t fileSpace = H5Dget_space(dset);
    if (fileSpace < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Dget_space failed for:" << pNorm;
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    if (H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, count, nullptr) < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Sselect_hyperslab failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    hid_t memSpace = H5Screate_simple(1, count, nullptr);
    if (memSpace < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Screate_simple failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    out.resize(static_cast<int>(totalBytes64));

    if (H5Dread(dset, memType, memSpace, fileSpace, H5P_DEFAULT, out.data()) < 0) {
        qCritical() << "[THdfSession] readDataset(rank1): H5Dread failed for:" << pNorm;
        out.clear();
        H5Sclose(memSpace);
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    H5Sclose(memSpace);
    H5Sclose(fileSpace);
    H5Tclose(memType);
    H5Dclose(dset);

    if (outDims) {
        outDims->push_back(elemCount);
    }
    if (logOut) {
        *logOut =
            QString("Read rank-1 dataset %1\nStart=%2 Count=%3 ElementBytes=%4 TotalBytes=%5")
                .arg(pNorm)
                .arg(QString::number(static_cast<qulonglong>(startElem)))
                .arg(QString::number(static_cast<qulonglong>(elemCount)))
                .arg(QString::number(static_cast<qulonglong>(elemBytes)))
                .arg(QString::number(static_cast<qulonglong>(totalBytes64)));
    }

    return true;
}

bool THdfSession::readDataset(const QString &datasetPath, quint64 rowStart, quint64 rowCount, quint64 colStart, quint64 colCount, QByteArray &out, QVector<quint64> *outDims, QString *logOut) const
{
    out.clear();
    if (outDims) outDims->clear();
    if (logOut) logOut->clear();

    if (!isOpen()) {
        qCritical() << "[THdfSession] readDataset(rank2): file not open";
        return false;
    }

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm)) {
        qCritical() << "[THdfSession] readDataset(rank2): not a dataset:" << pNorm;
        return false;
    }

    if (rowCount == 0 || colCount == 0) {
        qCritical() << "[THdfSession] readDataset(rank2): rowCount/colCount is 0 for:" << pNorm;
        return false;
    }

    const DatasetInfo info = datasetInfo(pNorm);
    if (!info.valid) {
        qCritical() << "[THdfSession] readDataset(rank2): datasetInfo invalid for:" << pNorm;
        return false;
    }
    if (info.rank != 2) {
        qCritical() << "[THdfSession] readDataset(rank2): expected rank 2, got" << info.rank << "for:" << pNorm;
        return false;
    }
    if (info.dims.size() < 2) {
        qCritical() << "[THdfSession] readDataset(rank2): dims invalid for:" << pNorm;
        return false;
    }

    const quint64 rows = static_cast<quint64>(info.dims[0]);
    const quint64 cols = static_cast<quint64>(info.dims[1]);

    if (rowStart >= rows || colStart >= cols) {
        qCritical() << "[THdfSession] readDataset(rank2): start out of range for:" << pNorm
                    << "rowStart=" << rowStart << "rows=" << rows
                    << "colStart=" << colStart << "cols=" << cols;
        return false;
    }
    if (rowCount > (rows - rowStart) || colCount > (cols - colStart)) {
        qCritical() << "[THdfSession] readDataset(rank2): selection exceeds dataset for:" << pNorm
                    << "rowStart=" << rowStart << "rowCount=" << rowCount << "rows=" << rows
                    << "colStart=" << colStart << "colCount=" << colCount << "cols=" << cols;
        return false;
    }

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Dopen2 failed for:" << pNorm;
        return false;
    }

    hid_t memType = H5Dget_type(dset);
    if (memType < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Dget_type failed for:" << pNorm;
        H5Dclose(dset);
        return false;
    }

    const H5T_class_t cls = H5Tget_class(memType);
    const size_t elemBytes = H5Tget_size(memType);

    bool supported = false;
    if (cls == H5T_INTEGER) {
        supported = (elemBytes == 1 || elemBytes == 2 || elemBytes == 4 || elemBytes == 8);
    } else if (cls == H5T_FLOAT) {
        supported = (elemBytes == 4 || elemBytes == 8);
    }

    if (!supported || elemBytes == 0) {
        qCritical() << "[THdfSession] readDataset(rank2): unsupported element type for:" << pNorm
                    << "class=" << cls << "bytes=" << static_cast<qulonglong>(elemBytes);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    const quint64 selElems = rowCount * colCount;
    const quint64 totalBytes64 = selElems * static_cast<quint64>(elemBytes);
    if (totalBytes64 > static_cast<quint64>(std::numeric_limits<int>::max())) {
        qCritical() << "[THdfSession] readDataset(rank2): selection too large for QByteArray for:" << pNorm
                    << "bytes=" << static_cast<qulonglong>(totalBytes64);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    // hsize_t bounds
    const quint64 hsMax = static_cast<quint64>(std::numeric_limits<hsize_t>::max());
    if (rowStart > hsMax || colStart > hsMax || rowCount > hsMax || colCount > hsMax) {
        qCritical() << "[THdfSession] readDataset(rank2): selection exceeds hsize_t range for:" << pNorm;
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    const hsize_t start[2]{ static_cast<hsize_t>(rowStart), static_cast<hsize_t>(colStart) };
    const hsize_t count[2]{ static_cast<hsize_t>(rowCount), static_cast<hsize_t>(colCount) };

    hid_t fileSpace = H5Dget_space(dset);
    if (fileSpace < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Dget_space failed for:" << pNorm;
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    if (H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, count, nullptr) < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Sselect_hyperslab failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    hid_t memSpace = H5Screate_simple(2, count, nullptr);
    if (memSpace < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Screate_simple failed for:" << pNorm;
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    out.resize(static_cast<int>(totalBytes64));

    if (H5Dread(dset, memType, memSpace, fileSpace, H5P_DEFAULT, out.data()) < 0) {
        qCritical() << "[THdfSession] readDataset(rank2): H5Dread failed for:" << pNorm;
        out.clear();
        H5Sclose(memSpace);
        H5Sclose(fileSpace);
        H5Tclose(memType);
        H5Dclose(dset);
        return false;
    }

    H5Sclose(memSpace);
    H5Sclose(fileSpace);
    H5Tclose(memType);
    H5Dclose(dset);

    if (outDims) {
        outDims->push_back(rowCount);
        outDims->push_back(colCount);
    }
    if (logOut) {
        *logOut =
            QString("Read rank-2 dataset %1\nRowStart=%2 RowCount=%3 ColStart=%4 ColCount=%5 ElementBytes=%6 TotalBytes=%7")
                .arg(pNorm)
                .arg(QString::number(static_cast<qulonglong>(rowStart)))
                .arg(QString::number(static_cast<qulonglong>(rowCount)))
                .arg(QString::number(static_cast<qulonglong>(colStart)))
                .arg(QString::number(static_cast<qulonglong>(colCount)))
                .arg(QString::number(static_cast<qulonglong>(elemBytes)))
                .arg(QString::number(static_cast<qulonglong>(totalBytes64)));
    }

    return true;
}

bool THdfSession::appendRawSlice(const QString &datasetPath, QByteArrayView payload, quint64 startByte, quint64 byteCount, quint64 cols, const QString &typeText, QString *logOut) const
{
    auto addLog = [&](const QString &line) {
        if (!logOut) return;
        if (!logOut->isEmpty()) *logOut += "\n";
        *logOut += line;
    };

    if (!isOpen()) {
        qCritical() << "[THdfSession] appendRawSlices: file not open";
        addLog("ERROR: file not open");
        return false;
    }

    // ---- resolve selected type ----
    H5T_class_t wantClass = H5T_NO_CLASS;
    H5T_sign_t  wantSign  = H5T_SGN_NONE;
    size_t      wantSize  = 0;
    hid_t       memType   = H5I_INVALID_HID;

    if (!parseSelectedType_native(typeText, wantClass, wantSign, wantSize, memType)) {
        qCritical() << "[THdfSession] appendRawSlices: unsupported typeText:" << typeText;
        addLog(QString("ERROR: unsupported type '%1'").arg(typeText));
        return false;
    }

    const quint64 elementBytes = static_cast<quint64>(wantSize);

    // ---- validate payload slice ----
    const quint64 total = static_cast<quint64>(payload.size());

    if (startByte > total) {
        qCritical() << "[THdfSession] appendRawSlices: startByte out of range:" << startByte << "total:" << total;
        addLog(QString("ERROR: startByte out of range (%1 > %2)").arg(startByte).arg(total));
        return false;
    }
    if (byteCount == 0) {
        qCritical() << "[THdfSession] appendRawSlices: byteCount is 0";
        addLog("ERROR: byteCount is 0");
        return false;
    }
    if (startByte + byteCount > total) {
        qCritical() << "[THdfSession] appendRawSlices: range exceeds payload size:"
                    << "start" << startByte << "count" << byteCount << "total" << total;
        addLog("ERROR: range exceeds payload size");
        return false;
    }

    // Must align to element size (Option A, no conversion).
    if (elementBytes == 0 || (byteCount % elementBytes) != 0) {
        const quint64 rem = (elementBytes == 0) ? byteCount : (byteCount % elementBytes);
        qCritical() << "[THdfSession] appendRawSlices: misaligned byteCount:" << byteCount
                    << "elementBytes:" << elementBytes << "rem:" << rem;
        addLog(QString("ERROR: misaligned slice: byteCount (%1) not divisible by elementBytes (%2)")
                   .arg(byteCount).arg(elementBytes));
        return false;
    }

    const quint64 elementCount = byteCount / elementBytes;

    if (cols > 0) {
        // rank-2 requires full rows
        if (elementCount % cols != 0) {
            qCritical() << "[THdfSession] appendRawSlices: elementCount not divisible by cols:"
                        << elementCount << "cols:" << cols;
            addLog(QString("ERROR: elementCount (%1) not divisible by cols (%2)")
                       .arg(elementCount).arg(cols));
            return false;
        }
    }

    // ---- dataset checks ----
    const QString pNorm = normalizePath(datasetPath);

    if (!isDataset(pNorm)) {
        qCritical() << "[THdfSession] appendRawSlices: not a dataset:" << pNorm;
        addLog("ERROR: target is not a dataset");
        return false;
    }

    const DatasetInfo info = datasetInfo(pNorm);
    if (!info.valid) {
        qCritical() << "[THdfSession] appendRawSlices: failed to query dataset info:" << pNorm;
        addLog("ERROR: failed to query dataset info");
        return false;
    }

    // Must be extendible along dim 0.
    if (info.maxDims.size() < 1 || info.maxDims[0] != H5S_UNLIMITED) {
        qCritical() << "[THdfSession] appendRawSlices: dataset is not unlimited in dim 0:" << pNorm;
        addLog("ERROR: dataset is not unlimited in dim 0");
        return false;
    }

    // Must be chunked for extension to work.
    if (!info.chunked) {
        qCritical() << "[THdfSession] appendRawSlices: dataset is not chunked:" << pNorm;
        addLog("ERROR: dataset is not chunked (cannot extend)");
        return false;
    }

    // Rank policy from cols:
    //  - cols == 0 => must be rank-1
    //  - cols  > 0 => must be rank-2 with fixed second dim == cols
    const int modeRank = (cols == 0 ? 1 : 2);

    if (modeRank == 1) {
        if (info.rank != 1) {
            qCritical() << "[THdfSession] appendRawSlices: rank mismatch, expected 1, got" << info.rank << "path:" << pNorm;
            addLog("ERROR: dataset rank mismatch (expected rank 1)");
            return false;
        }
    } else {
        if (info.rank != 2 || info.dims.size() < 2 || info.maxDims.size() < 2) {
            qCritical() << "[THdfSession] appendRawSlices: rank mismatch or invalid dims for rank-2:" << pNorm;
            addLog("ERROR: dataset rank/dims invalid (expected rank 2)");
            return false;
        }

        // fixed second dim must match
        const hsize_t wantColsH = static_cast<hsize_t>(cols);
        if (info.dims[1] != wantColsH) {
            qCritical() << "[THdfSession] appendRawSlices: dataset cols mismatch, have" << info.dims[1]
                        << "want" << wantColsH << "path:" << pNorm;
            addLog("ERROR: dataset second dimension mismatch");
            return false;
        }

        // keep it fixed: reject unlimited/varying columns
        if (info.maxDims[1] != wantColsH) {
            qCritical() << "[THdfSession] appendRawSlices: dataset max cols mismatch, have" << info.maxDims[1]
                        << "want" << wantColsH << "path:" << pNorm;
            addLog("ERROR: dataset max second dimension must equal cols");
            return false;
        }
    }

    // ---- verify dataset datatype matches selected typeText ----
    {
        hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
        if (dset < 0) {
            qCritical() << "[THdfSession] appendRawSlices: failed to open dataset for type check:" << pNorm;
            addLog("ERROR: failed to open dataset");
            return false;
        }

        hid_t t = H5Dget_type(dset);
        if (t < 0) {
            H5Dclose(dset);
            qCritical() << "[THdfSession] appendRawSlices: failed to read dataset type:" << pNorm;
            addLog("ERROR: failed to read dataset type");
            return false;
        }

        const H5T_class_t gotClass = H5Tget_class(t);
        const size_t gotSize = H5Tget_size(t);

        bool typeOk = false;
        if (gotClass == wantClass && gotSize == wantSize) {
            if (wantClass == H5T_INTEGER) {
                const H5T_sign_t gotSign = H5Tget_sign(t);
                typeOk = (gotSign == wantSign);
            } else if (wantClass == H5T_FLOAT) {
                typeOk = true;
            }
        }

        H5Tclose(t);
        H5Dclose(dset);

        if (!typeOk) {
            qCritical() << "[THdfSession] appendRawSlices: dataset type mismatch:" << pNorm << "wanted:" << typeText;
            addLog(QString("ERROR: dataset datatype does not match selected type '%1'").arg(typeText));
            return false;
        }
    }

    // ---- compute rowsToAppend (extend along dim0) ----
    quint64 rowsToAppend_u64 = 0;
    if (modeRank == 1) {
        rowsToAppend_u64 = elementCount; // append elements
    } else {
        rowsToAppend_u64 = elementCount / cols; // append rows
    }

    hsize_t rowsToAppend = 0;
    if (!u64_to_hsize(rowsToAppend_u64, rowsToAppend)) {
        qCritical() << "[THdfSession] appendRawSlices: rowsToAppend too large:" << rowsToAppend_u64;
        addLog("ERROR: append size too large");
        return false;
    }

    if (rowsToAppend == 0) {
        addLog("Nothing to append (rowsToAppend==0)");
        return true;
    }

    addLog(QString("File: %1").arg(m_filePath));
    addLog(QString("Dataset: %1").arg(pNorm));
    addLog(QString("Type: %1 (%2 bytes/element)").arg(typeText).arg(QString::number(elementBytes)));
    addLog(QString("Slice: startByte=%1 byteCount=%2").arg(startByte).arg(byteCount));
    addLog(QString("Write: rank=%1 cols=%2 elements=%3 extend0=%4")
               .arg(modeRank)
               .arg(QString::number(cols))
               .arg(QString::number(elementCount))
               .arg(QString::number(rowsToAppend_u64)));

    // ---- open dataset + extend ----
    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0) {
        qCritical() << "[THdfSession] appendRawSlices: failed to open dataset:" << pNorm;
        addLog("ERROR: failed to open dataset");
        return false;
    }

    QVector<hsize_t> newDims = info.dims;
    if (newDims.size() < 1) {
        qCritical() << "[THdfSession] appendRawSlices: invalid dims for dataset:" << pNorm;
        addLog("ERROR: invalid dataset dims");
        H5Dclose(dset);
        return false;
    }

    // overflow-safe extend
    if (rowsToAppend > 0 && newDims[0] > (std::numeric_limits<hsize_t>::max() - rowsToAppend)) {
        qCritical() << "[THdfSession] appendRawSlices: dim0 overflow on extend:" << pNorm;
        addLog("ERROR: dataset size overflow on extend");
        H5Dclose(dset);
        return false;
    }

    const hsize_t oldDim0 = info.dims[0];
    newDims[0] = oldDim0 + rowsToAppend;

    auto rollbackExtent = [&]() {
        QVector<hsize_t> rollbackDims = info.dims;
        // restore original dim0
        rollbackDims[0] = oldDim0;
        H5Dset_extent(dset, rollbackDims.data());
    };

    if (H5Dset_extent(dset, newDims.data()) < 0) {
        qCritical() << "[THdfSession] appendRawSlices: H5Dset_extent failed for:" << pNorm;
        addLog("ERROR: failed to extend dataset (H5Dset_extent)");
        H5Dclose(dset);
        return false;
    }

    hid_t fileSpace = H5Dget_space(dset);
    if (fileSpace < 0) {
        rollbackExtent();
        qCritical() << "[THdfSession] appendRawSlices: H5Dget_space failed for:" << pNorm;
        addLog("ERROR: failed to get file dataspace");
        H5Dclose(dset);
        return false;
    }

    const unsigned char *src =
        reinterpret_cast<const unsigned char*>(payload.data()) + static_cast<ptrdiff_t>(startByte);

    bool ok = true;

    if (modeRank == 1) {
        const hsize_t offset[1] = { oldDim0 };
        const hsize_t count[1]  = { rowsToAppend };

        if (H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, offset, nullptr, count, nullptr) < 0) {
            qCritical() << "[THdfSession] appendRawSlices: hyperslab select failed (rank-1) for:" << pNorm;
            addLog("ERROR: hyperslab select failed (rank-1)");
            ok = false;
        } else {
            const hsize_t memDims[1] = { rowsToAppend };
            hid_t memSpace = H5Screate_simple(1, memDims, nullptr);
            if (memSpace < 0) {
                qCritical() << "[THdfSession] appendRawSlices: failed to create memSpace (rank-1)";
                addLog("ERROR: failed to create memSpace (rank-1)");
                ok = false;
            } else {
                if (H5Dwrite(dset, memType, memSpace, fileSpace, H5P_DEFAULT, src) < 0) {
                    qCritical() << "[THdfSession] appendRawSlices: H5Dwrite failed (rank-1) for:" << pNorm;
                    addLog("ERROR: write failed (rank-1)");
                    ok = false;
                }
                H5Sclose(memSpace);
            }
        }
    } else {
        hsize_t colsH = 0;
        if (!u64_to_hsize(cols, colsH)) {
            qCritical() << "[THdfSession] appendRawSlices: cols too large:" << cols;
            addLog("ERROR: cols too large");
            ok = false;
        } else {
            const hsize_t offset[2] = { oldDim0, 0 };
            const hsize_t count[2]  = { rowsToAppend, colsH };

            if (H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, offset, nullptr, count, nullptr) < 0) {
                qCritical() << "[THdfSession] appendRawSlices: hyperslab select failed (rank-2) for:" << pNorm;
                addLog("ERROR: hyperslab select failed (rank-2)");
                ok = false;
            } else {
                const hsize_t memDims[2] = { rowsToAppend, colsH };
                hid_t memSpace = H5Screate_simple(2, memDims, nullptr);
                if (memSpace < 0) {
                    qCritical() << "[THdfSession] appendRawSlices: failed to create memSpace (rank-2)";
                    addLog("ERROR: failed to create memSpace (rank-2)");
                    ok = false;
                } else {
                    if (H5Dwrite(dset, memType, memSpace, fileSpace, H5P_DEFAULT, src) < 0) {
                        qCritical() << "[THdfSession] appendRawSlices: H5Dwrite failed (rank-2) for:" << pNorm;
                        addLog("ERROR: write failed (rank-2)");
                        ok = false;
                    }
                    H5Sclose(memSpace);
                }
            }
        }
    }

    if(!ok){
        rollbackExtent();
        H5Sclose(fileSpace);
        H5Dclose(dset);
        addLog("ERROR: append failed");
        return false;
    }

    H5Sclose(fileSpace);
    H5Dclose(dset);

    addLog("OK: append completed");
    return true;
}

QString THdfSession::datasetTypeText(const QString &datasetPath) const
{
    if (!isOpen())
        return QString();

    const QString pNorm = normalizePath(datasetPath);
    if (!isDataset(pNorm))
        return QString();

    hid_t dset = H5Dopen2(m_fileId, pNorm.toUtf8().constData(), H5P_DEFAULT);
    if (dset < 0)
        return QString();

    hid_t t = H5Dget_type(dset);
    if (t < 0) {
        H5Dclose(dset);
        return QString();
    }

    const H5T_class_t cls = H5Tget_class(t);
    const size_t sz = H5Tget_size(t);

    QString out;

    if (cls == H5T_INTEGER) {
        const H5T_sign_t sign = H5Tget_sign(t);

        if (sz == 1) out = (sign == H5T_SGN_NONE) ? "uint8"  : "int8";
        if (sz == 2) out = (sign == H5T_SGN_NONE) ? "uint16" : "int16";
        if (sz == 4) out = (sign == H5T_SGN_NONE) ? "uint32" : "int32";
        // NOTE: you purposely don’t expose 64-bit types in the wizard; keep it consistent:
        // if (sz == 8) out = (sign == H5T_SGN_NONE) ? "uint64" : "int64";
    }
    else if (cls == H5T_FLOAT) {
        if (sz == 4) out = "float32";
        if (sz == 8) out = "float64";
    }

    H5Tclose(t);
    H5Dclose(dset);
    return out;
}

bool THdfSession::datasetMatchesTypeText(const QString &datasetPath,
                                         const QString &expectedTypeText) const
{
    const QString got = datasetTypeText(datasetPath).trimmed().toLower();
    const QString want = expectedTypeText.trimmed().toLower();
    if (got.isEmpty() || want.isEmpty())
        return false;
    return got == want;
}
