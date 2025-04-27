#include "CloudMesureToolPlugin.h"
#include "MatchTool.h"

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
	vecTools.push_back(QPair<QString, QString>("ImageDeal",tr("ImageDeal")));
	vecTools.push_back(MatchTool::GetToolName());
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

QIcon CloudMesureToolPlugin::getQIcon()
{
	return QIcon();
}

QString CloudMesureToolPlugin::getDescription()
{
	return QString(tr("Auto Match"));
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
	node = new MatchTool();
	return 0;
}
