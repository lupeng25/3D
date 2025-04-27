#pragma once

#include <QWidget>
#include "ui_QtVisionWidgets.h"
#include "Pages/QtWidgetsBase.h"
#include "ImageShow/QGraphicsViews.h"
#include "Data/dataVar.h"
#include "ComSocket.h"

class QtVisionWidgets : public QtWidgetsBase,public RecieveData
{
	Q_OBJECT

public:
	QtVisionWidgets(QWidget *parent = Q_NULLPTR);
	~QtVisionWidgets();
public:
	virtual	int	RecieveTCP(CTcpClient* sock, QString& str);
	virtual	int	DisConnect(CTcpClient* sock) ;
public slots:

public:
	void CheckConConnected();
	QStringList getSerialPortList();
	void Connected();
	virtual void initUI();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);
	virtual int	NewProject();

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);
signals:

public slots :

protected:
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);

private slots :
	//void on_btnConnect_clicked();
	void slotUpDateSerialPort();

	//void on_btnSendMsg_clicked();
	void SendInstructMessage(const QString &message);
	void RecieveInstructMessage(const QString &message);
private:
	Ui::QtVisionWidgetsClass ui;

	QMutex				m_QMutex;
};
