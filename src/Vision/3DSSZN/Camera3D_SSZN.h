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
	virtual int SetStream(CameraStream* stream);

private:
	/// \brief getHeightUpperLower �߶�������ʾ�����޻�ȡ
	/// \param _upper              ����
	/// \param _lower              ����
	void GetHeightUpperLower(double& _upper, double& _lower);

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
	virtual	int SendVisionData(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	virtual	int SendRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	//����Ƿ��ڲɼ�
	virtual int	IsCamCapture();

	//����Ƿ�����
	virtual	int	IsCamConnect();
public:

	static void CameraTcpConnectFunc(int dwDeviceId, int cmd);
	/// \brief BatchOneTimeCallBack     �ص�����
	/// \param info
	/// \param data
	static void BatchOneTimeCallBack(const void *info,	const SR7IF_Data *data);

	/// \brief CallHighSpeedData  ��������ͨ�ŵĻص������ӿ�.
	/// \param pBuffer            ָ�򴢴��Ҫ���ݵĻ�������ָ��.
	/// \param uSize              ÿ����Ԫ(��)���ֽ�����.
	/// \param uCount             �洢��pBuffer�е��ڴ�ĵ�Ԫ����
	/// \param uNotify            �жϻ������������жϵ�֪ͨ.
	/// \param uUser              �û��Զ�����Ϣ.
	///
	static void CallHighSpeedData(char* pBuffer,
		unsigned int  uSize,
		unsigned int  uCount,
		unsigned int  uNotify,
		unsigned int uUser);
public:
	bool					m_bIsConnected;
	SR7IF_ETHERNET_CONFIG	m_SREthernetConFig;			//sdk dll ���Ӳ�������
	int						m_nCurrentDeviceID	= 0;
	int						m_nCurrentDealID	= 1;	//�䷽��
	
	int						m_BatchPoint;				//��������������
	int						m_BatchWidth;				//�������
	double					m_XinterVal;                //X���
	double					m_YinterVal;                //Y���
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