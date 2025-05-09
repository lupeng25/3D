﻿/****************************************************************************
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

#ifndef THEMEMENU_H
#define THEMEMENU_H

#include <QMenu>

class ThemeMenu : public QMenu
{
    Q_OBJECT
private:
    QList<QAction*> actions;
    QList<QString> actionNames;
    QList<QString> actionicons;
    QMap<QString, QAction*> actionMaps;
    QAction* themeAction1;
    QAction* themeAction2;

private:
    void initData();
    void initUI();
    void initConnect();

public:
    explicit ThemeMenu(QWidget *parent = 0);
public:
	void Load();
	void Save();
signals:

public slots:
    void changeTheme1();
    void changeTheme2();
    void changeTheme3();
    void changeTheme4();
    void changeTheme5();
    void changeTheme6();
    void changeTheme_blank();
    void changeThemeFromFile();
    void updateCheckedAction(QAction* action);
};

#endif // THEMEMENU_H
