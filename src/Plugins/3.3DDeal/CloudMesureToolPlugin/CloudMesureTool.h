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
	QVector<CommonData> vecRoi;					//ROi����
	bool				bEnableLink = false;
	QString				strLinkName;			//�������б�
	EnumALargrim		eAlargrim;				//�㷨����
	double				dHighValue;				//���ֵ
	double				dLowValue;				//��Сֵ
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

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//�ڵ�ִ���߼�

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//�ڵ�ִ���߼�

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//�ڵ�ִ���߼�
public:	//���к���
	/*static*/ int ExcutePlaneFit(HObject& hobj,HObject &region,int iCheckMode,QMap<QString, QString>& mapParam,double& dValue);	//��ȡƽ���
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:		//���в���
	HObject					m_ROIModelRegion;	//��ROI����ת��Ϊ��״����
	HObject					m_ROIModelRegion1;	//��ROI����ת��Ϊ��״����

	QString					m_strAlgorithm		= "huber";		//��ϵķ���
	int						m_iIterations		= 5;
	float					m_iClippingFactor	= 2.0;

	float					m_fLowLimit			= 0;
	float					m_fHighLimit		= 0;

	bool					m_bEnableShowRegion = true;
	QVector<tagCloudMesure>	m_vecMesureRegion;

};
#endif