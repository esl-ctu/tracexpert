#ifndef TSCENARIOWIDGET_H
#define TSCENARIOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>

#include "../projectunit/tprojectunitwidget.h"
#include "tscenariocontainer.h"

/*!
 * \brief The TScenarioWidget class represents a widget for managing Scenarios, the "Scenario Manager".
 *
 * The class represents a widget for managing Scenarios, the "Scenario Manager".
 * It allows the user to add, edit, rename and remove Scenarios through the GUI.
 *
 */
class TScenarioWidget : public TProjectUnitWidget {
    Q_OBJECT

public:
    explicit TScenarioWidget(TScenarioContainer * protocolContainer, QWidget * parent = nullptr);

};

#endif // TSCENARIOWIDGET_H
