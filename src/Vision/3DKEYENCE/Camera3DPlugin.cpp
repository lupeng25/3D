#include "Camera3DPlugin.h"
#include "Camera3D_KEYENCE.h"

Camera3DPlugin::Camera3DPlugin(QObject *parent)
    : QObject(parent)
{
}

QString Camera3DPlugin::getVision()
{
	return "V20240130";
}

QPair<QString, QString>  Camera3DPlugin::GetToolName()
{
	return QPair<QString, QString>(QString::fromLocal8Bit(""),
		QString::fromLocal8Bit(""));
}

ENUM_PLUGIN_TYPE Camera3DPlugin::getPluginType()
{
	return ENUM_PLUGIN_TYPE_CAMERA;
}

QVector<CameraInfo> Camera3DPlugin::getVecInfo()
{
	QVector<CameraInfo> vecCam = Camera3D_KEYENCE::getVecInfo();
	return vecCam;
}

QString Camera3DPlugin::getName()
{
	return "Camera3DPlugin";
}

QString Camera3DPlugin::getDescription()
{
	return	"Camera3DPlugin"
			"3DCamera"
			"KEYENCE";
}

int Camera3DPlugin::Initialize()
{
	return Camera3D_KEYENCE::Initialize();
}

int Camera3DPlugin::Finalize()
{
	return Camera3D_KEYENCE::Finalize();
}

int Camera3DPlugin::Init()
{
	return 0;
}

int Camera3DPlugin::Exit()
{
	return 0;
}

int Camera3DPlugin::setCamerainterface(QString strCamera, Camerainterface *& cam)
{
	Camera3D_KEYENCE *_Pcam = new Camera3D_KEYENCE();
	cam = _Pcam;
	m_MapCamera3D_KEYENCE.insert(strCamera, _Pcam);
	return 0;
}

int Camera3DPlugin::setFlowInterface(Node *& node)
{
	return 0;
}
