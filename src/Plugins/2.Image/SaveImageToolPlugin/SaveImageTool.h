#pragma once
#ifndef SaveImageTool_H
#define SaveImageTool_H

#include "FlowchartTool.h"

enum EnumImgType
{
	EnumImgType_JPG,
	EnumImgType_JPEG,
	EnumImgType_PNG,
	EnumImgType_BMP,
	EnumImgType_TIFF,
};

class SaveImageTool :public FlowchartTool
{
	Q_OBJECT
public:
	SaveImageTool();
	virtual~SaveImageTool();
public:
	QString	GetFileExName(EnumImgType type);
	QString	GetFileExFileName(int itype);
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//�ڵ�ִ���߼�

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:

public:	//���в���
	MiddleParam*	m_PMiddleParam = nullptr;
public:
	int				m_iToolType = 1;					//���淽ʽ
	EnumImgType		m_EnumImgType = EnumImgType_TIFF;
	QString			m_strImgPath ;						//�����ļ�·��
	QString			m_strFolderPath = "d:\\";			//�����ļ���·��
	int				m_iSaveExType =	3;
};
#endif // SaveImageTool