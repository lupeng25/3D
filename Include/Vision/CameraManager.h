#pragma once
#include "Camerainterface.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "Comm.h"

#ifndef _CAMERAMANAGER_EXPORT_
#define _CAMERAMANAGER_API_ _declspec(dllexport)
#else
#define _CAMERAMANAGER_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _CAMERAMANAGER_API_ CameraManager
{
public:
	static CameraManager& Instance();

public:
	//关闭相机
	int Exit();

public:
	int GetData(QJsonObject& strData);

	int SetData(QJsonObject& strData);

public:
	int GetCamData(Camerainterface* cam, QJsonObject& str);

	int SetCamData(Camerainterface* cam, QJsonObject& str);
public:
	int DeleteCamera(QString strCam);

	Camerainterface* FindCamera(QString strCam);
public:
	QMap<QString, Camerainterface*>	m_MapCameras;
};

