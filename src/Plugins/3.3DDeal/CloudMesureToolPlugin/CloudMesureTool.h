#pragma once
#ifndef CloudMesureTool_H
#define CloudMesureTool_H

#include "FlowchartTool.h"

enum EnumALargrim {
	ALargrim_Medium,
	ALargrim_Max,
	ALargrim_Mean,
	ALargrim_Min,
	ALargrim_GrayHisto,
};
struct tagCloudMesure {
	QVector<CommonData> vecRoi;					//ROi参数
	bool				bEnableLink = false;
	QString				strLinkName;			//服务器列表
	EnumALargrim		eAlargrim;				//算法类型
	double				dHighValue;				//最大值
	double				dLowValue;				//最小值
	double				dMeasureValue;
	tagCloudMesure() {
		bEnableLink		= false;
		strLinkName		= "";
		vecRoi.clear();
		eAlargrim		= ALargrim_Medium;
		dHighValue		= 1;
		dLowValue		= 0;
		dMeasureValue	= 0;
	}
};

//#include "Common.h"
class CloudMesureTool :public FlowchartTool
{
	Q_OBJECT
public:
	CloudMesureTool();
	~CloudMesureTool();
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
public:		//运行参数
	HObject					m_ROIModelRegion;	//将ROI参数转化为形状区域
	HObject					m_ROIModelRegion1;	//将ROI参数转化为形状区域

	QString					m_strAlgorithm		= "huber";		//拟合的方法
	int						m_iIterations		= 5;
	float					m_iClippingFactor	= 2.0;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	bool					m_bEnableShowRegion = true;
	QVector<tagCloudMesure>	m_vecMesureRegion;

};
#endif