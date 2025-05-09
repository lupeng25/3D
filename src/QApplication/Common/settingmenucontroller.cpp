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

#include "settingmenucontroller.h"
#include "dialogs/settingdialog.h"
#include "dialogs/aboutdialog.h"
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include "mainwindow.h"
#include "Data/dataVar.h"

SettingMenuController::SettingMenuController(QObject *parent) :
    QObject(parent)
{

}

void SettingMenuController::showSettingDialog()
{
    MainWindow::getInstance()->show();
	if (dataVar::Instance().iLoginLevel < 1) {
		dataVar::Instance().pManager->notify(tr("Warnning"), tr("The Current Permissions Are Insufficient!"), NotifyType_Warn);
		return;
	}

	dataVar::Instance().CenterMainWindow(dataVar::Instance().pSettingDialog);
	dataVar::Instance().pSettingDialog->LoadData();
	dataVar::Instance().pSettingDialog->show();
	dataVar::Instance().pSettingDialog->raise();
}

void SettingMenuController::checkUpdate()
{
    QDesktopServices::openUrl(QUrl("https://github.com/dragondjf/QCFramer"));
}

void SettingMenuController::onlineHelp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/dragondjf/QCFramer"));
}

void SettingMenuController::visitOfficialSite()
{
    QDesktopServices::openUrl(QUrl("https://github.com/dragondjf/QCFramer"));
}

void SettingMenuController::showAboutUs()
{
    MainWindow::getInstance()->show();
    AboutDialog* aboutDialog = new AboutDialog(dataVar::Instance().m_pWindow);
	dataVar::Instance().CenterMainWindow(aboutDialog);
	aboutDialog->exec();
}

void SettingMenuController::showKeyLicence()
{
	MainWindow::getInstance()->show();
	dataVar::Instance().CenterMainWindow(dataVar::Instance().pQtKeyWidgets);
	dataVar::Instance().pQtKeyWidgets->show();
	dataVar::Instance().pQtKeyWidgets->raise();
}

void SettingMenuController::closeMainWindow()
{
    MainWindow* p = MainWindow::getInstance();
    p->animationClose();
}
