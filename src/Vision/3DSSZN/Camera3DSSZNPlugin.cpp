#include "Camera3DSSZNPlugin.h"
#include "Camera3D_SSZN.h"

Camera3DSSPlugin::Camera3DSSPlugin(QObject *parent)
    : QObject(parent)
{
}

QString Camera3DSSPlugin::getVision()
{
	return "V20240228";
}

ENUM_PLUGIN_TYPE Camera3DSSPlugin::getPluginType()
{
	return ENUM_PLUGIN_TYPE_CAMERA;
}

QVector<CameraInfo> Camera3DSSPlugin::getVecInfo()
{
	QVector<CameraInfo> vecCam = Camera3D_SSZN::getVecInfo();
	return vecCam;
}

QString Camera3DSSPlugin::getName()
{
	return "Camera3DSSZNPlugin";
}

QVector<QPair<QString, QString>> Camera3DSSPlugin::GetLanguageToolName()
{
	return QVector<QPair<QString, QString>>();
}


int Camera3DSSPlugin::SetPluginParam(QString key, QString value)
{
	return 0;
}

int Camera3DSSPlugin::GetPluginParam(QString key, QString & value)
{
	return 0;
}

int Camera3DSSPlugin::GetAllPluginParam(QMap<QString, QString>& map)
{
	return 0;
}

QString Camera3DSSPlugin::getDescription()
{
	return	QString("%1\r\n%2\r\n%3\r\ndllVision%4\r\n")
		.arg("Camera3DSSZNPlugin")
		.arg("3DCamera")
		.arg("SSZN")
		.arg(Camera3D_SSZN::getDescription());
}

QIcon Camera3DSSPlugin::getQIcon()
{
	return QIcon();
}

int Camera3DSSPlugin::Initialize()
{
	return Camera3D_SSZN::Initialize();
}

int Camera3DSSPlugin::Finalize()
{
	return Camera3D_SSZN::Finalize();
}

int Camera3DSSPlugin::Init()
{
	return 0;
}

int Camera3DSSPlugin::Exit()
{
	return 0;
}

int Camera3DSSPlugin::setCamerainterface(QString strCamera, Camerainterface *& cam)
{
	cam		= new Camera3D_SSZN();
	cam->setName(strCamera);
	return 0;
}

int Camera3DSSPlugin::setFlowInterface(FlowchartGraphicsItem *& node)
{
	return 0;
}
