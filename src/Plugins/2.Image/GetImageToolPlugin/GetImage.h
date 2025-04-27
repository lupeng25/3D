#pragma once
#ifndef GETIMAGE_H
#define GETIMAGE_H

#include "FlowchartTool.h"
#include "Camerainterface.h"

#include "MyEvent.h"
enum eImgType
{
	eImgType_NotOpen = -2,
	eImgType_Error = -1,
	eImgType_None,
	eImgType_Gray,
	eImgType_Height,
	eImgType_HeightAndGray,
};

class GetImage :public FlowchartTool,public CameraStream
{
	Q_OBJECT
public:
	GetImage();
	virtual	~GetImage();

public:
	virtual	int SetData(QJsonObject & strData);

	virtual	int GetData(QJsonObject &strData);
public:
	static eImgType SplitImg(HObject&Image, HObject&hGray, HObject&hHeight);

public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual int RecieveBuffer(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	virtual int RecieveRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);

	virtual void StopExecute();
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:
	EnumNodeResult InitCamera(QString came, Camerainterface*& ptrcam);		//节点执行逻辑

public:
	QStringList		m_strImgPaths;
	int				m_iSelectedPathIndex = 0;
	HObject			m_hGrayImage;		//灰度图
	HObject			m_hHeightImage;		//亮度图

	eImgType		m_eRetn;
	eImgType		m_eOutPutImg;		//输出显示图像
public:
	int				m_iPathType = 0;
	QString			m_strImgPath;
	QString			m_strFolderPath;
	QString			m_strCamera;
	int				m_iTimeOut;
	Camerainterface*m_PtrCamera;
	QCreateEvent	m_hTrrigerEvent;
};
#endif // MOD_H
