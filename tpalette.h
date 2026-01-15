#ifndef TPALETTE_H
#define TPALETTE_H

#include <QColor>
#include <QChart>

class TPalette
{
public:
    TPalette() = delete;
    TPalette(const TPalette&) = delete;
    TPalette& operator=(const TPalette&) = delete;

    enum ColorRole {
        ErrorTooltipText,
        WarningTooltipText,
        InfoTooltipText
    };
    static QColor color(ColorRole colorRole);

    static QChart::ChartTheme chartTheme();
};

#endif // TPALETTE_H
