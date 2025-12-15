#include "tloglinewidget.h"
#include "qevent.h"

TLogLineWidget::TLogLineWidget(QWidget * parent) : QLabel(parent) {}

void TLogLineWidget::mousePressEvent(QMouseEvent * event) {
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QLabel::mousePressEvent(event);
}
