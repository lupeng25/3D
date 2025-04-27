#pragma once

#include <QtWidgets/QWidget>
#include "ui_frmflow.h"
#include <QWidget>
#include "QPlainTextEdit"
#include "Log/savelog.h"
#include "FlowGraphicsViews.h"
#include <QMap>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

class frmflow : public QWidget
{
	Q_OBJECT
public:
	explicit frmflow(QWidget *parent = Q_NULLPTR);
	~frmflow();

public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
public:
	void ClearAllTab();
public slots:
	void slot_AddFlow(QString strKey,QString strShow);
	void slot_ChangeFlow(QString strKey);
	void slot_ReNameFlow(QString strKey,QString strOld, QString strNew);
	void slot_DeleteFlow(QString strKey);
	void slot_SetFlowEnable(QString strKey, bool Endble);
public slots:
	void slot_BtnRunOnce();
	void slot_BtnRunCycle();
	void slot_BtnStop();

private slots:
	void on_btnRunOnce_clicked();
	void on_btnRunCycle_clicked();
	void on_btnStop_clicked();
	void Status_Stop();

Q_SIGNALS:
	void sig_AllCycleStop();
	void sendCurrentChangedSignal(int index);
public:
	void UpDateTab();
private:
	Ui::frmflowClass	ui;
};