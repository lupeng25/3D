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
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//�ڵ�ִ���߼�

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//���в���

public:
	int				m_iDelayTime = 100;			//��ʱʱ��
	int				m_iDelayType = 1;			//��ʱ��ʽ

};
#endif // DelayTool
