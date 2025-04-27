#include "QtMesWidgets.h"
#include "Data/dataVar.h"

QtMesWidgets::QtMesWidgets(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	m_pCTcpSocket = new CTcpSocket(this);
	connect(m_pCTcpSocket, &CTcpSocket::sig_ConnectedSocket,	this,	&QtMesWidgets::onTcpQConnected,		Qt::BlockingQueuedConnection);
	connect(m_pCTcpSocket, &CTcpSocket::sig_DisConnectedSocket, this,	&QtMesWidgets::onTcpQDisconnected,	Qt::BlockingQueuedConnection);
	connect(m_pCTcpSocket, &CTcpSocket::sig_ReadyRead,			this,	&QtMesWidgets::onTcpDataReadyRead,	Qt::BlockingQueuedConnection);

	Init();
}

QtMesWidgets::~QtMesWidgets()
{
	CloseUI();
}

void QtMesWidgets::CheckConnected()
{
	if (m_pCTcpSocket->isOpen()) {
		ui.btnConnect->setIcon(QIcon(":/images/skin/images/GreenDot.png"));
		ui.btnConnect->setText(tr("DisConnect"));
	}
	else {
		ui.btnConnect->setIcon(QIcon(":/images/skin/images/RedDot.png"));
		ui.btnConnect->setText(tr("Connect"));
	}
}

void QtMesWidgets::Connected()
{
	if (m_pCTcpSocket != nullptr)		{
		if (!m_pCTcpSocket->isOpen())	{
			m_pCTcpSocket->ConnectToServer(ui.widgetSocketIP->getIP().toStdString().c_str(), ui.spinSocketPort->value());
		}
	}
}

void QtMesWidgets::SendInstructMessage(const QString & message)
{
	ui.textEdit_Send->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "  " + message /*+ "\n"*/);
	ui.textEdit_Send->moveCursor(QTextCursor::End); //set to start
}

void QtMesWidgets::RecieveInstructMessage(const QString & message)
{
	m_QMutex.lock();
	ui.textEdit_Recieve->append(QDateTime::currentDateTime().toString("hh:mm:ss-zzz") + " " + message /*+ "\n"*/);
	ui.textEdit_Recieve->moveCursor(QTextCursor::End); //set to start
	m_QMutex.unlock();
}

void QtMesWidgets::onSelectedConnectedTCP(bool bOn)
{
	if (bOn) {
		ui.btnConnect->setIcon(QIcon(":/images/skin/images/GreenDot.png"));
		ui.btnConnect->setText(tr("DisConnect"));
	}
	else {
		ui.btnConnect->setIcon(QIcon(":/images/skin/images/RedDot.png"));
		ui.btnConnect->setText(tr("Connect"));
	}
}

bool QtMesWidgets::CheckPageAvalible()
{
	return false;
}

int QtMesWidgets::CloseUI()
{
	if (m_pCTcpSocket != nullptr) {
		//m_pCTcpSocket->disconnect(this);
		m_pCTcpSocket->Close();	delete	m_pCTcpSocket;	m_pCTcpSocket = nullptr;
	}
	return 0;
}

int QtMesWidgets::initData(QString& strError)
{
	return 0;
}

void QtMesWidgets::Init()
{    

}

int QtMesWidgets::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;

	strData.insert("QtMesWidgets", CommonParam);
	return 0;
}

int QtMesWidgets::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtMesWidgets").value().toObject();

	return 0;
}

int QtMesWidgets::NewProject()
{
	return 0;
}

void QtMesWidgets::on_btnSend_clicked()
{
	if (m_pCTcpSocket != nullptr) {
		if (m_pCTcpSocket->isOpen()) {
			m_pCTcpSocket->write(ui.textEdit_Send->toPlainText().toStdString());
		}
	}
}

int QtMesWidgets::GetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("widgetSocketIP", QString(ui.widgetSocketIP->getIP()));
	CommonParam.insert("spinSocketPort", QString::number(ui.spinSocketPort->value()));

	strData.insert("QtMesWidgets", CommonParam);

	return 0;
}

int QtMesWidgets::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtMesWidgets").value().toObject();
	if (CommonParam.contains("widgetSocketIP"))	ui.widgetSocketIP->setIP(CommonParam["widgetSocketIP"].toString());
	if (CommonParam.contains("spinSocketPort"))	ui.spinSocketPort->setValue(CommonParam["spinSocketPort"].toString().toInt());
	//MotionRobot::getInstance().SetIpAddress(ui.widgetSocketIP->getIP(), ui.spinSocketPort->value());
	if (dataVar::Instance().bAutoConnectedWhenStart)
	{
		Connected();
	}
	CheckConnected();
	return 0;
}

void QtMesWidgets::on_btnConnect_clicked()
{
	if (m_pCTcpSocket != nullptr) {
		if (m_pCTcpSocket->isOpen()){
			m_pCTcpSocket->Close();
		}
		else {
			m_pCTcpSocket->ConnectToServer(ui.widgetSocketIP->getIP().toStdString().c_str(), ui.spinSocketPort->value());
		}
	}
	CheckConnected();
}

void QtMesWidgets::onTcpQConnected(QString strkey, CTcpSocket * socket)
{
	try
	{
		if (socket != nullptr)
		{
			onSelectedConnectedTCP(m_pCTcpSocket->isOpen());

			QString	strClientMsg = QString("%1:%2 Connected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
			RecieveInstructMessage(strClientMsg);
		}
	}
	catch (...) {}
}

void QtMesWidgets::onTcpQDisconnected(QString strkey, CTcpSocket * socket)
{
	try
	{
		if (socket != nullptr)	{
			onSelectedConnectedTCP(m_pCTcpSocket->isOpen());
			QString	strClientMsg = QString("%1:%2 DisConnected").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
			RecieveInstructMessage(strClientMsg);

			disconnect(socket);
		}
	}
	catch (...) {}
}

void QtMesWidgets::onTcpDataReadyRead(CTcpSocket * socket, QString & str)
{
	if (socket != nullptr) {
		QString	strClient = QString("%1:%2").arg(socket->localAddress()).arg(QString::number(socket->localPort()));
		if (ui.ConnectedOBJ->currentText() == "AllConnected") {}
		else if (ui.ConnectedOBJ->currentText() != strClient) { return; }

		QString	strClientMsg = QString("%1:%2 %3").arg(socket->localAddress()).arg(QString::number(socket->localPort())).arg(str);
		RecieveInstructMessage(strClientMsg);
	}
}