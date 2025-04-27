#pragma once

#include <QWidget>
#include "ui_QtMesWidgets.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QTextCodec>
#include <QFile>
//#include "Common/ComSocket.h"
#include "ComSocket.h"
#include "Pages/QtWidgetsBase.h"
#include <QMutex>

class QtMesWidgets : public QtWidgetsBase
{
	Q_OBJECT
public:
	QtMesWidgets(QWidget *parent = Q_NULLPTR);
	~QtMesWidgets();
public:
	void CheckConnected();
	void Connected();
	void onSelectedConnectedTCP(bool bOn = false);

	void SendInstructMessage(const QString &message);
	void RecieveInstructMessage(const QString &message);

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

private slots :
	void onTcpQConnected(QString strkey, CTcpSocket*);
	void onTcpQDisconnected(QString strkey, CTcpSocket*);
	void onTcpDataReadyRead(CTcpSocket* socket, QString& str);

public:
	void Init();
public slots:
	void on_btnSend_clicked();
	void on_btnConnect_clicked();
private:
	Ui::QtMesWidgets ui;
	CTcpSocket*		m_pCTcpSocket{ nullptr };
	QMutex			m_QMutex;
};
