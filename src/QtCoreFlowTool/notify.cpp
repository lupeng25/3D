﻿#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPixmap>
#include <QFontMetrics>
#include <QDesktopServices>
#include "notify.h"

Notify::Notify (int displayTime, QWidget *parent) : QWidget(parent),
    displayTime(displayTime)
{
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|
                         Qt::Tool | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    backgroundLabel = new QLabel(this);
    backgroundLabel->move(0, 0);
    backgroundLabel->setObjectName("notify-background");

    QHBoxLayout *mainLayout = new QHBoxLayout(backgroundLabel);
    QVBoxLayout *contentLayout = new QVBoxLayout();

    iconLabel = new QLabel(backgroundLabel);
    iconLabel->setFixedWidth(30);
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel(backgroundLabel);
    titleLabel->setObjectName("notify-title");
	titleLabel->setWordWrap(true);

    bodyLabel = new QLabel(backgroundLabel);
	bodyLabel->setWordWrap(true);
    bodyLabel->setObjectName("notify-body");
    QFont font = bodyLabel->font();
    font.setPixelSize(20);
    bodyLabel->setFont(font);

    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(bodyLabel);

    mainLayout->addWidget(iconLabel);
    mainLayout->addSpacing(0);
    mainLayout->addLayout(contentLayout);

    closeBtn = new QPushButton("", backgroundLabel);
    closeBtn->setObjectName("notify-close-btn");
	closeBtn->setIcon(QIcon(":/skin/icon/icon/icons/light/appbar.close.png"));
    closeBtn->setFixedSize(60, 40);
    connect(closeBtn, &QPushButton::clicked, this, [this]{ Q_EMIT disappeared();});

    this->setStyleSheet("#notify-background {"
                        "border: 1px solid #ccc;"
                        "background:white;"
                        "border-radius: 4px;"
                        "} "
                        "#notify-title{"
                        "font-weight: bold;"
                        "color: #333;"
                        "font-size: 14px;"
                        "}"
                        "#notify-body{"
                        "color: #444;"
                        "}"
                        "#notify-close-btn{ "
                        "border: 0;"
                        "color: #999;"
                        "}"
                        "#notify-close-btn:hover{ "
                        "background: #ccc;"
                        "}");
}

void Notify::showGriant()
{
    this->show();

    titleLabel->setText(title);
    QPixmap tempPix = QPixmap(this->icon);
    tempPix = tempPix.scaled(QSize(30, 30), Qt::KeepAspectRatioByExpanding);
    iconLabel->setPixmap(tempPix);

    backgroundLabel->setFixedSize(this->size());
    closeBtn->move(backgroundLabel->width() - closeBtn->width(), 0);

    bodyLabel->setText(body);
	bodyLabel->setToolTip(body);
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
        animation->deleteLater();
        QTimer::singleShot(displayTime, this, [this](){
            this->hideGriant();
        });
    });
}

void Notify::hideGriant()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(this->windowOpacity());
    animation->setEndValue(0);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
        this->hide();
        animation->deleteLater();
        Q_EMIT disappeared();
    });
}

void Notify::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        hideGriant();
    }
}


void Notify::setBody(const QString &value)
{
    body = value;
}

void Notify::setTitle(const QString &value)
{
    title = value;
}

void Notify::setIcon(const QString &value)
{
    icon = value;
}