#include "CircularProgress.h"
#include "Theme.h"
#include <QPainter>
#include <QFont>

CircularProgress::CircularProgress(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(120, 120);
}

void CircularProgress::setPercentage(int pct) {
    pct_ = qBound(0, pct, 100);
    update();
}

void CircularProgress::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    QRectF r(8, 8, side - 16, side - 16);

    // Background ring
    p.setPen(QPen(QColor(Theme::Input), 10));
    p.setBrush(Qt::NoBrush);
    p.drawArc(r, 0, 360 * 16);

    // Foreground arc
    p.setPen(QPen(QColor(Theme::Gold), 10, Qt::SolidLine, Qt::RoundCap));
    int span = pct_ * 360 / 100;
    p.drawArc(r, 90 * 16, -span * 16);

    // Percentage text
    QFont f = font();
    f.setPointSize(24);
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor(Theme::Primary));
    p.drawText(r, Qt::AlignCenter, QString::number(pct_) + "%");

    // "Present" label
    QFont sf = font();
    sf.setPointSize(10);
    p.setFont(sf);
    p.setPen(QColor(Theme::Secondary));
    QRectF labelRect = r.adjusted(0, 40, 0, 0);
    p.drawText(labelRect, Qt::AlignCenter, "Present");
}
