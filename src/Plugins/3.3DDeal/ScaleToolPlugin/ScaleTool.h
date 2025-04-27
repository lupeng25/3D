#pragma once
#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "FlowchartTool.h"

struct sScale
{
	QString	strImg;
	float	fScale;
	float	fOffset;
	sScale()
	{
		strImg = "";
		fScale = 1;
		fOffset = 0;
	}
};

class ScaleTool :public FlowchartTool
{
	Q_OBJECT
public:
	ScaleTool();
	~ScaleTool();
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
public:
	//QVector<QVector<QString>>	m_vecCommonData;		//��������
	QVector<sScale> m_vecScale;
public:	//���в���

	HObject					m_dstImage;			//ͼ��

};
#endif // ScaleTool