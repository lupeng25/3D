#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QTextCodec>
#include <QSharedMemory>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QSettings>
#include <qdebug.h>
#include <QDateTime>
#include <QMenu>
#include <QMutex>
#include <QMutexLocker>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#define UseCADCPP 1

#include "DataBase/QWorld.h"
#include "notifymanager.h"
#include "Common/Dialogbase.h"
#include "Pages/QtWidgetsBase.h"
#include "licence.h"

#include "dialogs/userdialog.h"
#include "dialogs/settingdialog.h"
#include "dialogs/aboutdialog.h"

#include "Log/LogPage.h"
#include "Keys/QtKeyWidgets.h"
#include "Pages/QtWaitWidgetsClass.h"
#include "CnComm.h"
#include "CommonClass.h"
#include "halconcpp\HalconCpp.h"
#include "ComSocket.h"
#include "FlowGraphicsViews.h"
//#include "Motion/MotionIO.h"
#include "dialogs/ImageLayOut/frmImageLayout.h"

using namespace HalconCpp;

#ifndef _DATAVAR_EXPORT_
#define _DATAVAR_API_ _declspec(dllexport)
#else
#define _DATAVAR_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

enum EnumErrorCode
{
	ErrorCode_Param1Error = -10000,
	ErrorCode_Param2Error,
	ErrorCode_Param3Error,
	ErrorCode_Param4Error,
	ErrorCode_Param5Error,
	ErrorCode_Param6Error,
	ErrorCode_Param7Error,
	ErrorCode_Param8Error,
	ErrorCode_Param9Error,
	ErrorCode_RetnError,
	ErrorCode_AdressError,
	ErrorCode_FunCodeError,
	ErrorCode_DataLenthError,
};

enum EnumMenuOperator
{
	MenuOperator_Hand,
	MenuOperator_IgNore,
};

enum EnumLightType
{
	LightType_Green,
	LightType_Red,
	LightType_Yellow,
};

class _DATAVAR_API_ dataVar
{
public:
	static dataVar& Instance();
public:
	int Init();
	int Exit();
public:
	bool bAutoConnectedWhenStart;
	QString strLanguage;

public:
	//SQL
	SQLite sqlite;
	bool bol_connect;
	void InitDataBase();

	int iLoginLevel;
	QString strUserName;
	QDateTime startTime;
	QTimer *timer;              //定时器判断是否运行超时
public:
	QString applicationPath;
	QString ConfigFolder;
	QString DefaultParamFolder;
	QString strLogPath;

	//项目名称
	QString projectName;
public:
	//静态数据成员的声明 	
	QString soft_key;
	QWidget* m_pWindow;		//主窗口
	void CenterScreenWindow(QWidget* window);
	void CenterMainWindow(QWidget* window);
	void CenterWindow(QWidget* window, QWidget* subwindow);
	void ClearMenu(QMenu* menu);
public:	//运行参数
	static QByteArray hexStrToByteArray(const QString& str);
	static char convertHexChar(char ch);
	static QString byteArrayToHexStr(const QByteArray& data);
	static unsigned short ModbusCRC16(unsigned char * aData, unsigned int aSize);
	static uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen);
	static uint32_t CRC32(uint8_t *_pBuf, uint16_t _usLen);
	static uint16_t MB_CRC16(uint8_t *_pushMsg, uint8_t _usDataLen);
public:
	QMap<QString, QString>	m_mapParam;
public:
	QString software_name;
	QString software_StartImg;
	QString software_Info;
	QString software_vision;
	QString software_OtherInfo;
	QString software_CompanyName;
	QSize	SetDlgSize;					//设置窗口大小
public:
	frmImageLayout*							pfrmImageLayout{ nullptr };
	QtWaitWidgetsClass*						pQtWaitWidgetsClass = { nullptr	};
	QtKeyWidgets*							pQtKeyWidgets = nullptr;
	LogPage*								pLogPage = nullptr;
	userdialog*								pUserdialog = nullptr;
	SettingDialog*							pSettingDialog = nullptr;
	NotifyManager *							pManager = nullptr;
	QList<Dialogbase*>						m_DialogbaseLst;
	licence									m_licence;
	QMutex									m_Commandmutex;

public:		//线程
	bool		m_bFirstScan	= false;
	bool		m_bVisionThreadRun;
	bool		m_bVisionThreadNeedStop;	//线程停止
	int			m_iVisionThreadRunTime;		//视觉线程运行次数 当 < 0一直循环
	int			m_iVisionThreadAutoRun;		//当 < 0一直循环
	int			m_iVisionThreadTest;		//当 < 0一直循环

	QString		m_strUserName;
	int			m_iLevel = 0;
public:

	bool	bUseDefaultPassward = false;
	QString	strDefaultPassward = "000000";

	int		auto_log_save_days;
	bool	auto_run_system_FullScreen;
	bool	auto_run_system_start;
	int		auto_login_out_Time;
	//int		form_System_Precision;	//系统精度
	bool	Enable_Heartbeat_Second;			//心跳时间
	int		auto_Heartbeat_Second;				//心跳时间

	bool	bEnableRegistrationAlart;			//注册时间
	int		iRegistrationDays;

	int		iAllocateWays;						//分配方式
	int		iFlash_Three_Light;					//闪烁三色灯

	int		iSendCommandDelay;					//发送指令延时
	int		iRecieveCommandDelay;				//发送指令延时

	int		iMotionTimeOut;
	int		iVisionTimeOut;
	int		iDealVisionTimeOut;

public:
	//QWidgetList	VideoWidgets;			//视频控件集合
	//QVector< FlowGraphicsViews*>	m_vecPtr;				//软件流程
};