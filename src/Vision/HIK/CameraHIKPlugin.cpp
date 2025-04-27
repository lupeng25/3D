#include "CameraHIKPlugin.h"
#include "Camera_HIK.h"

CameraHIKPlugin::CameraHIKPlugin(QObject *parent)
    : QObject(parent)
{
}

QString CameraHIKPlugin::getVision()
{
	return "V20240418";
}

QVector<QPair<QString, QString>>  CameraHIKPlugin::GetLanguageToolName()
{
	QVector<QPair<QString, QString>> vecTools;
	vecTools.push_back(QPair<QString, QString>("HIK", tr("HIK")));
	return vecTools;
}

int CameraHIKPlugin::SetPluginParam(QString key, QString value)
{
	return 0;
}

int CameraHIKPlugin::GetPluginParam(QString key, QString & value)
{
	return 0;
}

int CameraHIKPlugin::GetAllPluginParam(QMap<QString, QString>& map)
{
	return 0;
}

ENUM_PLUGIN_TYPE CameraHIKPlugin::getPluginType()
{
	return ENUM_PLUGIN_TYPE_CAMERA;
}

QVector<CameraInfo> CameraHIKPlugin::getVecInfo()
{
	QVector<CameraInfo> vecCam = Camera_HIK::getVecInfo();
	return vecCam;
}

QString CameraHIKPlugin::getName()
{
	return "CameraHIKPlugin";
}

QString CameraHIKPlugin::getDescription()
{
	//return "Camera3DSSZNPlugin";
	return	QString("%1\r\n%2\r\n%3\r\ndllVision%4\r\n")
		.arg("CameraHIKPlugin")
		.arg("Camera_HIK")
		.arg(Camera_HIK::getDescription());
}

QIcon CameraHIKPlugin::getQIcon()
{
	return QIcon();
}

int CameraHIKPlugin::Initialize()
{
	return Camera_HIK::Initialize();
}

int CameraHIKPlugin::Finalize()
{
	return Camera_HIK::Finalize();
}

int CameraHIKPlugin::Init()
{
	return 0;
}

int CameraHIKPlugin::Exit()
{
	return 0;
}

int CameraHIKPlugin::setCamerainterface(QString strCamera, Camerainterface *& cam)
{
	//return 0;
	cam		= new Camera_HIK();
	cam->setName(strCamera);
	return 0;
}

//int CameraHIKPlugin::setFlowInterface(Vision::Node *& node)
//{
//	return 0;
//}
int CameraHIKPlugin::setFlowInterface(FlowchartGraphicsItem *& node)
{
	return 0;
}