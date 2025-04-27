#ifndef CAMERA3DPLUGIN_H
#define CAMERA3DPLUGIN_H

#include <QGenericPlugin>
#include "Pluginterface.h"
#include "Camerainterface.h"
#include <QMap>
#include "Camera3D_KEYENCE.h"

class Camera3DPlugin : public QObject, public Pluginterface
{
    Q_OBJECT
    Q_INTERFACES(Pluginterface)
    Q_PLUGIN_METADATA(IID Pluginterface_iid FILE "Camera3DPlugin.json")

public:
    explicit Camera3DPlugin(QObject *parent = nullptr);
public:
	virtual QString getVision() ;

	virtual QPair<QString, QString> GetToolName();

	virtual ENUM_PLUGIN_TYPE getPluginType();

	virtual QVector<CameraInfo> getVecInfo() ;
public:
	virtual QString getName();

	virtual QString getDescription();
public:
	virtual int Initialize();

	virtual int Finalize();
public:
	virtual int Init();

	virtual int Exit();
public:
	virtual int setCamerainterface(QString strCamera, Camerainterface*& cam);

	virtual int setFlowInterface(Node*& node);
public:
	QMap<QString, Camera3D_KEYENCE*> m_MapCamera3D_KEYENCE;
};

#endif // CAMERA3DPLUGIN_H