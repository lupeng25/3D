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

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑
public:	//运行函数
	/*static*/ int ExcutePlaneFit(HObject& hobj,HObject &region,int iCheckMode,QMap<QString, QString>& mapParam,double& dValue);	//获取平面度

public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();

public:	//运行参数
	HObject					m_ROISearchRegion;	//将ROI参数转化为形状区域

	QString					m_strAlgorithm		= "huber";		//拟合的方法
	int						m_iIterations		= 5;
	float					m_iClippingFactor	= 2.0;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	int						m_iFontSize = 8;
	QString					m_strLinkName;			//服务器列表
	bool					m_bEnableCom		= false;
	bool					m_bEnableShowRegion = true;
public:
	QVector<CommonData>		m_vecCommonData;		//公共数据ROI    MarkYZH20240330
};
#endif
