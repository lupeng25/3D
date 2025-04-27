#pragma once
#ifndef DelayTool_H
#define DelayTool_H

#include "FlowchartTool.h"

class DelayTool :public FlowchartTool
{
	Q_OBJECT
public:
	DelayTool();
	~DelayTool();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//运行参数

public:
	int				m_iDelayTime = 100;			//延时时间
	int				m_iDelayType = 1;			//延时方式

};
#endif // DelayTool
