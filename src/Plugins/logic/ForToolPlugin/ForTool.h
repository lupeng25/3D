#pragma once
#ifndef For_H
#define For_H

#include "flowchart_graphics_item.h"

class ForTool :public FlowchartGraphicsConditionItem
{
	Q_OBJECT
public:
	ForTool();
	~ForTool();
    //控制输出端口逻辑重载
    virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)override;
public:
	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
	virtual int SizeInPath(const QPointF &p, DIRECTION &b);
	virtual int PointInItem(const QPointF &p, DIRECTION &b);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	bool m_bFlag = false;
	QString	m_strSendContent = "123";			//服务器列表
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:

	int	m_iForNumber = 1;			//服务器列表
};

#endif // MOD_H