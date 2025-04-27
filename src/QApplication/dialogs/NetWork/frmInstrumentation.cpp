#include "frmInstrumentation.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QWidgetAction>
#include <QDateTime>
#include "databaseVar.h"
#include "data/datavar.h"
#include <QSerialPortInfo>

frmInstrumentation::frmInstrumentation(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	this->setWindowIcon(QIcon(":/image/icon/images/network.png"));
	//初始化标题栏
	initTitleBar();
	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	connect(ui.tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slot_DoubleClicked(int, int)));

	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidget, &QTableWidget::customContextMenuRequested, this, &frmInstrumentation::slot_CustomContextMenuRequested);		//这个坐标一般是相对于控件左上角

	global_serialport_content.clear();
	global_tcp_content.clear();
	
	UpDataSerialLst();
	connect(ui.comboInstrument, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInstrumentChanged(int)));

	ui.widgetIoIP->setIP("192.168.1.10");
	ui.widgetPlcIP->setIP("192.168.1.20");
	ui.widgetSocketIP->setIP("127.0.0.1");
	ui.textEdit_Recieve->document()->setMaximumBlockCount(200);
	ui.textEdit_Send->document()->setMaximumBlockCount(200);
	slotInstrumentChanged(ui.comboInstrument->currentIndex());
}

frmInstrumentation::~frmInstrumentation()
{
	this->deleteLater();
}

int frmInstrumentation::GetData(QJsonObject & strData)
{
	{	// 串口通信
		QJsonObject SerialPort;
		for (QMap<QString, SerialPort_Content>::Iterator iter =
			global_serialport_content.begin();	iter !=
			global_serialport_content.end(); iter++)
		{
			QJsonArray IoArray = {
				iter.value().global_serial_port,
				QString::number(iter.value().global_baud_rate),
				QString::number(iter.value().global_check_digit),
				QString::number(iter.value().global_data_bits),
				QString::number(iter.value().global_stop_bit),
				QString::number(iter.value().global_flowcontrol),
				iter.value().global_remark
			};
			SerialPort.insert(iter.key(), IoArray);
		}
		strData.insert("SerialPort", SerialPort);	}
	{	//TCP/IP服务器
		QJsonObject TcpContent;
		for (QMap<QString, Tcp_Content>::Iterator iter =
			global_tcp_content.begin();	iter !=
			global_tcp_content.end(); iter++)
		{
			QJsonArray IoArray = {
				QString::number(iter.value().global_protocol),
				iter.value().global_ip,
				QString::number(iter.value().global_port),
				iter.value().global_remark
			};
			TcpContent.insert(iter.key(), IoArray);
		}
		strData.insert("TcpContent", TcpContent);
	}
	{	//Instrumentation
		QJsonArray IoArray;
		for (int i = 0; i < ui.tableWidget->rowCount(); i++)
		{
			QJsonObject SerialPort;
			SerialPort.insert("Data", ui.tableWidget->item(i, 0)->data(1).toString());
			SerialPort.insert("text", ui.tableWidget->item(i, 0)->text());
			IoArray.append(SerialPort);
		}
		strData.insert("Instrumentation", IoArray);
	}
	UpDateLst();
	return 0;
}

int frmInstrumentation::SetData(QJsonObject & strData)
{
	ClearConnect();
	ui.tableWidget->setRowCount(0);
	{	//串口通信
		QJsonObject SerialPort = strData.find("SerialPort").value().toObject();
		global_serialport_content.clear();
		for (QJsonObject::iterator iter = SerialPort.begin(); iter != SerialPort.end(); iter++)
		{
			SerialPort_Content							Var_SerialPort;
			QJsonArray array_object						= iter.value().toArray();
			Var_SerialPort.global_serial_port			= array_object.at(0).toString();
			Var_SerialPort.global_baud_rate				= array_object.at(1).toString().toInt();
			Var_SerialPort.global_check_digit			= (QSerialPort::Parity)array_object.at(2).toString().toInt();
			Var_SerialPort.global_data_bits				= (QSerialPort::DataBits)array_object.at(3).toString().toInt();
			Var_SerialPort.global_stop_bit				= (QSerialPort::StopBits)array_object.at(4).toString().toInt();
			Var_SerialPort.global_flowcontrol			= (QSerialPort::FlowControl)array_object.at(5).toString().toInt();
			Var_SerialPort.global_remark				= array_object.at(6).toString();
			global_serialport_content.insert(iter.key(), Var_SerialPort);
		}	
	}
	{	//TCP/IP服务器
		QJsonObject TcpServer							= strData.find("TcpContent").value().toObject();
		global_tcp_content.clear();
		for (QJsonObject::iterator iter = TcpServer.begin(); iter != TcpServer.end(); iter++)
		{
			Tcp_Content									Var_TcpServer;
			QJsonArray array_object						= iter.value().toArray();
			Var_TcpServer.global_protocol				= (EnumTCPType)array_object.at(0).toString().toInt();
			Var_TcpServer.global_ip						= array_object.at(1).toString();
			Var_TcpServer.global_port					= array_object.at(2).toString().toInt();
			Var_TcpServer.global_remark					= array_object.at(3).toString();
			global_tcp_content.insert(iter.key(), Var_TcpServer);
		}	
	}
	{	//Instrumentation
		QJsonArray IoArray = strData.find("Instrumentation").value().toArray();
		ClearInstruct();
		for (size_t i = 0; i < IoArray.count(); i++)
		{
			QJsonObject SerialPort = IoArray.at(i).toObject();

			QTableWidgetItem* item_type = new QTableWidgetItem();
			if (SerialPort.contains("Data"))	item_type->setData(1, SerialPort["Data"].toString());
			if (SerialPort.contains("text"))	item_type->setText(SerialPort["text"].toString());
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			int count = ui.tableWidget->rowCount();
			ui.tableWidget->setRowCount(count + 1);  //设置行数
			ui.tableWidget->setItem(count, 0, item_type);
		}
		UpDataInstrument();
	}
	UpDateLst();
	DisConnected();
	return 0;
}

int frmInstrumentation::NewProject()
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数

	return 0;
}

void frmInstrumentation::UpDataInstrument()
{
	ClearInstrument();
	bool _bFirst = true;
	for (QMap<QString, SerialPort_Content>::iterator iter = global_serialport_content.begin(); iter != global_serialport_content.end(); iter++)
	{
		gVariable::Instance().SerialPortVar.serialport_value			= new MyCnCommort(this);
		gVariable::Instance().SerialPortVar.portname_value				= iter.value().global_serial_port;
		gVariable::Instance().SerialPortVar.baudrate_value				= iter.value().global_baud_rate;
		gVariable::Instance().SerialPortVar.parity_value				= iter.value().global_check_digit;
		gVariable::Instance().SerialPortVar.databits_value				= iter.value().global_data_bits;
		gVariable::Instance().SerialPortVar.stopbits_value				= iter.value().global_stop_bit;
		gVariable::Instance().SerialPortVar.flowcontrol_value			= iter.value().global_flowcontrol;
		gVariable::Instance().SerialPortVar.remark_value				= iter.value().global_remark;
		
		if (dataVar::Instance().bAutoConnectedWhenStart)
		{
			s_port		= gVariable::Instance().SerialPortVar.serialport_value;
			//s_port->setPortName(gVariable::Instance().SerialPortVar.portname_value);		//串口号
			//s_port->setBaudRate(gVariable::Instance().SerialPortVar.baudrate_value);  //波特率
			//s_port->setParity(gVariable::Instance().SerialPortVar.parity_value);
			//s_port->setDataBits(gVariable::Instance().SerialPortVar.databits_value);
			//s_port->setStopBits(gVariable::Instance().SerialPortVar.stopbits_value);
			//s_port->setFlowControl(gVariable::Instance().SerialPortVar.flowcontrol_value);
			DWORD dwPort = gVariable::Instance().SerialPortVar.portname_value.mid(3).toInt();
			DWORD dwBaudRate = gVariable::Instance().SerialPortVar.baudrate_value;
			BYTE btParity = gVariable::Instance().SerialPortVar.parity_value;	//校验位
			switch (gVariable::Instance().SerialPortVar.parity_value)
			{
			case QSerialPort::NoParity:		btParity = NOPARITY;	break;
			case QSerialPort::EvenParity:	btParity = EVENPARITY;	break;
			case QSerialPort::OddParity:	btParity = ODDPARITY;	break;
			case QSerialPort::SpaceParity:	btParity = SPACEPARITY;	break;
			case QSerialPort::MarkParity:	btParity = MARKPARITY;	break;
			default:	break;
			}
			BYTE btByteSize = gVariable::Instance().SerialPortVar.databits_value;
			BYTE btStopBits = ONESTOPBIT;
			switch (gVariable::Instance().SerialPortVar.stopbits_value)
			{
			case QSerialPort::OneStop:			btStopBits = ONESTOPBIT;	break;
			case QSerialPort::OneAndHalfStop:	btStopBits = ONE5STOPBITS;	break;
			case QSerialPort::TwoStop:			btStopBits = TWOSTOPBITS;	break;
			default:	break;
			}
			connect(s_port,		&MyCnCommort::sig_readyRead, this, &frmInstrumentation::onSerialRecieveData, Qt::BlockingQueuedConnection);
			bool b_port_state = s_port->Open(dwPort, dwBaudRate, btParity, btByteSize, btStopBits);
			onSelectedConnectedTCP(b_port_state);
			//bool b_port_state = s_port->open(QIODevice::ReadWrite);
			//onSelectedConnectedTCP(b_port_state);
			//ui.frame_SerialPort->setEnabled(!b_port_state);
		}
		gVariable::Instance().serialport_variable_link.insert(std::pair<QString, gVariable::SerialPort_Var>(iter.key(), gVariable::Instance().SerialPortVar));
		if (_bFirst)
		{
			ui.comboSerialPort->setCurrentText(gVariable::Instance().SerialPortVar.portname_value);
			ui.comboBaudRate->setCurrentText(QString::number(gVariable::Instance().SerialPortVar.baudrate_value));
			ui.comboCheckDigit->setCurrentText(QString::number(gVariable::Instance().SerialPortVar.parity_value));
			ui.comboDataBits->setCurrentText(QString::number(gVariable::Instance().SerialPortVar.databits_value));
			ui.comboStopBit->setCurrentText(QString::number(gVariable::Instance().SerialPortVar.stopbits_value));
			ui.comboFlowControl->setCurrentText(QString::number(gVariable::Instance().SerialPortVar.flowcontrol_value));
			ui.txtSPRemark->setText(gVariable::Instance().SerialPortVar.remark_value);
			_bFirst = false;
		}
	}
	_bFirst = true;
	for (QMap<QString, Tcp_Content>::iterator iter = global_tcp_content.begin(); iter != global_tcp_content.end(); iter++)
	{
		if (iter.value().global_protocol == EnumTCPType_server)
		{
			gVariable::Instance().SocketTcpServerVar.server_value			= new CTcpServerSocket();
			gVariable::Instance().SocketTcpServerVar.protocol_value			= iter.value().global_protocol;
			gVariable::Instance().SocketTcpServerVar.ip_value				= iter.value().global_ip;
			gVariable::Instance().SocketTcpServerVar.port_value				= iter.value().global_port;
			//gVariable::Instance().SocketTcpServerVar.mit_over_time_value	= iter.value().global_timeout;
			gVariable::Instance().SocketTcpServerVar.remark_value			= iter.value().global_remark;

			if (dataVar::Instance().bAutoConnectedWhenStart)				{
				//gVariable::Instance().SocketTcpServerVar.server_value		= new MyServer(this);
				server														= gVariable::Instance().SocketTcpServerVar.server_value;
				server->InitServer((char*)gVariable::Instance().SocketTcpServerVar.ip_value.toStdString().c_str(), gVariable::Instance().SocketTcpServerVar.port_value);
				//server->listen(QHostAddress(gVariable::Instance().SocketTcpServerVar.ip_value), gVariable::Instance().SocketTcpServerVar.port_value);
				ui.ConnectedOBJ->clear();
				ui.ConnectedOBJ->addItem("AllConnected");
				server->QObject::disconnect(this);
				
				connect(server,		&CTcpServerSocket::sig_ConnectedSocket,			this,	&frmInstrumentation::onTcpServiceQConnected,	Qt::BlockingQueuedConnection);
				connect(server,		&CTcpServerSocket::sig_DisConnectedSocket,		this,	&frmInstrumentation::onTcpServiceQDisconnected, Qt::BlockingQueuedConnection);
				connect(server,		&CTcpServerSocket::sig_ReadyRead,				this,	&frmInstrumentation::onTcpDataReadyRead,		Qt::BlockingQueuedConnection);
			}
			gVariable::Instance().sockettcpserver_variable_link.insert(std::pair<QString, gVariable::SocketTcpServer_Var>(iter.key(), gVariable::Instance().SocketTcpServerVar));
			if (_bFirst)
			{
				ui.comboProtocol->setCurrentIndex(gVariable::Instance().SocketTcpServerVar.protocol_value);
				ui.widgetSocketIP->setIP(gVariable::Instance().SocketTcpServerVar.ip_value);
				ui.spinSocketPort->setValue(gVariable::Instance().SocketTcpServerVar.port_value);
				ui.txtSocketRemark->setText(gVariable::Instance().SocketTcpServerVar.remark_value);
				_bFirst = false;
			}
		}
		else
		{
			gVariable::Instance().SocketTcpClientVar.client_value			= new CTcpSocket();
			gVariable::Instance().SocketTcpClientVar.protocol_value			= iter.value().global_protocol;
			gVariable::Instance().SocketTcpClientVar.ip_value				= iter.value().global_ip;
			gVariable::Instance().SocketTcpClientVar.port_value				= iter.value().global_port;
			//gVariable::Instance().SocketTcpServerVar.mit_over_time_value	  = iter.value().global_timeout;
			gVariable::Instance().SocketTcpClientVar.remark_value			= iter.value().global_remark;
			if (dataVar::Instance().bAutoConnectedWhenStart)				{
				client														= gVariable::Instance().SocketTcpClientVar.client_value;
				client->QObject::disconnect(this);
				connect(client, &CTcpSocket::sig_ConnectedSocket,		this, &frmInstrumentation::onTcpQConnected,		Qt::BlockingQueuedConnection);
				connect(client, &CTcpSocket::sig_DisConnectedSocket,	this, &frmInstrumentation::onTcpQDisconnected,	Qt::BlockingQueuedConnection);
				connect(client, &CTcpSocket::sig_ReadyRead,				this, &frmInstrumentation::onTcpDataReadyRead,	Qt::BlockingQueuedConnection);
				client->ConnectToServer(gVariable::Instance().SocketTcpServerVar.ip_value.toStdString().c_str(), gVariable::Instance().SocketTcpServerVar.port_value);

			}
			gVariable::Instance().sockettcpclient_variable_link.insert(std::pair<QString, gVariable::SocketTcpClient_Var>(iter.key(), gVariable::Instance().SocketTcpClientVar));
			if (_bFirst)
			{
				ui.comboProtocol->setCurrentIndex(gVariable::Instance().SocketTcpClientVar.protocol_value);
				ui.widgetSocketIP->setIP(gVariable::Instance().SocketTcpClientVar.ip_value);
				ui.spinSocketPort->setValue(gVariable::Instance().SocketTcpClientVar.port_value);
				ui.txtSocketRemark->setText(gVariable::Instance().SocketTcpClientVar.remark_value);
				_bFirst = false;
			}
		}
	}
}

void frmInstrumentation::ClearInstrument()
{
	for (auto iter : gVariable::Instance().serialport_variable_link)
	{
		if (iter.second.serialport_value != nullptr)
		{
			iter.second.serialport_value->disconnect(this);
			if(iter.second.serialport_value->IsOpen())
				iter.second.serialport_value->ClearError();
			iter.second.serialport_value->Close();
			iter.second.serialport_value->deleteLater();
		}
	}
	gVariable::Instance().serialport_variable_link.clear();

	for (auto iter : gVariable::Instance().sockettcpserver_variable_link)
	{
		if (iter.second.server_value != nullptr)
		{
			iter.second.server_value->QObject::disconnect(this);
			iter.second.server_value->Close();
		}
	}
	gVariable::Instance().sockettcpserver_variable_link.clear();

	for (auto iter : gVariable::Instance().sockettcpclient_variable_link)
	{
		if (iter.second.client_value != nullptr)
		{
			iter.second.client_value->QObject::disconnect(this);
			iter.second.client_value->Close();
		}
	}
	gVariable::Instance().sockettcpclient_variable_link.clear();

}

void frmInstrumentation::UpDateLst()
{
	gVariable::Instance().m_vecNames.clear();
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)	{
		gVariable::Instance().m_vecNames.push_back(ui.tableWidget->item(i,0)->text());
	}
}

void frmInstrumentation::DisConnected()
{
	for (auto iter : gVariable::Instance().serialport_variable_link)
	{
		if (iter.second.serialport_value != nullptr)
		{
			iter.second.serialport_value->disconnect(this);
		}
	}

	for (auto iter : gVariable::Instance().sockettcpserver_variable_link)
	{
		if (iter.second.server_value != nullptr)
		{
			iter.second.server_value->QObject::disconnect(this);
		}
	}

	for (auto iter : gVariable::Instance().sockettcpclient_variable_link)
	{
		if (iter.second.client_value != nullptr)
		{
			iter.second.client_value->QObject::disconnect(this);
		}
	}
}

void frmInstrumentation::initTitleBar()
{
}

void frmInstrumentation::ClearConnect()
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数
}

void frmInstrumentation::onButtonCloseClicked()
{
	for (auto iter : gVariable::Instance().sockettcpclient_variable_link)
		if (iter.second.client_value != nullptr) iter.second.client_value->QObject::disconnect(this);
	
	for (auto iter : gVariable::Instance().sockettcpserver_variable_link)
		if (iter.second.server_value != nullptr) iter.second.server_value->QObject::disconnect(this);
	
	for (auto iter : gVariable::Instance().serialport_variable_link)
		if (iter.second.serialport_value != nullptr) iter.second.serialport_value->QObject::disconnect(this);

	this->close();
}

void frmInstrumentation::on_comboProtocol_currentIndexChanged(int index)
{
	switch (index)
	{
	case 0:
		ui.lbl_ip->setText(tr("本地IP地址："));
		ui.lbl_port->setText(tr("本地端口号："));
		break;
	case 1:
		ui.lbl_ip->setText(tr("远程IP地址："));
		ui.lbl_port->setText(tr("远程端口号："));
		break;
	}
}

void frmInstrumentation::on_btnSendMsg_clicked()
{
	if (ui.lblType->text() == QString())
	{
		dataVar::Instance().pManager->notify(tr("选择仪器通讯名称或编号为空！"), NotifyType_Error);
		return;
	}
	QStringList strs = ui.lblType->text().split("_");
	if (strs[0] == "SerialPort")
	{
		comm_keys.clear();
		comm_keys = global_serialport_content.uniqueKeys();
		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == ui.lblType->text())
			{
				count = 1;
				ui.comboSerialPort->setCurrentText(global_serialport_content.value(key).global_serial_port);
				ui.comboBaudRate->setCurrentText(QString::number(global_serialport_content.value(key).global_baud_rate));
				ui.comboCheckDigit->setCurrentText(QString::number(global_serialport_content.value(key).global_check_digit));
				ui.comboDataBits->setCurrentText(QString::number(global_serialport_content.value(key).global_data_bits));
				ui.comboStopBit->setCurrentText(QString::number(global_serialport_content.value(key).global_stop_bit));
				ui.comboFlowControl->setCurrentText(QString::number(global_serialport_content.value(key).global_flowcontrol));
				ui.txtSPRemark->setText(global_serialport_content.value(key).global_remark);

			}
		}
		if (count == 0)
		{
			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
			return;
		}
		QString strTcpkey;
		for (int k = 0; k < comm_keys.length(); k++)
			if (comm_keys[k] == ui.lblType->text())
				strTcpkey = ui.lblType->text();

		if (strTcpkey.isEmpty() || gVariable::Instance().serialport_variable_link.count(strTcpkey) <= 0)
		{
			dataVar::Instance().pManager->notify("警告", ui.lblType->text() + "该仪器通讯参数未保存！",NotifyType_Warn);
			return;
		}

		s_port = gVariable::Instance().serialport_variable_link.at(strTcpkey).serialport_value;
		if (s_port == nullptr)
		{
			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("客户端未连接！"), NotifyType_Error);
			return;
		}
		if (!s_port->IsOpen())
		{
			dataVar::Instance().pManager->notify(	ui.lblType->text() + tr("客户端未连接！"), NotifyType_Error);
			return;
		}
		QString	strData = ui.textEdit_Send->toPlainText();
		s_port->Write(strData);
	}
	else if (strs[0] == "EthernetTcp")
	{
		//TCP/IP通信
		comm_keys.clear();
		comm_keys = global_tcp_content.uniqueKeys();
		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == ui.lblType->text())
			{
				count = 1;
				TcpContent.global_protocol = global_tcp_content.value(key).global_protocol;
				ui.comboProtocol->setCurrentIndex(TcpContent.global_protocol);
				ui.widgetSocketIP->setIP(global_tcp_content.value(key).global_ip);
				ui.spinSocketPort->setValue(global_tcp_content.value(key).global_port);
				ui.txtSocketRemark->setText(global_tcp_content.value(key).global_remark);
			}
		}
		if (count == 0)
		{
			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
			return;
		}
		if (TcpContent.global_protocol == EnumTCPType_server)
		{
			comm_keys.clear();
			comm_keys = global_tcp_content.uniqueKeys();
			QString strTcpkey;
			for (int k = 0; k < comm_keys.length(); k++)
				if (comm_keys[k] == ui.lblType->text())
					strTcpkey = ui.lblType->text();
			if (strTcpkey.isEmpty() || gVariable::Instance().sockettcpserver_variable_link.count(strTcpkey) <= 0)
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("未找到参数！"), NotifyType_Error);
				return;
			}
			server = gVariable::Instance().sockettcpserver_variable_link.at(strTcpkey).server_value;
			if (server == nullptr)
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("服务器未连接！"), NotifyType_Error);
				return;
			}
			if (!server->isListening())
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("服务器未连接！"), NotifyType_Error);
				return;
			}
			QString	strData = ui.textEdit_Send->toPlainText();
			if (ui.ConnectedOBJ->currentIndex() == 0)
			{
				for (auto item : server->m_vecCTcpClient)
					if (item->m_Client != nullptr) item->m_Client->write(strData);
			}
			else
			{
				int Index = ui.ConnectedOBJ->currentIndex() - 1;
				if (Index >= 0 )
				{
					server->m_vecCTcpClient[Index]->m_Client->write(strData);
				}
			}
		}
		else
		{
			comm_keys.clear();
			comm_keys = global_tcp_content.uniqueKeys();
			QString strTcpkey;
			for (int k = 0; k < comm_keys.length(); k++)
				if (comm_keys[k] == ui.lblType->text())
					strTcpkey = ui.lblType->text();
			if (strTcpkey.isEmpty() || gVariable::Instance().sockettcpclient_variable_link.count(strTcpkey) <= 0)
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("未找到参数！"), NotifyType_Error);

				return;
			}
			client = (CTcpSocket*)gVariable::Instance().sockettcpclient_variable_link.at(strTcpkey).client_value;
			if (client == nullptr)
			{
				dataVar::Instance().pManager->notify(ui.lblType->text() + tr("客户端未连接！"), NotifyType_Error);
				return;
			}
			if (!client->isOpen())
			{
				dataVar::Instance().pManager->notify(ui.lblType->text() + tr("客户端未连接！"), NotifyType_Error);
				return;
			}
			QString	strData = ui.textEdit_Send->toPlainText();
			client->write(strData);
		}
	}
}

void frmInstrumentation::slot_CustomContextMenuRequested(const QPoint & pos)
{
	QMenu*	m_menu = new QMenu(this);
	QPushButton* moveUpAction_buf = new QPushButton(m_menu);
	moveUpAction_buf->setObjectName("MoveUp");
	moveUpAction_buf->setText(tr("上移"));
	moveUpAction_buf->setIconSize(QSize(20, 20));
	moveUpAction_buf->setMinimumSize(QSize(120, 40));
	//moveUpAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/previous.png"));
	QWidgetAction* moveUpAction = new QWidgetAction(this);
	moveUpAction->setDefaultWidget(moveUpAction_buf);
	connect(moveUpAction_buf, &QPushButton::clicked, this, &frmInstrumentation::slot_MoveUp);
	connect(moveUpAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveUpAction);
	QPushButton* moveDownAction_buf = new QPushButton(m_menu);
	moveDownAction_buf->setObjectName("MoveDown");
	moveDownAction_buf->setText(tr("下移"));
	moveDownAction_buf->setIconSize(QSize(20, 20));
	moveDownAction_buf->setMinimumSize(QSize(120, 40));
	//moveDownAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/next.png"));
	QWidgetAction* moveDownAction = new QWidgetAction(this);
	moveDownAction->setDefaultWidget(moveDownAction_buf);
	connect(moveDownAction_buf, &QPushButton::clicked, this, &frmInstrumentation::slot_MoveDown);
	connect(moveDownAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveDownAction);
	QPushButton* deleteNameAction_buf = new QPushButton(m_menu);
	deleteNameAction_buf->setObjectName("Delete");
	deleteNameAction_buf->setText(tr("Delete"));
	deleteNameAction_buf->setIconSize(QSize(21, 21));
	deleteNameAction_buf->setMinimumSize(QSize(120, 40));
	//deleteNameAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/del.png"));
	QWidgetAction* deleteNameAction = new QWidgetAction(this);
	deleteNameAction->setDefaultWidget(deleteNameAction_buf);
	connect(deleteNameAction_buf, &QPushButton::clicked, this, &frmInstrumentation::slot_DeleteName);
	connect(deleteNameAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(deleteNameAction);
	m_menu->exec(QCursor::pos());

	if (moveUpAction_buf != nullptr)		delete moveUpAction_buf;		moveUpAction_buf = nullptr;
	if (moveUpAction != nullptr)			delete moveUpAction;			moveUpAction = nullptr;
	if (moveDownAction_buf != nullptr)		delete moveDownAction_buf;		moveDownAction_buf = nullptr;
	if (moveDownAction != nullptr)			delete moveDownAction;			moveDownAction = nullptr;
	if (deleteNameAction_buf != nullptr)	delete deleteNameAction_buf;	deleteNameAction_buf = nullptr;
	if (deleteNameAction != nullptr)		delete deleteNameAction;		deleteNameAction = nullptr;
	if (m_menu != nullptr)					delete m_menu;					m_menu = nullptr;
}

void frmInstrumentation::on_BtnClearRecieve_clicked()
{
	ui.textEdit_Recieve->clear();
}

void frmInstrumentation::on_BtnSend_clicked()
{
	ui.textEdit_Send->clear();
}

void frmInstrumentation::on_btnAdd_clicked()
{
	int count = ui.tableWidget->rowCount();
	QVector<QString> all_instrument = QVector<QString>(300);
	all_instrument.clear();
	for (int i = 0; i < count; i++)		all_instrument.append(ui.tableWidget->item(i, 0)->data(1).toString());

	QVector<int> all_instrument_max = QVector<int>(100);
	int value = 0;
	QTableWidgetItem* item_type;
	QString	strType;
	all_instrument_max.clear();
	switch (ui.comboInstrument->currentIndex()) {
	case 0:		{	
		for (int i = 0; i < count; i++)			{
			QStringList strs;
			strs.reserve(10);
			strs.clear();
			strs = all_instrument[i].split("_");
			if (strs[0] == "SerialPort")				{
				all_instrument_max.append(strs[1].toInt());
			}
		}
		if (all_instrument_max.size() != 0)			{
			qSort(all_instrument_max.begin(), all_instrument_max.end()); //排序
			value = GetValue(all_instrument_max);
		}
		ui.tableWidget->setRowCount(count + 1);  //设置行数
		if (value == 0 || all_instrument_max[0] > 1)			{
			value = 1;
		}

		strType = "SerialPort_" + QString::number(value);
		if (ui.lblType->text().isEmpty())	ui.lblType->setText(strType);
		item_type = new QTableWidgetItem(strType);
		item_type->setData(1, strType);
		{	//串口
			SerialPort_Content			searial;
			searial.global_serial_port = ui.comboSerialPort->currentText();
			searial.global_baud_rate = ui.comboBaudRate->currentText().toInt();
			searial.global_check_digit = (QSerialPort::Parity)ui.comboCheckDigit->currentText().toInt();
			searial.global_data_bits = (QSerialPort::DataBits)ui.comboDataBits->currentText().toInt();
			searial.global_stop_bit = (QSerialPort::StopBits)ui.comboStopBit->currentText().toInt();
			searial.global_flowcontrol = (QSerialPort::FlowControl)ui.comboFlowControl->currentText().toInt();
			searial.global_remark = ui.txtSPRemark->text();
			global_serialport_content.insert(strType, searial);
			SerialPortContent = searial;
		}
		item_type->setTextAlignment(Qt::AlignCenter);
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 0, item_type);
	}	break;
	case 1:
		for (int i = 0; i < count; i++)			{
			QStringList strs;
			strs.reserve(10);
			strs.clear();
			strs = all_instrument[i].split("_");
			if (strs[0] == "EthernetTcp")	all_instrument_max.append(strs[1].toInt());
		}
		if (all_instrument_max.size() != 0)
		{
			qSort(all_instrument_max.begin(), all_instrument_max.end()); //排序
			value = GetValue(all_instrument_max);
		}
		ui.tableWidget->setRowCount(count + 1);  //设置行数
		if (value == 0 || all_instrument_max[0] > 1)	{	value = 1;	}
		strType = + "EthernetTcp_" + QString::number(value);
		if (ui.lblType->text().isEmpty())	ui.lblType->setText(strType);
		item_type = new QTableWidgetItem(strType);
		item_type->setData(1, strType);

		{	//TCP/IP通信	客户端
			Tcp_Content	TCP;
			TCP.global_protocol		= (EnumTCPType)ui.comboProtocol->currentIndex();
			TCP.global_ip			= ui.widgetSocketIP->getIP();
			TCP.global_port			= ui.spinSocketPort->value();
			TCP.global_remark		= ui.txtSocketRemark->text();
			TcpContent = TCP;
			global_tcp_content.insert(strType, TCP);
		}
		item_type->setTextAlignment(Qt::AlignCenter);
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 0, item_type);
		break;
	}
}

int frmInstrumentation::GetValue(QVector<int> n)
{
	int low = 0;
	int high = n.count() - 1;
	while (low < high)
	{
		int mid = (low + high) / 2;
		if (n[mid] > n[0] + mid) high = mid;
		else low = mid;
		if (high - low == 1)
		{
			if (n[high] - n[low] == 1) low = high;
			break;
		}
	}
	return n[low] + 1;
}

void frmInstrumentation::on_btnConnect_clicked()
{
	//连接仪器
	if (ui.lblType->text() == QString())
	{
		dataVar::Instance().pManager->notify(ui.lblType->text() + tr("选择仪器通讯名称或编号为空！"), NotifyType_Error);
		return;
	}
	QStringList strs = ui.lblType->text().split("_");
	if (strs[0] == "SerialPort")
	{
		//串口通信
		comm_keys.clear();
		comm_keys = global_serialport_content.uniqueKeys();
		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == ui.lblType->text())
			{
				count = 1;
				ui.comboSerialPort->setCurrentText(global_serialport_content.value(key).global_serial_port);
				ui.comboBaudRate->setCurrentText(QString::number(global_serialport_content.value(key).global_baud_rate));
				ui.comboCheckDigit->setCurrentText(QString::number(global_serialport_content.value(key).global_check_digit));
				ui.comboDataBits->setCurrentText(QString::number(global_serialport_content.value(key).global_data_bits));
				ui.comboStopBit->setCurrentText(QString::number(global_serialport_content.value(key).global_stop_bit));
				ui.comboFlowControl->setCurrentText(QString::number(global_serialport_content.value(key).global_flowcontrol));
				ui.txtSPRemark->setText(QString(global_serialport_content.value(key).global_remark));
			}
		}
		if (count == 0)
		{
			dataVar::Instance().pManager->notify(	ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
			return;
		}
		QString strTcpkey;
		for (int k = 0; k < comm_keys.length(); k++)
			if (comm_keys[k] == ui.lblType->text())
				strTcpkey = ui.lblType->text();
		if (strTcpkey.isEmpty())
		{
			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("服务器未找到！"), NotifyType_Error);
			return;
		}
		bool server_state = false;
		bool bNeedConnected = false;
		if (gVariable::Instance().serialport_variable_link.count(strTcpkey) <= 0)
		{
			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);

			return;
		}

		s_port = gVariable::Instance().serialport_variable_link.at(strTcpkey).serialport_value;
		if (s_port != nullptr)
		{
			if (s_port->IsOpen() == true)
			{
				//断开
				s_port->Close();
				s_port->deleteLater();
				delete s_port;
				s_port = nullptr;
				bNeedConnected = false;
			}
			else
			{		//连接
				bNeedConnected = true;
			}
		}
		else
		{
			s_port = new MyCnCommort(this);
			bNeedConnected = true;
		}
		QString	strCom = ui.comboSerialPort->currentText();
		int iBaudRate = ui.comboBaudRate->currentText().toInt();
		QSerialPort::Parity parity;   //校验位
		QSerialPort::DataBits dataBits;  //数据位
		QSerialPort::StopBits stopBits;  //停止位
		QSerialPort::FlowControl flowControl;  //流控制
		{
			if (ui.comboCheckDigit->currentText() == "无")
			{
				parity = QSerialPort::NoParity;
			}
			else if (ui.comboCheckDigit->currentText() == "奇")
			{
				parity = QSerialPort::Parity::OddParity;
			}
			else if (ui.comboCheckDigit->currentText() == "偶")
			{
				parity = QSerialPort::Parity::EvenParity;
			}
			else if (ui.comboCheckDigit->currentText() == "标志")
			{
				parity = QSerialPort::Parity::MarkParity;
			}
			else if (ui.comboCheckDigit->currentText() == "空格")
			{
				parity = QSerialPort::Parity::SpaceParity;
			}
			if (ui.comboDataBits->currentText() == "5")
			{
				dataBits = QSerialPort::Data5;
			}
			else if (ui.comboDataBits->currentText() == "6")
			{
				dataBits = QSerialPort::Data6;
			}
			else if (ui.comboDataBits->currentText() == "7")
			{
				dataBits = QSerialPort::Data7;
			}
			else if (ui.comboDataBits->currentText() == "8")
			{
				dataBits = QSerialPort::Data8;
			}
			if (ui.comboStopBit->currentText() == "1")
			{
				stopBits = QSerialPort::OneStop;
			}
			else if (ui.comboStopBit->currentText() == "1.5")
			{
				stopBits = QSerialPort::OneAndHalfStop;
			}
			else if (ui.comboStopBit->currentText() == "2")
			{
				stopBits = QSerialPort::TwoStop;
			}
			if (ui.comboFlowControl->currentText() == "无")
			{
				flowControl = QSerialPort::FlowControl::NoFlowControl;
			}
			else if (ui.comboFlowControl->currentText() == "硬件")
			{
				flowControl = QSerialPort::FlowControl::HardwareControl;
			}
			else if (ui.comboFlowControl->currentText() == "软件")
			{
				flowControl = QSerialPort::FlowControl::SoftwareControl;
			}
		}
		{	gVariable::Instance().SerialPortVar.portname_value		= strCom;
			gVariable::Instance().SerialPortVar.baudrate_value		= iBaudRate;
			gVariable::Instance().SerialPortVar.parity_value		= parity;
			gVariable::Instance().SerialPortVar.databits_value		= dataBits;
			gVariable::Instance().SerialPortVar.stopbits_value		= stopBits;
			gVariable::Instance().SerialPortVar.flowcontrol_value	= flowControl;
			gVariable::Instance().SerialPortVar.serialport_value	= s_port;
			gVariable::Instance().SerialPortVar.remark_value		= ui.txtSocketRemark->text();
			if (gVariable::Instance().serialport_variable_link.count(ui.lblType->text()) > 0)
				gVariable::Instance().serialport_variable_link.erase(ui.lblType->text());
			gVariable::Instance().serialport_variable_link.insert(std::pair<QString, gVariable::SerialPort_Var>(ui.lblType->text(), gVariable::Instance().SerialPortVar));
		}
		bool b_port_state = false;
		if (bNeedConnected)
		{
			DWORD dwPort		= gVariable::Instance().SerialPortVar.portname_value.mid(3).toInt();
			DWORD dwBaudRate	= gVariable::Instance().SerialPortVar.baudrate_value;
			BYTE btParity		= gVariable::Instance().SerialPortVar.parity_value;	//校验位
			switch (gVariable::Instance().SerialPortVar.parity_value)
			{
			case QSerialPort::NoParity:		btParity = NOPARITY;	break;
			case QSerialPort::EvenParity:	btParity = EVENPARITY;	break;
			case QSerialPort::OddParity:	btParity = ODDPARITY;	break;
			case QSerialPort::SpaceParity:	btParity = SPACEPARITY;	break;
			case QSerialPort::MarkParity:	btParity = MARKPARITY;	break;
			default:	break;
			}
			BYTE btByteSize		= gVariable::Instance().SerialPortVar.databits_value;
			BYTE btStopBits		= ONESTOPBIT;
			switch (gVariable::Instance().SerialPortVar.stopbits_value)
			{
			case QSerialPort::OneStop:			btStopBits = ONESTOPBIT;	break;
			case QSerialPort::OneAndHalfStop:	btStopBits = ONE5STOPBITS;	break;
			case QSerialPort::TwoStop:			btStopBits = TWOSTOPBITS;	break;
			default:	break;
			}
			bool b_port_state	= s_port->Open(dwPort, dwBaudRate, btParity, btByteSize, btStopBits);

			s_port->QObject::disconnect(this);
			onSelectedConnectedTCP(b_port_state);
			ui.frame_SerialPort->setEnabled(!b_port_state);
			if (b_port_state == true)
			{
				connect(s_port, &MyCnCommort::sig_readyRead, this, &frmInstrumentation::onSerialRecieveData, Qt::BlockingQueuedConnection);

			}
			else
			{
				s_port->QObject::disconnect(this);
				delete s_port;
				s_port = nullptr;
				gVariable::Instance().SerialPortVar.serialport_value = s_port;
				if (gVariable::Instance().serialport_variable_link.count(ui.lblType->text()) > 0)
					gVariable::Instance().serialport_variable_link.erase(ui.lblType->text());
				gVariable::Instance().serialport_variable_link.insert(std::pair<QString, gVariable::SerialPort_Var>(ui.lblType->text(), gVariable::Instance().SerialPortVar));

				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("串口打开失败！"), NotifyType_Error);

				return;
			}
		}
		else
		{
			ui.frame_SerialPort->setEnabled(!b_port_state);
			onSelectedConnectedTCP(b_port_state);
		}
	}
	else if (strs[0] == "EthernetTcp")
	{
		//TCP/IP通信
		comm_keys.clear();
		comm_keys = global_tcp_content.uniqueKeys();
		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == ui.lblType->text())
			{
				count = 1;
				TcpContent.global_protocol			= global_tcp_content.value(key).global_protocol;
				ui.comboProtocol->setCurrentIndex(TcpContent.global_protocol);
				ui.widgetSocketIP->setIP(global_tcp_content.value(key).global_ip);
				ui.spinSocketPort->setValue(global_tcp_content.value(key).global_port);
				ui.txtSocketRemark->setText(global_tcp_content.value(key).global_remark);
			}
		}
		if (count == 0)
		{

			dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
			return;
		}
		if (TcpContent.global_protocol == EnumTCPType_server)
		{
			QString strTcpkey;
			for (int k = 0; k < comm_keys.length(); k++)
				if (comm_keys[k] == ui.lblType->text())
					strTcpkey = ui.lblType->text();
			if (strTcpkey.isEmpty())
			{

				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("服务器未找到！"), NotifyType_Error);

				return;
			}
			bool server_state = false;
			bool bNeedConnected = false;
			if (gVariable::Instance().sockettcpserver_variable_link.count(strTcpkey) <= 0)
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
				return;
			}

			server = gVariable::Instance().sockettcpserver_variable_link.at(strTcpkey).server_value;
			if (server != nullptr)
			{
				if (server->isListening() == true)
				{		
					//断开
					//遍历所有数据
					for(auto tcpSocket : server->m_vecCTcpClient)
					{
						if (tcpSocket->m_Client != nullptr)
						{
							if (tcpSocket->m_Client->isOpen())
								tcpSocket->m_Client->Close();//关闭连接
						}
					}
					server->Close();
					bNeedConnected = false;
				}
				else
				{		//连接
					bNeedConnected = true;
				}
			}
			else
			{
				server = new CTcpServerSocket(this);
				bNeedConnected = true;
			}
			{
				gVariable::Instance().SocketTcpServerVar.server_value					= server;
				gVariable::Instance().SocketTcpServerVar.connect_state					= server_state;
				gVariable::Instance().SocketTcpServerVar.protocol_value					= ui.comboProtocol->currentIndex();
				gVariable::Instance().SocketTcpServerVar.ip_value						= ui.widgetSocketIP->getIP();
				gVariable::Instance().SocketTcpServerVar.port_value						= ui.spinSocketPort->value();
				gVariable::Instance().SocketTcpServerVar.remark_value					= ui.txtSocketRemark->text();

				if (gVariable::Instance().sockettcpserver_variable_link.count(ui.lblType->text()) > 0)
					gVariable::Instance().sockettcpserver_variable_link.erase(ui.lblType->text());
				gVariable::Instance().sockettcpserver_variable_link.insert(std::pair<QString, gVariable::SocketTcpServer_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpServerVar));
			}
			if (bNeedConnected)
			{
				ui.ConnectedOBJ->clear();
				ui.ConnectedOBJ->addItem("AllConnected");
				ui.ConnectedOBJ->setCurrentIndex(0);
				server->QObject::disconnect(this);
				connect(server,		&CTcpServerSocket::sig_ConnectedSocket,			this,	&frmInstrumentation::onTcpServiceQConnected,	Qt::BlockingQueuedConnection);
				connect(server,		&CTcpServerSocket::sig_DisConnectedSocket,		this,	&frmInstrumentation::onTcpServiceQDisconnected, Qt::BlockingQueuedConnection);
				connect(server,		&CTcpServerSocket::sig_ReadyRead,				this,	&frmInstrumentation::onTcpDataReadyRead,		Qt::BlockingQueuedConnection);

				server_state = server->InitServer((char*)ui.widgetSocketIP->getIP().toStdString().c_str(), ui.spinSocketPort->value());
				onSelectedConnectedTCP(server_state);
				if (server_state == true)
				{
				}
				else
				{
					server->QObject::disconnect(this);
					delete server;
					server = nullptr;
					gVariable::Instance().SocketTcpServerVar.server_value = server;
					if (gVariable::Instance().sockettcpserver_variable_link.count(ui.lblType->text()) > 0)
						gVariable::Instance().sockettcpserver_variable_link.erase(ui.lblType->text());
					gVariable::Instance().sockettcpserver_variable_link.insert(std::pair<QString, gVariable::SocketTcpServer_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpServerVar));


					dataVar::Instance().pManager->notify( ui.lblType->text() + tr("服务器打开失败！"), NotifyType_Error);
					return;
				}
			}
			onSelectedConnectedTCP(server_state);
		}
		else
		{
			//ui.WidgetCommunication->setEnabled(false);
			ui.ConnectedOBJ->setVisible(false);
			ui.label_ConnectedOBJ->setVisible(false);

			comm_keys.clear();
			comm_keys = global_tcp_content.uniqueKeys();
			QString strTcpkey;
			for (int k = 0; k < comm_keys.length(); k++)
				if (comm_keys[k] == ui.lblType->text())
					strTcpkey = ui.lblType->text();
			if (strTcpkey.isEmpty())
			{

				dataVar::Instance().pManager->notify(ui.lblType->text() + tr("未找到参数！"), NotifyType_Error);
				return;
			}
			bool _bNeedConnected	= false;
			bool _bclient_connected = false;
			if (gVariable::Instance().sockettcpclient_variable_link.count(strTcpkey) <= 0)
			{
				dataVar::Instance().pManager->notify( ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
				return;
			}
			client = (CTcpSocket*)gVariable::Instance().sockettcpclient_variable_link.at(strTcpkey).client_value;
			if (client != nullptr)
			{
				if (gVariable::Instance().sockettcpclient_variable_link.at(strTcpkey).client_value->m_bConnected)
				{		//断开
					_bNeedConnected = false;
					//client->disconnected();
					client->Close();
				}
				else {	_bNeedConnected = true;		}
			}
			else
			{
				client				= new CTcpSocket();
				client->m_strKey	= strTcpkey;
				_bNeedConnected = true;
			}

			{	gVariable::Instance().SocketTcpClientVar.client_value		= client;
				gVariable::Instance().SocketTcpClientVar.connect_state		= _bclient_connected;
				gVariable::Instance().SocketTcpClientVar.protocol_value		= ui.comboProtocol->currentIndex();
				gVariable::Instance().SocketTcpClientVar.ip_value			= ui.widgetSocketIP->getIP();
				gVariable::Instance().SocketTcpClientVar.port_value			= ui.spinSocketPort->value();
				gVariable::Instance().SocketTcpClientVar.remark_value		= ui.txtSocketRemark->text();
				if(gVariable::Instance().sockettcpclient_variable_link.count(ui.lblType->text()) > 0)
					gVariable::Instance().sockettcpclient_variable_link.erase(ui.lblType->text());
				gVariable::Instance().sockettcpclient_variable_link.insert(std::pair<QString, gVariable::SocketTcpClient_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpClientVar));
			}
			client->m_strKey = strTcpkey;
			if (_bNeedConnected)
			{
				client->QObject::disconnect(this);
				connect(client, &CTcpSocket::sig_ConnectedSocket,		this, &frmInstrumentation::onTcpQConnected, Qt::BlockingQueuedConnection);
				connect(client, &CTcpSocket::sig_DisConnectedSocket,	this, &frmInstrumentation::onTcpQDisconnected, Qt::BlockingQueuedConnection);
				connect(client, &CTcpSocket::sig_ReadyRead,				this, &frmInstrumentation::onTcpDataReadyRead, Qt::BlockingQueuedConnection);
			
				ui.ConnectedOBJ->clear();
				ui.ConnectedOBJ->addItem("AllConnected");
				ui.ConnectedOBJ->setCurrentIndex(0);

				_bclient_connected = client->ConnectToServer((char*)ui.widgetSocketIP->getIP().toStdString().c_str(), ui.spinSocketPort->value());
				onSelectedConnectedTCP(_bclient_connected);
				if (_bclient_connected == false)
				{
					client->QObject::disconnect(this);
					delete client;	
					client = nullptr;
					gVariable::Instance().SocketTcpClientVar.client_value = client;
					//gVariable::Instance().sockettcpclient_variable_link.insert(ui.lblType->text(), gVariable::Instance().SocketTcpClientVar);
					if (gVariable::Instance().sockettcpclient_variable_link.count(ui.lblType->text()) > 0)
						gVariable::Instance().sockettcpclient_variable_link.erase(ui.lblType->text());
					gVariable::Instance().sockettcpclient_variable_link.insert(std::pair<QString, gVariable::SocketTcpClient_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpClientVar));

					dataVar::Instance().pManager->notify( ui.lblType->text() + tr("客户端连接失败！"), NotifyType_Error);
				}
			}
			else
			{
				onSelectedConnectedTCP(_bclient_connected);
			}
			return;
		}
	}
}

void frmInstrumentation::onTcpServiceQConnected(QString strkey, CTcpSocket * socket)
{
	try
	{
		if (socket != nullptr)
		{
			ui.ConnectedOBJ->addItem(QString("%1:%2").arg(socket->localAddress()).arg(QString::number(socket->localPort())));
			QString	strClientMsg = QString("%1:%2 Connected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
			RecieveInstructMessage(strClientMsg);
		}
	}
	catch (...) {}
}

void frmInstrumentation::onTcpServiceQDisconnected(QString strkey, CTcpSocket * socket)
{
	try
	{
		if (socket != nullptr)
		{
			QString str = QString("%1:%2").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
			for (size_t i = 0; i < ui.ConnectedOBJ->count(); i++)
			{
				if (str == ui.ConnectedOBJ->itemText(i))
				{
					ui.ConnectedOBJ->removeItem(i);	break;
				}
			}
			QString	strClientMsg = QString("%1:%2 DisConnected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
			RecieveInstructMessage(strClientMsg);

			disconnect(socket);
		}
	}
	catch (...) {}
}

void frmInstrumentation::onTcpQConnected(QString strkey, CTcpSocket * socket)
{
		try
		{
			if (socket != nullptr)
			{
				if (gVariable::Instance().sockettcpclient_variable_link.count(strkey))
				{
					ComSocket*	_client_value = (ComSocket*)gVariable::Instance().sockettcpclient_variable_link.at(strkey).client_value;
					if (_client_value != nullptr)
						if(strkey == ui.lblType->text())
							onSelectedConnectedTCP(_client_value->m_bIsConnected);
				}
	
				QString	strClientMsg = QString("%1:%2 Connected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
				RecieveInstructMessage(strClientMsg);
			}
		}
		catch (...) {}
}

void frmInstrumentation::onTcpQDisconnected(QString strkey, CTcpSocket * socket)
{
		try
		{
			if (socket != nullptr)
			{
				if (gVariable::Instance().sockettcpclient_variable_link.count(strkey))
				{
					ComSocket*	_client_value = (ComSocket*)gVariable::Instance().sockettcpclient_variable_link.at(strkey).client_value;
					if (_client_value != nullptr)
						if (strkey == ui.lblType->text())
							onSelectedConnectedTCP(_client_value->m_bIsConnected);
				}
				QString	strClientMsg = QString("%1:%2 DisConnected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
				RecieveInstructMessage(strClientMsg);

				disconnect(socket);
			}
		}
		catch (...) {}
}

void frmInstrumentation::slotInstrumentChanged(int index)
{
	switch (index) {
	//case 0: {
	//	ui.stackedWidget->setCurrentIndex(0);
	//}	break;
	//case 1: {
	//	ui.stackedWidget->setCurrentIndex(1);
	//}	break;
	case 0: {
		ui.stackedWidget->setCurrentIndex(2);
	}	break;
	case 1: {
		ui.stackedWidget->setCurrentIndex(3);
	}	break;
	}
}

void frmInstrumentation::onTcpDataReadyRead(CTcpSocket * socket, QString & str)
{
	if (socket != nullptr)	{
		QString	strClient	= QString("%1:%2").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
		if (ui.ConnectedOBJ->currentIndex()	== 0 ) {}
		else if (ui.ConnectedOBJ->currentText() != strClient) { return; }

		QString	strClientMsg = QString("%1:%2 %3").arg(socket->localAddress()).arg(QString::number(socket->localPort())).arg(str);
		RecieveInstructMessage(strClientMsg);
	}
}

void frmInstrumentation::onTcpReadyRead(CTcpSocket* socket)
{

}

void frmInstrumentation::onTcpClientConnected(CTcpSocket* socket)
{

}

void frmInstrumentation::onTcpDisconnected(CTcpSocket* socket)
{

}

void frmInstrumentation::onTcpServiceReadyRead(CTcpSocket* socket)
{

}

void frmInstrumentation::onSerialRecieveData(MyCnCommort* socket, QString& str)
{
	if (socket != nullptr)
	{
		QString	strClientMsg = QString("COM%1: %2").arg(QString::number(socket->GetPort())).arg(str);
		RecieveInstructMessage(strClientMsg);
	}
}

void frmInstrumentation::onSelectedConnectedTCP(bool bOn)
{
	if (bOn)	{	ui.btnConnect->setIcon(QIcon(":/image/icon/images/GreenDot.png"));	}
	else		{	ui.btnConnect->setIcon(QIcon(":/image/icon/images/RedDot.png"));	}
}

void frmInstrumentation::UpDataSerialLst()
{
	ui.comboSerialPort->clear();
	QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
	//将所有可以使用的串口设备添加到ComboBox中
	for (const QSerialPortInfo &serialPortInfo : serialPortInfos)	{
		QString CommonPort;
		CommonPort.append(serialPortInfo.portName());
		ui.comboSerialPort->addItem(CommonPort);
	}
}

void frmInstrumentation::ClearInstruct()
{
	int _iRowCount = ui.tableWidget->rowCount();
	for (int i = 0; i < _iRowCount; i++)	{
		ui.tableWidget->removeRow(0);
	}
}

void frmInstrumentation::resizeEvent(QResizeEvent * ev)
{
	QWidget::resizeEvent(ev);
	int count = ui.tableWidget->columnCount();
	double iWidth = ui.tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++)	{
		ui.tableWidget->setColumnWidth(i, iWidth);
	}

}

void frmInstrumentation::showEvent(QShowEvent * ev)
{
	QWidget::showEvent(ev);
}

void frmInstrumentation::hideEvent(QHideEvent * ev)
{
	QWidget::hideEvent(ev);
	DisConnected();
}

void frmInstrumentation::changeEvent(QEvent * ev)
{
	switch (ev->type())
	{
	case QEvent::LanguageChange: {
		ui.retranslateUi(this);
	}	break;
	default:
		break;
	}
	QWidget::changeEvent(ev);
}

void frmInstrumentation::on_btnSave_clicked()
{
	//保存参数
	if (ui.lblType->text() == QString())
	{
		dataVar::Instance().pManager->notify( ui.lblType->text() + tr("选择仪器通讯名称或编号为空！"), NotifyType_Error);
		return;
	}
	QStringList strs = ui.lblType->text().split("_");
	if (strs[0] == "SerialPort")
	{
		//串口通信		
		QString	strCom = ui.comboSerialPort->currentText();
		int iBaudRate = ui.comboBaudRate->currentText().toInt();
		QSerialPort::Parity parity;   //校验位
		QSerialPort::DataBits dataBits;  //数据位
		QSerialPort::StopBits stopBits;  //停止位
		QSerialPort::FlowControl flowControl;  //流控制
		{
			if (ui.comboCheckDigit->currentText() == "无")
			{
				parity = QSerialPort::NoParity;
			}
			else if (ui.comboCheckDigit->currentText() == "奇")
			{
				parity = QSerialPort::Parity::OddParity;
			}
			else if (ui.comboCheckDigit->currentText() == "偶")
			{
				parity = QSerialPort::Parity::EvenParity;
			}
			else if (ui.comboCheckDigit->currentText() == "标志")
			{
				parity = QSerialPort::Parity::MarkParity;
			}
			else if (ui.comboCheckDigit->currentText() == "空格")
			{
				parity = QSerialPort::Parity::SpaceParity;
			}
			if (ui.comboDataBits->currentText() == "5")
			{
				dataBits = QSerialPort::Data5;
			}
			else if (ui.comboDataBits->currentText() == "6")
			{
				dataBits = QSerialPort::Data6;
			}
			else if (ui.comboDataBits->currentText() == "7")
			{
				dataBits = QSerialPort::Data7;
			}
			else if (ui.comboDataBits->currentText() == "8")
			{
				dataBits = QSerialPort::Data8;
			}
			if (ui.comboStopBit->currentText() == "1")
			{
				stopBits = QSerialPort::OneStop;
			}
			else if (ui.comboStopBit->currentText() == "1.5")
			{
				stopBits = QSerialPort::OneAndHalfStop;
			}
			else if (ui.comboStopBit->currentText() == "2")
			{
				stopBits = QSerialPort::TwoStop;
			}
			if (ui.comboFlowControl->currentText() == "无")
			{
				flowControl = QSerialPort::FlowControl::NoFlowControl;
			}
			else if (ui.comboFlowControl->currentText() == "硬件")
			{
				flowControl = QSerialPort::FlowControl::HardwareControl;
			}
			else if (ui.comboFlowControl->currentText() == "软件")
			{
				flowControl = QSerialPort::FlowControl::SoftwareControl;
			}
		}
		{	gVariable::Instance().SerialPortVar.portname_value		= strCom;
			gVariable::Instance().SerialPortVar.baudrate_value		= iBaudRate;
			gVariable::Instance().SerialPortVar.parity_value		= parity;
			gVariable::Instance().SerialPortVar.databits_value		= dataBits;
			gVariable::Instance().SerialPortVar.stopbits_value		= stopBits;
			gVariable::Instance().SerialPortVar.flowcontrol_value	= flowControl;
			gVariable::Instance().SerialPortVar.serialport_value	= s_port;
			gVariable::Instance().SerialPortVar.remark_value		= ui.txtSocketRemark->text();
			gVariable::Instance().serialport_variable_link.insert(std::pair<QString, gVariable::SerialPort_Var>(ui.lblType->text(), gVariable::Instance().SerialPortVar));
		}
		SerialPortContent.global_serial_port			= gVariable::Instance().SerialPortVar.portname_value;
		SerialPortContent.global_baud_rate				= gVariable::Instance().SerialPortVar.baudrate_value;
		SerialPortContent.global_check_digit			= gVariable::Instance().SerialPortVar.parity_value;
		SerialPortContent.global_data_bits				= gVariable::Instance().SerialPortVar.databits_value;
		SerialPortContent.global_stop_bit				= gVariable::Instance().SerialPortVar.stopbits_value;
		SerialPortContent.global_flowcontrol			= gVariable::Instance().SerialPortVar.flowcontrol_value;
		SerialPortContent.global_remark					= gVariable::Instance().SerialPortVar.remark_value;
		global_serialport_content.insert(ui.lblType->text(), SerialPortContent);
	}
	else if (strs[0] == "EthernetTcp")
	{
		//TCP/IP通信 服务器
		if (ui.comboProtocol->currentIndex() == 0)	{
			gVariable::Instance().SocketTcpServerVar.server_value		= server;
			gVariable::Instance().SocketTcpServerVar.protocol_value		= ui.comboProtocol->currentIndex();
			gVariable::Instance().SocketTcpServerVar.ip_value			= ui.widgetSocketIP->getIP();
			gVariable::Instance().SocketTcpServerVar.port_value			= ui.spinSocketPort->value();
			gVariable::Instance().SocketTcpServerVar.remark_value		= ui.txtSocketRemark->text();
			gVariable::Instance().sockettcpserver_variable_link.insert(std::pair<QString, gVariable::SocketTcpServer_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpServerVar));

			if (gVariable::Instance().sockettcpclient_variable_link.count(ui.lblType->text()) > 0)
				if (gVariable::Instance().sockettcpclient_variable_link.at(ui.lblType->text()).client_value != nullptr)
					gVariable::Instance().sockettcpclient_variable_link.at(ui.lblType->text()).client_value->Close();
			gVariable::Instance().sockettcpclient_variable_link.erase(ui.lblType->text());
		}
		else {
			gVariable::Instance().SocketTcpClientVar.client_value		= client;
			gVariable::Instance().SocketTcpClientVar.protocol_value		= ui.comboProtocol->currentIndex();
			gVariable::Instance().SocketTcpClientVar.ip_value			= ui.widgetSocketIP->getIP();
			gVariable::Instance().SocketTcpClientVar.port_value			= ui.spinSocketPort->value();
			gVariable::Instance().SocketTcpClientVar.remark_value		= ui.txtSocketRemark->text();
			gVariable::Instance().sockettcpclient_variable_link.insert(std::pair<QString, gVariable::SocketTcpClient_Var>(ui.lblType->text(), gVariable::Instance().SocketTcpClientVar));

			if (gVariable::Instance().sockettcpserver_variable_link.count(ui.lblType->text()) > 0)
				if(gVariable::Instance().sockettcpserver_variable_link.at(ui.lblType->text()).server_value != nullptr)
					gVariable::Instance().sockettcpserver_variable_link.at(ui.lblType->text()).server_value->Close();
			gVariable::Instance().sockettcpserver_variable_link.erase(ui.lblType->text());
		}

		TcpContent.global_protocol						= (EnumTCPType)ui.comboProtocol->currentIndex();
		TcpContent.global_ip							= ui.widgetSocketIP->getIP();
		TcpContent.global_port							= ui.spinSocketPort->value();
		TcpContent.global_remark						= ui.txtSocketRemark->text();
		global_tcp_content.insert(ui.lblType->text(), TcpContent);
	}

	dataVar::Instance().pManager->notify( ui.lblType->text() + tr("保存参数完成！"), NotifyType_Info);

}

void frmInstrumentation::slot_DoubleClicked(int row, int column)
{
	onSelectedConnectedTCP(false);
	UpDataSerialLst();
	QString selectInstrument = ui.tableWidget->item(row, 0)->data(1).toString();
	ui.lblType->setText(selectInstrument);
	QStringList strs = selectInstrument.split("_");
	comm_keys.reserve(100);
	if (strs[0] == "SerialPort")
	{
		ui.stackedWidget->setCurrentIndex(2);
		comm_keys.clear();
		comm_keys = global_serialport_content.uniqueKeys();
		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == selectInstrument)
			{
				count = 1;
				ui.comboSerialPort->setCurrentText(global_serialport_content.value(key).global_serial_port);
				ui.comboBaudRate->setCurrentText(QString::number(global_serialport_content.value(key).global_baud_rate));
				ui.comboCheckDigit->setCurrentText(QString::number(global_serialport_content.value(key).global_check_digit));
				ui.comboDataBits->setCurrentText(QString::number(global_serialport_content.value(key).global_data_bits));
				ui.comboStopBit->setCurrentText(QString::number(global_serialport_content.value(key).global_stop_bit));
				ui.comboFlowControl->setCurrentText(QString::number(global_serialport_content.value(key).global_flowcontrol));
				ui.txtSPRemark->setText(global_serialport_content.value(key).global_remark);
				if (gVariable::Instance().serialport_variable_link.count(key) <= 0)
				{
					dataVar::Instance().pManager->notify(	ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Warn);
					return;
				}
				s_port = gVariable::Instance().serialport_variable_link.at(key).serialport_value;
				bool _bIsListening = false;
				if (s_port != nullptr)
				{
					s_port->disconnect(this);
					connect(s_port, &MyCnCommort::sig_readyRead, this, &frmInstrumentation::onSerialRecieveData, Qt::BlockingQueuedConnection);
				}
				if (s_port != nullptr)	if (s_port->IsOpen())	_bIsListening = true;
				onSelectedConnectedTCP(_bIsListening);
			}
		}
		if (count == 0)
		{
			dataVar::Instance().pManager->notify(ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Warn);
			return;
		}
	}
	else if (strs[0] == "EthernetTcp")
	{
		ui.stackedWidget->setCurrentIndex(3);
		comm_keys.clear();
		comm_keys = global_tcp_content.uniqueKeys();

		int count = 0;
		for (int k = 0; k < comm_keys.length(); k++)
		{
			QString key = comm_keys[k];
			if (key == selectInstrument )
			{
				count = 1;
				ui.comboProtocol->setCurrentIndex(global_tcp_content.value(key).global_protocol);
				ui.widgetSocketIP->setIP(global_tcp_content.value(key).global_ip);
				ui.spinSocketPort->setValue(global_tcp_content.value(key).global_port);
				ui.txtSocketRemark->setText(global_tcp_content.value(key).global_remark);

				if (global_tcp_content.value(key).global_protocol == EnumTCPType_server)
				{	//服务器
					if (gVariable::Instance().sockettcpserver_variable_link.count(key) <= 0)
					{
						dataVar::Instance().pManager->notify(ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Warn);

						return;
					}
					server = gVariable::Instance().sockettcpserver_variable_link.at(key).server_value;

					ui.ConnectedOBJ->clear();
					ui.ConnectedOBJ->addItem("AllConnected");
					bool _bIsListening = false;
					if (server != nullptr)
					{
						for (auto iter : server->m_vecCTcpClient)
							if (iter->m_Client != nullptr)
								ui.ConnectedOBJ->addItem(QString("%1:%2").arg(iter->m_Client->localAddress()).arg(QString::number(iter->m_Client->localPort())));
						server->QObject::disconnect(this);
						connect(server, &CTcpServerSocket::sig_ConnectedSocket,		this, &frmInstrumentation::onTcpServiceQConnected, Qt::BlockingQueuedConnection);
						connect(server, &CTcpServerSocket::sig_DisConnectedSocket,	this, &frmInstrumentation::onTcpServiceQDisconnected, Qt::BlockingQueuedConnection);
						connect(server, &CTcpServerSocket::sig_ReadyRead,			this, &frmInstrumentation::onTcpDataReadyRead, Qt::BlockingQueuedConnection);

						if (server != nullptr)	if (server->isListening())	_bIsListening = true;
					}
					onSelectedConnectedTCP(_bIsListening);
					ui.lbl_ip->setText(tr("本地IP地址："));
					ui.lbl_port->setText(tr("本地端口号："));
				}
				else
				{	//客户端
					if (gVariable::Instance().sockettcpclient_variable_link.count(key) <= 0)
					{
						dataVar::Instance().pManager->notify(	ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Warn);
						return;
					}
					client = gVariable::Instance().sockettcpclient_variable_link.at(key).client_value;
					if (client != nullptr)
					{
						client->QObject::disconnect(this);
						connect(client, &CTcpSocket::sig_ConnectedSocket,		this, &frmInstrumentation::onTcpQConnected,		Qt::BlockingQueuedConnection);
						connect(client, &CTcpSocket::sig_DisConnectedSocket,	this, &frmInstrumentation::onTcpQDisconnected,	Qt::BlockingQueuedConnection);
						connect(client, &CTcpSocket::sig_ReadyRead,				this, &frmInstrumentation::onTcpDataReadyRead,	Qt::BlockingQueuedConnection);

						//connect(client, &QTcpSocket::readyRead, this, [=]() {	onTcpReadyRead(client);	});

					}
					bool _bIsListening = false;
					if (client != nullptr)
						if (client->isOpen())
							_bIsListening = true;

					onSelectedConnectedTCP(_bIsListening);
					ui.lbl_ip->setText(tr("远程IP地址："));
					ui.lbl_port->setText(tr("远程端口号："));
				}
			}
		}
		if (count == 0)
		{
			dataVar::Instance().pManager->notify(	ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Warn);
			return;
		}
	}
}

void frmInstrumentation::slot_MoveUp()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmInstrumentation::slot_MoveDown()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmInstrumentation::slot_DeleteName()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex != -1)
	{
		QString str = ui.tableWidget->item(rowIndex, 0)->data(1).toString();
		QStringList strs = str.split("_");
		if (strs[0] == "SerialPort")
		{
			if (gVariable::Instance().serialport_variable_link.count(str) <= 0)			{
				dataVar::Instance().pManager->notify(ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
				return;
			}
			//串口通信
			if (gVariable::Instance().serialport_variable_link.at(str).serialport_value != nullptr)
				if (gVariable::Instance().serialport_variable_link.at(str).serialport_value->IsOpen())
					gVariable::Instance().serialport_variable_link.at(str).serialport_value->Close();
			global_serialport_content.remove(str);
			gVariable::Instance().serialport_variable_link.erase(str);
		}
		else if (strs[0] == "EthernetTcp")
		{
			//TCP/IP通信
			if (global_tcp_content.count(str))
				if(global_tcp_content.value(str).global_protocol == EnumTCPType_server)
				{
					if (gVariable::Instance().sockettcpserver_variable_link.count(str) <= 0)					{
						dataVar::Instance().pManager->notify(ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
						return;
					}
					if (gVariable::Instance().sockettcpserver_variable_link.at(str).server_value != nullptr)
						if (gVariable::Instance().sockettcpserver_variable_link.at(str).server_value->isListening())
							gVariable::Instance().sockettcpserver_variable_link.at(str).server_value->Close();
					gVariable::Instance().sockettcpserver_variable_link.erase(str);
				}
				else
				{
					if (gVariable::Instance().sockettcpclient_variable_link.count(str) <= 0)					{
						dataVar::Instance().pManager->notify(ui.lblType->text() + tr("该仪器通讯参数未保存！"), NotifyType_Error);
						return;
					}
					if (gVariable::Instance().sockettcpclient_variable_link.at(str).client_value != nullptr)
						if (gVariable::Instance().sockettcpclient_variable_link.at(str).client_value->isOpen())
							gVariable::Instance().sockettcpclient_variable_link.at(str).client_value->Close();
					gVariable::Instance().sockettcpclient_variable_link.erase(str);
				}
			global_tcp_content.remove(str);
		}
		ui.tableWidget->removeRow(rowIndex);
	}
	int count = ui.tableWidget->rowCount();
	if (count == 0)
	{
		ui.lblType->clear();
		global_serialport_content.clear();
		global_tcp_content.clear();
		gVariable::Instance().serialport_variable_link.clear();
		gVariable::Instance().sockettcpserver_variable_link.clear();
		gVariable::Instance().sockettcpclient_variable_link.clear();
	}
}

void frmInstrumentation::moveRow(QTableWidget* pTable, int nFrom, int nTo)
{
	if (pTable == NULL) return;
	pTable->setFocus();
	if (nFrom == nTo) return;
	if (nFrom < 0 || nTo < 0) return;
	int nRowCount = pTable->rowCount();
	if (nFrom >= nRowCount || nTo > nRowCount) return;
	if (nTo < nFrom) nFrom++; pTable->insertRow(nTo);
	int nCol = pTable->columnCount();
	for (int i = 0; i < nCol; i++)
	{
		pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
	}
	if (nFrom < nTo)
	{
		nTo--;
	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}

void frmInstrumentation::SendInstructMessage(const QString & message)
{
	ui.textEdit_Send->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "  " + message /*+ "\n"*/);
	ui.textEdit_Send->moveCursor(QTextCursor::End); //set to start
}

void frmInstrumentation::RecieveInstructMessage(const QString & message)
{
	m_QMutex.lock();
	ui.textEdit_Recieve->append(QDateTime::currentDateTime().toString("hh:mm:ss-zzz") + " " + message /*+ "\n"*/);
	ui.textEdit_Recieve->moveCursor(QTextCursor::End); //set to start
	m_QMutex.unlock();
}
