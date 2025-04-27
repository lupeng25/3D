#pragma once
/**********************
2024 0312
制作人	:			米承松
功能	:			使用基恩士相机进行取图然后进行设置相关参数
能设置的参数有以下功能:
CameraIPAdress		Ip地址
CameraIPPort		端口号
CameraDealID		配方号
CameraRowCount		行长度
CameraXInterVal		X行间距 (0.02)

获取参数有以下功能:
CameraIPAdress		Ip地址
CameraIPPort		端口号
CameraSerial		序列号
CameraRowCount		行长度
CameraDealID		配方号
CameraDeviceId		装置ID
CameraModels		模型名
CameraXInterVal		X间距
***********************/

#include <Windows.h>
#include "LJX8_IF.h"
#include "LJX8_ErrorCode.h"
#include "CameraInterface.h"
#include <time.h>
#include <QPair>

class Camera3D_KEYENCE :public Camerainterface, public QObject
{
public:
	Camera3D_KEYENCE();
	~Camera3D_KEYENCE();
public:
	virtual QString getName();

	virtual	void setName(const QString& strName);

	virtual QString getCameraType();

public:		//动态库
	static int Initialize();

	static int Finalize();

	static QString GetVision();

	static QVector<CameraInfo> getVecInfo();
public:
	//初始化
	int Init();

	//退出
	int Exit();

public:
	//开始
	virtual int Start();

	//停止
	virtual int Stop();
public:
	//开始采集
	virtual int	StartVedioLive();

	//停止采集
	virtual int	StopVedioLive();

	//清除内存
	virtual int	ClearMemory();

public:
	virtual int SetStream(CameraStream* stream);

public:
	//触发
	virtual int Trigger();

	//等待触发
	virtual int WaitTrigger(int iTimeOut = 500);
public:
	//设置参数
	virtual int	SetCameraParam(QString key, QString value);

	//获取参数
	virtual int	GetCameraParam(QString key, QString&value);

	//获取所有参数
	virtual int	GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue);

public:
	virtual int RecieveBuffer(QString strCamera, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	//当使用当前接口时8为使用Rgray 使用 其他都应该Ggray Bgray为nullptr 
	virtual int RecieveRGBBuffer(QString strCamera, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);
public:
	//相机是否在采集
	virtual int	IsCamCapture();

	//相机是否连接
	virtual	int	IsCamConnect();
public:
	static void ReceiveHighSpeedData(BYTE* pBuffer, DWORD dwSize, DWORD dwCount, DWORD dwNotify, DWORD dwUser);
	static void CountProfileReceive(BYTE* pBuffer, DWORD dwSize, DWORD dwCount, DWORD dwNotify, DWORD dwUser);
	static void ReceiveHighSpeedSimpleArray(LJX8IF_PROFILE_HEADER* pProfileHeaderArray, WORD* pHeightProfileArray, WORD* pLuminanceProfileArray, DWORD dwLuminanceEnable, DWORD dwProfileDataCount, DWORD dwCount, DWORD dwNotify, DWORD dwUser);
	static void CountSimpleArrayReceive(LJX8IF_PROFILE_HEADER* pProfileHeaderArray, WORD* pHeightProfileArray, WORD* pLuminanceProfileArray, DWORD dwLuminanceEnable, DWORD dwProfileDataCount, DWORD dwCount, DWORD dwNotify, DWORD dwUser);
public:
	bool					m_bIsConnected;
	LJX8IF_ETHERNET_CONFIG	ethernetConfig;

	bool					m_bIsArrayOnly = true;
	bool					m_bIsCountOnly = false; 
	DWORD					m_dwProfileCount = 1;	//一次性接受多少数据

	void*					m_hTrrigerHand;
	CameraStream*			m_ptrCamera;

	int						m_nCurrentDeviceID;
	int						m_nCurrentDealID = 1;	//配方号
	int						m_BatchPoint;			//批处理行数设置
	int						m_BatchWidth;			//轮廓宽度
	double					m_XinterVal;            //X间距
	double					m_YinterVal;            //Y间距
	QString					m_strCameraName;
	static std::map<QString,Camera3D_KEYENCE*>	m_VecCamera;

};