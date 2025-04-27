#pragma once
#include <QtWidgets/QWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMap>

#ifndef _FRMSETBASE_EXPORT_
#define _FRMSETBASE_API_ _declspec(dllexport)
#else
#define _FRMSETBASE_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _FRMSETBASE_API_ frmSetBase : public QWidget
{
	Q_OBJECT
public:
	explicit frmSetBase(QWidget *parent = Q_NULLPTR);
	virtual	~frmSetBase();
public:
	void changeEvent(QEvent * event);
public:
	virtual bool CheckPageAvalible();
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	ResetData();
	virtual int	NewProject();
	virtual int UpdateUI();
	virtual int ScanTime();
	virtual void ChangesetTitleColor();
	virtual void RetranslationUi();
public:
	virtual	QVector<QPair<QString, QString>> GetVecSetName();

	static int AddWndSet(frmSetBase* widget);

public:
	static 	QVector<frmSetBase*> m_vecFrm;
};