#ifndef TPROTOCOLMANAGER_H
#define TPROTOCOLMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include "tprotocolmodel.h"
#include "tprotocolcontainer.h"
#include "tprotocoleditor.h"

class TProtocolWidget : public QWidget {
    Q_OBJECT
public:
    explicit TProtocolWidget(TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

signals:

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRemoveButtonClicked();
    void onEditorFinished(int finished);

private:
    void openEditor();

    TProtocolEditor * m_protocolEditor;
    qsizetype m_editedItemIndex;

    QTableView * m_protocolView;
    TProtocolContainer * m_protocolContainer;
};

#endif // TPROTOCOLMANAGER_H
