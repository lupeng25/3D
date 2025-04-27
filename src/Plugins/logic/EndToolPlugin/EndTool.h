#pragma once
#ifndef End_H
#define End_H

#include "flowchart_graphics_item.h"

class EndTool :public FlowchartGraphicsRectItem
{
	Q_OBJECT

public:
	EndTool(QPointF pos=QPointF(0,0));
	~EndTool();
    //控制输出端口逻辑重载
    //virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑

public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();

public:
	int m_iLogicValue = 0;

	int	m_iForNumber = 1;			//服务器列表
	int	m_iNumber = 1;			//服务器列表
};

#endif // MOD_H
