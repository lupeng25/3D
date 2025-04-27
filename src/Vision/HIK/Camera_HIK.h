#pragma once
#include <Windows.h>
//#include "LJX8_IF.h"
#include "CameraInterface.h"
#include <Qobject>
#include <mutex>
#include "SDK\Include\MvCameraControl.h"

typedef struct _ImageData
{
	void*    pDataBuffer;           //当前数据缓冲区的地址
	int      nImgWidth;				//每帧数据图像的宽度
	int      nImgHeight;			//每帧数据图像的高度
	int      nBitCount;				//每个像素所占的字节数
	int      nDevId;				//当前设备索引号       //***********
}ImageData, *pImageData;

class Camera_HIK :public Camerainterface,public QObject
{
public:
	Camera_HIK();
	~Camera_HIK();
public:
	virtual QString getName();

	virtual	void setName(const QString& strName);

	virtual QString getCameraType();

	static QString getDescription();
public:		//动态库
	static int Initialize();

	static int Finalize();
	static bool m_bFinalize;

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
	//设备是否连接
	long scvIsConnected(bool& isConnect);
	bool scvIsConnected();
	long SetConnectedFlag(bool isConnect);

public:
	virtual int SetStream(CameraStream* stream);

private:
	/// \brief getHeightUpperLower 高度区间显示上下限获取
	/// \param _upper              上限
	/// \param _lower              下限
	void GetHeightUpperLower(double& _upper, double& _lower);

private:
	long scvSetTriggerMode(EnumCameraTriggerMode trigger);
	long scvGetTriggerMode(EnumCameraTriggerMode& trigger);

	//帧率
	long scvSetFPS(double FPS);
	long scvGetFPS(double& fps);

	//亮度
	long scvSetBrightness(double bright);
	long scvGetBrightness(double& bright);

	//对比度
	long scvSetContrast(double contrast);
	long scvGetContrast(double& contrast);

	//快门
	long scvGetExposureParm(double& exposure);
	long scvGetMaxExposureParm(double& exposure);
	long scvGetMinExposureParm(double& exposure);
	long scvSetExposureParm(long val);

	//增益
	long scvGetGainParm(double& gain);
	long scvGetMaxGainParm(double& gain);
	long scvGetMinGainParm(double& gain);
	long scvSetGain(long val);

	//触发延时
	long scvSetTriggerDelay(double Delay);
	long scvGetTriggerDelay(double& Delay);
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
	//数据回调
	void frameReady(ImageData Data);
	void LostDev();

public:
	virtual	int SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	virtual	int SendRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	//相机是否在采集
	virtual int	IsCamCapture();

	//相机是否连接
	virtual	int	IsCamConnect();
public:

public:
	volatile long			m_vlIsConnected;

	int						m_nCurrentDealID	= 1;	//配方号
	
	int						m_iTimeOut = 5000;
	EnumCameraTriggerMode	m_eTrrigerMode;

	void*					m_hTrrigerHand;
	CameraStream*			m_ptrCamera;
	QString					m_strCameraName;
	MV_CC_DEVICE_INFO       m_sDeviceInfo;

private:
	void *					m_hCamera;			//相机句柄
	std::mutex				m_lockMutex;
};