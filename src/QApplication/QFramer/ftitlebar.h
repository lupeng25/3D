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

#ifndef FTITLEBAR_H
#define FTITLEBAR_H

#include "ftoolbutton.h"
#include <QFrame>
#include <QLabel>
#include <QVector>
#include <QHBoxLayout>

#define Title_Height 50

class FTitleBar : public QFrame
{
	Q_OBJECT
private:
	QIcon* logoIcon{ nullptr };
	QIcon* settingsIcon{ nullptr };
	QIcon* skinIcon{ nullptr };
	QIcon* minIcon{ nullptr };
	QIcon* maxIcon{ nullptr };
	QIcon* normalIcon{ nullptr };
	QIcon* closeIcon{ nullptr };
	QLabel* titleLabel{ nullptr };
	QLabel* ContentLabel{ nullptr };
	FToolButton* logoButton{ nullptr };
	FToolButton* settingButton{ nullptr };
	FToolButton* skinButton{ nullptr };
	FToolButton* fixButton{ nullptr };
	FToolButton* minButton{ nullptr };
	FToolButton* maxButton{ nullptr };
	FToolButton* closeButton{ nullptr };
	bool normal_max_flag;
	bool fixflag;
	QHBoxLayout* m_playout{ nullptr };
	QVector<FToolButton*> m_vecpBtn;

	//最大化、最小化变量
	QPoint m_restorePos;
	QSize m_restoreSize;
public:
	void RegisterBuutton(FToolButton* pbtn);

private:
	void initData();
	void initConnect();
	void initUI();
	void ChangeLanguage();
public:
	QString title;
	static FTitleBar* instance;
public:
	explicit FTitleBar(QWidget *parent = 0);
	static FTitleBar* getInstace();

	bool getFixedflag();
	bool getNormalMaxFlag();

	void setTitleBarHeight(int height);
	void setTitleBarWidth(int width);
	void setSettingMenu(QMenu* menu);

	void setLogoButton(const char * str, const char *objectName = NULL);
	FToolButton* getLogoButton();
	void setLogoButtonVisible(bool visible);
	bool isLogoButtonVisible();

	void setTitleLabel(QString str, const char *objectName = NULL);
	QLabel* getTitleLabel();
	void setTitleLabelVisible(bool visible);
	bool isTitleLabelVisible();

	void setContentLabel(QString str, const char *objectName = NULL);
	QLabel* getContentLabel();
	void setContentLabelVisible(bool visible);
	bool isContentLabelVisible();

	void setSettingButton(const char * str, const char *objectName = NULL);
	FToolButton* getSettingButton();
	void setSettingButtonVisible(bool visible);
	bool isSettingButtonVisible();

	void setSkinButton(const char * str, const char *objectName = NULL);
	FToolButton* getSkinButton();
	void setSkinButtonVisible(bool visible);
	bool isSkinButtonVisible();

	void setFixButton(const char * str, const char *objectName = NULL);
	FToolButton* getFixButton();
	void setFixButtonVisible(bool visible);
	bool isFixButtonVisible();

	void setMinButton(const char * str, const char *objectName = NULL);
	FToolButton* getMinButton();
	void setMinButtonVisible(bool visible);
	bool isMinButtonVisible();

	void setMaxButton(const char * str, const char *objectName = NULL);
	FToolButton* getMaxButton();
	void setMaxButtonVisible(bool visible);
	bool isMaxButtonVisible();

	void setCloseButton(const char * str, const char *objectName = NULL);
	FToolButton* getCloseButton();
	void setCloseButtonVisible(bool visible);
	bool isCloseButtonVisible();

	//保存最大化前窗口的位置及大小
	void saveRestoreInfo(const QPoint point, const QSize size);
	//获取最大化前窗口的位置及大小
	void getRestoreInfo(QPoint& point, QSize& size);
protected:
	void changeEvent(QEvent *) override;
signals:
	void maximumed();
	void minimuned();
	void normaled();
	void closed();

public slots:
	void switchMaxMin();
	void changeSkin();
	void clearChecked();
	void changeFix();
	void updateMaxRestore();
	void updateMaxRestoreStatus();
	void RestoreMaxStatus();
};

#endif // MAINWINDOW_H