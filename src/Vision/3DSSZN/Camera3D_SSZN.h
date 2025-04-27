#pragma once
#include <Windows.h>
//#include "LJX8_IF.h"
#include "SR7Link.h"
#include "CameraInterface.h"
#include <Qobject>
#include <QMap>

class Camera3D_SSZN :public Camerainterface,public QObject
{
public:
	Camera3D_SSZN();
	~Camera3D_SSZN();
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
	virtual int SetStream(CameraStream* stream);

private:
	/// \brief getHeightUpperLower 高度区间显示上下限获取
	/// \param _upper              上限
	/// \param _lower              下限
	void GetHeightUpperLower(double& _upper, double& _lower);

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
	virtual	int SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	virtual	int SendRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	//相机是否在采集
	virtual int	IsCamCapture();

	//相机是否连接
	virtual	int	IsCamConnect();
public:

	static void CameraTcpConnectFunc(int dwDeviceId, int cmd);
	/// \brief BatchOneTimeCallBack     回调函数
	/// \param info
	/// \param data
	static void BatchOneTimeCallBack(const void *info,	const SR7IF_Data *data);

	/// \brief CallHighSpeedData  高速数据通信的回调函数接口.
	/// \param pBuffer            指向储存概要数据的缓冲区的指针.
	/// \param uSize              每个单元(行)的字节数量.
	/// \param uCount             存储在pBuffer中的内存的单元数量
	/// \param uNotify            中断或批量结束等中断的通知.
	/// \param uUser              用户自定义信息.
	///
	static void CallHighSpeedData(char* pBuffer,
		unsigned int  uSize,
		unsigned int  uCount,
		unsigned int  uNotify,
		unsigned int uUser);
public:
	bool					m_bIsConnected;
	SR7IF_ETHERNET_CONFIG	m_SREthernetConFig;			//sdk dll 连接参数配置
	int						m_nCurrentDeviceID	= 0;
	int						m_nCurrentDealID	= 1;	//配方号
	
	int						m_BatchPoint;				//批处理行数设置
	int						m_BatchWidth;				//轮廓宽度
	double					m_XinterVal;                //X间距
	double					m_YinterVal;                //Y间距
	//bool					m_bIOTrriger;
	int						m_iTimeOut = 5000;
	int						m_iGetCallMode = 1;
	int						m_iFreqValue = 400;

	void*					m_hTrrigerHand;
	CameraStream*			m_ptrCamera;
	QString					m_strCameraName;
private:
	static std::map<QString,Camera3D_SSZN*>	m_mapCamera3D_SSZN;
};