#include "Camera_HIK.h"
#include <Windows.h>
#include <qdebug.h>

//抓图线程
/*图像抓取回调函数*/
void __stdcall ImageCallBack(BYTE * pData, MV_FRAME_OUT_INFO* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		Camera_HIK *pThis = (Camera_HIK*)pUser;
		ImageData tempData;
		tempData.pDataBuffer = pData;
		tempData.nImgHeight = pFrameInfo->nHeight;
		tempData.nImgWidth = pFrameInfo->nWidth;
		switch (pFrameInfo->enPixelType) {
		case PixelType_Gvsp_Mono8: {
			tempData.nBitCount = 8;	
		}break;
		case PixelType_Gvsp_BayerGR8:
		case PixelType_Gvsp_BayerRG8:
		case PixelType_Gvsp_BayerGB8:
		case PixelType_Gvsp_BayerBG8: { 
			tempData.nBitCount = 24;
		}break;
		default:
			break;
		}
		pThis->frameReady(tempData);
	}
}

//掉线函数
void __stdcall Lost(unsigned int nMsgType, void* pUser)
{
	Camera_HIK *pThis = (Camera_HIK*)pUser;
	if (nMsgType == 0x8001)
	{
		pThis->LostDev();
	}
}

bool Camera_HIK::m_bFinalize		= false;
Camera_HIK::Camera_HIK()/*:QObject(this)*/
{
	SetConnectedFlag(false);
	m_hTrrigerHand						= ::CreateEventA(NULL, TRUE, FALSE, NULL);

	m_iTimeOut							= 500;

}

Camera_HIK::~Camera_HIK()
{
	SetEvent(m_hTrrigerHand);
	if(m_hTrrigerHand != nullptr) CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand	= nullptr;
}

QString Camera_HIK::getName()
{
	return m_strCameraName;
}

void Camera_HIK::setName(const QString & strName)
{
	m_strCameraName = strName;
}

QString Camera_HIK::getCameraType()
{
	return "Camera_HIK";
}

QString Camera_HIK::getDescription()
{
	//return QString(SR7IF_GetVersion());
	return "Camera_HIK";
}

QVector<CameraInfo> Camera_HIK::getVecInfo()
{
	QVector<CameraInfo> _vecInfo;
	int nTLaysType = MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_GIGE_DEVICE | MV_GENTL_CAMERALINK_DEVICE |
		MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE;
	//1 枚举设备
	MV_CC_DEVICE_INFO_LIST m_stDeviceList = { 0 };
	int nRet = MV_CC_EnumDevices(nTLaysType, &m_stDeviceList);
	if (m_stDeviceList.nDeviceNum > 0)
	{
		for (int i = 0; i < m_stDeviceList.nDeviceNum; ++i)
		{
			CameraInfo _Info;
			MV_CC_DEVICE_INFO* pDeviceInfo	= m_stDeviceList.pDeviceInfo[i];
			if (NULL == pDeviceInfo)		continue;
			int nIp1, nIp2, nIp3, nIp4;
			if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
			{
				_Info.strCameraName		= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
				_Info.strCameraBrand	= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName);
				nIp1 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
				nIp2 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
				nIp3 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
				nIp4 = ( pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
				_Info.strCameraIP		= QString("%1,%2,%3,%4")
					.arg(QString::number(nIp1))
					.arg(QString::number(nIp2))
					.arg(QString::number(nIp3))
					.arg(QString::number(nIp4));
				_Info.strCameraKey			= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
				_Info.strCameraModelName	= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
				_Info.strSerialNumber		= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
				_Info.strManufactureInfo	= QString((char *)pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerSpecificInfo);
			}
			else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
			{
				_Info.strCameraName			= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
				_Info.strCameraBrand		= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chVendorName);
				_Info.strCameraIP			= QString("USB");
				_Info.strCameraKey			= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
				_Info.strCameraModelName	= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
				_Info.strSerialNumber		= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
				_Info.strManufactureInfo	= QString((char *)pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName);
			}
			else if (pDeviceInfo->nTLayerType == MV_CAMERALINK_DEVICE)
			{
				_Info.strCameraName			= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chFamilyName);
				_Info.strCameraBrand		= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chManufacturerName);
				_Info.strCameraIP			= QString("CamLink");
				_Info.strCameraKey			= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chFamilyName);
				_Info.strCameraModelName	= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chModelName);
				_Info.strSerialNumber		= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chSerialNumber);
				_Info.strManufactureInfo	= QString((char *)pDeviceInfo->SpecialInfo.stCamLInfo.chManufacturerName);
			}
			_vecInfo.push_back(_Info);
		}
	}
	return _vecInfo;
}

int Camera_HIK::Initialize()
{
	MV_CC_Initialize();
	m_bFinalize = true;
	return 0;
}

int Camera_HIK::Finalize()
{
	MV_CC_Finalize();
	m_bFinalize = false;
	return 0;
}

int Camera_HIK::Init()
{
	long myRetn = SMCV_RETN_SUCCESS;
	int nTLaysType = MV_GIGE_DEVICE | MV_USB_DEVICE | MV_CAMERALINK_DEVICE;

	//1 枚举设备
	MV_CC_DEVICE_INFO_LIST m_stDeviceList = { 0 };
	int nRet = MV_CC_EnumDevices(nTLaysType, &m_stDeviceList);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}

	int Index = -1;
	if (m_stDeviceList.nDeviceNum > 0)
	{
		for (int i = 0; i < m_stDeviceList.nDeviceNum; ++i)
		{
			if (strcmp(m_strCameraName.toStdString().c_str(), (const char*)m_stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chUserDefinedName) == 0)	//USB3.0
			{
				Index = i;
				break;
			}
			else if (strcmp(m_strCameraName.toStdString().c_str(), (const char*)m_stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName) == 0)	//GIGE
			{
				Index = i;
				break;
			}
		}
	}
	else//枚举设备数为0，打开失败
	{
		return SMCV_RETN_FAILIURE;
	}

	if (Index < 0)//没有找到相机
	{
		return SMCV_RETN_FAILIURE;
	}
	//2 生成操作句柄
	//memset(&m_sDeviceInfo,0,sizeof(MV_CC_DEVICE_INFO));
	memcpy(&m_sDeviceInfo, m_stDeviceList.pDeviceInfo[Index], sizeof(MV_CC_DEVICE_INFO));
	nRet = MV_CC_CreateHandle(&m_hCamera, &m_sDeviceInfo);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}

	//3 打开设备
	nRet = MV_CC_OpenDevice(m_hCamera);
	if (nRet != MV_OK)
	{
		return SMCV_RETN_FAILIURE;
	}

	// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
	if (m_stDeviceList.pDeviceInfo[Index]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = MV_CC_GetOptimalPacketSize(m_hCamera);
		if (nPacketSize > 0)
		{
			nRet = MV_CC_SetIntValue(m_hCamera, "GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
			}
		}
		else
		{
			printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
		}
	}

	//4 设置采集模式
	nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerMode", MV_TRIGGER_MODE_OFF);
	if (nRet != MV_OK)
	{
		return SMCV_RETN_FAILIURE;
	}

	//5 注册回调函数
	nRet = MV_CC_RegisterImageCallBack(m_hCamera, ImageCallBack, this);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}

	//设置触发源
	nRet = MV_CC_SetTriggerSource(m_hCamera, MV_TRIGGER_SOURCE_SOFTWARE);
	if (nRet != MV_OK)
	{
		return SMCV_RETN_FAILIURE;
	}

	//6 设置相图像像素格式
	int enValue = PixelType_Gvsp_Mono8;
	nRet = MV_CC_SetPixelFormat(m_hCamera, enValue);
	if (nRet != MV_OK)
	{
		return SMCV_RETN_FAILIURE;
	}

	//设置心跳时间
	nRet = MV_CC_SetIntValue(m_hCamera, "GevHeartbeatTimeout", 20000);

	//注册掉线函数
	nRet = MV_CC_RegisterExceptionCallBack(m_hCamera, Lost, this);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}

	SetConnectedFlag(true);
	return 0;
}

int Camera_HIK::Exit()
{
	if (NULL == m_hCamera)
	{
		return MV_E_HANDLE;
	}

	MV_CC_CloseDevice(m_hCamera);

	int nRet = MV_CC_DestroyHandle(m_hCamera);
	m_hCamera = NULL;

	SetConnectedFlag(false);
	return 0;
}

int Camera_HIK::SetStream(CameraStream * stream)
{
	m_lockMutex.lock();
	m_ptrCamera = stream;
	m_lockMutex.unlock();
	return 0;
}

int Camera_HIK::Start()
{


	return 0;
}

int Camera_HIK::Stop()
{

	return 0;
}

int Camera_HIK::StartVedioLive()
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//开始采集
	int nRet = MV_CC_StartGrabbing(m_hCamera);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;
}

int Camera_HIK::StopVedioLive()
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}

	//停止采集
	int nRet = MV_CC_StopGrabbing(m_hCamera);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	//m_bIsLive = false;
	return myRetn;
}

void Camera_HIK::GetHeightUpperLower(double & _upper, double & _lower)
{
}

int Camera_HIK::Trigger()
{
	ResetEvent(m_hTrrigerHand);
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		SetConnectedFlag(false);
		return SMCV_RETN_NOINIT;
	}

	int nRet = MV_CC_TriggerSoftwareExecute(m_hCamera);
	if (MV_OK != nRet)
		return SMCV_RETN_FAILIURE;
	return myRetn;

}

int Camera_HIK::WaitTrigger(int iTimeOut)
{
	DWORD dwRetn = WaitForSingleObject(m_hTrrigerHand, iTimeOut);
	if (dwRetn == WAIT_TIMEOUT)
		dwRetn = -1;
	else if (dwRetn >= WAIT_OBJECT_0)
		dwRetn = dwRetn - WAIT_OBJECT_0;
	return dwRetn;
}

int Camera_HIK::SetCameraParam(QString key, QString value)
{
	if (key == CameraMode)
	{
		m_eTrrigerMode = (EnumCameraTriggerMode)value.toInt();
		scvSetTriggerMode(m_eTrrigerMode);
	}
	else if (key == CameraExposure)
	{
		double _dValue = value.toDouble();
		return scvSetExposureParm(_dValue);
	}
	else if (key == CameraGain)
	{
		double _dValue = value.toDouble();
		return scvSetGain(_dValue);
	}
	else if (key == CameraGamma)
	{

	}
	else if (key == CameraContrast)
	{
		double _dValue = value.toDouble();
		return scvSetContrast(_dValue);
	}
	else if (key == CameraBrightness)
	{
		double _dValue = value.toDouble();
		return scvSetBrightness(_dValue);
	}
	else if (key == CameraTriggerDelay)
	{
		double	_dDelay;
		scvGetTriggerDelay(_dDelay);
		value = QString::number(_dDelay);
	}
	return 0;
}

int Camera_HIK::GetCameraParam(QString key, QString & value)
{
	if (key == CameraMode)
	{
		scvGetTriggerMode(m_eTrrigerMode);
		value = QString::number(m_eTrrigerMode);
	}
	else if (key == CameraExposure)
	{
		double	_dExposure;
		scvGetExposureParm(_dExposure);
		value = QString::number(_dExposure);
	}
	else if (key == CameraMaxExposure)
	{
		double	_dExposure;
		scvGetMaxExposureParm(_dExposure);
		value = QString::number(_dExposure);
	}
	else if (key == CameraMinExposure)
	{
		double	_dExposure;
		scvGetMinExposureParm(_dExposure);
		value = QString::number(_dExposure);
	}
	else if (key == CameraGain)
	{
		double	_dGain;
		scvGetGainParm(_dGain);
		value = QString::number(_dGain);
	}
	else if (key == CameraMaxGain)
	{
		double	_dGain;
		scvGetMaxGainParm(_dGain);
		value = QString::number(_dGain);
	}
	else if (key == CameraMinGain)
	{
		double	_dGain;
		scvGetMinGainParm(_dGain);
		value = QString::number(_dGain);
	}
	else if (key == CameraContrast)
	{
		double	_dContrast;
		scvGetContrast(_dContrast);
		value = QString::number(_dContrast);
	}
	else if (key == CameraBrightness)
	{
		double	_dBrightness;
		scvGetBrightness(_dBrightness);
		value = QString::number(_dBrightness);
	}
	else if (key == CameraTriggerDelay)
	{
		double	_dDelay;
		scvGetTriggerDelay(_dDelay);
		value = QString::number(_dDelay);
	}
	return 0;
}

int Camera_HIK::GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue)
{
	mapValue.clear();
	scvGetTriggerMode(m_eTrrigerMode);
	double	_dBrightness;
	double	_dContrast;
	double	_dExposure;
	double	_dMaxExposure;
	double	_dMinExposure;
	double	_dGain;
	double	_dMaxGain;
	double	_dMinGain;
	double	_dDelay;
	scvGetBrightness(_dBrightness);
	scvGetContrast(_dContrast);
	scvGetExposureParm(_dExposure);
	scvGetMaxExposureParm(_dMaxExposure);
	scvGetMinExposureParm(_dMinExposure);
	scvGetGainParm(_dGain);
	scvGetMaxGainParm(_dMaxGain);
	scvGetMinGainParm(_dMinGain);
	scvGetTriggerDelay(_dDelay);

	mapValue.push_back(QPair<QString, QString>(CameraMode, ParamToValue(tr(CameraMode.toStdString().c_str()), 0, ParamType_Int, QString::number(m_eTrrigerMode))));
	mapValue.push_back(QPair<QString, QString>(CameraExposure, ParamToValue(tr(CameraExposure.toStdString().c_str()), 0, ParamType_Double, QString::number(_dExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraMaxExposure, ParamToValue(tr(CameraMaxExposure.toStdString().c_str()), 0, ParamType_Double, QString::number(_dMaxExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraMinExposure, ParamToValue(tr(CameraMinExposure.toStdString().c_str()), 0, ParamType_Double, QString::number(_dMinExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraGain, ParamToValue(tr(CameraGain.toStdString().c_str()), 0, ParamType_Double, QString::number(_dGain))));
	mapValue.push_back(QPair<QString, QString>(CameraMaxGain, ParamToValue(tr(CameraMaxGain.toStdString().c_str()), 0, ParamType_Double, QString::number(_dMaxGain))));
	mapValue.push_back(QPair<QString, QString>(CameraMinGain, ParamToValue(tr(CameraMinGain.toStdString().c_str()), 0, ParamType_Double, QString::number(_dMinGain))));
	mapValue.push_back(QPair<QString, QString>(CameraContrast, ParamToValue(tr(CameraContrast.toStdString().c_str()), 0, ParamType_Double, QString::number(_dContrast))));
	mapValue.push_back(QPair<QString, QString>(CameraBrightness, ParamToValue(tr(CameraBrightness.toStdString().c_str()), 0, ParamType_Double, QString::number(_dBrightness))));
	mapValue.push_back(QPair<QString, QString>(CameraTriggerDelay,	ParamToValue(tr(CameraTriggerDelay.toStdString().c_str()),0, ParamType_Double,	QString::number(_dDelay))));

	return 0;
}

long Camera_HIK::scvSetFPS(double FPS)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	int nRet = MV_CC_SetFrameRate(m_hCamera, FPS);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;

}

long Camera_HIK::scvGetFPS(double& fps)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	MVCC_FLOATVALUE struFloatValue = { 0 };
	int nRet = MV_CC_GetFrameRate(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	fps = struFloatValue.fCurValue;

	return myRetn;
}

long Camera_HIK::scvSetBrightness(double bright)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	int nRet = MV_CC_SetBrightness(m_hCamera, (int)bright);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;
}

long Camera_HIK::scvGetBrightness(double& bright)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	MVCC_INTVALUE struIntValue = { 0 };
	int nRet = MV_CC_GetBrightness(m_hCamera, &struIntValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	bright = struIntValue.nCurValue;
	return myRetn;
}

long Camera_HIK::scvSetContrast(double contrast)
{
	int c = (int)contrast;

	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}


	return 0;
}

long Camera_HIK::scvGetContrast(double& contrast)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}


	return 0;
}

long Camera_HIK::scvGetExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	MVCC_FLOATVALUE struFloatValue = { 0 };
	int nRet = MV_CC_GetExposureTime(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	exposure = struFloatValue.fCurValue;
	return myRetn;
}

long Camera_HIK::scvGetMaxExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	MVCC_INTVALUE struIntValue = { 0 };
	int nRet = MV_CC_GetAutoExposureTimeUpper(m_hCamera, &struIntValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	exposure = struIntValue.nCurValue;
	return myRetn;
}

long Camera_HIK::scvGetMinExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
		return SMCV_RETN_NOINIT;
	//----------------
	MVCC_INTVALUE struIntValue = { 0 };
	int nRet = MV_CC_GetAutoExposureTimeLower(m_hCamera, &struIntValue);
	if (MV_OK != nRet)
		return SMCV_RETN_FAILIURE;
	exposure = struIntValue.nCurValue;
	return myRetn;
}

long Camera_HIK::scvSetExposureParm(long val)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	int nRet = MV_CC_SetExposureTime(m_hCamera, val);//val单位为10us。
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;
}

long Camera_HIK::scvGetGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	MVCC_FLOATVALUE struFloatValue = { 0 };
	int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	gain = struFloatValue.fCurValue;
	return myRetn;
}

long Camera_HIK::scvGetMaxGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	MVCC_FLOATVALUE struFloatValue = { 0 };
	int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	gain = 100;
	gain = struFloatValue.fMax;

	return myRetn;
}

long Camera_HIK::scvGetMinGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	MVCC_FLOATVALUE struFloatValue = { 0 };
	int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	//gain = 0;
	gain = struFloatValue.fMin;
	return myRetn;
}

long Camera_HIK::scvSetGain(long val)
{
	long myRetn = SMCV_RETN_SUCCESS;

	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	float fValue = val;
	int nRet = MV_CC_SetGain(m_hCamera, fValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	//m_Gain = fValue;
	return myRetn;
}

long Camera_HIK::scvSetTriggerDelay(double Delay)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	MVCC_FLOATVALUE struFloatValue = { 0 };

	int nRet = MV_CC_GetTriggerDelay(m_hCamera, &struFloatValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	Delay = 100;
	Delay = struFloatValue.fMax;

	return myRetn;
}

long Camera_HIK::scvGetTriggerDelay(double& Delay)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	float fValue = Delay;
	int nRet = MV_CC_SetTriggerDelay(m_hCamera, fValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	//m_Gain = fValue;
	return myRetn;
}

void Camera_HIK::frameReady(ImageData Data)
{
	if (Data.pDataBuffer != NULL)
	{
		SetConnectedFlag(true);
		switch (Data.nBitCount) {
		case 8: {	
			SendVisionData(m_strCameraName, (unsigned char*)Data.pDataBuffer, NULL, Data.nImgWidth, Data.nImgHeight, Data.nBitCount, 1, ImgType_Byte, Data.nImgHeight);
		}	break;
		case 24: {	
		
		}	break;
		}
	}
	else
	{
		SetConnectedFlag(false);
	}
}

void Camera_HIK::LostDev()
{
	SetConnectedFlag(false);
}

int Camera_HIK::SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	int _iRetn = 0;
	m_lockMutex.lock();
	if(m_ptrCamera != nullptr)	_iRetn = m_ptrCamera->RecieveBuffer(strCamName,gray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
	m_lockMutex.unlock();
	return _iRetn;
}

int Camera_HIK::SendRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	int _iRetn = 0;
	m_lockMutex.lock();
	if (m_ptrCamera != nullptr)	_iRetn = m_ptrCamera->RecieveRGBBuffer(strCamName,Rgray, Ggray, Bgray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
	m_lockMutex.unlock();
	return _iRetn;
}

int Camera_HIK::IsCamCapture()
{
	return true;
}

int Camera_HIK::IsCamConnect()
{
	return m_vlIsConnected;
}

long Camera_HIK::scvSetTriggerMode(EnumCameraTriggerMode trigger)
{
	long myRetn = SMCV_RETN_SUCCESS;

	if (!IsCamConnect())
		return SMCV_RETN_NOINIT;

	int TriggerSource = 0;
	int nRet = MV_OK;
	switch (trigger)
	{
	case CameraMode_SoftTrigger: //软件触发
		TriggerSource = 7;
		break;
	case CameraMode_TriggerLine1_RisingEdge:	//为外部触发，	0，上升沿
	case CameraMode_TriggerLine1_FallingEdge:	//外部触发，	0，下降沿
		TriggerSource = 0;
	case CameraMode_TriggerLine2_RisingEdge:	//为外部触发，	0，上升沿
	case CameraMode_TriggerLine2_FallingEdge:	//外部触发，	0，下降沿
		TriggerSource = 1;
	case CameraMode_TriggerLine3_RisingEdge:	//为外部触发，	0，上升沿
	case CameraMode_TriggerLine3_FallingEdge:	//外部触发，	0，下降沿
		TriggerSource = 2;
	case CameraMode_TriggerLine4_RisingEdge:	//为外部触发，	0，上升沿
	case CameraMode_TriggerLine4_FallingEdge:	//外部触发，	0，下降沿
		TriggerSource = 3;
		break;
	default:
	case CameraMode_Continnue: //连续模式
		TriggerSource = 9;
		break;
	}
	if (TriggerSource == 7)	//软触发
	{
		//nRet = MV_CC_SetTriggerMode(m_hCamera,1 );//1表示打开触发模式，0表示关闭触发模式
		nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerMode", 1);
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;

		nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerSource", TriggerSource);//设置触发源
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;

		//设置闪光灯输出
		nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line1
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;

		nRet = MV_CC_SetBoolValue(m_hCamera, "StrobeEnable", 0);//关闭闪光灯输出
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;
	}
	else if (TriggerSource == 9)
	{
		nRet = MV_CC_SetTriggerMode(m_hCamera, 0);//1表示打开触发模式，0表示关闭触发模式
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;
	}
	else		//非软触发触发方式
	{
		nRet = MV_CC_SetTriggerMode(m_hCamera, 1);//1表示打开触发模式，0表示关闭触发模式
		if (MV_OK != nRet)
			return SMCV_RETN_FAILIURE;

		//设置触发线
		if (TriggerSource == 0)//触发源1
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 0);//选中Line2
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
		}
		else if (TriggerSource == 2)//触发源2
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line2
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
		}
		else if (TriggerSource == 3)//触发源3
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 2);//选中Line2
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
		}
		else  if (TriggerSource == 4)//触发源4
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 3);//选中Line2
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;

			nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			if (MV_OK != nRet)
				return SMCV_RETN_FAILIURE;
		}

		/*0:RisingEdge
		1 : FallingEdge
		2.LevelHigh
		3.LevelLow*/
		if (CameraMode_TriggerLine1_RisingEdge == trigger
			|| CameraMode_TriggerLine2_RisingEdge == trigger
			|| CameraMode_TriggerLine3_RisingEdge == trigger
			|| CameraMode_TriggerLine4_RisingEdge == trigger)
		{
			nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerActivation", 1);//设置上升沿触发
			if (MV_OK != nRet)
			{
				return SMCV_RETN_FAILIURE;
			}
		}
		else if (CameraMode_TriggerLine1_FallingEdge == trigger
			|| CameraMode_TriggerLine2_FallingEdge == trigger
			|| CameraMode_TriggerLine4_FallingEdge == trigger
			|| CameraMode_TriggerLine3_FallingEdge == trigger)
		{
			nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerActivation", 0);//设置下降沿触发
			if (MV_OK != nRet)
			{
				return SMCV_RETN_FAILIURE;
			}
		}
		//设置闪光灯输出
		nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line1
		if (MV_OK != nRet)
		{
			return SMCV_RETN_FAILIURE;
		}
		nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 8);//设置Line1为闪光灯
		if (MV_OK != nRet)
		{
			return SMCV_RETN_FAILIURE;
		}
		nRet = MV_CC_SetEnumValue(m_hCamera, "LineSource", 0);
		if (MV_OK != nRet)
		{
			return SMCV_RETN_FAILIURE;
		}
		nRet = MV_CC_SetBoolValue(m_hCamera, "StrobeEnable", 1);//打开闪光灯输出
		if (MV_OK != nRet)
		{
			return SMCV_RETN_FAILIURE;
		}
	}
	m_eTrrigerMode = trigger;
	return myRetn;
}

long Camera_HIK::scvGetTriggerMode(EnumCameraTriggerMode& trigger)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!scvIsConnected())
		return SMCV_RETN_NOINIT;

	MVCC_ENUMVALUE struEnumValue = { 0 };
	int nRet = MV_CC_GetTriggerMode(m_hCamera, &struEnumValue);
	if (MV_OK != nRet)
	{
		return SMCV_RETN_FAILIURE;
	}
	bool bIstrigger = (struEnumValue.nCurValue == 1 ? true : false);
	if (bIstrigger)
		trigger = m_eTrrigerMode;
	//else
	//	trigger = SMCV_TRIGGER_MODE_RUNING;
	return myRetn;
}


int Camera_HIK::ClearMemory()
{
	return 0;
}


long Camera_HIK::scvIsConnected(bool& isConnect)
{
	return (isConnect = m_vlIsConnected);
}

bool Camera_HIK::scvIsConnected()
{
	return m_vlIsConnected;
}

long Camera_HIK::SetConnectedFlag(bool isConnect)
{
	return _InterlockedExchange(&m_vlIsConnected, isConnect);
}
