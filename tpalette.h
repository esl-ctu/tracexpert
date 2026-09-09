#ifndef TPALETTE_H
#define TPALETTE_H

#include <QColor>
#include <QChart>
#include <QIcon>
#include <QPixmap>

#define ALPHA_THRESHOLD 32
#define SATURATION_THRESHOLD 16
#define MONOCHROME_PERCENTAGE 95

class TPalette
{
public:
    TPalette() = delete;
    TPalette(const TPalette&) = delete;
    TPalette& operator=(const TPalette&) = delete;

    enum ColorRole {
        ErrorTooltipText,
        WarningTooltipText,
        InfoTooltipText,
        EditorCurrentLineHighlight,
        CommunicationLogReceivedHighlight,
        CommunicationLogSentHighlight,
        ErrorBase,
        WarningBase,
        InfoBase
    };
    static QColor color(ColorRole colorRole);

    static QChart::ChartTheme chartTheme();

    /*! Recolours monochrome (single-hue) pixmaps to QPalette::WindowText when a
     *  dark scheme is active. Full-colour icons and light schemes are left as-is. */
    static QPixmap themedPixmap(const QPixmap & pixmap);
    static QIcon themedIcon(const QString & resourcePath);
};

#endif // TPALETTE_H
