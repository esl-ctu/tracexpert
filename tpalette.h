#ifndef TPALETTE_H
#define TPALETTE_H

#include <QColor>

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
};

#endif // TPALETTE_H
