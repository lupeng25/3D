#pragma once
#ifndef GlueMeasureTool_H
#define GlueMeasureTool_H

#include "FlowchartTool.h"
#include "HalconCpp.h"

using namespace HalconCpp;
class GlueMeasureTool :public FlowchartTool
{
	Q_OBJECT
public:
	GlueMeasureTool();
	~GlueMeasureTool();

	enum EnumCheckType	{
		CheckType_Percent,
		CheckType_Max,
		CheckType_Min,
		CheckType_Mean,
		CheckType_Average,
	};
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
	int ExcutePlaneFit(HObject& hobj,HObject &region,int iCheckMode,QMap<QString, QString>& mapParam,double& dValue);	//��ȡƽ���
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//���в���
	EnumCheckType			m_eEnumCheckType;
	double					m_dLowPercent;
	double					m_dHighPercent;
	int						m_iInvalidValue = -9000;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	int						m_iFontSize = 8;
	QString					m_strLinkName;			//�������б�
	bool					m_bEnableCom			= false;
	bool					m_bEnableShowRegion		= true;
	bool					m_bEnableShowResult		= true;
	bool					m_bEnableshowDrawResult = true;
	QVector<QVector<MeasureRect2>>	m_vecMeasurelines2D;

public:
	QJsonArray				m_vecROIData;			//ROI����
};
#endif