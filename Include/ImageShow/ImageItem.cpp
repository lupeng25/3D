#include "ImageShow/ImageItem.h"
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

ImageItem::ImageItem(QWidget* parent) : QGraphicsPixmapItem(nullptr)
{

}

void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	QPointF mousePosition = event->pos();
	int R, G, B;
	int x, y;
	x = mousePosition.x();
	y = mousePosition.y();
	if (mousePosition.x() < 0){	x = 0;	}
	if (mousePosition.y() < 0){	y = 0;	}
	QImage Img = pixmap().toImage();
	if (x < Img.width() && y < Img.height()){
		Img.pixelColor(x, y).getRgb(&R, &G, &B);
	}
	QString	strimgtype, strInfo;
	emit MouseMoveInImg(QPoint(x, y), strimgtype, strInfo);
	QString InfoVal;
	if (strInfo.isEmpty()){
		InfoVal = QString("W:%1,H:%2 | X:%3,Y:%4 | R:%5,G:%6,B:%7").arg(QString::number(w)).arg(QString::number(h))
				.arg(QString::number(x))
				.arg(QString::number(y))
				.arg(QString::number(R))
				.arg(QString::number(G))
				.arg(QString::number(B));
	}
	else
		InfoVal = QString("W:%1,H:%2 | X:%3,Y:%4 | %5").arg(QString::number(w)).arg(QString::number(h))
				.arg(QString::number(x))
				.arg(QString::number(y))
				.arg(strInfo);

	emit RGBValue(InfoVal);
}

void ImageItem::slot_ClassifierValue(QString InfoVal)
{
	emit ClassifierValue(InfoVal);
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QGraphicsPixmapItem::paint(painter, option, widget);
	if (m_bShowCross)
	{
		painter->drawLine(0, pixmap().height() / 2.0, pixmap().width(), pixmap().height() / 2.0);
		painter->drawLine(pixmap().width() / 2.0, 0, pixmap().width() / 2.0, pixmap().height());
	}

}