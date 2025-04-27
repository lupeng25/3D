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
#include <QMenu>
//#include <GraphicsView.h>
#include "FlowGraphicsViews.h"
#include "NodeManager.h"
#include <QMap>
#include <QDateTime>
#include "notifymanager.h"

#ifndef _DATABASEVAR_EXPORT_
#define _DATABASEVAR_API_ _declspec(dllexport)
#else
#define _DATABASEVAR_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _DATABASEVAR_API_ databaseVar
{
public:
	static databaseVar& Instance();
public:
	int Init();
	int Exit();

	QMap<QString, QString> m_mapParam;
public:
	bool	bAutoConnectedWhenStart;
	int		iLanguage;
	int		m_iCheckMode;			//检测模式
	int		m_iCheckNGMotion;		//NG执行的动作
public:
	QString applicationPath;
	QString ConfigFolder;
	QString DefaultParamFolder;
	//项目名称
	QString projectName;

public:
	//静态数据成员的声明 	
	QString soft_key;
	QWidget* m_pWindow;
	void CenterScreenWindow(QWidget* window);
	void CenterMainWindow(QWidget* window);
	void CenterWindow(QWidget* window, QWidget* subwindow);
	void ClearMenu(QMenu* menu);
public:
	QString software_name;
	QString software_vision;

public:
	QColor m_TitleBackColor;
	QColor m_BackColor;

public:		//线程
	//bool	m_bVisionThreadRun;
	//bool	m_bVisionThreadNeedStop;		//线程停止
	//int	m_iVisionThreadRunTime;			//视觉线程运行次数 当 < 0一直循环
	//bool	m_iVisionThreadFirstRun;		//首次运行

	//系统设置
	int					form_System_Precision;	//系统精度
	NodeManager*		m_NotifyManager;
	NotifyManager *		pManager = nullptr;
	int form_Layout_Number;
public:
	QWidgetList					VideoWidgets;			//视频控件集合
	QVector< FlowGraphicsViews*>	m_vecPtr;				//软件流程
};
