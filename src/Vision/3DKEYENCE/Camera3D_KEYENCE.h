#pragma once
/**********************
2024 0312
������	:			�׳���
����	:			ʹ�û���ʿ�������ȡͼȻ�����������ز���
�����õĲ��������¹���:
CameraIPAdress		Ip��ַ
CameraIPPort		�˿ں�
CameraDealID		�䷽��
CameraRowCount		�г���
CameraXInterVal		X�м�� (0.02)

��ȡ���������¹���:
CameraIPAdress		Ip��ַ
CameraIPPort		�˿ں�
CameraSerial		���к�
CameraRowCount		�г���
CameraDealID		�䷽��
CameraDeviceId		װ��ID
CameraModels		ģ����
CameraXInterVal		X���
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

public:		//��̬��
	static int Initialize();

	static int Finalize();

	static QString GetVision();

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
	virtual int RecieveBuffer(QString strCamera, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	//��ʹ�õ�ǰ�ӿ�ʱ8Ϊʹ��Rgray ʹ�� ������Ӧ��Ggray BgrayΪnullptr 
	virtual int RecieveRGBBuffer(QString strCamera, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);
public:
	//����Ƿ��ڲɼ�
	virtual int	IsCamCapture();

	//����Ƿ�����
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
	DWORD					m_dwProfileCount = 1;	//һ���Խ��ܶ�������

	void*					m_hTrrigerHand;
	CameraStream*			m_ptrCamera;

	int						m_nCurrentDeviceID;
	int						m_nCurrentDealID = 1;	//�䷽��
	int						m_BatchPoint;			//��������������
	int						m_BatchWidth;			//�������
	double					m_XinterVal;            //X���
	double					m_YinterVal;            //Y���
	QString					m_strCameraName;
	static std::map<QString,Camera3D_KEYENCE*>	m_VecCamera;

};