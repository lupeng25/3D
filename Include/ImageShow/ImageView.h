#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H
#include <QWidget>
#include <QImage>
#include <QLabel>

class ImageView : public QWidget
{
	Q_OBJECT

public:
	ImageView(QWidget *parent = nullptr);
	~ImageView();
public:
	void SetText(QString strText);
public:
	void DispImg(QImage img);
	void SetZoomScale(double dscale);
	QImage m_Image;
protected:
	void contextMenuEvent(QContextMenuEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

private:
	qreal m_ZoomValue = 1.0;
	int m_XPtInterval = 0;
	int m_YPtInterval = 0;
	QPoint m_OldPos;
	bool m_Pressed = false;
	qreal DefaultZoom = 1.0;

	QLabel *labPage;       //总页数标签

public slots:
	void onLoadImage(void);
	void onZoomInImage(void);
	void onZoomOutImage(void);
	void onPresetImage(void);
};
#endif
