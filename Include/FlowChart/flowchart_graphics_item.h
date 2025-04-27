#pragma once
#include <QTextEdit>
#include <QLineEdit>
#include "flowchar_graphics_item_base.h"

class FlowcharGraphicsLink;

#pragma region 流程矩形
class _COREFLOWTOOL_API_ FlowchartGraphicsRectItem
	: public QObject
	, public QGraphicsRectItem
	, public FlowchartGraphicsItem
{
	Q_OBJECT
public:
	explicit FlowchartGraphicsRectItem(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartGraphicsRectItem();
public:
	virtual	void SetDisable(bool bValue);
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	// 修改文本内容
	virtual void SetText(QString _content, QString _tooltip = "") ;

	// 获取界面信息
	virtual FlowchartInforBase GetItemInformation() ;
	//*********************************************************************
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	/****************获取图元点--中心、左、右、上、下*******************/
	virtual void SetCenterPoint(QPointF pt) { center_point_ = pt;	setPos(center_point_); };
	virtual QPointF GetCenterPoint()		{ center_point_ = pos(); return center_point_; };
	virtual QPointF GetLeftPoint()			{ return mapToScene(left_point_);		};
	virtual QPointF GetRightPoint()			{ return mapToScene(right_point_);		};
	virtual QPointF GetTopPoint()			{ return mapToScene(top_point_);		};
	virtual QPointF GetBottomPoint()		{ return mapToScene(bottom_point_);	};
	virtual int SizeInPath(const QPointF &p, DIRECTION &b);
	virtual int PointInItem(const QPointF &p, DIRECTION &b);
private:
	// 绘制文本内容
	void DrawItemText(QPainter *_painter);
public:
	virtual QRectF boundingRect() const;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
	static QString GetToolName();
};
#pragma endregion 流程矩形

#pragma region 判定
class _COREFLOWTOOL_API_ FlowchartGraphicsConditionItem
	: public QObject
	, public QGraphicsPathItem
	, public FlowchartGraphicsItem
{
	Q_OBJECT
public:
	explicit FlowchartGraphicsConditionItem(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartGraphicsConditionItem();
public:
	virtual	void SetDisable(bool bValue);
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);

public:
	// 修改文本内容
	virtual void SetText(QString _content, QString _tooltip = "") ;

	// 获取界面信息
	virtual FlowchartInforBase GetItemInformation() override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	/****************获取图元点--中心、左、右、上、下*******************/
	virtual void SetCenterPoint(QPointF pt) { center_point_ = pt;	setPos(center_point_); };
	virtual QPointF GetCenterPoint()		{ center_point_ = pos();return center_point_; };
	virtual QPointF GetLeftPoint()			{ return mapToScene(left_point_);		};
	virtual QPointF GetRightPoint()			{ return mapToScene(right_point_);		};
	virtual QPointF GetTopPoint()			{ return mapToScene(top_point_);		};
	virtual QPointF GetBottomPoint()		{ return mapToScene(bottom_point_);		};
	virtual int SizeInPath(const QPointF &p, DIRECTION &b);
	virtual int PointInItem(const QPointF &p, DIRECTION &b);
private:
	// 绘制文本内容
	void DrawItemText(QPainter *_painter);
public:
	virtual QRectF boundingRect() const;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();

	static QString GetToolName();
};
#pragma endregion 判定

#pragma region 转换
class _COREFLOWTOOL_API_ FlowchartConvertion
	: public FlowchartGraphicsRectItem
{
	Q_OBJECT
public:
	explicit FlowchartConvertion(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	explicit FlowchartConvertion(Port::PortDataType port1type, Port::PortDataType port2type, QPointF pos = QPointF(0, 0));
	~FlowchartConvertion();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QString GetToolName();
};
#pragma endregion 转换

#pragma region 启动流程
class _COREFLOWTOOL_API_ FlowchartStartNode
	: public FlowchartGraphicsRectItem
{
	Q_OBJECT
public:
	explicit FlowchartStartNode(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartStartNode();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
};
#pragma endregion 启动流程

#pragma region IF
class _COREFLOWTOOL_API_ FlowchartIfNode
	: public FlowchartGraphicsConditionItem
{
	Q_OBJECT
public:
	explicit FlowchartIfNode(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartIfNode();
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QString GetToolName();
public:
	int m_iLogicValue = 0;
};
#pragma endregion IF

#pragma region 节点流程
class _COREFLOWTOOL_API_ FlowchartNode	: public FlowchartGraphicsRectItem
{
	Q_OBJECT
public:
	explicit FlowchartNode(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartNode();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QString GetToolName();
};
#pragma endregion 节点流程

#pragma region 合并流程
class _COREFLOWTOOL_API_ FlowchartCombineNode
	: public FlowchartGraphicsRectItem
{
	Q_OBJECT
public:
	explicit FlowchartCombineNode(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartCombineNode();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QString GetToolName();
};
#pragma endregion 合并流程

#pragma region 循环流程
class _COREFLOWTOOL_API_ FlowchartForNode
	: public FlowchartGraphicsConditionItem
{
	Q_OBJECT
public:
	explicit FlowchartForNode(FlowchartInforBase _infor = FlowchartInforBase(), QObject *parent = nullptr);
	~FlowchartForNode();
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
	virtual int SizeInPath(const QPointF &p, DIRECTION &b);
	virtual int PointInItem(const QPointF &p, DIRECTION &b);
protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
public:
	virtual	QPair<QString, QString> GetNodeTypeName();
public:
	static QString GetToolName();
private:
	bool m_bFlag = false;
	int	m_iForNumber = 1;			//服务器列表
};
#pragma endregion 循环流程