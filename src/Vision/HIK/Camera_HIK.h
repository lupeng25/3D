#pragma once
#include <Windows.h>
//#include "LJX8_IF.h"
#include "CameraInterface.h"
#include <Qobject>
#include <mutex>
#include "SDK\Include\MvCameraControl.h"

typedef struct _ImageData
{
	void*    pDataBuffer;           //��ǰ���ݻ������ĵ�ַ
	int      nImgWidth;				//ÿ֡����ͼ��Ŀ��
	int      nImgHeight;			//ÿ֡����ͼ��ĸ߶�
	int      nBitCount;				//ÿ��������ռ���ֽ���
	int      nDevId;				//��ǰ�豸������       //***********
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
public:		//��̬��
	static int Initialize();

	static int Finalize();
	static bool m_bFinalize;

	static QVector<CameraInfo> getVecInfo();
public:
	//��ʼ��
	int Init();

	//�˳�
	int Exit();

public:
	//��ʼ
	virtual int Start();

	//ֹͣ
	virtual int Stop();
public:
	//��ʼ�ɼ�
	virtual int	StartVedioLive();

	//ֹͣ�ɼ�
	virtual int	StopVedioLive();

	//����ڴ�
	virtual int	ClearMemory();

public:
	//�豸�Ƿ�����
	long scvIsConnected(bool& isConnect);
	bool scvIsConnected();
	long SetConnectedFlag(bool isConnect);

public:
	virtual int SetStream(CameraStream* stream);

private:
	/// \brief getHeightUpperLower �߶�������ʾ�����޻�ȡ
	/// \param _upper              ����
	/// \param _lower              ����
	void GetHeightUpperLower(double& _upper, double& _lower);

private:
	long scvSetTriggerMode(EnumCameraTriggerMode trigger);
	long scvGetTriggerMode(EnumCameraTriggerMode& trigger);

	//֡��
	long scvSetFPS(double FPS);
	long scvGetFPS(double& fps);

	//����
	long scvSetBrightness(double bright);
	long scvGetBrightness(double& bright);

	//�Աȶ�
	long scvSetContrast(double contrast);
	long scvGetContrast(double& contrast);

	//����
	long scvGetExposureParm(double& exposure);
	long scvGetMaxExposureParm(double& exposure);
	long scvGetMinExposureParm(double& exposure);
	long scvSetExposureParm(long val);

	//����
	long scvGetGainParm(double& gain);
	long scvGetMaxGainParm(double& gain);
	long scvGetMinGainParm(double& gain);
	long scvSetGain(long val);

	//������ʱ
	long scvSetTriggerDelay(double Delay);
	long scvGetTriggerDelay(double& Delay);
public:
	//����
	virtual int Trigger();

	//�ȴ�����
	virtual int WaitTrigger(int iTimeOut = 500);
public:
	//���ò���
	virtual int	SetCameraParam(QString key, QString value);

	//��ȡ����
	virtual int	GetCameraParam(QString key, QString&value);

	//��ȡ���в���
	virtual int	GetAllCameraParam(QVector<QPair<QString, QString>>&mapValue);

public:
	//���ݻص�
	void frameReady(ImageData Data);
	void LostDev();

public:
	virtual	int SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	virtual	int SendRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	//����Ƿ��ڲɼ�
	virtual int	IsCamCapture();

	//����Ƿ�����
	virtual	int	IsCamConnect();
public:

public:
	volatile long			m_vlIsConnected;

	int						m_nCurrentDealID	= 1;	//�䷽��
	
	int						m_iTimeOut = 5000;
	EnumCameraTriggerMode	m_eTrrigerMode;

	void*					m_hTrrigerHand;
	CameraStream*			m_ptrCamera;
	QString					m_strCameraName;
	MV_CC_DEVICE_INFO       m_sDeviceInfo;

private:
	void *					m_hCamera;			//������
	std::mutex				m_lockMutex;
};