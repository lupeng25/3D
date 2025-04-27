#pragma once
#ifndef MatchTool_H
#define MatchTool_H

#include "FlowchartTool.h"
#include "Camerainterface.h"
class MatchTool :public FlowchartTool
{
	Q_OBJECT
public:
	MatchTool();
	~MatchTool();
public:
	void ClearMatchShapeModel();
	void ClearMatchNCCModel();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:

public:	//运行参数
	EnumModeType			m_EMatchtype;
	float					m_fStartAngle;
	float					m_fEndAngle;
	float					m_fMinScore;
	int						m_iMatchNumber;
	float					m_fMatchGreedy;
	float					m_fMatchOverLap;
	int						m_iSubPixType;

	HObject					m_ROISearchRegion;	//将ROI参数转化为形状区域
	HTuple					m_hNCCModel;
	HTuple					m_hShapeModel;
	HTuple					m_hHomMat2D;
public:
	QVector<CommonData>		m_vecCommonData;		//公共数据ROI    MarkYZH20240330
	QVector<CommonData>		m_vecSearchData;		//公共数据ROI    MarkYZH20240330
	QVector<sDrawCross>		m_VecCreateCross;		//创建模板的交叉线

};
#endif