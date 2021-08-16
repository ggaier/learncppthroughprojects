#include "analogclock.h"

#include <QPainter>
#include <QTime>
#include <QTimer>

AnalogClock::AnalogClock(QWidget* parent)
    : QWidget(parent)
{
    //提供了timer的高级编程接口.要使用QTimer, 要把timeout()的信号连接到对应的slots上
    //然后调用start()方法
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&AnalogClock::update));
    //每个一秒钟收到一次timeout方法
    timer->start(1000);

    setWindowTitle(tr("Analog Clock"));
    resize(200, 200);
}

void AnalogClock::paintEvent(QPaintEvent* event)
{
    //时针的三角形
    static const QPoint hourHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };

    static const QPoint minuteHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -70)
    };
    QColor hourColor(127, 0, 127);
    QColor minuteColor(0, 127, 127, 191);

    int side = qMin(width(), height());
    QTime time = QTime::currentTime();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);
    painter.save();
    //一圈总共12个小时, 所以一个小时30度
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    //画出12根小时时间线
    painter.setPen(hourColor);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);
    painter.save();
    painter.rotate(6.0*(time.minute()+time.second()/60.0));
    painter.drawConvexPolygon(minuteHand, 3);
    painter.restore();

    painter.setPen(minuteColor);
    //一小时60分钟, 每分钟都要画一个分针时间线.
    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0) {
            painter.drawLine(92, 0, 96, 0);
        }
        painter.rotate(6);
    }


}
















