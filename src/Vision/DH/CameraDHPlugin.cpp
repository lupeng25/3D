#include "CameraDHPlugin.h"
#include "Camera_DH.h"

CameraHIKPlugin::CameraHIKPlugin(QObject *parent)
    : QObject(parent)
{
}

QString CameraHIKPlugin::getVision()
{
	return "V20240418";
}

QVector<QPair<QString, QString>> CameraHIKPlugin::GetLanguageToolName()
{
	return QVector<QPair<QString, QString>>();
}

ENUM_PLUGIN_TYPE CameraHIKPlugin::getPluginType()
{
	return ENUM_PLUGIN_TYPE_CAMERA;
}

QVector<CameraInfo> CameraHIKPlugin::getVecInfo()
{
	QVector<CameraInfo> vecCam = Camera_DH::getVecInfo();
	return vecCam;
}

QString CameraHIKPlugin::getName()
{
	return "CameraHIKPlugin";
}

QIcon CameraHIKPlugin::getQIcon()
{
	return QIcon();
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

QString CameraHIKPlugin::getDescription()
{
	//return "Camera3DSSZNPlugin";
	return	QString("%1\r\n%2\r\n%3\r\ndllVision%4\r\n")
		.arg("CameraHIKPlugin")
		.arg("Camera_HIK")
		.arg(Camera_DH::getDescription());
}

int CameraHIKPlugin::Initialize()
{
	return Camera_DH::Initialize();
}

int CameraHIKPlugin::Finalize()
{
	return Camera_DH::Finalize();
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
	cam		= new Camera_DH();
	cam->setName(strCamera);
	return 0;
}

int CameraHIKPlugin::setFlowInterface(FlowchartGraphicsItem *& cam)
{
	return 0;
}

//int CameraHIKPlugin::setFlowInterface(Vision::Node *& node)
//{
//	return 0;
//}
