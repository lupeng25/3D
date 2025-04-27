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

#include "settingmenu.h"
#include "mainwindow.h"
#include "functionpages/rightfloatwindow.h"
#include <QApplication>

SettingMenu::SettingMenu(QWidget *parent) :
    QMenu(parent)
{
    initData();
    initUI();
    initController();
    initConnect();
	ChangeLanguage();
}

void SettingMenu::initData()
{
	actionNames.clear();
    actionNames	
		//<< tr("Show flyWindow")
		<< ("Settings")
		<< ("KeyWidget")
		//<< tr("Check update") 
		//<< tr("Online help")
		//<< tr("Office site") 
		<< ("About us")
		//<< tr("Quit")
		;
}

void SettingMenu::initUI()
{
    for(int i=0; i< actionNames.length() ; ++i){
        actions.append(new QAction(actionNames.at(i), this));
        actionMaps[actionNames.at(i)] = actions.at(i);
    }
    addActions(actions);
}

void SettingMenu::initController()
{
    controller = new SettingMenuController();
}

void SettingMenu::initConnect()
{
    connect(actionMaps[actionNames.at(0)],			SIGNAL(triggered()), controller,	SLOT(showSettingDialog()));
    connect(actionMaps[actionNames.at(1)],			SIGNAL(triggered()), controller,	SLOT(showKeyLicence()));
    connect(actionMaps[actionNames.at(2)],			SIGNAL(triggered()), controller,	SLOT(showAboutUs()));
}

void SettingMenu::ChangeLanguage()
{
	actionLanguageNames.clear();
	actionLanguageNames
		//<< tr("Show flyWindow")
		<< tr("Settings")
		<< tr("KeyWidget")
		//<< tr("Show rightBar")
		//<< tr("Check update") 
		//<< tr("Online help")
		//<< tr("Office site") 
		<< tr("About us")
		//<< tr("Quit")
		;
	for (int i = 0; i < actionNames.length(); ++i) {
		if (i < actionLanguageNames.size())	{
			actions.at(i)->setText(actionLanguageNames.at(i));
			actions.at(i)->setToolTip(actionLanguageNames.at(i));
		}
	}
}

QMap<QString, QAction*> SettingMenu::getActionMaps()
{
    return actionMaps;
}

void SettingMenu::switchActionState()
{
    //MainWindow::getInstance()->getFlyWidget()->setVisible(!MainWindow::getInstance()->getFlyWidget()->isVisible());

    //if(MainWindow::getInstance()->getFlyWidget()->isVisible()){
    //    actionMaps[tr("Show flyWindow")]->setText(tr("Hide flyWindow"));
    //}
    //else {
    //    actionMaps[tr("Show flyWindow")]->setText(tr("Show flyWindow"));
    //}
}

void SettingMenu::switchFloatWindow()
{
    //MainWindow::getInstance()->getRightFloatWindow()->setVisible(!MainWindow::getInstance()->getRightFloatWindow()->isVisible());
}
