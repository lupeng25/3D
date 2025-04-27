#include "ImageView.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QStyleOption>
#include <QPainter>
#include <QFileDialog>

ImageView::ImageView(QWidget *parent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setObjectName("ImageViewShow");
	labPage = new QLabel();
}

ImageView::~ImageView()
{

}

void ImageView::SetText(QString strText)
{
	labPage->setText(strText);
}

void ImageView::DispImg(QImage img)
{
	m_Image = img;
}

void ImageView::SetZoomScale(double dscale)
{
	m_ZoomValue = dscale;
	DefaultZoom = dscale;
}

void ImageView::contextMenuEvent(QContextMenuEvent *event)
{
	QPoint pos = event->pos();
	pos = this->mapToGlobal(pos);
	QMenu *menu = new QMenu(this);

	QAction *zoomInAction = new QAction(tr("Zoom In"));
	QObject::connect(zoomInAction, &QAction::triggered, this, &ImageView::onZoomInImage);
	menu->addAction(zoomInAction);

	QAction *zoomOutAction = new QAction(tr("Zoom Out"));
	QObject::connect(zoomOutAction, &QAction::triggered, this, &ImageView::onZoomOutImage);
	menu->addAction(zoomOutAction);

	QAction *presetAction = new QAction(tr("Preset"));
	QObject::connect(presetAction, &QAction::triggered, this, &ImageView::onPresetImage);
	menu->addAction(presetAction);

	menu->exec(pos);
}

void ImageView::paintEvent(QPaintEvent *event)
{
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	if (m_Image.isNull())
		return QWidget::paintEvent(event);

	int width = qMin(m_Image.width(), this->width());
	int height = width * 1.0 / (m_Image.width() * 1.0 / m_Image.height());
	height = qMin(height, this->height());
	width = height * 1.0 * (m_Image.width() * 1.0 / m_Image.height());

	painter.translate(this->width() / 2 + m_XPtInterval, this->height() / 2 + m_YPtInterval);

	painter.scale(m_ZoomValue, m_ZoomValue);

	QRect picRect(-width / 2, -height / 2, width, height);
	painter.drawImage(picRect, m_Image);
}

void ImageView::wheelEvent(QWheelEvent *event)
{
	int value = event->delta();
	if (value > 0)
		onZoomInImage();
	else
		onZoomOutImage();

	this->update();
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
	m_OldPos = event->pos();
	m_Pressed = true;
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_Pressed)
		return QWidget::mouseMoveEvent(event);

	this->setCursor(Qt::SizeAllCursor);
	QPoint pos = event->pos();
	int xPtInterval = pos.x() - m_OldPos.x();
	int yPtInterval = pos.y() - m_OldPos.y();

	m_XPtInterval += xPtInterval;
	m_YPtInterval += yPtInterval;

	m_OldPos = pos;
	this->update();
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
	m_Pressed = false;
	this->setCursor(Qt::ArrowCursor);
}

void ImageView::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	labPage->setGeometry(width() / 2.0,0,width() ,40);

}

void ImageView::onLoadImage(void)
{
	QString imageFile = QFileDialog::getOpenFileName(this, "Open Image", "./", tr("Images (*.png *.xpm *.jpg)"));

	QFile file(imageFile);
	if (!file.exists())
		return;

	m_Image.load(imageFile);
}

void ImageView::onZoomInImage(void)
{
	m_ZoomValue += 0.2;
	this->update();
}

void ImageView::onZoomOutImage(void)
{
	m_ZoomValue -= 0.2;
	if (m_ZoomValue <= 0)
	{
		m_ZoomValue = 0.2;
		return;
	}

	this->update();
}

void ImageView::onPresetImage(void)
{
	m_ZoomValue = DefaultZoom;
	m_XPtInterval = 0;
	m_YPtInterval = 0;
	this->update();
}
