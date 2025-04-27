#include "Camera3D_KEYENCE.h"
#include <Windows.h>
#include <QNetworkInterface>
#include <QTcpSocket>

std::map<QString, Camera3D_KEYENCE*>	Camera3D_KEYENCE::m_VecCamera;
Camera3D_KEYENCE::Camera3D_KEYENCE()
{
	m_nCurrentDeviceID = 0;
	ethernetConfig.abyIpAddress[0]	= 192;
	ethernetConfig.abyIpAddress[1]	= 168;
	ethernetConfig.abyIpAddress[2]	= 0;
	ethernetConfig.abyIpAddress[3]	= 1;
	ethernetConfig.wPortNo			= 24691;
	m_bIsConnected					= false;
	m_hTrrigerHand					= ::CreateEventA(NULL, TRUE, FALSE, NULL);

}

Camera3D_KEYENCE::~Camera3D_KEYENCE()
{
	ClearMemory();
	StopVedioLive();
	Stop();
	Exit();
	m_VecCamera.erase(m_strCameraName);
	SetEvent(m_hTrrigerHand);
	if (m_hTrrigerHand != nullptr) 
		CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand = nullptr;
}

QString Camera3D_KEYENCE::getName()
{
	return m_strCameraName;
}

void Camera3D_KEYENCE::setName(const QString & strName)
{
	m_strCameraName = strName;
}

QString Camera3D_KEYENCE::getCameraType()
{
	return "Camera3D_KEYENCE";
}

QVector<CameraInfo> Camera3D_KEYENCE::getVecInfo()
{
	QVector<CameraInfo> _vecInfo;

	QStringList ips;
	QList<QNetworkInterface> networkinterfaces = QNetworkInterface::allInterfaces();
	foreach(QNetworkInterface inter, networkinterfaces)
	{
		QNetworkInterface::InterfaceFlags t_interFlags = inter.flags();
		if (t_interFlags & QNetworkInterface::IsUp &&
			t_interFlags & QNetworkInterface::IsRunning)
		{
			foreach(QNetworkAddressEntry entry, inter.addressEntries())
			{
				if (entry.ip() != QHostAddress::LocalHost
					&& entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
				{
					ips.append(entry.ip().toString());
				}
			}
		}
	}
	QTcpSocket t_socket(0);
	t_socket.abort();
	for (auto iter : ips)
	{
		QString t_ip = iter;
		for (int y = 1; y < 255; y++)
		{
			t_socket.abort();
			t_ip.replace(10, t_ip.size() - 10, QString::number(y));
			if (t_ip == iter)	{	continue;	}

			t_socket.connectToHost(t_ip, 24691);		//端口固定的，后续可以做成选择端口扫描	
			bool bConnected = t_socket.waitForConnected(2);
			if (bConnected)
			{
				CameraInfo _inFo;
				_inFo.strCameraIP		= t_ip;
				_inFo.strCameraName		= t_ip;
				_inFo.strCameraBrand	= "KEYENCE";

				_vecInfo.push_back(_inFo);
			}
		}
	}
	t_socket.disconnected();
	t_socket.close();
	t_socket.abort();

	return _vecInfo;
}

int Camera3D_KEYENCE::Initialize()
{
	LONG lRc = LJX8IF_Initialize();
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

int Camera3D_KEYENCE::Finalize()
{
	LONG lRc = LJX8IF_Finalize();
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

QString Camera3D_KEYENCE::GetVision()
{
	LJX8IF_VERSION_INFO info = LJX8IF_GetVersion();
	return QString("%1.%2.%3.%4")
		.arg(QString::number(info.nMajorNumber))
		.arg(QString::number(info.nMinorNumber))
		.arg(QString::number(info.nRevisionNumber))
		.arg(QString::number(info.nBuildNumber));
}

int Camera3D_KEYENCE::Init()
{
	LONG lRc = LJX8IF_EthernetOpen((LONG)m_nCurrentDeviceID, &ethernetConfig);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	QString value;
	GetCameraParam(CameraRowCount, value);
	m_dwProfileCount	= m_BatchPoint;
	m_dwProfileCount	= max(0, m_dwProfileCount);

	if (m_bIsArrayOnly)
	{
		DWORD _dwHighSpeedPortNo = ethernetConfig.wPortNo + 1;
		lRc = LJX8IF_InitializeHighSpeedDataCommunicationSimpleArray((LONG)m_nCurrentDeviceID, &ethernetConfig,
			(WORD)_dwHighSpeedPortNo, (m_bIsCountOnly ? CountSimpleArrayReceive : ReceiveHighSpeedSimpleArray),
			m_dwProfileCount, m_nCurrentDeviceID);
	}
	else
	{
		DWORD _dwHighSpeedPortNo = ethernetConfig.wPortNo + 1;
		lRc = LJX8IF_InitializeHighSpeedDataCommunication((LONG)m_nCurrentDeviceID, &ethernetConfig,
			(WORD)_dwHighSpeedPortNo,
			(m_bIsCountOnly ? CountProfileReceive : ReceiveHighSpeedData),
			m_dwProfileCount, m_nCurrentDeviceID);
	}

	if (lRc != LJX8IF_RC_OK)
		return lRc;
	LJX8IF_HIGH_SPEED_PRE_START_REQ request;	//发送相关的开始点
	request.bySendPosition			= 2;
	LJX8IF_PROFILE_INFO				profileInfo;
	lRc = LJX8IF_PreStartHighSpeedDataCommunication((LONG)m_nCurrentDeviceID, &request, &profileInfo);

	if (lRc != LJX8IF_RC_OK)
		return lRc;

	m_bIsConnected = true;
	return 0;
}

int Camera3D_KEYENCE::Exit()
{
	LONG lRc = LJX8IF_CommunicationClose((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	m_bIsConnected = false;
	return 0;
}

int Camera3D_KEYENCE::SetStream(CameraStream * stream)
{
	m_ptrCamera = stream;
	return 0;
}

int Camera3D_KEYENCE::Start()
{
	LONG lRc = LJX8IF_StartHighSpeedDataCommunication((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

int Camera3D_KEYENCE::Stop()
{
	LONG lRc = LJX8IF_StopHighSpeedDataCommunication((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	//结束
	lRc = LJX8IF_FinalizeHighSpeedDataCommunication((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

int Camera3D_KEYENCE::StartVedioLive()
{

	return 0;
}

int Camera3D_KEYENCE::StopVedioLive()
{
	LONG lRc = LJX8IF_StopMeasure((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

int Camera3D_KEYENCE::Trigger()
{
	ResetEvent(m_hTrrigerHand);
	LONG lRc = LJX8IF_StartMeasure((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	//LONG lRc = LJX8IF_Trigger((LONG)m_nCurrentDeviceID);
	//if (lRc != LJX8IF_RC_OK)
	//	return lRc;
	return 0;
}

int Camera3D_KEYENCE::WaitTrigger(int iTimeOut)
{
	DWORD dwRetn = WaitForSingleObject(m_hTrrigerHand, iTimeOut);
	if (dwRetn == WAIT_TIMEOUT)
		dwRetn = -1;
	else if (dwRetn >= WAIT_OBJECT_0)
		dwRetn = dwRetn - WAIT_OBJECT_0;
	return dwRetn;
}

int Camera3D_KEYENCE::SetCameraParam(QString key, QString value)
{
	LONG lRc = LJX8IF_RC_OK;
	if (key == CameraMode)
	{

	}
	else if (key == CameraIPAdress || CameraName == key)		//相机名称
	{
		QStringList params = value.split(".");
		if (params.size() > 0)	ethernetConfig.abyIpAddress[0] = (char)params[0].toInt();
		if (params.size() > 1)	ethernetConfig.abyIpAddress[1] = (char)params[1].toInt();
		if (params.size() > 2)	ethernetConfig.abyIpAddress[2] = (char)params[2].toInt();
		if (params.size() > 3)	ethernetConfig.abyIpAddress[3] = (char)params[3].toInt();
	}
	else if (key == CameraIPPort)
	{
		ethernetConfig.wPortNo = value.toInt();
	}
	else if (key == CameraDealID)
	{
		if (m_nCurrentDealID == value.toInt())
		{
			return LJX8IF_RC_OK;
		}
		m_nCurrentDealID = value.toInt();
		return	LJX8IF_ChangeActiveProgram(m_nCurrentDeviceID, m_nCurrentDealID);
	}
	else if (key == CameraRowCount)
	{
		if (m_BatchPoint == value.toInt())
		{
			m_BatchPoint		= value.toInt();
			m_dwProfileCount	= m_BatchPoint;
			return LJX8IF_RC_OK;
		}
		m_BatchPoint = value.toInt();
		m_dwProfileCount = m_BatchPoint;
		LJX8IF_TARGET_SETTING	lpset;
		lpset.byCategory = 0;
		lpset.byItem = 10;
		lpset.byType = (BYTE)(16 + m_nCurrentDealID);
		lpset.byTarget1 = 0;
		lpset.byTarget2 = 0;
		lpset.byTarget3 = 0;
		lpset.byTarget4 = 0;
		int	* _pNewArray = new int[1];
		memset(_pNewArray, 0, sizeof(int) * 1);
		DWORD dwError;
		_pNewArray[0] = m_dwProfileCount;
		lRc = LJX8IF_SetSetting(m_nCurrentDeviceID, 1, lpset, _pNewArray, 4,&dwError);
		delete[]_pNewArray;
	}
	else if (key == CameraXInterVal)
	{
		if (m_XinterVal == value.toInt())
		{
			return LJX8IF_RC_OK;
		}
		m_XinterVal = value.toInt();
		return	LJX8IF_SetXpitch(m_nCurrentDeviceID, m_XinterVal);
	}
	return lRc;
}

int Camera3D_KEYENCE::GetCameraParam(QString key, QString & value)
{
	LONG lRc = LJX8IF_RC_OK;
	if (key == CameraMode)
	{

	}
	else if (key == CameraIPAdress)
	{
		value = QString("%1.%2.%3.%4")
			.arg(QString::number(ethernetConfig.abyIpAddress[0]))
			.arg(QString::number(ethernetConfig.abyIpAddress[1]))
			.arg(QString::number(ethernetConfig.abyIpAddress[2]))
			.arg(QString::number(ethernetConfig.abyIpAddress[3]));
	}
	else if (key == CameraIPPort)
	{
		value = QString::number(ethernetConfig.wPortNo);
	}
	else if (key == CameraSerial)
	{
		const int SERIAL_NUMBER_DATA_LENGTH = 16;
		CHAR szControllerSerialNumber[SERIAL_NUMBER_DATA_LENGTH];
		CHAR szHeadSerialNumber[SERIAL_NUMBER_DATA_LENGTH];

		lRc = LJX8IF_GetSerialNumber((LONG)m_nCurrentDeviceID, &szControllerSerialNumber[0], &szHeadSerialNumber[0]);
		value = QString("Controller:%1 Head:%2").append(szControllerSerialNumber[0]).append(szHeadSerialNumber[0]);
	}
	else if (key == CameraRowCount)
	{
		LJX8IF_TARGET_SETTING	lpset;
		lpset.byCategory		= 0;
		lpset.byItem			= 10;
		lpset.byType			= (BYTE)(16 + m_nCurrentDealID);
		lpset.byTarget1			= 0;
		lpset.byTarget2			= 0;
		lpset.byTarget3			= 0;
		lpset.byTarget4			= 0;
		int	* _pNewArray		= new int[1];
		memset(_pNewArray,0,sizeof(int) * 1);

		lRc = LJX8IF_GetSetting(m_nCurrentDeviceID,1, lpset, _pNewArray,4);
		m_BatchPoint = _pNewArray[0];
		delete[]_pNewArray;
		m_dwProfileCount = m_BatchPoint;
		value = QString::number(m_BatchPoint);
	}
	else if (key == CameraDealID)
	{
		BYTE programNo = 0;
		lRc = LJX8IF_GetActiveProgram((LONG)m_nCurrentDeviceID, &programNo);
		m_nCurrentDealID = programNo;
		value = QString::number(m_nCurrentDealID);
	}
	else if (key == CameraDeviceId)
	{
		value = QString::number(m_nCurrentDeviceID);
	}
	else if (key == CameraModels)
	{
		const int SERIAL_NUMBER_DATA_LENGTH = 1024;
		CHAR szControllerSerialNumber[SERIAL_NUMBER_DATA_LENGTH];
		lRc = LJX8IF_GetHeadModel((LONG)m_nCurrentDeviceID, szControllerSerialNumber);
		value = QString("%1").append(szControllerSerialNumber[0]);
	}
	else if (key == CameraXInterVal)
	{
		DWORD dwXpitch = 0;
		lRc = LJX8IF_GetXpitch(m_nCurrentDeviceID, &dwXpitch);
		m_XinterVal = dwXpitch;
		value = QString::number(m_XinterVal);
	}
	return lRc;
}

int Camera3D_KEYENCE::GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue)
{
	mapValue.clear();
	QString strValue;
	long lRc = GetCameraParam(CameraRowCount, strValue);
	if (lRc != LJX8IF_RC_OK)	return lRc;

	lRc = GetCameraParam(CameraRowCount, strValue);
	if (lRc != LJX8IF_RC_OK)	return lRc;

	lRc = GetCameraParam(CameraXInterVal, strValue);
	if (lRc != LJX8IF_RC_OK)	return lRc;

	mapValue.push_back(QPair<QString, QString>(CameraDeviceId,		
		ParamToValue(tr(CameraDeviceId.toStdString().c_str()), 0,		ParamType_String,	QString::number(m_nCurrentDeviceID))));
	mapValue.push_back(QPair<QString, QString>(CameraDealID,		
		ParamToValue(tr(CameraDealID.toStdString().c_str()), 1,		ParamType_Int,		QString::number(m_nCurrentDealID))));
	mapValue.push_back(QPair<QString, QString>(CameraRowCount,		
		ParamToValue(tr(CameraRowCount.toStdString().c_str()), 1,		ParamType_Int,		QString::number(m_BatchPoint))));
	mapValue.push_back(QPair<QString, QString>(CameraIPAdress,		
		ParamToValue(tr(CameraIPAdress.toStdString().c_str()), 0,		ParamType_String,	QString("%1.%2.%3.%4")
		.arg(QString::number(ethernetConfig.abyIpAddress[0]))
		.arg(QString::number(ethernetConfig.abyIpAddress[1]))
		.arg(QString::number(ethernetConfig.abyIpAddress[2]))
		.arg(QString::number(ethernetConfig.abyIpAddress[3])))));
	mapValue.push_back(QPair<QString, QString>(CameraIPPort,	
		ParamToValue(tr(CameraIPPort.toStdString().c_str()), 0,	ParamType_Int,		QString::number(ethernetConfig.wPortNo))));
	mapValue.push_back(QPair<QString, QString>(CameraXInterVal,
		ParamToValue(tr(CameraXInterVal.toStdString().c_str()), 1,	ParamType_Double,	QString::number(m_XinterVal))));

	return 0;
}

int Camera3D_KEYENCE::RecieveBuffer(QString strCamera, unsigned char * gray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	return m_ptrCamera->RecieveBuffer(strCamera,gray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
}

int Camera3D_KEYENCE::RecieveRGBBuffer(QString strCamera, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	return m_ptrCamera->RecieveRGBBuffer(strCamera,Rgray, Ggray, Bgray, IntensityData, iwidth, iheight, ibit, fscale, imgtype, icount);
}

int Camera3D_KEYENCE::IsCamCapture()
{
	return 0;
}

int Camera3D_KEYENCE::IsCamConnect()
{
	return m_bIsConnected;
}

int Camera3D_KEYENCE::ClearMemory()
{
	LONG lRc = LJX8IF_ClearMemory((LONG)m_nCurrentDeviceID);
	if (lRc != LJX8IF_RC_OK)
		return lRc;
	return 0;
}

void Camera3D_KEYENCE::ReceiveHighSpeedData(BYTE * pBuffer, DWORD dwSize, DWORD dwCount, DWORD dwNotify, DWORD dwUser)
{
	if ((dwNotify != 0) || (dwNotify & 0x10000) != 0) return;
	if (dwCount == 0) return;

	if (m_VecCamera.size() > dwUser)
	{
		//m_VecCamera[dwUser]->ReceiveHighSpeedData();
	}

}

void Camera3D_KEYENCE::CountProfileReceive(BYTE * pBuffer, DWORD dwSize, DWORD dwCount, DWORD dwNotify, DWORD dwUser)
{
	if ((dwNotify != 0) || (dwNotify & 0x10000) != 0)	return;
	if (dwCount == 0)	return;
	if (m_VecCamera.size() > dwUser)
	{
		//m_VecCamera[dwUser]->ReceiveHighSpeedData();
	}
}

void Camera3D_KEYENCE::ReceiveHighSpeedSimpleArray(LJX8IF_PROFILE_HEADER * pProfileHeaderArray, WORD * pHeightProfileArray, WORD * pLuminanceProfileArray, DWORD dwLuminanceEnable,
	DWORD dwProfileDataCount, DWORD dwCount, DWORD dwNotify, DWORD dwUser)
{
	if ((dwNotify != 0) || (dwNotify & 0x10000) != 0)	return;
	if (dwCount == 0)	return;

	Camera3D_KEYENCE*_PtrCam = nullptr;
	for (auto iter : m_VecCamera)	{	if (iter.second->m_nCurrentDeviceID == dwUser)	{	_PtrCam = iter.second;	}	}
	if (_PtrCam != nullptr)
	{
		_PtrCam->RecieveBuffer(_PtrCam->m_strCameraName,(unsigned char*)pLuminanceProfileArray,(int*)pHeightProfileArray, dwProfileDataCount, dwCount/* + 1*/,8,1, ImgType_uInt2,dwCount/* + 1*/);
	}
}

void Camera3D_KEYENCE::CountSimpleArrayReceive(LJX8IF_PROFILE_HEADER * pProfileHeaderArray, WORD * pHeightProfileArray, WORD * pLuminanceProfileArray, DWORD dwLuminanceEnable, 
	DWORD dwProfileDataCount, DWORD dwCount, DWORD dwNotify, DWORD dwUser)
{
	if ((dwNotify != 0) || (dwNotify & 0x10000) != 0) return;
	if (dwCount == 0) return;

	Camera3D_KEYENCE*_PtrCam = nullptr;
	for (auto iter : m_VecCamera) { if (iter.second->m_nCurrentDeviceID == dwUser) { _PtrCam = iter.second; } }
	if (_PtrCam != nullptr)
	{
		_PtrCam->RecieveBuffer(_PtrCam->m_strCameraName,(unsigned char*)pLuminanceProfileArray, (int*)pHeightProfileArray, dwProfileDataCount, dwCount/* + 1*/, 8, 1, ImgType_uInt2, dwCount/* + 1*/);
	}
}