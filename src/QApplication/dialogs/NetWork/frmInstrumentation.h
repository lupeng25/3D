#pragma once

#include <QDialog>
#include "ui_frmInstrumentation.h"
#include "gvariable.h"
#include "mytitlebar.h"
#include "ComSocket.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <Qmutex>
#include "frmBase.h"
#include "frmSetBase.h"
#include "QFramer/fbasedialog.h"

class frmInstrumentation : public QWidget
{
	Q_OBJECT
public:
	frmInstrumentation(QWidget* parent = Q_NULLPTR);
	~frmInstrumentation();
public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
public:
	void UpDataInstrument();
	void ClearInstrument();
	void UpDateLst();
	void DisConnected();
private:
	Ui::frmInstrumentationClass ui;
private:
	void initTitleBar();
	void ClearConnect();
	int GetValue(QVector<int> n);
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);
private slots:
	void onButtonCloseClicked();
	void SendInstructMessage(const QString &message);
	void RecieveInstructMessage(const QString &message);
private slots:
	void on_btnAdd_clicked();
	void on_btnConnect_clicked();
	void on_btnSave_clicked();
	void slot_MoveUp();
	void slot_MoveDown();
	void slot_DeleteName();
	void slot_DoubleClicked(int, int);
	void on_comboProtocol_currentIndexChanged(int);
	void on_btnSendMsg_clicked();
	void slot_CustomContextMenuRequested(const QPoint &pos);
	void on_BtnClearRecieve_clicked();
	void on_BtnSend_clicked();
private slots:
	void onTcpServiceQConnected(QString strkey, CTcpSocket*);
	void onTcpServiceQDisconnected(QString strkey, CTcpSocket*);
	void onTcpQConnected(QString strkey, CTcpSocket*);
	void onTcpQDisconnected(QString strkey, CTcpSocket*);
	void slotInstrumentChanged(int);
	void onTcpDataReadyRead(CTcpSocket* socket, QString& str);
	void onSerialRecieveData(MyCnCommort* socket, QString& str);
	void onTcpReadyRead(CTcpSocket*);
	void onTcpClientConnected(CTcpSocket*);
	void onTcpDisconnected(CTcpSocket*);
	void onTcpServiceReadyRead(CTcpSocket*);
	void onSelectedConnectedTCP(bool bOn = false);
	void UpDataSerialLst();
	void ClearInstruct();
protected:
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
	void changeEvent(QEvent * ev);
private:	
	QList<QString>		comm_keys;
	MyCnCommort*		s_port			= nullptr;
	CTcpServerSocket*	server			= nullptr;
	CTcpSocket*			client			= nullptr;
	QString				init_key_value;
	QMutex				m_QMutex;
public:
	//串口通信
	typedef struct SERIALPORTCONTENT
	{
		QString						global_serial_port;
		int							global_baud_rate;
		QSerialPort::Parity			global_check_digit;
		QSerialPort::DataBits		global_data_bits;
		QSerialPort::StopBits		global_stop_bit;
		QSerialPort::FlowControl	global_flowcontrol;
		QString						global_remark;
	} SerialPort_Content;
	SerialPort_Content SerialPortContent;
	QMap<QString, SerialPort_Content> global_serialport_content;
	
	enum EnumTCPType
	{
		EnumTCPType_server,
		EnumTCPType_client,
	};
	//TCP/IP通信
	typedef struct TCPCONTENT
	{
		EnumTCPType global_protocol;
		QString global_ip;
		int global_port;
		QString global_remark;
			
	} Tcp_Content;
	Tcp_Content TcpContent;
	QMutex	m_socketLock;
	QMap<QString, Tcp_Content> global_tcp_content;
};