#pragma once

#include <QGraphicsPixmapItem>

#ifndef _IMAGEITEM_EXPORT_
#define _IMAGEITEM_API_ _declspec(dllexport)
#else
#define _IMAGEITEM_API_ _declspec(dllimport)
#endif // !_IMAGEITEM_EXPORT_

//通过鼠标点选获取当前灰度值
class _IMAGEITEM_API_ ImageItem :public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ImageItem(QWidget *parent = nullptr);
protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);  

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void slot_ClassifierValue(QString InfoVal);
signals:
	void MouseMoveInImg(QPoint pos,QString& imgtype,QString& strInfo);
    void RGBValue(QString InfoVal);
    void ClassifierValue(QString InfoVal);
public:
	int					w;
	int					h;
	bool				m_bShowCross = false;
};