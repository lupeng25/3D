#ifndef CAMERA3DPLUGIN_H
#define CAMERA3DPLUGIN_H

#include <QGenericPlugin>
#include "Pluginterface.h"
#include "Camerainterface.h"
#include <QMap>
#include "Camera3D_SSZN.h"

class Camera3DSSPlugin : public QObject, public Pluginterface
{
    Q_OBJECT
    Q_INTERFACES(Pluginterface)
    Q_PLUGIN_METADATA(IID Pluginterface_iid FILE "Camera3DSSZNPlugin.json")

public:
    explicit Camera3DSSPlugin(QObject *parent = nullptr);
public:
	virtual QString getVision() ;

	virtual ENUM_PLUGIN_TYPE getPluginType();

	virtual QVector<CameraInfo> getVecInfo() ;
public:
	virtual QString getName();

	virtual QVector<QPair<QString, QString>> GetLanguageToolName();
public:
	virtual int SetPluginParam(QString key, QString value);

	virtual int GetPluginParam(QString key, QString& value);

	virtual int GetAllPluginParam(QMap<QString, QString>&map);

	virtual QString getDescription();

	virtual QIcon getQIcon();
public:
	virtual int Initialize();

	virtual int Finalize();
public:
	virtual int Init();

	virtual int Exit();
public:
	virtual int setCamerainterface(QString strCamera, Camerainterface*& cam);

	virtual int setFlowInterface(FlowchartGraphicsItem*& cam); 
public:



};
#endif // CAMERA3DPLUGIN_H