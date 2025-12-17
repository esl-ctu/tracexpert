#ifndef THDFSESSION_H
#define THDFSESSION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QByteArrayView>
#include <cstddef>

#include <tscope.h>

#include <hdf5.h>

class THdfSession : public QObject
{
    Q_OBJECT
public:
    enum class NodeType { Missing, Group, Dataset, Other, Error };

    struct DatasetInfo {
        bool valid = false;
        int rank = 0;
        QVector<hsize_t> dims;
        QVector<hsize_t> maxDims;
        H5T_class_t typeClass = H5T_NO_CLASS;
        bool chunked = false;
        QVector<hsize_t> chunkDims;
    };

    struct DatasetCreateParams {
        QString path;                        // absolute path, e.g. "/channels/CH1/traces"
        hid_t elementType = H5I_INVALID_HID; // e.g. H5T_NATIVE_FLOAT
        QVector<hsize_t> initialDims;        // e.g. [0, 1024]
        QVector<hsize_t> maxDims;            // e.g. [H5S_UNLIMITED, 1024]
        QVector<hsize_t> chunkDims;          // e.g. [1, 1024]
        bool enableDeflate = false;
        int deflateLevel = 4;                // 0..9
        bool takeOwnershipOfElementType = false;
    };

    explicit THdfSession(QObject *parent = nullptr);
    ~THdfSession() override;

    THdfSession(const THdfSession&) = delete;
    THdfSession& operator=(const THdfSession&) = delete;

    // File lifecycle
    bool isOpen() const;
    QString filePath() const;
    hid_t fileId() const;
    bool openExisting(const QString &path); // returns false if corrupt/truncated/unopenable
    bool createNew(const QString &path);    // creates and opens RW
    void close();

    static bool isLikelyHdf5BySignature(const QString &path);
    static QString normalizePath(QString p);
    static QString parentPath(const QString &p);

    NodeType nodeType(const QString &path) const;
    bool pathExists(const QString &path) const;
    bool isGroup(const QString &path) const;
    bool isDataset(const QString &path) const;
    bool isGroupEmpty(const QString &groupPath) const;   // true if group has 0 links
    bool canAppendAlongFirstDim(const QString &datasetPath, int expectedRank, H5T_class_t expectedTypeClass) const;

    QStringList listChildren(const QString &groupPath) const;
    QStringList listGroups(const QString &groupPath) const;
    QStringList listDatasets(const QString &groupPath) const;

    bool ensureGroup(const QString &groupPath) const;
    bool createGroup(const QString &groupPath) const;
    bool removeLink(const QString &path) const;          // deletes the link (dataset or empty group)        

    DatasetInfo datasetInfo(const QString &datasetPath) const;
    quint64 datasetStorageBytes(const QString &datasetPath) const;   // actual allocated bytes in file (can be 0 for empty)
    quint64 datasetElementBytes(const QString &datasetPath) const;   // sizeof(element) in bytes
    quint64 datasetElementCount(const QString &datasetPath) const;   // product(dims)

    // Traces metadata
    bool ensureMetadataGroup(const QString &groupPath) const;   // create group + required datasets (or validate if exists)
    bool validateMetadataGroup(const QString &groupPath) const; // validate schema + parallel-length consistency    

    bool createDataset(const DatasetCreateParams &p) const;

    struct TraceAppendHandle {
        QString datasetPath;

        hid_t dset = H5I_INVALID_HID;
        hid_t fileSpace = H5I_INVALID_HID;
        hid_t memSpace = H5I_INVALID_HID;

        hsize_t cols = 0;               // samplesPerTrace
        hsize_t nextRow = 0;            // absolute row index to write next
        hsize_t endRowExclusive = 0;    // safety bound after extend
        TScope::TSampleType sampleType = TScope::TSampleType::TReal32;
        bool active = false;
    };

    bool beginAppendTraces(const QString &datasetPath, hsize_t samplesPerTrace, TScope::TSampleType expectedType, hsize_t rowsToAppend, TraceAppendHandle &out) const;
    bool appendTraceRow(TraceAppendHandle &h, const void *data, TScope::TSampleType providedType, std::size_t bytes) const;
    void endAppendTraces(TraceAppendHandle &h) const;

    bool appendTracesMetadataRecord(const QString &groupPath, quint64 first_trace, quint64 trace_count, const QString &timestamp, const QString &settings) const;

    // Append a raw byte slice (no conversion) to an existing dataset.
    //
    // - The dataset must exist, be chunked, and be extendible along dim 0.
    // - The dataset datatype must match typeText (Option A: no conversion).
    // - startByte/byteCount refer to the provided payload (in BYTES).
    //
    // cols is ELEMENTS per row:
    //  - cols == 0 : treat as rank-1 append of (byteCount/elementBytes) elements
    //  - cols  > 0 : treat as rank-2 append of rows = (byteCount/elementBytes)/cols, cols fixed
    bool appendRawSlice(const QString &datasetPath, QByteArrayView payload, quint64 startByte, quint64 byteCount, quint64 cols, const QString &typeText, QString *logOut = nullptr) const;


    // Read raw dataset selection into a QByteArray.
    // The data are returned as "native" bytes as produced by H5Dread using the dataset's own datatype as mem type.
    // Supported element types: integer/float (8/16/32/64-bit), supported ranks: 1 or 2.
    //
    // Rank-1: reads [startElem, startElem+elemCount).
    bool readDataset(const QString &datasetPath, quint64 startElem, quint64 elemCount, QByteArray &out, QVector<quint64> *outDims = nullptr, QString *logOut = nullptr) const;

    // Rank-2: reads rectangular hyperslab:
    // rows [rowStart, rowStart+rowCount), cols [colStart, colStart+colCount).
    bool readDataset(const QString &datasetPath, quint64 rowStart, quint64 rowCount, quint64 colStart, quint64 colCount, QByteArray &out, QVector<quint64> *outDims = nullptr, QString *logOut = nullptr) const;



private:
    QString m_filePath;
    hid_t m_fileId = H5I_INVALID_HID;
};

#endif // THDFSESSION_H
