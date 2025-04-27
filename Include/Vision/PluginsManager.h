#pragma once
#include <QDir>
#include <QtWidgets/QApplication>
#include "Pluginterface.h"
#include <QList>
#include <QtPlugin>

#ifndef _PLUGINSMANAGER_EXPORT_
#define _PLUGINSMANAGER_API_ _declspec(dllexport)
#else
#define _PLUGINSMANAGER_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _PLUGINSMANAGER_API_ PluginsManager
{
public:
	static PluginsManager& Instance();

public:
	int Init();
	int Exit();

public:
	int LoadPlugins();
	int LoadCamPlugins();

	QVector<CameraInfo> getVecCameras();
	QStringList getCameraNames();

	Camerainterface* InitCamera(QString strCam);

	bool FindCamera(QString strCam);

public:
	QList<Pluginterface*> m_ToolPluginLst;
	QList<Pluginterface*> m_CamPluginLst;	//相机插件
};

