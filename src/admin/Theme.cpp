#include "Theme.h"

#include <QPainter>

Theme::Theme(QWidget *parent)
    : QWidget(parent)
{
    present = 70;
    late = 15;
    absent = 15;

    setMinimumSize(170,170);
}

void Theme::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF rect(15, 15, width() - 30, height() - 30);

    int total = present + late + absent;

    if (total == 0)
        return;

    double start = 90;

    auto drawPiece = [&](const QColor &color, int value)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(color);

        double angle = 360.0 * value / total;

        p.drawPie(rect,
                  start * 16,
                  -angle * 16);

        start -= angle;
    };

    // Draw chart sections
    drawPiece(Theme::Present, present);
    drawPiece(Theme::Late, late);
    drawPiece(Theme::Absent, absent);

    // Inner circle
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::Background);
    p.drawEllipse(rect.adjusted(35, 35, -35, -35));

    // Percentage text
    p.setPen(Theme::TextPrimary);
    p.setFont(QFont("", 18, QFont::Bold));

    p.drawText(rect,
               Qt::AlignCenter,
               QString("%1%").arg(present));
}