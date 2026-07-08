#include "ActionCard.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QSvgWidget>
#include <QEnterEvent>


ActionCard::ActionCard(const QString &iconPath,
                       const QString &title,
                       const QString &description,
                       const QString &buttonText,
                       const QColor &accentColor,
                       QWidget *parent )
    ActionCard::ActionCard(const QString &iconPath,
                           const QString &title,
                           const QString &description,
                           const QString &buttonText,
                           const QColor &accentColor,
                           QWidget *parent)
    : QFrame(parent),
    m_accentColor(accentColor)
{
    setFixedSize(360, 340);

    setObjectName("card");

    setStyleSheet(R"(
        QFrame#card{
            background:#1E293B;
            border:1px solid #334155;
            border-radius:20px;
        }

        QLabel{
            border:none;
            background:transparent;
        }

        QPushButton{
            background:#3B82F6;
            color:white;
            border:none;
            border-radius:10px;
            padding:12px;
            font-family:"Montserrat";
            font-size:14px;
            font-weight:600;
        }

        QPushButton:hover{
            background:#2563EB;
        }
    )");

    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setOffset(0,10);
    shadow->setColor(QColor(0,0,0,120));

    setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28,28,28,28);
    layout->setSpacing(18);

    icon = new QSvgWidget(iconPath, this);
    icon->setFixedSize(60,60);

    titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    titleLabel->setStyleSheet(R"(
        font-family:"Playfair Display";
        font-size:20px;
        font-weight:700;
        color:white;
    )");

    descriptionLabel = new QLabel(description);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);

    descriptionLabel->setStyleSheet(R"(
        font-family:"Montserrat";
        font-size:14px;
        color:#94A3B8;
    )");

    button = new QPushButton(buttonText + "  →");
    button->setCursor(Qt::PointingHandCursor);

    connect(button,
            &QPushButton::clicked,
            this,
            &ActionCard::clicked);

    layout->addWidget(icon,0,Qt::AlignCenter);
    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();
    layout->addWidget(button);

    originalGeometry = geometry();
}

void ActionCard::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);

    setStyleSheet(R"(
        QFrame#card{
            background:#1E293B;
            border:2px solid #3B82F6;
            border-radius:20px;
        }

        QLabel{
            border:none;
            background:transparent;
        }

        QPushButton{
            background:#3B82F6;
            color:white;
            border:none;
            border-radius:10px;
            padding:12px;
        }
    )");

    shadow->setBlurRadius(45);
}

void ActionCard::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);

    setStyleSheet(R"(
        QFrame#card{
            background:#1E293B;
            border:1px solid #334155;
            border-radius:20px;
        }

        QLabel{
            border:none;
            background:transparent;
        }

        QPushButton{
            background:#3B82F6;
            color:white;
            border:none;
            border-radius:10px;
            padding:12px;
        }
    )");

    shadow->setBlurRadius(30);
}