#pragma once
#ifndef FLOWCHARTTOOL_H
#define FLOWCHARTTOOL_H

#include "flowchart_graphics_item.h"
#include "HalconCpp.h"

#include "Comm.h"
using namespace HalconCpp;

#ifndef _FLOWCHARTTOOL_EXPORT_
#define _FLOWCHARTTOOL_API_ _declspec(dllexport)
#else
#define _FLOWCHARTTOOL_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _FLOWCHARTTOOL_API_ FlowchartTool :public FlowchartNode
{
	Q_OBJECT
public:
	FlowchartTool();
	~FlowchartTool();
public:
	static int ChangeROIDataToRegion(QJsonArray &vecData, HObject& region);
	static int ChangeParamTovecMeasure(QJsonArray &vecData, QVector<QVector<MeasureRect2>>& vecMeasure);

	static int CheckMetrixType(QString strCom, HTuple&Metrix);
	static int ChangeParamToRegion(QVector<CommonData> &vecData, HObject& region);
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	QString					m_strInImg;
public:
	QMap<QString, QString>	m_mapParam;			//图像参数
	QMap<QString, QString>	m_mapShowParam;		//显示参数
};
#endif