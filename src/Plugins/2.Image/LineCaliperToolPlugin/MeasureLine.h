#pragma once
#ifndef MEASURELINE_H
#define MEASURELINE_H

#include "FlowchartTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

class MeasureLine :public FlowchartTool
{
	Q_OBJECT
public:
	MeasureLine();
	MeasureLine(QPointF pos);
	~MeasureLine();

public:
	static void gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
		HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth);
	static void pts_to_best_line(HObject *ho_Line, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
		HTuple *hv_Row1, HTuple *hv_Column1, HTuple *hv_Row2, HTuple *hv_Column2);
	static void rake(HObject ho_Image, HObject *ho_Regions, HTuple hv_Elements, HTuple hv_DetectHeight,
		HTuple hv_DetectWidth, HTuple hv_Sigma, HTuple hv_Threshold, HTuple hv_Transition,
		HTuple hv_Select, HTuple hv_Row1, HTuple hv_Column1, HTuple hv_Row2, HTuple hv_Column2,
		HTuple *hv_ResultRow, HTuple *hv_ResultColumn);
public:
	void ClearModel();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

	void UpdateParam();
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:
	HTuple					m_handleModel;	//模板句柄
	QVector<LineCaliperP>	m_vecResultLines;

	QString					m_strLinkName;			//服务器列表
	bool					m_bEnableCom = false;
	bool					m_bEnableShowRegion = true;
	bool					m_bEnableShowResult = true;
	bool					m_bEnableshowDrawResult = true;
	int						m_iFontSize = 8;

public:
	QJsonArray				m_vecROIData;			//ROI数据
public:
	QString					m_strSendContent;		//发送的内容
	QString					m_strImg;					//	
};
#endif // MOD_H