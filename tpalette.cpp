#include "tpalette.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>

QColor TPalette::color(TPalette::ColorRole colorRole)
{
    bool dark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;

    if (colorRole == TPalette::ColorRole::ErrorTooltipText)
        return dark ? QColor(255, 85, 85) : Qt::darkRed;

    if (colorRole == TPalette::ColorRole::WarningTooltipText)
        return dark ? QColor(255, 169, 77) : QColor(255, 140, 0);

    if (colorRole == TPalette::ColorRole::InfoTooltipText)
        return dark ? QColor(85, 170, 255) : Qt::darkBlue;

    if (colorRole == TPalette::ColorRole::EditorCurrentLineHighlight)
        return dark ? QColor(80, 80, 40) : QColor(Qt::yellow).lighter(160);

    if (colorRole == TPalette::ColorRole::CommunicationLogReceivedHighlight)
        return dark ? QColor(255, 120, 120) : Qt::red;

    if (colorRole == TPalette::ColorRole::CommunicationLogSentHighlight)
        return dark ? QColor(100, 180, 255) : Qt::blue;

    if (colorRole == TPalette::ColorRole::ErrorBase)
        return dark ? QColor(120, 40, 40) : QColor(255, 200, 200);

    return QColor();
}

QChart::ChartTheme TPalette::chartTheme()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? QChart::ChartThemeDark : QChart::ChartThemeLight;
}
