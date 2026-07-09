#ifndef ACTIONCARD_H
#define ACTIONCARD_H

#include <QFrame>
#include <QColor>

class QLabel;
class QPushButton;
class QGraphicsDropShadowEffect;
class QPropertyAnimation;
class QSvgWidget;

class ActionCard : public QFrame
{
    Q_OBJECT

public:
    explicit ActionCard(const QString &iconPath,
                        const QString &title,
                        const QString &description,
                        const QString &buttonText,
                        const QColor &accent,
                        QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QWidget *topAccent;

    QLabel *titleLabel;
    QLabel *descriptionLabel;

    QPushButton *button;

    QSvgWidget *icon;

    QGraphicsDropShadowEffect *shadow;

    QColor accentColor;

    QRect originalGeometry;
};

#endif
