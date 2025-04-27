#pragma once
#ifndef PlaneFitTool_H
#define PlaneFitTool_H

#include "FlowchartTool.h"

class PlaneFitTool :public FlowchartTool
{
	Q_OBJECT
public:
	PlaneFitTool();
	~PlaneFitTool();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

	void UpdateParam();
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//�ڵ�ִ���߼�

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//�ڵ�ִ���߼�

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//�ڵ�ִ���߼�
public:	//���к���
	/*static*/ int ExcutePlaneFit(HObject& hobj,HObject &region,int iCheckMode,QMap<QString, QString>& mapParam,double& dValue);	//��ȡƽ���

public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();

public:	//���в���
	HObject					m_ROISearchRegion;	//��ROI����ת��Ϊ��״����

	QString					m_strAlgorithm		= "huber";		//��ϵķ���
	int						m_iIterations		= 5;
	float					m_iClippingFactor	= 2.0;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	int						m_iFontSize = 8;
	QString					m_strLinkName;			//�������б�
	bool					m_bEnableCom		= false;
	bool					m_bEnableShowRegion = true;
public:
	QVector<CommonData>		m_vecCommonData;		//��������ROI    MarkYZH20240330
};
#endif
