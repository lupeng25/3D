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

#include "fcenterwindow.h"
#include "ftitlebar.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QLabel>
#include <QTime>


FCenterWindow::FCenterWindow(QWidget *parent)
	:QFrame(parent)
{
	//qDebug("FCenterWindow init");
	initData();
	initUI();
	initConnect();
}

void FCenterWindow::initData()
{
	preindex = 0;
}

void FCenterWindow::initUI()
{
	setObjectName(QString("FCenterWindow"));
	navagationBar = new FNavgationBar();
	OtherNavagationBar = new FNavgationBar();
	stackWidget = new QStackedWidget();

	navlayout = new QBoxLayout(QBoxLayout::TopToBottom);

	Barlayout = new QBoxLayout(QBoxLayout::TopToBottom);
	Barlayout->addWidget(navagationBar);
	Barlayout->addWidget(OtherNavagationBar);
	Barlayout->setStretch(0, 1);
	//navlayout->addWidget(navagationBar);
	navlayout->addLayout(Barlayout);
	//navlayout->addWidget(OtherNavagationBar);
	//OtherNavagationBar->addNavgationTile("UserPage", "UserPage");
	//OtherNavagationBar->setStyleSheet("background-color: transparent;");


	navlayout->addWidget(stackWidget);
	navlayout->setContentsMargins(0, 0, 0, 0);
	navlayout->setSpacing(0);

	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(FTitleBar::getInstace());
	mainLayout->addLayout(navlayout);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);
}

void FCenterWindow::swicthLayout(QBoxLayout::Direction direction)
{
	navlayout->setDirection(direction);
	//Barlayout->setDirection(direction);
}

void FCenterWindow::setAlignment(Alignment_Direction direction)
{
	switch (direction) {
	case TopLeft:
		navagationBar->setObjectName(QString("FNavgationBar_bottom"));
		navagationBar->setAlignment_topLeft();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_bottom"));
		OtherNavagationBar->setAlignment_topRight();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::TopToBottom);
		break;
	case TopCenter:
		navagationBar->setObjectName(QString("FNavgationBar_bottom"));
		navagationBar->setAlignment_topCenter();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_bottom"));
		OtherNavagationBar->setAlignment_topRight();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::TopToBottom);
		break;
	case TopRight:
		navagationBar->setObjectName(QString("FNavgationBar_bottom"));
		navagationBar->setAlignment_topRight();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_bottom"));
		OtherNavagationBar->setAlignment_topLeft();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::TopToBottom);
		break;
	case RightTop:
		navagationBar->setObjectName(QString("FNavgationBar_left"));
		navagationBar->setAlignment_rightTop();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_left"));
		OtherNavagationBar->setAlignment_rightBottom();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::RightToLeft);
		break;
	case RightCenter:
		navagationBar->setObjectName(QString("FNavgationBar_left"));
		navagationBar->setAlignment_rightCenter();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_left"));
		OtherNavagationBar->setAlignment_rightBottom();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::RightToLeft);
		break;
	case RightBottom:
		navagationBar->setObjectName(QString("FNavgationBar_left"));
		navagationBar->setAlignment_rightBottom();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_left"));
		OtherNavagationBar->setAlignment_rightTop();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::RightToLeft);
		break;
	case BottomRight:
		navagationBar->setObjectName(QString("FNavgationBar_top"));
		navagationBar->setAlignment_bottomRight();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_top"));
		OtherNavagationBar->setAlignment_bottomLeft();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::BottomToTop);
		break;
	case BottomCenter:
		navagationBar->setObjectName(QString("FNavgationBar_top"));
		navagationBar->setAlignment_bottomCenter();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_top"));
		OtherNavagationBar->setAlignment_bottomRight();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::BottomToTop);
		break;
	case BottomLeft:
		navagationBar->setObjectName(QString("FNavgationBar_top"));
		navagationBar->setAlignment_bottomLeft();
		navagationBar->resize(QSize(stackWidget->width(), navagationBar->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_top"));
		OtherNavagationBar->setAlignment_bottomRight();
		OtherNavagationBar->resize(QSize(OtherNavagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::BottomToTop);
		break;
	case LeftBottom:
		navagationBar->setObjectName(QString("FNavgationBar_right"));
		navagationBar->setAlignment_leftBottom();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_right"));
		OtherNavagationBar->setAlignment_leftTop();
		OtherNavagationBar->resize(QSize(navagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::LeftToRight);
		break;
	case LeftCenter:
		navagationBar->setObjectName(QString("FNavgationBar_right"));
		navagationBar->setAlignment_leftCenter();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_right"));
		OtherNavagationBar->setAlignment_leftBottom();
		OtherNavagationBar->resize(QSize(navagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::LeftToRight);
		break;
	case LeftTop:
		navagationBar->setObjectName(QString("FNavgationBar_right"));
		navagationBar->setAlignment_leftTop();
		navagationBar->resize(QSize(navagationBar->width(), stackWidget->height()));
		navagationBar->adjustSize();

		OtherNavagationBar->setObjectName(QString("FNavgationBar_right"));
		OtherNavagationBar->setAlignment_leftBottom();
		OtherNavagationBar->resize(QSize(navagationBar->width(), OtherNavagationBar->height()));
		OtherNavagationBar->adjustSize();
		swicthLayout(QBoxLayout::LeftToRight);
		break;
	default:
		break;
	}
}

void FCenterWindow::initConnect()
{
	connect(navagationBar, SIGNAL(indexChanged(int)), this, SLOT(switchscreen(int)));
	connect(OtherNavagationBar, SIGNAL(sigButtonClicked(int)), this, SIGNAL(sigOtherBtnClicked(int)));
}

void FCenterWindow::addWidget(const QString &tile, const QString &obejctName, QWidget *widget)
{
	navagationBar->addNavgationTile(tile, obejctName);
	widget->setObjectName(obejctName);
	stackWidget->addWidget(widget);
}

void FCenterWindow::addOtherButton(const QString & tile, const QString & Name)
{
	OtherNavagationBar->addNavgationTile(tile, Name);
}

FNavgationBar* FCenterWindow::getNavgationBar() {
	return navagationBar;
}

FNavgationBar * FCenterWindow::getOtherNavgationBar()
{
	return OtherNavagationBar;
}

void FCenterWindow::ChangeLanguage()
{
}

void FCenterWindow::changeEvent(QEvent * ev)
{
	switch (ev->type()) {
	case QEvent::LanguageChange: {
		ChangeLanguage();
	}	break;
	default:	break;
	}
}

bool FCenterWindow::switchscreen(const int index)
{
	int n = 0;
	if (index == stackWidget->currentIndex())
	{
		return true;
	}
	else if (index > stackWidget->currentIndex())
	{
		n = 2;
	}
	else if (index < stackWidget->currentIndex())
	{
		n = 6;
	}
	if (emit sigScreenSwitch(index) < 0)
	{
		return false;
	}

	stackWidget->setCurrentIndex(index);

	//    QTime time;
	//    time= QTime::currentTime();
	//    qsrand(time.msec()+time.second()*1000);
	//    int n = qrand()%9;
	//    n = 0;
	switch (n) {
	case 0:
		cloudAntimation(animationTop);
		break;
	case 1:
		cloudAntimation(animationTopRight);
		break;
	case 2:
		cloudAntimation(animationRight);
		break;
	case 3:
		cloudAntimation(animationBottomRight);
		break;
	case 4:
		cloudAntimation(animationBottom);
		break;
	case 5:
		cloudAntimation(animationBottomLeft);
		break;
	case 6:
		cloudAntimation(animationLeft);
		break;
	case 7:
		cloudAntimation(animationTopLeft);
		break;
	case 8:
		cloudAntimation(animationCenter);
		break;
	default:
		break;
	}
	preindex = index;
	return true;

}

void FCenterWindow::switchscreen()
{
	stackWidget->setCurrentIndex(currentIndex);
}

void FCenterWindow::cloudAntimation(animation_Direction direction)
{
	QLabel* circle = new QLabel(stackWidget->currentWidget());
	QLabel* line = new QLabel(this);
	line->setObjectName(QString("AntimationLine"));
	line->resize(0, 2);
	line->show();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	circle->setPixmap(QPixmap::grabWidget(stackWidget->widget(preindex), stackWidget->widget(preindex)->geometry()));
#else
	circle->setPixmap(stackWidget->widget(preindex)->grab());
#endif

	//    circle->setScaledContents(true);
	circle->show();
	circle->resize(stackWidget->currentWidget()->size());
	QPropertyAnimation *animation = new QPropertyAnimation(circle, "geometry");

	animation->setDuration(1000);
	animation->setStartValue(circle->geometry());

	QPropertyAnimation* animation_line = new QPropertyAnimation(line, "size");
	animation_line->setDuration(1000);
	animation_line->setEasingCurve(QEasingCurve::OutCubic);

	switch (direction) {
	case animationTop:
		animation->setEndValue(QRect(circle->x(), circle->y() - 10, circle->width(), 0));
		break;
	case animationTopRight:
		animation->setEndValue(QRect(circle->width(), 0, 0, 0));
		break;
	case animationRight:
		line->move(0, stackWidget->y() - 2);
		animation->setEndValue(QRect(circle->width() + 3, 0, 0, circle->height()));
		animation_line->setStartValue(QSize(0, 2));
		animation_line->setEndValue(QSize(stackWidget->width(), 2));
		break;
	case animationBottomRight:
		animation->setEndValue(QRect(circle->width(), circle->height(), 0, 0));
		break;
	case animationBottom:
		animation->setEndValue(QRect(0, circle->height() + 10, circle->width(), 0));
		break;
	case animationBottomLeft:
		animation->setEndValue(QRect(0, circle->height(), 0, 0));
		break;
	case animationLeft:
		animation->setEndValue(QRect(-3, 0, 0, circle->height()));
		line->move(stackWidget->x(), stackWidget->y() - 2);
		animation_line->setStartValue(QSize(0, 2));
		animation_line->setEndValue(QSize(stackWidget->width(), 2));
		break;
	case animationTopLeft:
		animation->setEndValue(QRect(0, 0, 0, 0));
		break;
	case animationCenter:
		animation->setEndValue(QRect(circle->width() / 2, circle->height() / 2, 0, 0));
		break;
	default:
		break;
	}
	animation->setEasingCurve(QEasingCurve::OutCubic);

	QPropertyAnimation* animation_opacity = new QPropertyAnimation(circle, "windowOpacity");
	animation_opacity->setDuration(1000);
	animation_opacity->setStartValue(1);
	animation_opacity->setEndValue(0);
	animation_opacity->setEasingCurve(QEasingCurve::OutCubic);

	QParallelAnimationGroup *group = new QParallelAnimationGroup;

	connect(group, SIGNAL(finished()), circle, SLOT(hide()));
	connect(group, SIGNAL(finished()), circle, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), line, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation_opacity, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation_line, SLOT(deleteLater()));
	group->addAnimation(animation);
	group->addAnimation(animation_opacity);
	group->addAnimation(animation_line);
	group->start();
}
