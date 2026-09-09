#include "tpalette.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>
#include <QImage>
#include <QPainter>

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

    if (colorRole == TPalette::ColorRole::WarningBase)
        return dark ? QColor(110, 90, 40) : QColor(255, 255, 200);

    if (colorRole == TPalette::ColorRole::InfoBase)
        return dark ? QColor(40, 69, 90) : QColor(225, 235, 255);

    return QColor();
}

QChart::ChartTheme TPalette::chartTheme()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? QChart::ChartThemeDark : QChart::ChartThemeLight;
}

/*! Returns true if every opaque pixel is (near-)grey, i.e. the artwork carries
 *  no hue of its own and can safely be recoloured as a single-colour glyph. */
static bool isMonochrome(const QImage & image)
{
    qint64 opaqueCount = 0;
    qint64 greyCount = 0;

    for (int y = 0; y < image.height(); y++) {
        const QRgb * line = reinterpret_cast<const QRgb *>(image.constScanLine(y));

        for (int x = 0; x < image.width(); x++) {
            const QRgb pixel = line[x];

            if (qAlpha(pixel) <= ALPHA_THRESHOLD)
                continue;

            opaqueCount++;

            const int maximum = qMax(qMax(qRed(pixel), qGreen(pixel)), qBlue(pixel));
            const int minimum = qMin(qMin(qRed(pixel), qGreen(pixel)), qBlue(pixel));

            if (maximum - minimum <= SATURATION_THRESHOLD)
                greyCount++;
        }
    }

    return opaqueCount > 0 && (100 * greyCount / opaqueCount) >= MONOCHROME_PERCENTAGE;
}

QPixmap TPalette::themedPixmap(const QPixmap & pixmap)
{
    if (pixmap.isNull())
        return pixmap;

    if (QGuiApplication::styleHints()->colorScheme() != Qt::ColorScheme::Dark)
        return pixmap;

    if (!isMonochrome(pixmap.toImage().convertToFormat(QImage::Format_ARGB32)))
        return pixmap;

    QPixmap tinted(pixmap.size());
    tinted.setDevicePixelRatio(pixmap.devicePixelRatio());
    tinted.fill(Qt::transparent);

    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), QGuiApplication::palette().color(QPalette::WindowText));
    painter.end();

    return tinted;
}

QIcon TPalette::themedIcon(const QString & resourcePath)
{
    return QIcon(themedPixmap(QPixmap(resourcePath)));
}
