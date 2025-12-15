#include "tscenariowidget.h"

#include <QListWidget>
#include <QLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>

TScenarioWidget::TScenarioWidget(TScenarioContainer * scenarioContainer, QWidget * parent) :
    TProjectUnitWidget(scenarioContainer, parent)
{
    m_fileTypeFilter = "TraceXpert scenario file (*.txps)";
}
