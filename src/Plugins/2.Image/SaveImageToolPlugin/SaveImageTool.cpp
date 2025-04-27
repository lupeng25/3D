#include "SaveImageTool.h"
#include <QFileDialog>
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmSaveImageTool.h"
#include "databaseVar.h"
#include <QDatetime>
#include "qdatetime.h"
#include "Camerainterface.h"

SaveImageTool::SaveImageTool() :FlowchartTool()
{
	// GetItemId() = GetToolName();
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
}

SaveImageTool::~SaveImageTool()
{
	Clearport();
}

QString SaveImageTool::GetFileExName(EnumImgType type)
{
	switch (type){
	case EnumImgType_JPG:
		return "jpg";
	case EnumImgType_JPEG:
		return "jpeg";
	case EnumImgType_PNG:
		return "png";
	case EnumImgType_BMP:
		return "bmp";
	case EnumImgType_TIFF:
		return "tiff";
	default:
		break;
	}
	return "jpg";
}

QString SaveImageTool::GetFileExFileName(int itype)
{
	QString _strFile;
	if (itype == 0){
		_strFile = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
	}	
	else if (itype == 1){
		_strFile = QString("%1").arg(QDateTime::currentDateTime().toString("hh-mm-ss"));
	}
	else if (itype == 2){
		_strFile = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz"));
	}
	else{
		_strFile = QString("%1").arg(QDateTime::currentDateTime().toString("hh-mm-ss--zzz"));
	}
	return _strFile;
}

int SaveImageTool::SetData(QJsonObject & strData){
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("SaveConfig").value().toArray();
	if (!array_object.empty())	{
		m_iToolType		= array_object.at(0).toString().toInt();
		m_EnumImgType	= (EnumImgType)array_object.at(1).toString().toInt();
		m_strImgPath	= array_object.at(2).toString();		
		m_strFolderPath = array_object.at(3).toString();
		m_iSaveExType	= array_object.at(4).toString().toInt();
	}
	return 0;
}

int SaveImageTool::GetData(QJsonObject & strData){

	FlowchartTool::GetData(strData);

	QJsonArray ConFig = {
		QString::number(m_iToolType),
		QString::number(m_EnumImgType),
		m_strImgPath,
		m_strFolderPath,
		QString::number(m_iSaveExType),
	};
	strData.insert("SaveConfig", ConFig);
	return 0;
}

void SaveImageTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){
	frmSaveImageTool * frm = new frmSaveImageTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->Save();
	frm->m_pTool = nullptr;
	delete frm;
}

EnumNodeResult SaveImageTool::InitBeforeRun(MiddleParam& param, QString& strError){
	return NodeResult_OK;
}

EnumNodeResult SaveImageTool::Execute(MiddleParam& param, QString& strError)
{
	m_PMiddleParam = &param;
	if (param.MapPImgVoid.count(m_strInImg) > 0){
		std::string strExName		= GetFileExName(m_EnumImgType).toLocal8Bit();
		std::string strFilePath		= "";

		HObject& hImg				= *((HObject*)param.MapPImgVoid[m_strInImg]);
		HTuple hType;
		GetImageType(hImg,			&hType);
		if (hType.S() == ImgType_Real)			strExName = "tiff";
		else if (hType.S() == ImgType_Int4)		strExName = "tiff";
		else if (hType.S() == ImgType_Int8)		strExName = "tiff";
		else if (hType.S() == ImgType_complex)	strExName = "tiff";
		else if (hType.S() == ImgType_Int1)		strExName = "tiff";
		else if (hType.S() == ImgType_Int2)		strExName = "tiff";

		switch (m_iToolType)	{
		case 0: {	//保存文件路径
			if (m_strImgPath.isEmpty()) { SetError(QString("File %1 is Empty").arg(m_strImgPath)); return NodeResult_Error; }
			strFilePath = m_strImgPath.toLocal8Bit();
			WriteImage(hImg, strExName.c_str(),0, strFilePath.c_str());
		}	break;
		default: {	//保存文件夹路径
			QString _strFile = m_strFolderPath; 
			if (m_iSaveExType != 0 && m_iSaveExType != 2){
				_strFile = QString("%1/%2").arg(m_strFolderPath).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
				QDir dir(_strFile);
				if (!dir.exists())	dir.mkdir(_strFile);
			}
			else{
				QDir dir(_strFile);
				if (!dir.exists())	dir.mkdir(_strFile);
			}
			strFilePath = (QString("%1/%2").arg(_strFile).arg(GetFileExFileName(m_iSaveExType))).toLocal8Bit();
			if (strFilePath.empty()) { SetError(QString("File %1 is Empty").arg(strFilePath.c_str())); return NodeResult_Error; }
			WriteImage(hImg, strExName.c_str(), 0, strFilePath.c_str());
		}	break;
		}
	}
	else{
		SetError(QString("Image %1 didn't Exist").arg(m_strInImg));
		return NodeResult_ParamError;
	}

	return NodeResult_OK;
}

QPair<QString, QString> SaveImageTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> SaveImageTool::GetToolName()
{
	return QPair<QString, QString>("SaveImage", tr("SaveImage"));
}
