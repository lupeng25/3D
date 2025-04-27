/****************************************************************************
**
** Copyright (C) 2014 dragondjf
**
** QFramer is a frame based on Qt5.3, you will be more efficient with it. 
** As an Qter, Qt give us a nice coding experience. With user interactive experience(UE) 
** become more and more important in modern software, deveployers should consider business and UE.
** So, QFramer is born. QFramer's goal is to be a mature solution 
** which you only need to be focus on your business but UE for all Qters.
**
** Version	: 0.2.5.0
** Author	: dragondjf
** Website	: https://github.com/dragondjf
** Project	: https://github.com/dragondjf/QCFramer
** Blog		: http://my.oschina.net/dragondjf/home/?ft=atme
** Wiki		: https://github.com/dragondjf/QCFramer/wiki
** Lincence: LGPL V2
** QQ: 465398889
** Email: dragondjf@gmail.com, ding465398889@163.com, 465398889@qq.com
** 
****************************************************************************/

#include "aboutdialog.h"
#include "QFramer/fbasepushbutton.h"
#include "QFramer/fshadowlabel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDesktopWidget>
#include <QApplication>
#include "Data/dataVar.h"


AboutDialog::AboutDialog(QWidget *parent) :
    FBaseDialog(parent)
{
    initUI();
    initConnect();
	LanguageChange();
}

void AboutDialog::initUI()
{
    normalSize = QSize(700, 500);

    FShadowLabel *authorLabel = new FShadowLabel(tr("author:MCS"));
    authorLabel->setObjectName(QString("content"));

    FShadowLabel *versionLabel = new FShadowLabel(tr("version:") + dataVar::Instance().software_vision);
    versionLabel->setObjectName(QString("content"));

	FShadowLabel *UpdateLabel = new FShadowLabel(dataVar::Instance().software_Info);
	UpdateLabel->setObjectName(QString("content"));
	UpdateLabel->setAlignment(Qt::AlignLeft);


	FShadowLabel *copyrightLabel = new FShadowLabel(dataVar::Instance().software_CompanyName/*tr("Copyright 2024-2050 dongguan kaiyirong. All rights reserved")*/, this);
	copyrightLabel->setObjectName(QString("version"));

    enterButton = new FBasePushButton();
    enterButton->setText(tr("Close"));
    QHBoxLayout* controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(enterButton);
    controlLayout->addSpacing(10);

    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(authorLabel);
	mainLayout->addSpacing(10);
    mainLayout->addWidget(versionLabel);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(UpdateLabel,4);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(copyrightLabel);
	mainLayout->addSpacing(10);
    mainLayout->addLayout(controlLayout);
    mainLayout->addSpacing(10);

}

void AboutDialog::initConnect()
{
//    BaseDialog::initConnect();
    connect(enterButton, SIGNAL(clicked()), this, SLOT(animationClose()));
}

void AboutDialog::LanguageChange()
{
	getTitleBar()->getContentLabel()->setText(tr("AboutDialog"));
}

void AboutDialog::showEvent(QShowEvent *event)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");

    QDesktopWidget* desktopWidget = QApplication::desktop();
    int x = (desktopWidget->availableGeometry().width() - normalSize.width()) / 2;
    int y = (desktopWidget->availableGeometry().height() - normalSize.height()) / 2;

    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->setDuration(1500);
    animation->setStartValue(QRect(x, 0, normalSize.width(), normalSize.height()));
    animation->setEndValue(QRect(x, y, normalSize.width(), normalSize.height()));
    animation->setEasingCurve(QEasingCurve::OutElastic);
    animation->start();
    QDialog::showEvent(event);
}

void AboutDialog::animationClose()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    connect(animation, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(animation, SIGNAL(finished()), this, SLOT(close()));
    animation->setDuration(1500);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
}
