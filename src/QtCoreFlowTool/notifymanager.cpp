
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
//#include "notifymanager.h"
#include "notifymanager.h"

const int RIGHT = 10;
const int BOTTOM = 10;

const int HEIGHT = 150;
const int WIDTH = 400;

const int SPACE = 30;

NotifyManager::NotifyManager(QObject *parent): QObject(parent),
    maxCount(5),
    displayTime(10 * 1000),
	MaxDisplayCount(10)
{

}

void NotifyManager::notify(const QString &title, const QString &body,
                           const QString &icon)
{
	if (dataQueue.size() < MaxDisplayCount)
		dataQueue.enqueue(NotifyData(icon, title, body));
    showNext();
}

void NotifyManager::notify(const QString & title, const QString & body, EnumNotifyType IconType)
{
	if (dataQueue.size() < MaxDisplayCount)
	{
		switch (IconType)
		{
		case NotifyType_Info:
			dataQueue.enqueue(NotifyData(":/image/icon/images/Info.png", title, QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		case NotifyType_Warn:
			dataQueue.enqueue(NotifyData(":/image/icon/images/warning.png", title, QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		case NotifyType_Error:
			dataQueue.enqueue(NotifyData(":/image/icon/images/error.png", title, QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		default:
			break;
		}
	}
	showNext();
}

void NotifyManager::notify(const QString & body, EnumNotifyType IconType)
{
	if (dataQueue.size() < MaxDisplayCount)	{
		switch (IconType)		{
		case NotifyType_Info:
			dataQueue.enqueue(NotifyData(":/image/icon/images/Info.png", tr("Info"), QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		case NotifyType_Warn:
			dataQueue.enqueue(NotifyData(":/image/icon/images/warning.png", tr("Warnning"), QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		case NotifyType_Error:
			dataQueue.enqueue(NotifyData(":/image/icon/images/error.png", tr("Error"), QString(QDateTime::currentDateTime().toString("hh:mm:ss") + ":" + body)));
			break;
		default:
			break;
		}
	}
	showNext();
}

void NotifyManager::setMaxCount(int count)
{
    maxCount = count;
}

void NotifyManager::setDisplayTime(int ms)
{
    displayTime = ms;
}

void NotifyManager::ClearAll()
{
	for (auto iter : notifyList)
		if(iter != nullptr)
			(iter)->hideGriant();
	dataQueue.clear();
	notifyList.clear();
	this->rearrange();
	showNext();
}

// 调整所有提醒框的位置
void NotifyManager::rearrange()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();
    QPoint bottomRignt = desktopRect.bottomRight();

    QList<Notify*>::iterator i;
    for (i = notifyList.begin(); i != notifyList.end(); ++i) {
        int index = notifyList.indexOf((*i));

        QPoint pos = bottomRignt -
                QPoint(WIDTH + RIGHT,
                       (HEIGHT + SPACE) * (index + 1) - SPACE + BOTTOM);
        QPropertyAnimation *animation = new QPropertyAnimation((*i), "pos", this);
        animation->setStartValue((*i)->pos());
        animation->setEndValue(pos);
        animation->setDuration(300);
        animation->start();

        connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
            animation->deleteLater();
        });
    }
}

void NotifyManager::showNext()
{
    if(notifyList.size() >= maxCount || dataQueue.isEmpty()) {
        return;
    }
    NotifyData data = dataQueue.dequeue();
    Notify *notify = new Notify(this->displayTime);
    notify->setIcon(data.icon);
    notify->setTitle(data.title);
    notify->setBody(data.body);

    notify->setFixedSize(WIDTH, HEIGHT);

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();

    // 计算提醒框的位置
    QPoint bottomRignt = desktopRect.bottomRight();
    QPoint pos = bottomRignt - QPoint(notify->width() + RIGHT, (HEIGHT + SPACE) * (notifyList.size() + 1) - SPACE + BOTTOM);

    notify->move(pos);
    notify->showGriant();
    notifyList.append(notify);

    connect(notify, &Notify::disappeared, this, [notify, this](){
		ClearAll();
        this->notifyList.removeAll(notify);
        this->rearrange();

        // 如果列表是满的，重排完成后显示
        if(this->notifyList.size() == this->maxCount - 1){
            QTimer::singleShot(300, this, [this]{
               this->showNext();
            });
        } else {
            this->showNext();
        }
        notify->deleteLater();
    });
}