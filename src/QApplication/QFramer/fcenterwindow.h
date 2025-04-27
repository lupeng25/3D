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

#ifndef FCENTERWINDOW_H
#define FCENTERWINDOW_H

#include <QFrame>
#include <QWidget>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QPixmap>
#include <QVector>
#include "fnavgationbar.h"
#include "Pages/QtWidgetsBase.h"

class FCenterWindow : public QFrame
{
	Q_OBJECT
private:
	FNavgationBar* navagationBar{ nullptr };
	FNavgationBar* OtherNavagationBar{ nullptr };

	QStackedWidget* stackWidget{ nullptr };
	QBoxLayout* navlayout{ nullptr };
	QVBoxLayout* mainLayout{ nullptr };
	QBoxLayout* Barlayout{ nullptr };
	int currentIndex;
	int preindex;
public:
	enum Alignment_Direction {
		TopLeft,
		TopCenter,
		TopRight,
		RightTop,
		RightCenter,
		RightBottom,
		BottomRight,
		BottomCenter,
		BottomLeft,
		LeftBottom,
		LeftCenter,
		LeftTop,
	};

	enum animation_Direction {
		animationTop,
		animationRight,
		animationBottom,
		animationLeft,
		animationTopLeft,
		animationTopRight,
		animationBottomRight,
		animationBottomLeft,
		animationCenter
	};
public:
	explicit FCenterWindow(QWidget *parent = 0);
	void initData();
	void initUI();
	void initConnect();
	void addWidget(const QString& tile, const QString &obejctName, QWidget* widget);
	void addOtherButton(const QString& tile, const QString &Name);

	void setAlignment(Alignment_Direction direction);
	FNavgationBar* getNavgationBar();
	FNavgationBar* getOtherNavgationBar();

protected:
	virtual void ChangeLanguage();
protected:
	void changeEvent(QEvent * ev) override;
public:
signals :
	int sigScreenSwitch(int);
	int sigOtherBtnClicked(int);
public slots:
	bool switchscreen(const int index);
	void switchscreen();
	void swicthLayout(QBoxLayout::Direction direction);
	virtual void cloudAntimation(animation_Direction direction);
};

#endif // FCENTERWINDOW_H
