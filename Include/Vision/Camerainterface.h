#ifndef CAMERAINTERFACE_H
#define CAMERAINTERFACE_H

#include <QMap>
#include <QtPlugin>
#include <QString>
#include <QVector>
#include "Comm.h"

//返回值
enum _SMCV_RETN
{
	SMCV_RETN_SUCCESS,				//成功
	SMCV_RETN_NOREG,				//没注册，或者使用过期
	SMCV_RETN_NOINIT,				//没初始化
	SMCV_RETN_OVERRANGE_CAMERAL,	//相机超出范围
	SMCV_RETN_OVERRANGE_SINDEX,		//显示图超出范围
	SMCV_RETN_OVERRANGE_TINDEX,		//模板图超出范围
	SMCV_RETN_OVERRANGE_DATA,		//数据超出范围
	SMCV_RETN_FAILIURE,				//失败
	SMCV_RETN_NOTTRIGGLEMODE,		//不是触发模式
	SMCV_RETN_FAILIUER_THREAD,		//内部线程失败
};

enum EnumModeType
{
	EnumModeType_Shape,
	EnumModeType_NCC,
};

typedef	struct tagCameraInfo
{
	QString strCameraName;			//相机名称
	QString strCameraBrand;			//相机品牌
	QString strCameraIP;			//相机IP
	QString strCameraKey;			//相机IP
	QString strCameraModelName;		//相机型号
	QString strSerialNumber;		//相机序列号
	QString strManufactureInfo;		//相机制造信息
}CameraInfo;

enum EnumCameraTriggerMode         //相机模式
{
	CameraMode_Continnue,					//图像显示模式
	CameraMode_SoftTrigger,                 //图像处理模式
	CameraMode_TriggerLine1_RisingEdge,     //线1触发
	CameraMode_TriggerLine1_FallingEdge,    //线1触发
	CameraMode_TriggerLine2_RisingEdge,     //线2触发
	CameraMode_TriggerLine2_FallingEdge,    //线2触发
	CameraMode_TriggerLine3_RisingEdge,     //线3触发
	CameraMode_TriggerLine3_FallingEdge,    //线3触发
	CameraMode_TriggerLine4_RisingEdge,     //线4触发
	CameraMode_TriggerLine4_FallingEdge,    //线4触发
};

//图像类型
//图像类型
const QString CameraMode			= "CameraMode";
const QString CameraExposure		= "CameraExposure";
const QString CameraMaxExposure		= "CameraMaxExposure";
const QString CameraMinExposure		= "CameraMinExposure";
const QString CameraGain			= "CameraGain";
const QString CameraMaxGain			= "CameraMaxGain";
const QString CameraMinGain			= "CameraMinGain";
const QString CameraBrightness		= "CameraBrightness";
const QString CameraGamma			= "CameraGamma";
const QString CameraContrast		= "CameraContrast";
const QString CameraTriggerDelay	= "CameraTriggerDelay";
const QString ImgType_Byte			= "byte";
const QString ImgType_Real			= "real";
const QString ImgType_Int1			= "int1";
const QString ImgType_Int2			= "int2";
const QString ImgType_Int4			= "int4";
const QString ImgType_Int8			= "int8";
const QString ImgType_uInt2			= "uint2";
const QString ImgType_complex		= "complex";
const QString ImgType_cyclic		= "cyclic";
const QString ImgType_direction		= "direction";

const QString CameraWidth			= "CameraWidth";
const QString CameraXInterVal		= "CameraXInterVal";	//X轴间距
const QString CameraHeight			= "CameraHeight";
const QString CameraRowCount		= "CameraRowCount";
const QString CameraYInterVal		= "CameraYInterVal";	//Y轴间距
const QString CameraZHeight			= "CameraZHeight";
const QString CameraZInterVal		= "CameraZInterVal";	//Z轴间距

const QString CameraIPAdress		= "CameraIPAdress";		//IP地址
const QString CameraIPPort			= "CameraIPPort";		//IP地址
const QString CameraName			= "CameraName";			//相机名称

const QString CameraDeviceId		= "CameraDeviceId";		//相机处理ID
const QString CameraDealID			= "ActiveProgram";		//相机配方号
const QString CameraSerial			= "CameraSerial";		//CameraSerial
const QString CameraModels			= "CameraModels";		//CameraModels

const QString CameraLicenseKey		= "CameraLicenseKey";	//CameraLicenseKey

class CameraStream
{
public:
	virtual int RecieveBuffer(QString strCamName,unsigned char* gray, int* IntensityData, int iwidth,int iheight,int ibit,float fscale,QString imgtype,int icount) = 0;

	//当使用当前接口时8为使用Rgray 使用 其他都应该Ggray Bgray为nullptr 
	virtual int RecieveRGBBuffer(QString strCamName,unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount) = 0;
};

//定义接口
class Camerainterface
{
public:
	virtual ~Camerainterface() {};
public:
	virtual QString getName() = 0;

	virtual void setName(const QString& strName) = 0;

	virtual QString getCameraType() = 0;
public:
	virtual int Init() = 0;

	virtual int Exit() = 0;
public:
	virtual int Start() = 0;

	virtual int Stop() = 0;

	virtual int StartVedioLive() = 0;

	virtual int StopVedioLive() = 0;
public:
	virtual int SetStream(CameraStream* obj) = 0;

	virtual int Trigger() = 0;

	virtual int WaitTrigger(int timeout = -1) = 0;
public:
	virtual int SetCameraParam(QString key, QString value) = 0;

	virtual int GetCameraParam(QString key, QString& value) = 0;

	virtual int GetAllCameraParam( QVector<QPair<QString, QString>>&mapValue) = 0;
public:
	virtual int IsCamCapture() = 0;

	virtual int IsCamConnect() = 0;
};

////一定是唯一的标识符
//#define Pluginterface_iid "Examples.Plugin.Pluginterface"
//QT_BEGIN_NAMESPACE
//Q_DECLARE_INTERFACE(Pluginterface, Pluginterface_iid)
//QT_END_NAMESPACE

#endif // CAMERAINTERFACE_H

