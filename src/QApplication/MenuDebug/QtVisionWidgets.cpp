#include "QtVisionWidgets.h"
#include <QSerialPortInfo>
#include <QStringList>
#include "Vision/PluginsManager.h"
#include "Vision/CameraManager.h"

QtVisionWidgets::QtVisionWidgets(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);

	initUI();
}

QtVisionWidgets::~QtVisionWidgets()
{
	CloseUI();
}

int QtVisionWidgets::RecieveTCP(CTcpClient * socket, QString & str)
{
	if (socket != nullptr)
	{
		QString	strClientMsg = QString("%1:%2 %3").arg(socket->localAddress()).arg(QString::number(socket->localPort())).arg(str);
		RecieveInstructMessage(strClientMsg);
	}
	return 0;
}

int QtVisionWidgets::DisConnect(CTcpClient * sock)
{

	return 0;
}

QStringList QtVisionWidgets::getSerialPortList()
{
	QStringList serialPortList;
	QList<QSerialPortInfo> portInfoList = QSerialPortInfo::availablePorts();
	for (int i = 0; i<portInfoList.size(); ++i) {
		QSerialPortInfo portInfo = portInfoList.at(i);
		serialPortList.append(portInfo.portName());
	}
	return serialPortList;
}

void QtVisionWidgets::Connected()
{

}

void QtVisionWidgets::CheckConConnected()
{
	//if (!dataVar::Instance().m_CTcpClient.isOpen())	{
	//	ui.btnConnect_2->setIcon(QIcon(":/images/skin/images/RedDot.png"));
	//	ui.btnConnect_2->setText(tr("Connect"));
	//}
	//else	{
	//	ui.btnConnect_2->setIcon(QIcon(":/images/skin/images/GreenDot.png"));
	//	ui.btnConnect_2->setText(tr("DisConnect"));
	//}
}

void QtVisionWidgets::initUI()
{	
	ui.tabWidget->setCurrentIndex(0);

}

bool QtVisionWidgets::CheckPageAvalible()
{
	return false;
}

int QtVisionWidgets::CloseUI()
{
	int _iTabCount = ui.tabWidgetCamera->count();
	//for (int i = 0; i < _iTabCount; i++)
	//{
	//	((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->CloseUI();
	//}

	return 0;
}

int QtVisionWidgets::initData(QString& strError)
{
	//for (size_t i = 0; i < ui.tabWidgetCamera->count(); i++)
	//{
	//	((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->initData(strError);
	//}
	ui.tabWidgetCamera->setCurrentIndex(0);
	CheckConConnected();

	return 0;
}

int QtVisionWidgets::GetData(QJsonObject & strData)
{
	QJsonArray ConFig;
	//for (size_t i = 0; i < ui.tabWidgetCamera->count(); i++)
	//{
	//	QJsonObject CParam;
	//	((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->GetData(CParam);
	//	CParam.insert("Name" + QString::number(i), ((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->objectName());
	//	ConFig.append(CParam);
	//}
	strData.insert("QtVisionWidgets", ConFig);
	return 0;
}

int QtVisionWidgets::SetData(QJsonObject & strData)
{
	QJsonArray CommonParam = strData.find("QtVisionWidgets").value().toArray();
	//int _iTabCount = ui.tabWidgetCamera->count();
	//QString	strError;
	//for (size_t i = 0; i < CommonParam.count(); i++)
	//{
	//	QJsonObject CParam = CommonParam.at(i).toObject();
	//	QString	strObject = "Name" + QString::number(i);
	//	QString	_strObjectName = strObject;
	//	if (CParam.contains(strObject))		_strObjectName = CParam[strObject].toString();
	//	QtVisionWidget* widget = ((QtVisionWidget*)ui.tabWidgetCamera->widget(i));// new QtSCrewWidget();
	//	if (widget != nullptr) {
	//		widget->m_iIndex = i;
	//		widget->setObjectName(_strObjectName);
	//		//widget->initData(strError);
	//		widget->SetData(CParam);
	//	}
	//}
	//if (dataVar::Instance().bUseVision) {
	//	ui.tabWidget->removeTab(0);
	//	//QWidget* tabWidget = ui.tabWidget->widget(0); // 获取第二个Tab的QWidget
	//	//tabWidget->hide(); // 隐藏该Tab
	//}
	return 0;
}

int QtVisionWidgets::NewProject()
{
	return 0;
}

int QtVisionWidgets::GetSystemData(QJsonObject & strData)
{
	//串口
	QJsonObject CommonParam;
	//CommonParam.insert("comboSCrewType", QString(ui.comboSCrewType->currentText()));
	//CommonParam.insert("comboSerialPort", QString(ui.comboSerialPort->currentText()));
	//CommonParam.insert("comboBaudRate", QString(ui.comboBaudRate->currentText()));
	//CommonParam.insert("comboDataBits", QString(ui.comboDataBits->currentText()));
	//CommonParam.insert("comboCheckDigit", QString(ui.comboCheckDigit->currentText()));
	//CommonParam.insert("comboStopBit", QString(ui.comboStopBit->currentText()));
	//CommonParam.insert("comboFlowControl", QString(ui.comboFlowControl->currentText()));
	//CommonParam.insert("widgetSocketIP", QString(ui.widgetSocketIP->getIP()));
	//CommonParam.insert("spinSocketPort", QString::number(ui.spinSocketPort->value()));
	strData.insert("QtVisionWidgets", CommonParam);

	QJsonArray ConFig;
	//for (size_t i = 0; i < ui.tabWidgetCamera->count(); i++)
	//{
	//	QJsonObject CParam;
	//	((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->GetSystemData(CParam);
	//	CParam.insert("Name" + QString::number(i), ((QtVisionWidget*)ui.tabWidgetCamera->widget(i))->objectName());
	//	ConFig.append(CParam);
	//}
	strData.insert("QtVisionParamWidgets", ConFig);
	return 0;
}

int QtVisionWidgets::SetSystemData(QJsonObject & strData)
{
	QJsonArray ConFig = strData.find("QtVisionParamWidgets").value().toArray();
	//int _iTabCount = ui.tabWidgetCamera->count();
	//QString	strError;
	//for (size_t i = 0; i < ConFig.count(); i++)
	//{
	//	QJsonObject CParam	= ConFig.at(i).toObject();
	//	QString	strObject	= "Name" + QString::number(i);
	//	QString	_strObjectName = strObject;
	//	if (CParam.contains(strObject))		_strObjectName = CParam[strObject].toString();
	//	QtVisionWidget* widget = ((QtVisionWidget*)ui.tabWidgetCamera->widget(i));
	//	if (widget != nullptr) {
	//		widget->m_iIndex = i;
	//		widget->setObjectName(_strObjectName);
	//		widget->initData(strError);
	//		widget->SetSystemData(CParam);
	//	}
	//}


	if (dataVar::Instance().bAutoConnectedWhenStart) {
		Connected();
	}	
	CheckConConnected();
	return 0;
}

//void QtVisionWidgets::on_btnConnect_clicked()
//{
//	//if (MotionSerialPort::getInstance().m_pCnComm->IsOpen()) {
//	//	MotionSerialPort::getInstance().m_pCnComm->Close();
//	//}
//	//else
//	//{
//	//	QString	_strCom		= ui.comboSerialPort->currentText();
//	//	DWORD dwPort		= ui.comboSerialPort->currentText().mid(3).toInt();
//	//	DWORD dwBaudRate	= ui.comboBaudRate->currentText().toInt();
//	//	BYTE btParity = NOPARITY;
//	//	switch (ui.comboCheckDigit->currentIndex()) {
//	//	case 0: {	btParity = NOPARITY;		} break;
//	//	case 1: {	btParity = ODDPARITY;		} break;
//	//	case 2: {	btParity = EVENPARITY;		} break;
//	//	case 3: {	btParity = MARKPARITY;		} break;
//	//	case 4: {	btParity = SPACEPARITY;		} break;
//	//	default:	break;
//	//	}
//	//	BYTE btByteSize = 8;
//	//	switch (ui.comboDataBits->currentIndex()) {
//	//	case 0: {	btByteSize = 5;		} break;
//	//	case 1: {	btByteSize = 6;		} break;
//	//	case 2: {	btByteSize = 7;		} break;
//	//	case 3: {	btByteSize = 8;		} break;
//	//	default:	break;
//	//	}
//	//	BYTE btStopBits = ONESTOPBIT;
//	//	switch (ui.comboStopBit->currentIndex()) {
//	//	case 0: {	btStopBits = ONESTOPBIT;		} break;
//	//	case 1: {	btStopBits = ONE5STOPBITS;		} break;
//	//	case 2: {	btStopBits = TWOSTOPBITS;		} break;
//	//	default:	break;
//	//	}
//	//	MotionSerialPort::getInstance().m_pCnComm->Open(dwPort, dwBaudRate, btParity, btByteSize, btStopBits);
//	//}
//	CheckConConnected();
//}


void QtVisionWidgets::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
	//dataVar::Instance().m_CTcpClient.RegisterRecieveData(this);
}

void QtVisionWidgets::hideEvent(QHideEvent * ev)
{
	QtWidgetsBase::hideEvent(ev);
	//dataVar::Instance().m_CTcpClient.RemoveRecieveData(this);
}

void QtVisionWidgets::slotUpDateSerialPort()
{
	//QString	_strCom = ui.comboSerialPort->currentText();
	//ui.comboSerialPort->clear();
	//ui.comboSerialPort->addItems(getSerialPortList());
	//ui.comboSerialPort->setCurrentText(_strCom);
}
//
//void QtVisionWidgets::on_btnSendMsg_clicked()
//{
//	//QString	strData = ui.textEdit_Send->toPlainText();
//	//dataVar::Instance().m_CTcpClient.write(strData);
//	//SendInstructMessage(strData);
//}

void QtVisionWidgets::SendInstructMessage(const QString & message)
{
	//ui.textEdit_Send->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "  " + message /*+ "\n"*/);
	//ui.textEdit_Send->moveCursor(QTextCursor::End); //set to start
}

void QtVisionWidgets::RecieveInstructMessage(const QString & message)
{
	//m_QMutex.lock();
	//ui.textEdit_Recieve->append(QDateTime::currentDateTime().toString("hh:mm:ss-zzz") + " " + message /*+ "\n"*/);
	//ui.textEdit_Recieve->moveCursor(QTextCursor::End); //set to start
	//m_QMutex.unlock();
}