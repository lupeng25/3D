#include "Camera3D_SSZN.h"
#include <Windows.h>
#include <qdebug.h>

std::map<QString, Camera3D_SSZN*>	Camera3D_SSZN::m_mapCamera3D_SSZN;
bool Camera3D_SSZN::m_bFinalize		= false;
Camera3D_SSZN::Camera3D_SSZN()/*:QObject(this)*/
{
	m_nCurrentDeviceID					= 0;

	/* 从界面获取IP地址--界面IP输入值 */
	m_SREthernetConFig.abyIpAddress[0]	= 192;
	m_SREthernetConFig.abyIpAddress[1]	= 168;
	m_SREthernetConFig.abyIpAddress[2]	= 0;
	m_SREthernetConFig.abyIpAddress[3]	= 10;

	m_bIsConnected						= false;
	m_hTrrigerHand						= ::CreateEventA(NULL, TRUE, FALSE, NULL);

	m_BatchPoint						= 1;
	m_BatchWidth						= 1;
	m_XinterVal							= 1;
	m_YinterVal							= 1;
	//m_bIOTrriger						= 1;
	m_iTimeOut							= 500;

}

Camera3D_SSZN::~Camera3D_SSZN()
{
	SetEvent(m_hTrrigerHand);
	if(m_hTrrigerHand != nullptr) CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand	= nullptr;
}

QString Camera3D_SSZN::getName()
{
	return m_strCameraName;
}

void Camera3D_SSZN::setName(const QString & strName)
{
	m_strCameraName = strName;
}

QString Camera3D_SSZN::getCameraType()
{
	return "Camera3D_SSZN";
}

QString Camera3D_SSZN::getDescription()
{
	return QString(SR7IF_GetVersion());
}

QVector<CameraInfo> Camera3D_SSZN::getVecInfo()
{
	QVector<CameraInfo> _vecInfo;
	int	_iReadNum;
	SR7IF_ETHERNET_CONFIG* eth = SR7IF_SearchOnline(&_iReadNum, 5000);
	for (size_t i = 0; i < _iReadNum; i++)	{
		if (eth != nullptr)		{
			CameraInfo _CameraIn;
			_CameraIn.strCameraBrand		= "sszn";
			_CameraIn.strCameraIP			= QString("%1.%2.%3.%4").arg(QString::number(eth->abyIpAddress[0]))
				.arg(QString::number(eth->abyIpAddress[1]))
				.arg(QString::number(eth->abyIpAddress[2]))
				.arg(QString::number(eth->abyIpAddress[3]));

			_CameraIn.strCameraName			= _CameraIn.strCameraIP;
			_CameraIn.strSerialNumber		= QString(SR7IF_GetHeaderSerial(i,0));
			_CameraIn.strCameraModelName	= QString(SR7IF_GetModels(i));
			_vecInfo.push_back(_CameraIn);
		}
	}

	return _vecInfo;
}

int Camera3D_SSZN::Initialize()
{
	if (!m_bFinalize)
	{
		m_mapCamera3D_SSZN.clear();
	}
	m_bFinalize = true;
	return 0;
}

int Camera3D_SSZN::Finalize()
{
	if (m_bFinalize)
	{
		if (m_mapCamera3D_SSZN.size() > 0)
		{
			for (auto iter : m_mapCamera3D_SSZN)
			{
				iter.second->ClearMemory();
				iter.second->StopVedioLive();
				iter.second->Stop();
				iter.second->Exit();
			}
		}
		m_mapCamera3D_SSZN.clear();
	}
	m_bFinalize = false;
	return 0;
}

int Camera3D_SSZN::Init()
{  
	int _iRetn = SR7IF_EthernetOpenExt(m_nCurrentDeviceID, &m_SREthernetConFig,2000, CameraTcpConnectFunc);
	if (_iRetn < 0)   //连接失败
	{
		return _iRetn;
	}

	m_BatchWidth = SR7IF_ProfileDataWidth(m_nCurrentDeviceID, NULL);
	if (m_iGetCallMode == 2)		//多次回调
	{
		_iRetn = SR7IF_HighSpeedDataEthernetCommunicationInitalize(m_nCurrentDeviceID,
			&m_SREthernetConFig, 0, CallHighSpeedData, m_iFreqValue, m_nCurrentDeviceID);
	}
	else if (m_iGetCallMode == 1)	//一次性回调
	{
		_iRetn = SR7IF_SetBatchOneTimeDataHandler(m_nCurrentDeviceID, BatchOneTimeCallBack);
	}
	else							//无限循环
	{

	}
	m_mapCamera3D_SSZN.insert(std::pair<QString, Camera3D_SSZN*>(QString::number(m_nCurrentDeviceID),this));
	m_bIsConnected = true;
	return 0;
}

int Camera3D_SSZN::Exit()
{
	m_bIsConnected = false;

	//停止批处理
	int _iRetn = SR7IF_StopMeasure(m_nCurrentDeviceID);

	if (m_mapCamera3D_SSZN.count(QString::number(m_nCurrentDeviceID))> 0)
		m_mapCamera3D_SSZN.erase(QString::number(m_nCurrentDeviceID));

	_iRetn = SR7IF_CommClose(m_nCurrentDeviceID);
	return 0;
}

int Camera3D_SSZN::SetStream(CameraStream * stream)
{
	m_ptrCamera = stream;
	return 0;
}

int Camera3D_SSZN::Start()
{
	int _iRetn = SR7IF_StartMeasureWithCallback(m_nCurrentDeviceID, 1);
	if (_iRetn < 0)   //连接失败
	{
		return _iRetn;
	}


	return 0;
}

int Camera3D_SSZN::Stop()
{

	return 0;
}

int Camera3D_SSZN::StartVedioLive()
{
	//批处理行数、轮廓宽度、X间距获取
	m_BatchPoint	= SR7IF_ProfilePointCount(m_nCurrentDeviceID, NULL);
	m_BatchWidth	= SR7IF_ProfileDataWidth(m_nCurrentDeviceID, NULL);
	m_XinterVal		= SR7IF_ProfileData_XPitch(m_nCurrentDeviceID, NULL);
	return 0;
}

int Camera3D_SSZN::StopVedioLive()
{
	return 0;
}

void Camera3D_SSZN::GetHeightUpperLower(double & _upper, double & _lower)
{
	const char* _version = SR7IF_GetModels(m_nCurrentDeviceID);   //型号获取
	QString str_Version;
	str_Version = QString(_version);

	double m_dHeightRange = 8.4;
	if (str_Version == /*tr*/("SR7050") || str_Version == /*tr*/("SR7060D"))
	{
		m_dHeightRange = 3;
	}
	else if (str_Version == /*tr*/("SR7080"))
	{
		m_dHeightRange = 9;
	}
	else if (str_Version == /*tr*/("SR7140"))
	{
		m_dHeightRange = 15;
	}
	else if (str_Version == /*tr*/("SR7240"))
	{
		m_dHeightRange = 24;
	}
	else if (str_Version == /*tr*/("SR7400"))
	{
		m_dHeightRange = 60;
	}
	else if (str_Version == /*tr*/("SR7300"))
	{
		m_dHeightRange = 150;
	}
	else if (str_Version == /*tr*/("SR6060"))
	{
		m_dHeightRange = 15;
	}
	else if (str_Version == /*tr*/("SR6030"))
	{
		m_dHeightRange = 8;
	}
	else if (str_Version == /*tr*/("SR6070"))
	{
		m_dHeightRange = 16;
	}
	else if (str_Version == /*tr*/("SR6071"))
	{
		m_dHeightRange = 40;
	}
	else if (str_Version == /*tr*/("SR6130"))
	{
		m_dHeightRange = 100;
	}
	else if (str_Version == /*tr*/("SR6260"))
	{
		m_dHeightRange = 240;
	}
	else if (str_Version == /*tr*/("SR8020"))
	{
		m_dHeightRange = 6;
	}
	else if (str_Version == /*tr*/("SR8060"))
	{
		m_dHeightRange = 20;
	}
	else if (str_Version == /*tr*/("SR7900"))
	{
		m_dHeightRange = 450;
	}
	else if (str_Version == /*tr*/("SR7060"))
	{
		m_dHeightRange = 6;
	}
	else if (str_Version == /*tr*/("SR71600"))
	{
		m_dHeightRange = 1500;
	}

	_upper = m_dHeightRange;
	_lower = -m_dHeightRange;
}

int Camera3D_SSZN::Trigger()
{
	ResetEvent(m_hTrrigerHand);
	int _iRetn = 0;
	if (m_iGetCallMode != 1)	//无限循环 //多次回调
	{
		_iRetn = SR7IF_StartIOTriggerMeasure(m_nCurrentDeviceID, m_iTimeOut);
	}
	else	//一次性回调
	{
		_iRetn = SR7IF_TriggerOneBatch(m_nCurrentDeviceID);
	}
	if (_iRetn < 0)   //连接失败
	{
		return _iRetn;
	}

	return 0;
}

int Camera3D_SSZN::WaitTrigger(int iTimeOut)
{
	DWORD dwRetn = WaitForSingleObject(m_hTrrigerHand, iTimeOut);
	if (dwRetn == WAIT_TIMEOUT)
		dwRetn = -1;
	else if (dwRetn >= WAIT_OBJECT_0)
		dwRetn = dwRetn - WAIT_OBJECT_0;
	return dwRetn;
}

int Camera3D_SSZN::SetCameraParam(QString key, QString value)
{
	if (key == CameraMode)
	{
		int	mode = value.toInt();
		if (mode == 0)		//循环多次
		{
			//批处理行数、轮廓宽度、X间距获取
			m_BatchPoint	= SR7IF_ProfilePointCount(m_nCurrentDeviceID, NULL);
			m_BatchWidth	= SR7IF_ProfileDataWidth(m_nCurrentDeviceID, NULL);
			m_XinterVal		= SR7IF_ProfileData_XPitch(m_nCurrentDeviceID, NULL);
		}
	}
	else if(key == CameraIPAdress)
	{
		QStringList params = value.split(".");
		if (params.size() > 0)	m_SREthernetConFig.abyIpAddress[0] = (char)params[0].toInt();
		if (params.size() > 1)	m_SREthernetConFig.abyIpAddress[1] = (char)params[1].toInt();
		if (params.size() > 2)	m_SREthernetConFig.abyIpAddress[2] = (char)params[2].toInt();
		if (params.size() > 3)	m_SREthernetConFig.abyIpAddress[3] = (char)params[3].toInt();
	}
	else if (key == CameraDealID)
	{
		m_nCurrentDealID	= value.toInt();
		return	SR7IF_SwitchProgram(m_nCurrentDeviceID, m_nCurrentDealID);
	}
	else if (key == CameraRowCount)
	{
		m_BatchPoint = value.toInt();
		return	SR7IF_SetSetting(m_nCurrentDeviceID, 0x02,-1,0x00,0x0a,0, &m_BatchPoint,2);
	}
	return 0;
}

int Camera3D_SSZN::GetCameraParam(QString key, QString & value)
{
	if (key == CameraMode)
	{

	}
	else if (key == CameraIPAdress)
	{
		value = QString("%1.%2.%3.%4")
			.arg(QString::number(m_SREthernetConFig.abyIpAddress[0]))
			.arg(QString::number(m_SREthernetConFig.abyIpAddress[1]))
			.arg(QString::number(m_SREthernetConFig.abyIpAddress[2]))
			.arg(QString::number(m_SREthernetConFig.abyIpAddress[3]));
	}
	else if (key == CameraWidth)
	{
		m_BatchWidth = SR7IF_ProfileDataWidth(m_nCurrentDeviceID, NULL);
		value = QString::number(m_BatchWidth);
	}
	else if (key == CameraHeight)
	{
		m_BatchPoint = SR7IF_ProfilePointSetCount(m_nCurrentDeviceID, NULL);
		value = QString::number(m_BatchPoint);
	}
	else if (key == CameraZInterVal)
	{
		m_XinterVal = SR7IF_ProfileData_XPitch(m_nCurrentDeviceID, NULL);
		value = QString::number(m_XinterVal);
	}
	else if (key == CameraDealID)
	{
		value = QString::number(m_nCurrentDealID);
	}
	else if (key == CameraSerial)
	{
		value = SR7IF_GetHeaderSerial(m_nCurrentDeviceID, 0);
	}
	else if (key == CameraModels)
	{
		value = SR7IF_GetModels(m_nCurrentDeviceID);
	}
	else if (key == CameraDeviceId)
	{
		value = QString::number(m_nCurrentDeviceID);
	}
	else if (key == CameraRowCount)
	{
		m_BatchPoint = SR7IF_ProfilePointSetCount(m_nCurrentDeviceID, NULL);
		value = QString::number(m_BatchPoint);
	}
	else if (key == CameraLicenseKey)
	{
		unsigned short RemainDay[128] = { 0 };
		SR7IF_GetLicenseKey(m_nCurrentDeviceID, RemainDay);
		value = QString::number(RemainDay[m_nCurrentDeviceID]);
	}
	return 0;
}

int Camera3D_SSZN::GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue)
{
	mapValue.clear();
	m_BatchPoint	= SR7IF_ProfilePointSetCount(m_nCurrentDeviceID, NULL);
	m_BatchWidth	= SR7IF_ProfileDataWidth(m_nCurrentDeviceID, NULL);
	m_XinterVal		= SR7IF_ProfileData_XPitch(m_nCurrentDeviceID, NULL);
	mapValue.push_back(QPair<QString, QString>(CameraWidth,		
		ParamToValue(tr(CameraWidth.toStdString().c_str()), 0, ParamType_Int,	QString::number(m_BatchWidth))));
	mapValue.push_back(QPair<QString, QString>(CameraRowCount,	
		ParamToValue(tr(CameraRowCount.toStdString().c_str()),1, ParamType_Int,	QString::number(m_BatchPoint))));
	mapValue.push_back(QPair<QString, QString>(CameraZInterVal,	
		ParamToValue(tr(CameraZInterVal.toStdString().c_str()),0, ParamType_Double,QString::number(m_XinterVal))));
	mapValue.push_back(QPair<QString, QString>(CameraDealID,	
		ParamToValue(tr(CameraDealID.toStdString().c_str()),1, ParamType_Int,	QString::number(m_nCurrentDealID))));
	mapValue.push_back(QPair<QString, QString>(CameraIPAdress,		
		ParamToValue(tr(CameraIPAdress.toStdString().c_str()),1, ParamType_String, QString("%1.%2.%3.%4")
		.arg(QString::number(m_SREthernetConFig.abyIpAddress[0]))
		.arg(QString::number(m_SREthernetConFig.abyIpAddress[1]))
		.arg(QString::number(m_SREthernetConFig.abyIpAddress[2]))
		.arg(QString::number(m_SREthernetConFig.abyIpAddress[3])))));
	mapValue.push_back(QPair<QString, QString>(CameraSerial,	
		ParamToValue(tr(CameraSerial.toStdString().c_str()),0, ParamType_String, SR7IF_GetHeaderSerial(m_nCurrentDeviceID,0))));
	mapValue.push_back(QPair<QString, QString>(CameraModels,		
		ParamToValue(tr(CameraModels.toStdString().c_str()),0, ParamType_String, SR7IF_GetModels(m_nCurrentDeviceID))));
	mapValue.push_back(QPair<QString, QString>(CameraDeviceId,		
		ParamToValue(tr(CameraDeviceId.toStdString().c_str()),0, ParamType_String, QString::number(m_nCurrentDeviceID))));
	unsigned short RemainDay[128] = { 0 };
	SR7IF_GetLicenseKey(m_nCurrentDeviceID, RemainDay);
	mapValue.push_back(QPair<QString, QString>(CameraLicenseKey,	
		ParamToValue(tr(CameraLicenseKey.toStdString().c_str()), 0, ParamType_String, QString::number(RemainDay[m_nCurrentDeviceID]))));

	return 0;
}

int Camera3D_SSZN::SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	return m_ptrCamera->RecieveBuffer(strCamName,gray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
}

int Camera3D_SSZN::SendRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	return m_ptrCamera->RecieveRGBBuffer(strCamName,Rgray, Ggray, Bgray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
}

int Camera3D_SSZN::IsCamCapture()
{
	return true;
}

int Camera3D_SSZN::IsCamConnect()
{
	return m_bIsConnected;
}

void Camera3D_SSZN::CameraTcpConnectFunc(int dwDeviceId, int cmd)
{
	if (m_mapCamera3D_SSZN.count(QString::number(dwDeviceId)) > 0)
	{
		Camera3D_SSZN* _Pthis = m_mapCamera3D_SSZN.at(QString::number(dwDeviceId));

	}
}

int Camera3D_SSZN::ClearMemory()
{
	return 0;
}

//一次回调
void Camera3D_SSZN::BatchOneTimeCallBack(const void * info, const SR7IF_Data * data)
{
	const SR7IF_STR_CALLBACK_INFO * conInfo = (const SR7IF_STR_CALLBACK_INFO*)info;
	if (conInfo->returnStatus != SR7IF_OK)
	{
		QString strErrorInfo = /*tr*/QString("回调停止，返回值：%1").arg(conInfo->returnStatus);
		//qDebug() << strErrorInfo;
		return;
	}
	if (conInfo->BatchPoints == 0)
	{
		QString strErrorInfo = /*tr*/QString("回调停止,图像没有长度返回值：%1").arg(conInfo->returnStatus);
		//qDebug() << strErrorInfo;
		return;
	}
	int	_iNumber = 0;
	if (m_mapCamera3D_SSZN.count(QString::number(_iNumber)) > 0)
	{
		Camera3D_SSZN* _Pthis = m_mapCamera3D_SSZN.at(QString::number(_iNumber));
		if (_Pthis != nullptr)
		{
			//高度数据获取--相机A
			const int* mTmpData = SR7IF_GetBatchProfilePoint(data, 0);

			//灰度数据获取--相机A
			const unsigned char* mTmpGraydata = SR7IF_GetBatchIntensityPoint(data, 0);

			//编码器数据获取--相机A
			const unsigned int* mTmpEncoderdata = SR7IF_GetBatchEncoderPoint(data, 0);

			_Pthis->SendVisionData(_Pthis->m_strCameraName,(unsigned char*)mTmpGraydata, (int*)mTmpData, conInfo->xPoints,
				conInfo->BatchPoints, 8, 1, ImgType_Int4, conInfo->BatchPoints);
		}
	}		//传感器数量
	if (conInfo->HeadNumber >= 2)
	{
		_iNumber = 1;
		if (m_mapCamera3D_SSZN.count(QString::number(_iNumber)) > 0)
		{
			Camera3D_SSZN* _Pthis = m_mapCamera3D_SSZN.at(QString::number(_iNumber));
			if (_Pthis != nullptr)
			{
				//高度数据获取--相机A
				const int* mTmpData = SR7IF_GetBatchProfilePoint(data, _iNumber);

				//灰度数据获取--相机A
				const unsigned char* mTmpGraydata = SR7IF_GetBatchIntensityPoint(data, _iNumber );

				//编码器数据获取--相机A
				const unsigned int* mTmpEncoderdata = SR7IF_GetBatchEncoderPoint(data, _iNumber);

				_Pthis->SendVisionData(_Pthis->m_strCameraName, (unsigned char*)mTmpGraydata, (int*)mTmpData, conInfo->xPoints,
					conInfo->BatchPoints, 8, 1, ImgType_Int4, conInfo->BatchPoints);
			}
		}
	}		//传感器数量
	if (conInfo->HeadNumber >= 3)
	{
		_iNumber = 2;
		if (m_mapCamera3D_SSZN.count(QString::number(_iNumber)) > 0)
		{
			Camera3D_SSZN* _Pthis = m_mapCamera3D_SSZN.at(QString::number(_iNumber));

			if (_Pthis != nullptr)
			{
				//高度数据获取--相机A
				const int* mTmpData = SR7IF_GetBatchProfilePoint(data, _iNumber);

				//灰度数据获取--相机A
				const unsigned char* mTmpGraydata = SR7IF_GetBatchIntensityPoint(data, _iNumber);

				//编码器数据获取--相机A
				const unsigned int* mTmpEncoderdata = SR7IF_GetBatchEncoderPoint(data, _iNumber);

				_Pthis->SendVisionData(_Pthis->m_strCameraName,(unsigned char*)mTmpGraydata, (int*)mTmpData, conInfo->xPoints,
					conInfo->BatchPoints, 8, 1, ImgType_Int4, conInfo->BatchPoints);
			}
		}
	}
}

void Camera3D_SSZN::CallHighSpeedData(char * pBuffer, unsigned int uSize, unsigned int uCount, unsigned int uNotify, unsigned int uUser)
{
	if (uNotify != 0)
	{
		if (uNotify & 0x08)
		{
			QString strErrorInfo = /*tr*/QString("批处理超时，返回值：0x%1").arg(QString::number(uNotify, 16));
			return;
		}
	}
	if (uCount == 0 || uSize == 0)
	{
		QString strErrorInfo = /*tr*/QString("批处理超时，图像返回0行返回值：0x%1").arg(QString::number(uNotify, 16));
		return;
	}







}