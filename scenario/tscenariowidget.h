#ifndef TSCENARIOWIDGET_H
#define TSCENARIOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>

#include "tscenariocontainer.h"
#include "../tmainwindow.h"

/*!
 * \brief The TScenarioWidget class represents a widget for managing Scenarios, the "Scenario Manager".
 *
 * The class represents a widget for managing Scenarios, the "Scenario Manager".
 * It allows the user to add, edit, rename and remove Scenarios through the GUI.
 *
 */
class TScenarioWidget : public QWidget {
    Q_OBJECT

public:
    explicit TScenarioWidget(TScenarioContainer * protocolContainer, QWidget * parent = nullptr);

    void openEditor(TScenarioModel * scenario);

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRowDoubleClicked(const QModelIndex & index);
    void onRemoveButtonClicked();
    void onRenameButtonClicked();

private:
    TMainWindow * m_mainWindow;

    QTableView * m_scenarioView;
    TScenarioContainer * m_scenarioContainer;
};


#endif // TSCENARIOWIDGET_H
