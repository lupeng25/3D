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

#ifndef FTABWIDGET_H
#define FTABWIDGET_H

#include <QWidget>
#include <QStackedLayout>
#include"fcheckablebutton.h"
#include"QBoxLayout"


#include "Pages/QtWidgetsBase.h"
class FTabWidget : public QtWidgetsBase
{
    Q_OBJECT

private:
    QList<FCheckabelButton*> buttons;
    QList<QString> buttonTitles;

private:
	virtual void initUI();
	virtual void initConnect();
public:
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);
public:
	QWidget* tabTile{ nullptr };
    QBoxLayout* mainLayout{ nullptr };
	QBoxLayout* pageLayout{ nullptr };
    QStackedLayout* stackLayout{ nullptr };
    QVBoxLayout* tabLayout{ nullptr };
public:
    explicit FTabWidget(QWidget *parent = 0);
    QList<FCheckabelButton*> getButtons();

	QVector<QtWidgetsBase*>	m_vecpWidget;
signals:
        void indexChanged(int i);
public slots:
    void addWidget(const QString &tile, const QString &objectName, QWidget *widget);
    void setButtonChecked();
};

#endif // FTABWIDGET_H
