#include "CloudMesureToolPlugin.h"
#include "ScriptEditTool.h"

CloudMesureToolPlugin::CloudMesureToolPlugin(QObject *parent)
    : QObject(parent)
{

}

QString CloudMesureToolPlugin::getVision()
{
	return "V20241121";
}

QVector<QPair<QString, QString>> CloudMesureToolPlugin::GetLanguageToolName()
{
	QVector<QPair<QString, QString>> vecTools;
	vecTools.push_back(QPair<QString, QString>("SystemSet", tr("SystemSet")));
	vecTools.push_back(ScriptEditTool::GetToolName());
	return vecTools;
}

ENUM_PLUGIN_TYPE CloudMesureToolPlugin::getPluginType()
{
	return ENUM_PLUGIN_TYPE_TOOL;
}

QVector<CameraInfo> CloudMesureToolPlugin::getVecInfo()
{
	return QVector<CameraInfo>();
}

QString CloudMesureToolPlugin::getName()
{
	return QString();
}

QString CloudMesureToolPlugin::getDescription()
{
	return QString();
}


QIcon CloudMesureToolPlugin::getQIcon()
{
	return QIcon();
}

int CloudMesureToolPlugin::SetPluginParam(QString key, QString value)
{
	return 0;
}

int CloudMesureToolPlugin::GetPluginParam(QString key, QString & value)
{
	return 0;
}

int CloudMesureToolPlugin::GetAllPluginParam(QMap<QString, QString>& map)
{
	return 0;
}

int CloudMesureToolPlugin::Initialize()
{
	return 0;
}

int CloudMesureToolPlugin::Finalize()
{
	return 0;
}

int CloudMesureToolPlugin::Init()
{
	return 0;
}

int CloudMesureToolPlugin::Exit()
{
	return 0;
}

int CloudMesureToolPlugin::setCamerainterface(QString strCamera, Camerainterface *& cam)
{
	return 0;
}

int CloudMesureToolPlugin::setFlowInterface(FlowchartGraphicsItem *& node)
{
	node = new ScriptEditTool();
	return 0;
}
