#pragma once
#ifndef DECOMPOSETOOL_H
#define DECOMPOSETOOL_H

#include "HalconCpp.h"
#include "Comm.h"
using namespace HalconCpp;
#include "FlowchartTool.h"

class DeComposeTool :public FlowchartTool
{
	Q_OBJECT
public:
	DeComposeTool();
	~DeComposeTool();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//�ڵ�ִ���߼�

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//�ڵ�ִ���߼�

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//���в���
	HObject					m_dstImage;			//ͼ��
	QVector<HObject>		m_vecOutImg;
};
#endif // DeComposeTool