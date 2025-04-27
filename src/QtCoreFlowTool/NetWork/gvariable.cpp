#include "gvariable.h"

gVariable & gVariable::Instance()
{
	// TODO: 在此处插入 return 语句
	static gVariable ins;
	return ins;
}

void gVariable::Exit()
{
	for (auto iter : serialport_variable_link)
		if (iter.second.serialport_value != nullptr)
			iter.second.serialport_value->Close();
	serialport_variable_link.clear();
	for (auto iter : sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) {
			iter.second.server_value->Close();
			iter.second.server_value->m_bConnected = false;
		}
	sockettcpserver_variable_link.clear();
	for (auto iter : sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) {
			iter.second.client_value->Close();
			iter.second.client_value->m_bConnected = false;
		}
	sockettcpclient_variable_link.clear();

}

int gVariable::SendMsg(QString strKey, QString strMsg)
{
	int _iRetn = ReConnect(strKey);
	if (_iRetn <= 0) return _iRetn;
	for (auto iter : serialport_variable_link)
		if (iter.second.serialport_value != nullptr) {
			if (iter.first == strKey) {
				if (iter.second.serialport_value->IsOpen()) {
					iter.second.serialport_value->Write(strMsg);
					return 1;
				}
				else return 0;
			}
		}
	for (auto iter : sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) {
			if (iter.first == strKey) {
				if (iter.second.server_value->isOpen()) {
					for (auto client : iter.second.server_value->m_vecCTcpClient)	{
						if (client->m_Client != nullptr)
							client->m_Client->write(strMsg);
					}
					return 1;
				}
				else return 0;
			}
		}
	for (auto iter : sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) {
			if (iter.first == strKey) {
				if (!iter.second.client_value->isOpen()) {
					if (iter.second.client_value->isOpen()) {
						iter.second.client_value->write(strMsg);
						return 1;
					}
					else return 0;
				}
				else return 1;
			}
		}
	return 0;
}

int gVariable::GetMsg(QString strKey, QString & strMsg, int iTimeOut)
{
	int _iRetn = ReConnect(strKey);
	if (_iRetn <= 0) return _iRetn;
	for (auto iter : serialport_variable_link)
		if (iter.second.serialport_value != nullptr) {
			if (iter.first == strKey) {
				if (iter.second.serialport_value->IsOpen()) {
					return iter.second.serialport_value->WaitMsg(strMsg, iTimeOut);
				}
				else return 0;
			}
		}
	for (auto iter : sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) {
			if (iter.first == strKey) {
				if (iter.second.server_value->isOpen()) {
					return iter.second.server_value->WaitMsg(strMsg, iTimeOut);
				}
				else return 0;
			}
		}
	for (auto iter : sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) {
			if (iter.first == strKey) {
				if (iter.second.client_value->isOpen()) {
					return iter.second.client_value->WaitMsg(strMsg, iTimeOut);
				}
				else return 0;
			}
		}
	return 0;
}

void gVariable::ClearRecieve()
{
	for (auto iter : serialport_variable_link)
		if (iter.second.serialport_value != nullptr) {
			iter.second.serialport_value->m_vecPtrRecieveData.clear();
			iter.second.serialport_value->ClearRecieve();

		}
	for (auto iter : sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) {
			iter.second.server_value->m_vecPtrRecieveData.clear();
			iter.second.server_value->ClearRecieve();
		}
	for (auto iter : sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) {
			iter.second.client_value->m_vecPtrRecieveData.clear();
			iter.second.client_value->ClearRecieve();
		}
}

int gVariable::ReConnect(QString strKey)
{
	for (auto iter : serialport_variable_link)
		if (iter.second.serialport_value != nullptr) {
			if (iter.first == strKey)		{
				if (!iter.second.serialport_value->IsOpen())	{
					DWORD dwPort		= iter.second.portname_value.mid(3).toInt();
					DWORD dwBaudRate	= iter.second.baudrate_value;
					BYTE btParity		= iter.second.parity_value;	//校验位
					switch (iter.second.parity_value)				{
					case QSerialPort::NoParity:		btParity = NOPARITY;	break;
					case QSerialPort::EvenParity:	btParity = EVENPARITY;	break;
					case QSerialPort::OddParity:	btParity = ODDPARITY;	break;
					case QSerialPort::SpaceParity:	btParity = SPACEPARITY;	break;
					case QSerialPort::MarkParity:	btParity = MARKPARITY;	break;
					default:	break;
					}
					BYTE btByteSize = iter.second.databits_value;
					BYTE btStopBits = ONESTOPBIT;
					switch (iter.second.stopbits_value)		{
					case QSerialPort::OneStop:			btStopBits = ONESTOPBIT;	break;
					case QSerialPort::OneAndHalfStop:	btStopBits = ONE5STOPBITS;	break;
					case QSerialPort::TwoStop:			btStopBits = TWOSTOPBITS;	break;
					default:	break;
					}
					bool b_port_state = iter.second.serialport_value->Open(dwPort, dwBaudRate, btParity, btByteSize, btStopBits);
					if (!iter.second.serialport_value->IsOpen()) {
						return -1;
					}
					else return 1;
				}	
				else return 1;
			}
		}
	for (auto iter : sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) {
			if (iter.first == strKey) {
				if (!iter.second.server_value->isOpen()) {
					iter.second.server_value->InitServer((char*)iter.second.ip_value.toStdString().c_str(), iter.second.port_value);
					if (!iter.second.server_value->isOpen()) {
						return -1;
					}
					else return 1;
				}
				else return 1;
			}
		}
	for (auto iter : sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) {
			if (iter.first == strKey) {
				if (!iter.second.client_value->isOpen()) {
					iter.second.client_value->ConnectToServer((char*)iter.second.ip_value.toStdString().c_str(), iter.second.port_value);
					if (!iter.second.client_value->isOpen()) {
						return -1;
					}
					else return 1;
				}
				else return 1;
			}
		}
	return 0;
}

QVector<QString> gVariable::GetAllVariable()
{
	QVector<QString> _vec;

	return _vec;
}
