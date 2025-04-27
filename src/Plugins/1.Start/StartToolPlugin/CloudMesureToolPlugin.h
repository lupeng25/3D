#ifndef ExportCSVToolPlugin_H
#define ExportCSVToolPlugin_H

#include <QGenericPlugin>
#include "Pluginterface.h"
#include "Camerainterface.h"
#include <QVector>

class CloudMesureToolPlugin : public QObject, public Pluginterface
{
    Q_OBJECT
    Q_INTERFACES(Pluginterface)
    Q_PLUGIN_METADATA(IID Pluginterface_iid FILE "CloudMesureToolPlugin.json")
public:
    explicit CloudMesureToolPlugin(QObject *parent = nullptr);
public:
	virtual QString getVision() ;

	virtual QVector<QPair<QString, QString>> GetLanguageToolName();

	virtual ENUM_PLUGIN_TYPE getPluginType();

	virtual QVector<CameraInfo> getVecInfo() ;
public:
	virtual QString getName();

	virtual QIcon getQIcon(); 
	
	virtual QString getDescription();

public:
	virtual int SetPluginParam(QString key, QString value);

	virtual int GetPluginParam(QString key, QString& value);

	virtual int GetAllPluginParam(QMap<QString, QString>&map);
public:
	virtual int Initialize();

	virtual int Finalize();
public:
	virtual int Init();

	virtual int Exit();
public:
	virtual int setCamerainterface(QString strCamera, Camerainterface*& cam);

	virtual int setFlowInterface(FlowchartGraphicsItem*& node);

};

#endif // CAMERA3DPLUGIN_H
