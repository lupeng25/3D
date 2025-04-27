#include "Camera3D_KEYENCEPlugin.h"
#include "Camera3D_KEYENCE.h"

Camera3DPlugin::Camera3DPlugin(QObject *parent)
    : QObject(parent)
{
}

QString Camera3DPlugin::getVision()
{
	return "V20240130";
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

QVector<QPair<QString, QString>> Camera3DPlugin::GetLanguageToolName()
{
	return QVector<QPair<QString, QString>>();
}

int Camera3DPlugin::SetPluginParam(QString key, QString value)
{
	return 0;
}

int Camera3DPlugin::GetPluginParam(QString key, QString & value)
{
	return 0;
}

int Camera3DPlugin::GetAllPluginParam(QMap<QString, QString>& map)
{
	return 0;
}
QString Camera3DPlugin::getDescription()
{
	return	QString(
		"Camera3DPlugin"
		"3DCamera"
		"KEYENCE"
		"Vision:%1").arg(Camera3D_KEYENCE::GetVision());
}

QIcon Camera3DPlugin::getQIcon()
{
	return QIcon();
}

int Camera3DPlugin::Initialize()
{
	Camera3D_KEYENCE::m_VecCamera.clear();
	return Camera3D_KEYENCE::Initialize();
}

int Camera3DPlugin::Finalize()
{
	Camera3D_KEYENCE::m_VecCamera.clear();
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
	_Pcam->setName(strCamera);
	//Camera3D_KEYENCE::m_VecCamera.push_back(_Pcam);
	Camera3D_KEYENCE::m_VecCamera.insert(std::pair<QString, Camera3D_KEYENCE*>(strCamera,_Pcam));
	return 0;
}

int Camera3DPlugin::setFlowInterface(FlowchartGraphicsItem *& node)
{
	node = nullptr;
	return 0;
}
