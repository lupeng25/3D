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

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑
public:	//运行函数
	int ExcutePlaneFit(HObject& hobj,HObject &region,int iCheckMode,QMap<QString, QString>& mapParam,double& dValue);	//获取平面度
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:	//运行参数
	EnumCheckType			m_eEnumCheckType;
	double					m_dLowPercent;
	double					m_dHighPercent;
	int						m_iInvalidValue = -9000;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	int						m_iFontSize = 8;
	QString					m_strLinkName;			//服务器列表
	bool					m_bEnableCom			= false;
	bool					m_bEnableShowRegion		= true;
	bool					m_bEnableShowResult		= true;
	bool					m_bEnableshowDrawResult = true;
	QVector<QVector<MeasureRect2>>	m_vecMeasurelines2D;

public:
	QJsonArray				m_vecROIData;			//ROI数据
};
#endif