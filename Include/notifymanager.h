#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <QObject>
#include <QQueue>

#include "notify.h"

enum EnumNotifyType
{
	NotifyType_Info,
	NotifyType_Warn,
	NotifyType_Error,
};

class LQFORMWIDGETSHARED_EXPORT NotifyManager : public QObject
{
    Q_OBJECT
public:
    explicit NotifyManager( QObject *parent = 0);

    void notify(const QString &title, const QString &body,const QString &icon);
	void notify(const QString &title, const QString &body,EnumNotifyType icon);
	void notify(const QString &body, EnumNotifyType icon);

    void setMaxCount(int count);
    void setDisplayTime(int ms);

	void ClearAll();
private:
    class NotifyData {
    public:
        NotifyData(const QString &icon, const QString &title,
                   const QString &body):
            icon(icon),
            title(title),
            body(body)
        {
        }

        QString icon;
        QString title;
        QString body;
    };

    void rearrange();
    void showNext();

    QQueue<NotifyData> dataQueue;
    QList<Notify*> notifyList;
    int maxCount;
    int displayTime;
	int MaxDisplayCount;
};

#endif // NOTIFYMANAGER_H
