#ifndef DECLAREINTERFACE_H
#define DECLAREINTERFACE_H
#include <QMap>
#include <QtPlugin>
#include <QObject>
#include <QICon>
#include <QString>
#include <QVector>
#include "Camerainterface.h"
//#include "Node.h"
#include "../FlowChart/flowchar_graphics_item_base.h"

enum ENUM_PLUGIN_TYPE
{
	ENUM_PLUGIN_TYPE_CAMERA,
	ENUM_PLUGIN_TYPE_TOOL,
	ENUM_PLUGIN_TYPE_STD,
};

//定义接口
class Pluginterface
{
public:
	virtual ~Pluginterface() {};
public:
	virtual QString getVision() = 0;

	virtual ENUM_PLUGIN_TYPE getPluginType() = 0;

	virtual QVector<CameraInfo> getVecInfo() = 0;
public:
	virtual QString getName() = 0;

	virtual QIcon getQIcon() = 0;

	virtual QVector<QPair<QString,QString>> GetLanguageToolName() = 0;
public:
	virtual int SetPluginParam(QString key, QString value) = 0;

	virtual int GetPluginParam(QString key, QString& value) = 0;

	virtual int GetAllPluginParam(QMap<QString, QString>&map) = 0;

	virtual QString getDescription() = 0;
public:
	//初始化动态库
	virtual int Initialize() = 0;

	virtual int Finalize() = 0;
public:
	//初始化
	virtual int Init() = 0;

	virtual int Exit() = 0;
public:
	virtual int setCamerainterface(QString strCamera, Camerainterface*& cam) = 0;
public:
	virtual int setFlowInterface(FlowchartGraphicsItem*& cam) = 0;
};

//一定是唯一的标识符
#define Pluginterface_iid "Examples.Plugin.Pluginterface"

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(Pluginterface, Pluginterface_iid)
QT_END_NAMESPACE

#endif // DECLAREINTERFACE_H

