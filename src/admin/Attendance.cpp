#include "Attendance.h"

#include <QPainter>

Attendance::Attendance(QWidget *parent)
    : QWidget(parent)
{
    present = 70;
    late = 15;
    absent = 15;

    setMinimumSize(170,170);
}

void Attendance::setValues(int p,int l,int a)
{
    present=p;
    late=l;
    absent=a;

    update();
}

void Attendance::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);

    QRectF rect(15,15,width()-30,height()-30);

    int total=present+late+absent;

    if(total==0)
        return;

    double start=90;

    auto drawPiece=[&](QColor color,int value)
    {
        p.setPen(Qt::NoPen);

        p.setBrush(color);

        double angle=360.0*value/total;

        p.drawPie(rect,
                  start*16,
                  -angle*16);

        start-=angle;
    };

    drawPiece(QColor("#4CAF50"),present);
    drawPiece(QColor("#C9A227"),late);
    drawPiece(QColor("#D9534F"),absent);

    p.setBrush(QColor("#121712"));

    p.drawEllipse(rect.adjusted(35,35,-35,-35));

    p.setPen(Qt::white);

    p.setFont(QFont("Segoe UI",18,QFont::Bold));

    p.drawText(rect,
               Qt::AlignCenter,
               QString("%1%").arg(present));
}