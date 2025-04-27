#pragma once

#include <QMap>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSerialPort>
#include "winsock2.h"
#include <vector>
#include "ComSocket.h"
using namespace std;

#ifndef _GVARIABLE_EXPORT_
#define _GVARIABLE_API_ _declspec(dllexport)
#else
#define _GVARIABLE_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _GVARIABLE_API_ gVariable
{	
public:
	static gVariable& Instance();
public:
	void Exit();
public:
	int SendMsg(QString	strKey, QString	strMsg);
	int GetMsg(QString	strKey, QString& strMsg,int iTimeOut = 2000);
	void ClearRecieve();
	int ReConnect(QString strKey);

public:
	QVector<QString> GetAllVariable();
public:		
	typedef struct SERIALPORTVAR  //串口通信
	{
		MyCnCommort*				serialport_value = nullptr;
		int							connect_state;
		QString						portname_value;
		int							baudrate_value;
		QSerialPort::Parity			parity_value;
		QSerialPort::DataBits		databits_value;
		QSerialPort::StopBits		stopbits_value;
		QSerialPort::FlowControl	flowcontrol_value;
		QString						remark_value;
	} SerialPort_Var;
	SerialPort_Var SerialPortVar;
	std::map<QString, SerialPort_Var> serialport_variable_link;

	typedef struct SOCKETTCPSERVERVAR  //TCP/IP服务器
	{
		CTcpServerSocket*			server_value = nullptr;
		int							connect_state;
		int							protocol_value;
		QString						ip_value;
		int							port_value;
		QString						remark_value;
	} SocketTcpServer_Var;
	SocketTcpServer_Var SocketTcpServerVar;
	std::map<QString, SocketTcpServer_Var> sockettcpserver_variable_link;

	typedef struct SOCKETTCPCLIENTVAR  //TCP/IP客户端
	{
		CTcpSocket*					client_value = nullptr;
		int							connect_state;
		int							protocol_value;
		QString						ip_value;
		int							port_value;
		QString						remark_value;
	} SocketTcpClient_Var;
	SocketTcpClient_Var SocketTcpClientVar;
	std::map<QString, SocketTcpClient_Var> sockettcpclient_variable_link;

	QVector<QString>	m_vecNames;
};