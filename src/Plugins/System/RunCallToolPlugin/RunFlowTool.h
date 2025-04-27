#pragma once
#ifndef RunFlowTool_H
#define RunFlowTool_H

#include "FlowchartTool.h"
//#include "HalconCpp.h"
//using namespace HalconCpp;

class RunFlowTool :public FlowchartTool
{
	Q_OBJECT
public:
	RunFlowTool();
	~RunFlowTool();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//½ÚµãÖ´ÐÐÂß¼­

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:

	QString	m_strFlow;

};
#endif // MOD_H