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

#include "fmainwindow.h"
#include "futil.h"
#include <QtCore>

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QDir>


void FMainWindow::ChangeLanguage()
{
}

FMainWindow::FMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //qDebug("mainwindow init");
    initData();
    initUI();
    initConnect();
}

void FMainWindow::initData()
{
    leftbuttonpressed = false;
    lockmoved = false;
    locked = false;
}

void FMainWindow::initUI()
{
    titleBar = FTitleBar::getInstace();

    setObjectName(QString("FMainWindow"));
    QDesktopWidget* desktopWidget = QApplication::desktop();
    setMaximumSize(desktopWidget->availableGeometry().size());
    //readSettings();

    setWindowFlags(Qt::FramelessWindowHint);
    setWindowTitle("QFramer");

    pstatusbar = new QStatusBar(this);
    pstatusbar->setFixedHeight(35);
    setStatusBar(pstatusbar);
    trayicon = new QSystemTrayIcon(QIcon(QString(":/images/skin/images/QApplication.png")), this);
    trayicon->setObjectName(QString("trayicon"));
    trayicon->setToolTip(QString(qApp->applicationName()));
    trayicon->show();

    //flyWidget = new FlyWidget(this);
    //flyWidget->move(desktopWidget->availableGeometry().width() * 0.8, desktopWidget->availableGeometry().height() *0.2);
}

void FMainWindow::initConnect( )
{
    connect(titleBar, SIGNAL(minimuned()),	this, SIGNAL(sigHidden()));
    //connect(titleBar, SIGNAL(closed()),		this, SIGNAL(Hidden()));
    connect(titleBar, SIGNAL(minimuned()),	this, SLOT(showMinimized()));
    //connect(titleBar, SIGNAL(minimuned()),	this, SLOT(showFlyWidget()));
    connect(titleBar, SIGNAL(maximumed()),	this, SLOT(swithMaxNormal()));

	connect(titleBar, &FTitleBar::closed, this, [=]() {
		emit sigHidden();
		bool bRetn = true;
		bRetn = emit sigClosed();
		if (bRetn)	{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
			QCoreApplication::exit(0); 
		}
	});
    connect(trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));
	connect(this, SIGNAL(sigUpdateMaxRestore()), titleBar, SLOT(updateMaxRestore()));

}

bool FMainWindow::isMoved()
{
    return lockmoved;
}

bool FMainWindow::isLocked()
{
    return locked;
}

void FMainWindow::setLockMoved(bool flag)
{
    lockmoved = flag;
}

void FMainWindow::setLocked(bool flag)
{
    locked = flag;
}

FTitleBar* FMainWindow::getTitleBar()
{
    return titleBar;
}

QStatusBar* FMainWindow::getStatusBar()
{
    return pstatusbar;
}

QSystemTrayIcon* FMainWindow::getQSystemTrayIcon()
{
    return trayicon;
}

void FMainWindow::swithMaxNormal()
{
    if(isMaximized())
    {
        showNormal();
    }else{
        showMaximized();
    }
}

void FMainWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() & Qt::LeftButton)
    {
        if(e->y() < titleBar->height() && e->x() > titleBar->width() - 120)
        {
            leftbuttonpressed = false;
        }
        else
        {
            dragPosition = e->globalPos() - frameGeometry().topLeft();
            leftbuttonpressed = true;
        }
    }
    e->accept();
}

void FMainWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->y() < titleBar->height() && e->x() < titleBar->width() - 120)
    {
		getTitleBar()->switchMaxMin();
        swithMaxNormal();
		//emit sigUpdateMaxRestore();
        e->accept();
    }else{
        e->ignore();
    }
}

void FMainWindow::SetCursorStyle(enum_Direction direction)
{
    //设置上下左右以及右上、右下、左上、坐下的鼠标形状
    switch(direction)
    {
    case eTop:
    case eBottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case eRight:
    case eLeft:
        setCursor(Qt::SizeHorCursor);
        break;
    case eNormal:
        setCursor(Qt::ArrowCursor);
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void FMainWindow::changeEvent(QEvent * ev)
{
	QMainWindow::changeEvent(ev);
	switch (ev->type()) {
	case QEvent::LanguageChange: {
		ChangeLanguage();
	}	break;
	default:
		break;
	}
}

void FMainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    leftbuttonpressed = false;
    titleBar->clearChecked();
    e->accept();
}

void FMainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(isMaximized())
    {
        e->ignore();
    }
    else
    {
        if(e->y() < titleBar->height() && e->x() > titleBar->width() - 120)
        {
            e->ignore();
        }
        else{
            if(leftbuttonpressed)
            {
                if(getTitleBar()->getFixedflag())
                {
                    move(e->globalPos() - dragPosition);
                }
                e->accept();
            }

        }
    }

}

void FMainWindow::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape){
        close();
    }
    else if (e->key() == Qt::Key_F11) {
        titleBar->getMaxButton()->click();
    }
}

void FMainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void FMainWindow::animationClose()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    connect(animation, SIGNAL(finished()), this, SLOT(close()));
    animation->setDuration(1500);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
}

void FMainWindow::onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
	{
        //单击
        case QSystemTrayIcon::Trigger:
			break;
            //双击
        case QSystemTrayIcon::DoubleClick:
            if(isHidden() || isMinimized())	{
                //恢复窗口显示
				getTitleBar()->updateMaxRestoreStatus();
                show();
                //一下两句缺一均不能有效将窗口置顶
                setWindowState(Qt::WindowActive);
                activateWindow();
                setLocked(locked);

				if (getTitleBar()->getNormalMaxFlag()) {	showNormal();	}
				else {			showMaximized();		}
            }
            else  {
				if(! locked)  {	showMinimized();    }
            }
            break;
        case QSystemTrayIcon::Context:
            break;
        default:
            break;
        }
}

//FMainWindow::~FMainWindow()
//{
//}