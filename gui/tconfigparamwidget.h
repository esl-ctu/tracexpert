#ifndef TCONFIGPARAMWIDGET_H
#define TCONFIGPARAMWIDGET_H

#include <QTreeWidget>

#include "tconfigparam.h"

#define TCONFIGPARAMWIDGETVER "cz.cvut.fit.TraceXpert.TConfigParamWidget/0.1"

/*!
 * \brief The TConfigParamWidget class represents a widget for displaying TConfigParam in a tree view.
 *
 * Each row of the tree view corresponds to one parameter. The subparameters are processed recursively.
 *
 * There are three columns in the tree view: label (name of parameter), input (value of parameter), and state icon (state of parameter).
 *
 * The widget is created using the parameter passed as an argument of the constructor. The parameter can be replaced using setParam method. The view is then redrawn instantly.
 *
 * Parameter constructed from current values of the inputs can be obtained using param method. All inputs are validated and successful validation is indicated by *ok argument. Error state icons are drawn next to each invalid input.
 */
class TConfigParamWidget : public QTreeWidget
{
    Q_OBJECT

public:
    TConfigParamWidget(const TConfigParam & param, QWidget * parent = nullptr);
    ~TConfigParamWidget();

public slots:
    TConfigParam param(bool * ok = nullptr);
    void setParam(const TConfigParam & param);

protected:
    bool readParam(TConfigParam & param, QTreeWidgetItem * parent);
    void addParam(TConfigParam & param, QTreeWidgetItem * parent);
    void refreshParam();

private:
    void drawLabel(const TConfigParam & param, QTreeWidgetItem * parent);
    void drawState(const TConfigParam & param, QTreeWidgetItem * parent);
    void drawInput(const TConfigParam & param, QTreeWidgetItem * parent);

    bool checkInput(TConfigParam & param, QTreeWidgetItem * parent);

    TConfigParam m_param;

    static QIcon stateIcon(TConfigParam::TState state, bool isLightened = false);
    static QString stateColor(TConfigParam::TState state);
};
#endif // TCONFIGPARAMWIDGET_H
