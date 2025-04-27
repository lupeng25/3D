#pragma once
#ifndef SAVEIMAGE_H
#define SAVEIMAGE_H

#include "FlowchartTool.h"

class ShowImage :public FlowchartTool
{
	Q_OBJECT
public:
	ShowImage();
	~ShowImage();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//½ÚµãÖ´ÐÐÂß¼­

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:
	int				m_iShowIndex	= 0;
	QVector< QString> m_vecCamParam;


};
#endif // MOD_H