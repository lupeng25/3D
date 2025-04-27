#pragma once

#include <QObject>
#include <QGraphicsPathItem>
#include "flowchart_graphics_item.h"
#include <QString>

class FlowcharGraphicsLink
	: public QObject
	, public QGraphicsPathItem
	, public FlowchartGraphicsItem
{
	Q_OBJECT
public:
	// 自循环只需要开始图元
	FlowcharGraphicsLink(FlowchartGraphicsItem* _start_item, FlowchartGraphicsItem* _end_item = nullptr, FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowcharGraphicsLink();
public:
	// 获取开始图元
	FlowchartGraphicsItem* GetStartItem();

	// 获取结束图元
	FlowchartGraphicsItem* GetEndItem();

	// 获取开始点
	QPointF GetStartPointF() { return start_point_; };

	// 获取结束点
	QPointF GetEndPointF() { return end_point_; };

	// 获取文本区域
	QRectF GetTextRect();

	// 更新箭头位置
	void UpdateLineArrow();

	//更新节点
	void UpdateNodeDir(Port*Port1, Port*Port2);

	QPointF GetItemPoint(FlowchartGraphicsItem* item, DIRECTION dir);

	// 设置文本
	virtual void SetText(QString _content, QString _tooltip = "") override;

	// 获取图元信息
	virtual FlowchartInforBase GetItemInformation() override;
private:
	// 绘制两图元连线
	void DrawTwoItemLine();

	// 绘制单图元连线
	void DrawOneItemLine();

	// 判断箭头连线起始点类型
	DrawLineAlignment CompareDrawPoint(QPointF _start_point, QPointF _end_point);

	// 绘制箭头连线
	void DrawLineArrow(QPainter *_painter);

	// 绘制文本
	void DrawLineText(QPainter *_painter);

	// 获取该角度连线
	QLineF caculateThirdPtFromTwoPoint(const QPointF& _point1, const QPointF& _point2, const double _lenght, const double _angle);
public:
	QPair<QString, QString> GetNodeTypeName();

protected:
	virtual QRectF boundingRect() const;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
private:
	const static int containsWidth		= 10;						// 可选范围长度
	const static int containsWidth1_2	= containsWidth >> 1;		// 可选范围一般长度
	const static int extendWidth		= 15;						// 边界线段延伸长度
	const static int arrayLength		= 12;						// 线头绘制范围 - 长度
	const static int arrayWidth			= 9;                        // 线头绘制范围 - 宽度
	void drawLineHead(const DIRECTION direction, const LINE_HEAD_TYPE lht, const int x, const int y,QPainterPath &linePath, QPainterPath &graphPath); // 绘制线头
	void drawStraightLine(int sx, int sy, int ex, int ey, QPainterPath &linePath, QPainterPath &graphPath);			// 直连线绘制
private:
	void SetLinkPort(Port*Port1, Port*Port2);
private:
	QPen								paintChartDrawPen;   // 图形画笔
	FlowchartGraphicsItem*				start_item_;
	FlowchartGraphicsItem*				end_item_;
	QPointF								text_point_;						// 文本绘制位置
	QRectF								text_rect_;
	QPointF								mouse_pressed_point_;				// 鼠标按下位置
	bool								mouse_pressed_flag_;				// 鼠标是否按下
	DIRECTION startDirect				= DIRECTION::NORTH;					// 开始点方向
	DIRECTION endDirect					= DIRECTION::NORTH;					// 结束点方向
	LINE_HEAD_TYPE startLineHeadType	= LINE_HEAD_TYPE::NOLINEARROW;			// 起始点线头
	LINE_HEAD_TYPE endLineHeadType		= LINE_HEAD_TYPE::LINEARROW;			// 结束点线头
	// ! 绘制曲线箭头
	QPointF								start_point_;						// 起点
	QPointF								end_point_;							// 终点
	//QPointF							mid_point_;							// 中位点
	QPainterPath						arrow_path_;						// 箭头曲线
	QPainterPath						linePath;							// 箭头曲线
};