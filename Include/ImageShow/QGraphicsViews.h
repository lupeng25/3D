#pragma once
#include <QPushButton>
#include <QGraphicsView>
#include <QScrollBar>
#include "BaseItem.h"
#include <QHBoxLayout>
#include <QMutex>
#include <mutex>

class QLabel;
class QMouseEvent;
class QGraphicsScenes;
class ImageItem;

class _BASEITEM_API_ QGraphicsViews : public QGraphicsView
{
	Q_OBJECT
public:
	QGraphicsViews(QWidget* parent = nullptr);
	~QGraphicsViews();
	struct MenuString
	{
		QString strText;
		QString strIcon;
	};
	using menuStr = MenuString;
public:
	void				RegisteredMenu(QString strkey, QString strText, QString strIcon);
	static		int		calcBytesPerLine(int width, int bitCounts);
	void				SetToFit(qreal val);
	void				ClearObj();
	void				ClearSelectedObj();
	void				setText(const QString& text);
	void				UpdateImg();
	void				NeedPaint(BaseItem* item,bool& bvalue,bool currentvalue);
	QGraphicsScenes*	pScene;
	DrawItem*			pdrawItem;
	ImageItem*			pImagItem;
	void				SetMouseModel(Qt::CursorShape shape);
signals:
	void				sig_MouseMoveInImg(QPoint pos, QString& imgtype, QString& strInfo);
	void				sig_MouseClicked(QList<QGraphicsItem *>& item);
	void				sig_MouseMouve(QList<QGraphicsItem *>& item);
	void				sig_MouseChange(QList<QGraphicsItem *>& item);
	void				sig_MouseMenuClicked(QString strkey);
	void				sig_ShapePointChange();

	void				sig_mousePressEvent	 (QMouseEvent* event);
	void				sig_mouseMoveEvent   (QMouseEvent* event);
	void				sig_mouseReleaseEvent(QMouseEvent* event);
public slots:
	void				ClearSelected();
	void				ClearImgShow();
	void				DispImage(QImage& Image,bool bShow = true); //显示图片
	void				ClearAllDrawItem();
	void				DispPoint(QVector<QPointF>& list, QColor color = QColor(0, 255, 0));//显示点集合
	void				DispLineRectPoint(QVector<sColorLineRect>& list, QColor color = QColor(0, 255, 0));//显示点集合
	void				DispRRectanglePoint(QVector<sColorRRectangle>& list, QColor color = QColor(0, 255, 0));//显示点集合
	void				DispLinePoint(QVector<sColorLine>& list, QColor color = QColor(0, 255, 0));//显示点集合
	void				DispTextList(QVector<sDrawText>& list);//显示点集合
	void				DispCrossPoint(QVector<sDrawCross>& list);//显示点集合
	void				DispColorPoints(QVector<sColorPoint>& list);//显示点集合
	void				DispColorCircle(QVector<sColorCircle>& list);//显示点集合
	void				DispREllipsePoint(QVector<sColorRRectangle>& list);//显示点集合
	//void				DispColorPoints(QVector<sColorPoint>& list);//显示点集合
	void				AddItems(BaseItem* item); //添加ROI	
	void				SetType(bool InfoFlg, bool GrayValueFlg);//设定显示信息
private slots:
	void				GetFit();
protected:
	virtual void		mousePressEvent(QMouseEvent* event) override;
	virtual void		mouseReleaseEvent(QMouseEvent* event) override;
	virtual void		mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void		mouseMoveEvent(QMouseEvent* event) override;
	virtual void		wheelEvent(QWheelEvent* event) override;
	virtual void		resizeEvent(QResizeEvent* event) override;   
	virtual void		drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void		paintEvent(QPaintEvent* event) override;
	virtual void		contextMenuEvent(QContextMenuEvent* event) override;   //右键菜单
	bool				event(QEvent *event) override;
public:
	QPixmap				image;
private:
	void				OnZoomInImage();
	void				OnZoomOutImage();
	void				OnFitImage();
	void				OnShowCrossImage();
	void				OnFullScreen();
	void				OnNormalScreen();
	void				ZoomFrame(double value);
	QScrollBar*			pHbar;
	QScrollBar*			pVbar;
	double				ZoomValue = 1;
	double				ZoomFit = 1;
	double				PixX = 0;
	double				PixY = 0;
	QWidget*			pfWidget;	
	QLabel*				pGrayValue; //显示灰度值
	QWidget*			pfClassifierWidget;
	QLabel*				pqClassifierValue; //显示类别
	QHBoxLayout*		pfClarityLayout;
	QHBoxLayout*		pfLayout;
	std::mutex			mutex;
	QMap<QString, menuStr> m_mapMenuStr;
private:
	Qt::CursorShape		m_curShape;
	int					MRSize	= 8; //拖拽区域大小
	QColor				color1	= QColor(30, 30, 30);           //颜色1
	QColor				color2	= QColor(50, 50, 50, 255);      //颜色2
	QPixmap				bgPix	= QPixmap(36, 36);              //背景图片
	void				drawBg();
	int					m_iSelectedIndex = -1;
};
