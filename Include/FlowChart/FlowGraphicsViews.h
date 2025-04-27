#pragma once

#include <QPushButton>
#include <QGraphicsView>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMutex>
#include "Comm.h"

#ifndef _COREFLOWTOOL_EXPORT_
#define _COREFLOWTOOL_API_ _declspec(dllexport)
#else
#define _COREFLOWTOOL_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class QLabel;
class QMouseEvent;
class FlowcharGraphicsLink;
class FlowchartGraphicsConditionItem;
class FlowchartGraphicsRectItem;
class FlowchartGraphicsItem;
class QGraphicsItem;
class NodeManager;

enum class SceneMode
{
	DrawLineItem,
	MoveItem
};

class _COREFLOWTOOL_API_ FlowGraphicsViews : public QGraphicsView
{
	Q_OBJECT
public:
	FlowGraphicsViews(QWidget* parent = nullptr);
	~FlowGraphicsViews();
public:
	int	GetGraphData(QJsonObject&str);
	int SetGraphData(QJsonObject&str);
public:
	void SetText(QString text);
	void SetToFit(qreal val);
	void ClearObj();
	void ClearSelectedObj();
	void SetMouseModel(Qt::CursorShape shape);
Q_SIGNALS:
	void sigAutoRunFinished();
public slots:
	void ClearSelected();
private slots:
	void GetFit();
	void OnDeleteSelectedItem();
	void OnDisableSelectedItem(bool bDisable = false);
protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;   
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;   //右键菜单
	virtual	void changeEvent(QEvent *ev) override;

public:
	// 添加图元连接线
	FlowcharGraphicsLink* AddStartToEndItem(FlowchartGraphicsItem* _start_item, FlowchartGraphicsItem* _end_item);
private:
	//检查当前的点是否有问题
	QList<QGraphicsItem*> CheckScenePointInSelectItems(QPointF& pt, DIRECTION& dir);

	// 对象转化	qt原生对象->图元基类对象
	FlowchartGraphicsItem* QGraphToFlow(QGraphicsItem* _item);

	// 对象转化	图元基类对象->qt原生对象
	QGraphicsItem* FlowToQGraph(FlowchartGraphicsItem* _item);

	// 设置鼠标绘制方式
	void SetSceneMode(SceneMode _model);

	//实现鼠标模式
	void SetMouseModel(FlowchartCursor _type);

	// 绘制磁吸线 水平
	void MagneticHLine();

	// 绘制磁吸线 垂直
	void MagneticVLine();
protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
	void showEvent(QShowEvent *event) override;

public:
	void Start();
	void Stop();
	void UpdateIndex(int index);
public:
	NodeManager*			m_pNodeManager{ nullptr };
	QPair<QString,QString>	m_strName;
	bool					m_bEnable = true;
private:
	void					OnZoomInImage();
	void					OnZoomOutImage();
	void					OnFitImage();
	void					OnFullScreen();
	void					OnNormalScreen();
	void					ZoomFrame(double value);
	void					CaptureGraphView(QGraphicsView* view, QPixmap&pixmap);
	void					UpDateShowItem();
	double					ZoomValue = 1;
	double					ZoomFit = 1;
	double					PixX = 0;
	double					PixY = 0;
	QPixmap					m_PixCapture;              //背景图片
private:
	Qt::CursorShape			m_curShape;
	int						MRSize	= 8; //拖拽区域大小
	QColor					color1	= QColor(30, 30, 30);           //颜色1
	QColor					color2	= QColor(50, 50, 50, 255);      //颜色2
	QPixmap					bgPix	= QPixmap(36, 36);              //背景图片
	void					drawBg();
	int						m_iSelectedIndex = -1;
	FlowchartCursor			m_CurrentCurSor;
	QRectF					m_Rect;
private:
	SceneMode				scene_mode_ = SceneMode::MoveItem;						// 鼠标编辑模式
	FlowchartGraphicsItem	*pTemp_flow_item_{ nullptr };							// 临时图元对象，拖动创建
	QPointF					mouse_move_point_, mouse_pressed_point_;				// 鼠标按下与鼠标移动位置
	QGraphicsLineItem		*pTemp_line_{ nullptr };								// 绘制连线临时对象 过程中删除  析构中无需删除
	QGraphicsItem			*pTemp_item_{ nullptr };								// 鼠标当前拖动对象 磁吸线会用到该对象
	QGraphicsLineItem		*pTemp_hline_{ nullptr }, *pTemp_vline_{ nullptr };		// 水平磁吸线	垂直磁吸线
	QList<QGraphicsItem*>	m_pVecSelectItems;										// 鼠标当前拖动对象 磁吸线会用到该对象
	bool					m_bPressDown = false;
	bool					m_bFirstShow = true;
	bool					m_bDragMode = false;
	QLabel*					m_pLabelName{ nullptr }; //显示类别
	QLabel*					m_pLabelDurTime{ nullptr }; //显示类别
	//QLabel*													m_pWidget{ nullptr };

};