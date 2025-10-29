#ifndef TLOGLINEWIDGET_H
#define TLOGLINEWIDGET_H

#include <QLabel>

class TLogLineWidget : public QLabel {
    Q_OBJECT

public:
    explicit TLogLineWidget(QWidget * parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent * event) override;
};

#endif // TLOGLINEWIDGET_H
