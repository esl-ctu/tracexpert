#ifndef THDFBROWSERWIDGET_H
#define THDFBROWSERWIDGET_H

#include <QWidget>
#include <QPointer>

class QTreeView;
class QStandardItemModel;
class QPushButton;
class QLabel;
class QModelIndex;
class QTextEdit;

class THdfSession;

class THdfBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit THdfBrowserWidget(QWidget *parent = nullptr);

    enum class SelectionPolicy {
        GroupsAndDatasets,
        DatasetsOnly,
        GroupsOnly
    };

    struct TDatasetTemplate {
        QString suggestedName;      // e.g. "Traces CH1"
        int rank = 2;
        QString initialDimsText;    // e.g. "0,1024"
        QString maxDimsText;        // e.g. "unlimited,1024"
        QString chunkDimsText;      // e.g. "64,1024" or "1,262144"
        QString typeText;           // e.g. "float32"
        bool deflateOn = false;
        int deflateLevel = 4;

        // read-only UI
        bool lockRank = false;
        bool lockInitialDims = false;
        bool lockMaxDims = false;
        bool lockType = false;

        // chunking
        bool chunk2DRowBounded = false;
        qulonglong chunkRowsMin = 1;
        qulonglong chunkRowsMax = 1;
        qulonglong chunkRowsDefault = 0; // 0 => use max
        qulonglong chunkColsFixed = 0;   // required when chunk2DRowBounded
    };

    void setNewDatasetTemplate(const TDatasetTemplate &t) { m_newDatasetTemplate = t; }
    void clearNewDatasetTemplate() { m_newDatasetTemplate = TDatasetTemplate{}; }
    TDatasetTemplate newDatasetTemplate() const { return m_newDatasetTemplate; }

    void setSelectionPolicy(SelectionPolicy p) { m_selectionPolicy = p; }
    SelectionPolicy selectionPolicy() const { return m_selectionPolicy; }

    void setSession(THdfSession *session);
    THdfSession* session() const { return m_session; }

    void refresh();

    bool selectPath(const QString &absPath);
    QString selectedPath() const;
    bool selectedIsDataset() const;

    void setTitleText(const QString &text);
    void setAutoPreselectSuggestedDataset(bool on) { m_autoPreselectSuggested = on; }

    void setAutoPreselectPath(const QString &path) { m_autoPreselectPath = path; }
    QString autoPreselectPath() const { return m_autoPreselectPath; }

signals:
    // Emitted on selection changes, but only when selection matches SelectionPolicy.
    void selectionChanged(const QString &path, int nodeType /*THdfSession::NodeType as int*/);

    // Emitted after successful creation.
    void groupCreated(const QString &path);
    void datasetCreated(const QString &path);

private:
    void buildModel();
    void addGroupRecursive(const QString &groupPath, QStandardItemModel *model, class QStandardItem *parentItem);

    QString itemPath(const QModelIndex &idx) const;

    void updateInfoPanel();

    void onSelectionChanged();
    void onNewGroup();
    void onNewDataset();
    void onRemove();

    bool selectionAllowed(int nodeTypeInt) const;

    QModelIndex findIndexByPath(const QString &absPath) const;
    static QString toAbsolutePath(const QString &maybeRelative);

    bool autoPreselectSuggestedDataset() const { return m_autoPreselectSuggested; }

    QModelIndex findFirstDatasetByLeafName(const QString &leafName) const;
    static QString leafNameFromPath(const QString &absPath);

    QPointer<THdfSession> m_session;

    QLabel *m_title = nullptr;
    QTreeView *m_tree = nullptr;
    QStandardItemModel *m_model = nullptr;

    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_newGroupBtn = nullptr;
    QPushButton *m_newDatasetBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;

    QString m_selectedPath;
    bool m_selectedIsDataset = false;

    TDatasetTemplate m_newDatasetTemplate;
    SelectionPolicy m_selectionPolicy = SelectionPolicy::GroupsAndDatasets;

    QLabel *m_infoTitle = nullptr;
    QTextEdit *m_infoText = nullptr;

    bool m_autoPreselectSuggested = false;
    QString m_autoPreselectPath;

};

#endif // THDFBROWSERWIDGET_H
