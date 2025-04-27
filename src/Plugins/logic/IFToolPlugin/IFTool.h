#pragma once
#ifndef IF_H
#define IF_H

#include "flowchart_graphics_item.h"

class IFTool :public FlowchartGraphicsConditionItem
{
	Q_OBJECT
public:
	IFTool(QPointF pos=QPointF(0,0));
	~IFTool();
public:
    //控制输出端口逻辑重载
    virtual QList<Port*> OutputStreamLogicExecution();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

	static int FindParamInLinkContent(QString strLinkContent,QString & strModel, QString & TypeVariable, QString & strValue);
	static EnumNodeResult ParamInExecute(QString strLinkContent, QString strLinkSecondContent, int iOperator,int& iValue);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

	virtual QList<PortInfo> CheckScenePortPoint(const QPointF &p, const Port::PortType& type);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);

public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:
	int				m_iLogicValue = 0;
	int				iLogicOperator;		//运算符
	QString			m_strSendContent	= "123";			//服务器列表
	QString			m_strSecondContent	= "123";			//服务器列表

};

#endif // MOD_H
