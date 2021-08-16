#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

#include <QObject>
#include <QPaintEvent>
#include <QWidget>

class AnalogClock : public QWidget
{
    Q_OBJECT
public:
    explicit AnalogClock(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

};

#endif // ANALOGCLOCK_H
