#pragma once

#include <QWidget>
#include "ui_QtCenterWidgets.h"

#include "Pages/QtWidgetsBase.h"
class QtCenterWidgets : public QWidget
{
	Q_OBJECT

public:
	QtCenterWidgets(QWidget *parent = Q_NULLPTR);
	~QtCenterWidgets();

public:
	virtual void initUI();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);
	virtual int initUIData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);

public:
	int AddWndSet(QtWidgetsBase* widget);
private:
	Ui::QtCenterWidgetsClass ui;
	QVector<QtWidgetsBase*> m_vecpWidgetsBase;
};
