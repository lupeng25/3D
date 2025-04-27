#pragma once
#ifndef VARIABLETOOL_H
#define VARIABLETOOL_H

#include "FlowchartTool.h"


class VariableTool :public FlowchartTool
{
	Q_OBJECT
public:
	VariableTool();
	~VariableTool();
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
	int				m_iShowIndex	= 0;
	QJsonArray		m_Variable;
	MiddleParam		m_CurMiddleParam;

};
#endif // MOD_H