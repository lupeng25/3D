#include "Camera_DH.h"
#include <Windows.h>
#include <qdebug.h>

static IMV_EPixelType gFormatTransferTbl[] =
{
	// Mono Format
	gvspPixelMono1p,
	gvspPixelMono8,
	gvspPixelMono10,
	gvspPixelMono10Packed,
	gvspPixelMono12,
	gvspPixelMono12Packed,

	// Bayer Format		
	gvspPixelBayRG8,
	gvspPixelBayGB8,
	gvspPixelBayBG8,
	gvspPixelBayRG10,
	gvspPixelBayGB10,
	gvspPixelBayBG10,
	gvspPixelBayRG12,
	gvspPixelBayGB12,
	gvspPixelBayBG12,
	gvspPixelBayRG10Packed,
	gvspPixelBayGB10Packed,
	gvspPixelBayBG10Packed,
	gvspPixelBayRG12Packed,
	gvspPixelBayGB12Packed,
	gvspPixelBayBG12Packed,
	gvspPixelBayRG16,
	gvspPixelBayGB16,
	gvspPixelBayBG16,
	gvspPixelBayRG10p,
	gvspPixelBayRG12p,

	gvspPixelMono1c,

	// RGB Format
	gvspPixelRGB8,
	gvspPixelBGR8,

	// YVR Format
	gvspPixelYUV411_8_UYYVYY,
	gvspPixelYUV422_8_UYVY,
	gvspPixelYUV422_8,
	gvspPixelYUV8_UYV,
};
#define gFormatTransferTblLen	sizeof(gFormatTransferTbl)/sizeof(gFormatTransferTbl[0])

static int findMatchCode(IMV_EPixelType code)
{
	for (int i = 0; i < gFormatTransferTblLen; ++i)
	{
		if (gFormatTransferTbl[i] == code)
		{
			return i;
		}
	}
	return -1;
}
//抓图线程
/*图像抓取回调函数*/
static void onGetFrame(IMV_Frame* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		Camera_DH *pThis = (Camera_DH*)pUser;
		if (pFrameInfo->frameInfo.size < 0)
		{
			return;
		}

		int idx = findMatchCode((pFrameInfo->frameInfo.pixelFormat));
		if (idx < 0)
		{
			return;
		}

		unsigned int BufferSize_;
		if (pFrameInfo->frameInfo.pixelFormat == gvspPixelMono8)
		{
			BufferSize_ = pFrameInfo->frameInfo.size;
		}
		else
		{
			BufferSize_ = pFrameInfo->frameInfo.width * pFrameInfo->frameInfo.height * 3;
		}
		ImageData tempData;
		tempData.pDataBuffer	= pFrameInfo->pData;
		tempData.nImgHeight		= pFrameInfo->frameInfo.height;
		tempData.nImgWidth		= pFrameInfo->frameInfo.width;
		tempData.nBitCount		= pFrameInfo->frameInfo.pixelFormat == gvspPixelMono8 ? 8 : 24; 
		pThis->frameReady(tempData);
	}
}

static void OnConnect(const IMV_SConnectArg* pConnectArg, void* pUser)
{
	Camera_DH *pThis = (Camera_DH*)pUser;
	if(pThis != nullptr)
	{
		pThis->deviceLinkNotifyProc(*pConnectArg);
	}

}

//掉线函数
void __stdcall Lost(unsigned int nMsgType, void* pUser)
{
	Camera_DH *pThis = (Camera_DH*)pUser;
	if (nMsgType == 0x8001)
	{
		pThis->LostDev();
	}
}

bool Camera_DH::m_bFinalize		= false;

Camera_DH::Camera_DH()
{
	SetConnectedFlag(false);
	m_hTrrigerHand						= ::CreateEventA(NULL, TRUE, FALSE, NULL);

	m_iTimeOut							= 500;
	//m_pCMvCamera = new CMvCamera();

}

Camera_DH::~Camera_DH()
{
	SetEvent(m_hTrrigerHand);
	if(m_hTrrigerHand != nullptr) CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand	= nullptr;
}

QString Camera_DH::getName()
{
	return m_strCameraName;
}

void Camera_DH::setName(const QString & strName)
{
	m_strCameraName = strName;
}

QString Camera_DH::getCameraType()
{
	return "Camera_DH";
}

QString Camera_DH::getDescription()
{
	//return QString(SR7IF_GetVersion());
	return "Camera_DH";
}

QVector<CameraInfo> Camera_DH::getVecInfo()
{
	QVector<CameraInfo> _vecInfo;
	IMV_DeviceList deviceInfoList;
	if (IMV_OK != IMV_EnumDevices(&deviceInfoList, interfaceTypeAll))
	{
		return _vecInfo;
	}
	if (deviceInfoList.nDevNum > 0)
	{
		for (unsigned int index = 0; index < deviceInfoList.nDevNum; index++)
		{
			CameraInfo _Info;
			_Info.strCameraName			= QString(deviceInfoList.pDevInfo[index].cameraName);
			_Info.strCameraBrand		= QString(deviceInfoList.pDevInfo[index].vendorName);
			_Info.strCameraIP			= QString();
			_Info.strCameraKey			= QString(deviceInfoList.pDevInfo[index].cameraKey);
			_Info.strCameraModelName	= QString(deviceInfoList.pDevInfo[index].modelName);
			_Info.strSerialNumber		= QString(deviceInfoList.pDevInfo[index].serialNumber);
			_Info.strManufactureInfo	= QString(deviceInfoList.pDevInfo[index].manufactureInfo);
			_vecInfo.push_back(_Info);
		}
	}
	return _vecInfo;
}

int Camera_DH::Initialize()
{
	//MV_CC_Initialize();
	m_bFinalize = true;
	return 0;
}

int Camera_DH::Finalize()
{
	//MV_CC_Finalize();
	m_bFinalize = false;
	return 0;
}

int Camera_DH::Init()
{
	if (!scvIsConnected())
	{
		if (m_devHandle)
		{
			IMV_DestroyHandle(m_devHandle);
			m_devHandle = NULL;
		}

		// 发现设备 
		// discover camera 
		IMV_DeviceList deviceInfoList;
		int ret = IMV_EnumDevices(&deviceInfoList, interfaceTypeAll);
		if (IMV_OK != ret)
		{
			printf("Enumeration devices failed! ErrorCode[%d]\n", ret);
			getchar();
			return -1;
		}
		int Index = -1;
		if (deviceInfoList.nDevNum > 0)
		{
			for (int i = 0; i < deviceInfoList.nDevNum; ++i)
			{
				if (strcmp(m_strCameraName.toStdString().c_str(), (const char*)deviceInfoList.pDevInfo[i].cameraName) == 0)	//USB3.0
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

		std::string cameraKey = m_strCameraName.toStdString();
		ret = IMV_CreateHandle(&m_devHandle, modeByDeviceUserID, (void*)cameraKey.c_str());
		if (IMV_OK != ret)
		{
			return -1;
		}

		if (IMV_OK != IMV_Open(m_devHandle))
		{
			return -1;
		}

		// 设置驱动包间隔时间       
		// Set packet timeout   
		ret = IMV_GIGE_SetInterPacketTimeout(m_devHandle, 50);
		if (IMV_OK != ret)       
		{
			printf("IMV_GIGE_SetInterPacketTimeoutfailed! ErrorCode[%d]\n", ret);
			return -1;
		}
		// 注册数据帧回调函数
		// Register data frame callback function
		ret = IMV_AttachGrabbing(m_devHandle, onGetFrame, NULL);
		if (IMV_OK != ret)
		{
			printf("Attach grabbing failed! ErrorCode[%d]\n", ret);
			return -1;
		}
		// 重新设备连接状态事件回调函数
		//// Device connection status event callback function again
		ret = IMV_SubscribeConnectArg(m_devHandle, OnConnect, this);
		if (IMV_OK != ret)
		{
			printf("IMV_SubscribeConnectArg failed!");
			return -1;
		}

		//// Device connection status event callback function
		//if (IMV_OK != IMV_SubscribeConnectArg(m_devHandle, OnConnect, this))
		//{
		//	return -1;
		//}

		SetConnectedFlag(true);
	}
	else
	{
		SetConnectedFlag(true);
	}

	return 0;
}

int Camera_DH::Exit()
{
	//if (NULL == m_hCamera)
	//{
	//	return MV_E_HANDLE;
	//}

	//MV_CC_CloseDevice(m_hCamera);

	//int nRet = MV_CC_DestroyHandle(m_hCamera);
	//m_hCamera = NULL;

	if ((NULL != m_devHandle) && (IMV_OK != IMV_Close(m_devHandle)))
	{
		printf("Close camera Failed!");
	}

	IMV_DestroyHandle(m_devHandle);
	m_devHandle = NULL;

	SetConnectedFlag(false);
	return 0;
}

int Camera_DH::SetStream(CameraStream * stream)
{
	m_lockMutex.lock();
	m_ptrCamera = stream;
	m_lockMutex.unlock();
	return 0;
}

int Camera_DH::Start()
{


	return 0;
}

int Camera_DH::Stop()
{

	return 0;
}

int Camera_DH::StartVedioLive()
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}

	//开始采集
	if (IMV_OK != IMV_AttachGrabbing(m_devHandle, onGetFrame, this))
	{
		return SMCV_RETN_FAILIURE;
	}

	if (IMV_OK != IMV_StartGrabbing(m_devHandle))
	{
		return SMCV_RETN_FAILIURE;
	}

	return myRetn;
}

int Camera_DH::StopVedioLive()
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}

	if (!IMV_IsGrabbing(m_devHandle))
	{
		return myRetn;
	}

	if (IMV_OK != IMV_StopGrabbing(m_devHandle))
	{
		return SMCV_RETN_FAILIURE;
	}
	//m_bIsLive = false;
	return myRetn;
}

void Camera_DH::GetHeightUpperLower(double & _upper, double & _lower)
{
}

int Camera_DH::Trigger()
{
	ResetEvent(m_hTrrigerHand);
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		SetConnectedFlag(false);
		return SMCV_RETN_NOINIT;
	}

	if (IMV_OK != IMV_ExecuteCommandFeature(m_devHandle, "TriggerSoftware"))
	{
		printf("TriggerSoftware failed!");
		return SMCV_RETN_FAILIURE;
	}
	//int nRet = MV_CC_TriggerSoftwareExecute(m_hCamera);
	//if (MV_OK != nRet)
	//	return SMCV_RETN_FAILIURE;
	return myRetn;

}

int Camera_DH::WaitTrigger(int iTimeOut)
{
	DWORD dwRetn = WaitForSingleObject(m_hTrrigerHand, iTimeOut);
	if (dwRetn == WAIT_TIMEOUT)
		dwRetn = -1;
	else if (dwRetn >= WAIT_OBJECT_0)
		dwRetn = dwRetn - WAIT_OBJECT_0;
	return dwRetn;
}

int Camera_DH::SetCameraParam(QString key, QString value)
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

int Camera_DH::GetCameraParam(QString key, QString & value)
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

int Camera_DH::GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue)
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

	mapValue.push_back(QPair<QString, QString>(CameraMode,			ParamToValue(tr(CameraMode.toStdString().c_str()),0, ParamType_Int,			QString::number(m_eTrrigerMode))));
	mapValue.push_back(QPair<QString, QString>(CameraExposure,		ParamToValue(tr(CameraExposure.toStdString().c_str()),0, ParamType_Double,	QString::number(_dExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraMaxExposure,	ParamToValue(tr(CameraMaxExposure.toStdString().c_str()),0, ParamType_Double,	QString::number(_dMaxExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraMinExposure,	ParamToValue(tr(CameraMinExposure.toStdString().c_str()),0, ParamType_Double,	QString::number(_dMinExposure))));
	mapValue.push_back(QPair<QString, QString>(CameraGain,			ParamToValue(tr(CameraGain.toStdString().c_str()),0, ParamType_Double,	QString::number(_dGain))));
	mapValue.push_back(QPair<QString, QString>(CameraMaxGain,		ParamToValue(tr(CameraMaxGain.toStdString().c_str()),0, ParamType_Double,	QString::number(_dMaxGain))));
	mapValue.push_back(QPair<QString, QString>(CameraMinGain,		ParamToValue(tr(CameraMinGain.toStdString().c_str()),0, ParamType_Double,	QString::number(_dMinGain))));
	mapValue.push_back(QPair<QString, QString>(CameraContrast,		ParamToValue(tr(CameraContrast.toStdString().c_str()),0, ParamType_Double,	QString::number(_dContrast))));
	mapValue.push_back(QPair<QString, QString>(CameraBrightness,	ParamToValue(tr(CameraBrightness.toStdString().c_str()),0, ParamType_Double,	QString::number(_dBrightness))));
	mapValue.push_back(QPair<QString, QString>(CameraTriggerDelay,	ParamToValue(tr(CameraTriggerDelay.toStdString().c_str()),0, ParamType_Double,	QString::number(_dDelay))));

	return 0;
}

long Camera_DH::scvSetFPS(double FPS)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//int nRet = MV_CC_SetFrameRate(m_hCamera, FPS);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "AcquisitionFrameRate", FPS))
	{
		printf("Set AcquisitionFrameRate value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;

}

long Camera_DH::scvGetFPS(double& fps)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "AcquisitionFrameRate", fps))
	{
		printf("Set AcquisitionFrameRate value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}

	//MVCC_FLOATVALUE struFloatValue = { 0 };
	//int nRet = MV_CC_GetFrameRate(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//fps = struFloatValue.fCurValue;

	return myRetn;
}

long Camera_DH::scvSetBrightness(double bright)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	//int nRet = MV_CC_SetBrightness(m_hCamera, (int)bright);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "Bright", bright))
	{
		printf("Set AcquisitionFrameRate value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;
}

long Camera_DH::scvGetBrightness(double& bright)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "Bright", bright))
	{
		printf("Set AcquisitionFrameRate value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	//MVCC_INTVALUE struIntValue = { 0 };
	//int nRet = MV_CC_GetBrightness(m_hCamera, &struIntValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//bright = struIntValue.nCurValue;
	return myRetn;
}

long Camera_DH::scvSetContrast(double contrast)
{
	int c = (int)contrast;

	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}


	return 0;
}

long Camera_DH::scvGetContrast(double& contrast)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}


	return 0;
}

long Camera_DH::scvGetExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	if (IMV_OK != IMV_GetDoubleFeatureValue(m_devHandle, "ExposureTime", &exposure))
	{
		printf("Set scvGetExposureParm value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	//MVCC_FLOATVALUE struFloatValue = { 0 };
	//int nRet = MV_CC_GetExposureTime(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//exposure = struFloatValue.fCurValue;
	return myRetn;
}

long Camera_DH::scvGetMaxExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	//MVCC_INTVALUE struIntValue = { 0 };
	//int nRet = MV_CC_GetAutoExposureTimeUpper(m_hCamera, &struIntValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//exposure = struIntValue.nCurValue;
	//----------------
	int	ret = IMV_GetDoubleFeatureMax(m_devHandle, "ExposureTime", &exposure);
	if (IMV_OK != ret)
	{
		printf("Get feature Max value failed! ErrorCode[%d]\n", ret);
		return ret;
	}
	return myRetn;
}

long Camera_DH::scvGetMinExposureParm(double& exposure)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
		return SMCV_RETN_NOINIT;
	//----------------
	int	ret = IMV_GetDoubleFeatureMin(m_devHandle, "ExposureTime", &exposure);   
	if (IMV_OK != ret) 
	{
		printf("Get feature minimum value failed! ErrorCode[%d]\n", ret);  
		return ret; 
	}
	//MVCC_INTVALUE struIntValue = { 0 };
	//int nRet = MV_CC_GetAutoExposureTimeLower(m_hCamera, &struIntValue);
	//if (MV_OK != nRet)
	//	return SMCV_RETN_FAILIURE;
	//exposure = struIntValue.nCurValue;
	return myRetn;
}

long Camera_DH::scvSetExposureParm(long val)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "ExposureTime", val))
	{
		printf("Set PixelFormat value failed!");
		//AfxMessageBox(_T("Set PixelFormat value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	//int nRet = MV_CC_SetExposureTime(m_hCamera, val);//val单位为10us。
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	return myRetn;
}

long Camera_DH::scvGetGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//----------------
	//MVCC_FLOATVALUE struFloatValue = { 0 };
	//int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//gain = struFloatValue.fCurValue;
	if (IMV_OK != IMV_GetDoubleFeatureValue(m_devHandle, "GainRaw", &gain))
	{
		printf("Set scvGetExposureParm value failed!");
		//AfxMessageBox(_T("Set AcquisitionFrameRate value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	return myRetn;
}

long Camera_DH::scvGetMaxGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//MVCC_FLOATVALUE struFloatValue = { 0 };
	//int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//gain = 100;
	//gain = struFloatValue.fMax;

	int	ret = IMV_GetDoubleFeatureMax(m_devHandle, "GainRaw", &gain);
	if (IMV_OK != ret)
	{
		printf("Get feature Max value failed! ErrorCode[%d]\n", ret);
		return ret;
	}
	return myRetn;
}

long Camera_DH::scvGetMinGainParm(double& gain)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//MVCC_FLOATVALUE struFloatValue = { 0 };
	//int nRet = MV_CC_GetGain(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	////gain = 0;
	//gain = struFloatValue.fMin;
	int	ret = IMV_GetDoubleFeatureMin(m_devHandle, "GainRaw", &gain);
	if (IMV_OK != ret)
	{
		printf("Get feature Max value failed! ErrorCode[%d]\n", ret);
		return ret;
	}
	return myRetn;
}

long Camera_DH::scvSetGain(long val)
{
	long myRetn = SMCV_RETN_SUCCESS;

	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	float fValue = val;
	//int nRet = MV_CC_SetGain(m_hCamera, fValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	if (IMV_OK != IMV_SetDoubleFeatureValue(m_devHandle, "GainRaw", fValue))
	{
		printf("Set PixelFormat value failed!");
		//AfxMessageBox(_T("Set PixelFormat value failed!"));
		return SMCV_RETN_FAILIURE;
	}
	//m_Gain = fValue;
	return myRetn;
}

long Camera_DH::scvSetTriggerDelay(double Delay)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//MVCC_FLOATVALUE struFloatValue = { 0 };

	//int nRet = MV_CC_GetTriggerDelay(m_hCamera, &struFloatValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//Delay = 100;
	//Delay = struFloatValue.fMax;

	return myRetn;
}

long Camera_DH::scvGetTriggerDelay(double& Delay)
{
	long myRetn = SMCV_RETN_SUCCESS;
	//----------------
	if (!IsCamConnect())
	{
		return SMCV_RETN_NOINIT;
	}
	//float fValue = Delay;
	//int nRet = MV_CC_SetTriggerDelay(m_hCamera, fValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//m_Gain = fValue;
	return myRetn;
}

void Camera_DH::frameReady(ImageData Data)
{
	if (Data.pDataBuffer != NULL)
	{
		SetConnectedFlag(true);
		SendVisionData(m_strCameraName,(unsigned char*)Data.pDataBuffer,NULL, Data.nImgWidth, Data.nImgHeight, Data.nBitCount,1, ImgType_Byte, Data.nImgHeight);
	}
	else
	{
		SetConnectedFlag(false);
	}
}

void Camera_DH::deviceLinkNotifyProc(IMV_SConnectArg connectArg)
{
	if (!m_devHandle)
	{
		printf("m_devHandle is NULL\n");
		return;
	}

	if (connectArg.event == offLine)
	{
		// 关闭相机
		// Close camera 
		if (IMV_OK != IMV_Close(m_devHandle))
		{
			printf("Close camera failed!\n");
			return;
		}

		while (IMV_OK != IMV_Open(m_devHandle))
		{
			Sleep(500);
		}

		int	ret = IMV_AttachGrabbing(m_devHandle, onGetFrame, NULL);
		if (IMV_OK != ret)
		{
			printf("Attach grabbing failed! ErrorCode[%d]\n", ret);
			return;
		}
		// 重新设备连接状态事件回调函数
		// Device connection status event callback function again
		if (IMV_OK != IMV_SubscribeConnectArg(m_devHandle, OnConnect, this))
		{
			printf("Subscribe connect Failed!\n");
			return;
		}
		m_bResumeGrabbing = IMV_IsGrabbing(m_devHandle);
		// 断线前不在拉流状态 
		// Not grabbing status before disConnect
		if (m_bResumeGrabbing)
		{
			// 开始拉流 
			// Start grabbing 
			//StartStreamGrabbing(true);
		}
		else {
			StartVedioLive();
		}
		//StopStreamGrabbing(true);

	}
	else if (onLine == connectArg.event)
	{
		m_bResumeGrabbing = IMV_IsGrabbing(m_devHandle);
		// 断线前不在拉流状态 
		// Not grabbing status before disConnect
		if (!m_bResumeGrabbing)
		{
			return;
		}

	}

}

void Camera_DH::LostDev()
{
	SetConnectedFlag(false);
}

int Camera_DH::SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	int _iRetn = 0;
	m_lockMutex.lock();
	_iRetn = m_ptrCamera->RecieveBuffer(strCamName,gray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
	m_lockMutex.unlock();
	return _iRetn;
}

int Camera_DH::SendRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	SetEvent(m_hTrrigerHand);
	int _iRetn = 0;
	m_lockMutex.lock();
	_iRetn = m_ptrCamera->RecieveRGBBuffer(strCamName,Rgray, Ggray, Bgray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
	m_lockMutex.unlock();
	return _iRetn;
}

int Camera_DH::IsCamCapture()
{
	return true;
}

int Camera_DH::IsCamConnect()
{
	return m_vlIsConnected;
}

long Camera_DH::scvSetTriggerMode(EnumCameraTriggerMode trigger)
{
	long myRetn = SMCV_RETN_SUCCESS;

	if (!IsCamConnect())
		return SMCV_RETN_NOINIT;

	int TriggerSource = 0;
	int nRet = 0;
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
		//nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerMode", 1);
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;

		// 设置触发源为软触发 
		// Set trigger source to Software 
		nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Software");
		if (IMV_OK != nRet)
		{
			printf("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		// 设置触发器 
		// Set trigger selector to FrameStart 
		nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
		if (IMV_OK != nRet)
		{
			printf("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		// Set trigger mode to On 
		nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "On");
		if (IMV_OK != nRet)
		{
			printf("Set triggerMode value failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		//nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerSource", TriggerSource);//设置触发源
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;

		////设置闪光灯输出
		//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line1
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;

		//nRet = MV_CC_SetBoolValue(m_hCamera, "StrobeEnable", 0);//关闭闪光灯输出
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;
	}
	else if (TriggerSource == 9)
	{
		//nRet = MV_CC_SetTriggerMode(m_hCamera, 0);//1表示打开触发模式，0表示关闭触发模式
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;
		// Set trigger mode to On 
		nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "Off");
		if (IMV_OK != nRet)
		{
			printf("Set triggerMode value failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}
	}
	else		//非软触发触发方式
	{
		// Set trigger mode to On 
		nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerMode", "On");
		if (IMV_OK != nRet)
		{
			printf("Set triggerMode value failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}
		//nRet = MV_CC_SetTriggerMode(m_hCamera, 1);//1表示打开触发模式，0表示关闭触发模式
		//if (MV_OK != nRet)
		//	return SMCV_RETN_FAILIURE;

		//设置触发线
		if (TriggerSource == 0)//触发源1
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Line1");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			// 设置触发器 
			// Set trigger selector to FrameStart 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 0);//选中Line2
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
			///* 0:Input
			//1 : Output
			//2 : Trigger
			//8 : Strobe*/
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
		}
		else if (TriggerSource == 2)//触发源2
		{
			// Set trigger source to Line1 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Line2");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			// 设置触发器 
			// Set trigger selector to FrameStart 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line2
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
			///* 0:Input
			//1 : Output
			//2 : Trigger
			//8 : Strobe*/
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
		}
		else if (TriggerSource == 3)//触发源3
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			// 设置触发器 
			// Set trigger source to Line1 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Line3");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			// Set trigger selector to FrameStart 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 2);//选中Line2
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
		}
		else  if (TriggerSource == 4)//触发源4
		{
			/* 0:Line0
			1 : Line1
			2 : Line2
			3 : Line3
			4 : Line4*/
			// 设置触发器 
			// Set trigger source to Line1 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSource", "Line4");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			// Set trigger selector to FrameStart 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerSelector", "FrameStart");
			if (IMV_OK != nRet)
			{
				printf("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 3);//选中Line2
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
			/* 0:Input
			1 : Output
			2 : Trigger
			8 : Strobe*/
			//nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 0);//设置Line2为输入
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetIntValue(m_hCamera, "LineDebouncerTime", 10);//触发防抖10us
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;

			//nRet = MV_CC_SetTriggerSource(m_hCamera, TriggerSource);//设置触发源
			//if (MV_OK != nRet)
			//	return SMCV_RETN_FAILIURE;
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
			//nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerActivation", 1);//设置上升沿触发
			//if (MV_OK != nRet)
			//{
			//	return SMCV_RETN_FAILIURE;
			//}
			// 设置外触发为上升沿（下降沿为FallingEdge） 
			// Set trigger activation to RisingEdge(FallingEdge in opposite) 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerActivation", "RisingEdge");
			if (IMV_OK != nRet)
			{
				printf("Set triggerActivation value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}
		}
		else if (CameraMode_TriggerLine1_FallingEdge == trigger
			|| CameraMode_TriggerLine2_FallingEdge == trigger
			|| CameraMode_TriggerLine4_FallingEdge == trigger
			|| CameraMode_TriggerLine3_FallingEdge == trigger)
		{
			// Set trigger activation to RisingEdge(FallingEdge in opposite) 
			nRet = IMV_SetEnumFeatureSymbol(m_devHandle, "TriggerActivation", "RisingEdge");
			if (IMV_OK != nRet)
			{
				printf("Set triggerActivation value failed! ErrorCode[%d]\n", nRet);
				return nRet;
			}

			//nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerActivation", 0);//设置下降沿触发
			//if (MV_OK != nRet)
			//{
			//	return SMCV_RETN_FAILIURE;
			//}
		}
		////设置闪光灯输出
		//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSelector", 1);//选中Line1
		//if (MV_OK != nRet)
		//{
		//	return SMCV_RETN_FAILIURE;
		//}
		//nRet = MV_CC_SetEnumValue(m_hCamera, "LineMode", 8);//设置Line1为闪光灯
		//if (MV_OK != nRet)
		//{
		//	return SMCV_RETN_FAILIURE;
		//}
		//nRet = MV_CC_SetEnumValue(m_hCamera, "LineSource", 0);
		//if (MV_OK != nRet)
		//{
		//	return SMCV_RETN_FAILIURE;
		//}
		//nRet = MV_CC_SetBoolValue(m_hCamera, "StrobeEnable", 1);//打开闪光灯输出
		//if (MV_OK != nRet)
		//{
		//	return SMCV_RETN_FAILIURE;
		//}
	}
	m_eTrrigerMode = trigger;
	return myRetn;
}

long Camera_DH::scvGetTriggerMode(EnumCameraTriggerMode& trigger)
{
	long myRetn = SMCV_RETN_SUCCESS;
	if (!scvIsConnected())
		return SMCV_RETN_NOINIT;

	//MVCC_ENUMVALUE struEnumValue = { 0 };
	//int nRet = MV_CC_GetTriggerMode(m_hCamera, &struEnumValue);
	//if (MV_OK != nRet)
	//{
	//	return SMCV_RETN_FAILIURE;
	//}
	//unsigned int nEntryNum = 0;
	//int	ret = IMV_GetEnumFeatureEntryNum(m_devHandle, "TriggerMode", &nEntryNum);
	//if (IMV_OK != ret)
	//{
	//	printf("Get TriggerMode settable enumeration number failed!\n");
	//	//AfxMessageBox(_T("Get TriggerMode settable enumeration number failed!"));
	//	return;
	//}
	//IMV_EnumEntryList enumEntryList;
	//enumEntryList.nEnumEntryBufferSize = sizeof(IMV_EnumEntryInfo) * nEntryNum;
	//enumEntryList.pEnumEntryInfo = (IMV_EnumEntryInfo*)malloc(sizeof(IMV_EnumEntryInfo) * nEntryNum);
	//if (NULL == enumEntryList.pEnumEntryInfo)
	//{
	//	printf("Malloc pEnumEntryInfo failed!\n");
	//	//AfxMessageBox(_T("Malloc pEnumEntryInfo failed!"));
	//	return;
	//}

	//if (IMV_OK != IMV_GetEnumFeatureEntrys(m_devHandle, "TriggerMode", &enumEntryList))
	//{
	//	free(enumEntryList.pEnumEntryInfo);
	//	enumEntryList.pEnumEntryInfo = NULL;
	//	printf("Get TriggerMode settable enumeration value list failed!\n");
	//	//AfxMessageBox(_T("Get TriggerMode settable enumeration value list failed!"));
	//	return;
	//}

	IMV_String triggerModeSymbol;
	int ret = IMV_GetEnumFeatureSymbol(m_devHandle, "TriggerMode", &triggerModeSymbol);
	if (IMV_OK != ret)
	{
		printf("Get TriggerMode symbol value failed!\n");
		//AfxMessageBox(_T("Get TriggerMode symbol value failed!"));
		return -1;
	}
	if (QString(triggerModeSymbol.str) == "On")
	{

	}
	
	//m_triggerMode = CString(_T(triggerModeSymbol.str));
	//bool bIstrigger = (struEnumValue.nCurValue == 1 ? true : false);
	//if (bIstrigger)
	//	trigger = m_eTrrigerMode;
	//else
	//	trigger = SMCV_TRIGGER_MODE_RUNING;
	return myRetn;
}

int Camera_DH::ClearMemory()
{
	return 0;
}

long Camera_DH::scvIsConnected(bool& isConnect)
{
	return (isConnect = m_vlIsConnected);
}

bool Camera_DH::scvIsConnected()
{
	return m_vlIsConnected;
}

long Camera_DH::SetConnectedFlag(bool isConnect)
{
	return _InterlockedExchange(&m_vlIsConnected, isConnect);
}
