#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>

#include "QFramer/futil.h"
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QTextCodec>
#include <QSharedMemory>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QSettings>
#include <qdebug.h>
#include <QIcon>
// Qt
#include <qthread.h>
#include <QProcess>
#include <QObject>
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

#include <QtConcurrent>
#include <QFuture>

#include "Log/savelog.h"
#include "Common/CLinguist.h"
#include "Common/QtSplashScreen.h"
#include "Dump/DADumpCapture.h"
#include "Data/dataVar.h"
#include "VisionInfo.h"
#include "databaseVar.h"

#include "Pages/QtStartWidgets.h"
#include "Vision/PluginsManager.h"
#include "Vision/CameraManager.h"
#include <cstdio>

#include "HalconCpp.h"
using namespace HalconCpp;

int main(int argc, char* argv[])
{
    //以下是针对高分屏的设置，有高分屏需求都需要按照下面进行设置
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif
    QApplication app(argc, argv);
	QApplication::setOrganizationName("QApplication");
	QApplication::setOrganizationDomain("QApplication");
	QApplication::setApplicationName("QApplication");
	QApplication::setWindowIcon(QIcon(":/image/icon/images/QApplication.png"));

	DA::DADumpCapture::initDump();

	//程序路径
	QString strPath = QApplication::applicationDirPath();
	SaveLog::Instance()->start();
	dataVar::Instance().Init();
	databaseVar::Instance().Init();
	dataVar::Instance().InitDataBase();

	QSharedMemory memory("QtApplication");
	if (!memory.create(1)) {
		MyMessageBox::question(nullptr, QObject::tr("The current program can only execute one"), QObject::tr("Alart"), MyMessageBox::Ok);
		return 0;
	}
	//软件信息
	dataVar::Instance().software_name	= "QApplication";
	dataVar::Instance().software_vision = SoftwareVision;
	dataVar::Instance().software_Info	= SoftwareModifyDate;
	QCoreApplication::setApplicationVersion(dataVar::Instance().software_vision);
	//字符集
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
	//切换语言
	QSettings settings(QApplication::applicationDirPath() + "/Config/config.ini", QSettings::IniFormat);
	settings.beginGroup("General");
	CLinguist::getInstance().ChangeLanguage((Language)settings.value(QString("language"), QString::number(Language_CN)).toString().toInt());
	//产品
	dataVar::Instance().projectName					= settings.value(QString("projectName"), QString("default")).toString();
	dataVar::Instance().software_name				= settings.value(QString("software_name"), QString("QApplication")).toString();
	dataVar::Instance().software_StartImg			= settings.value(QString("software_StartImg"), QString("default")).toString();
	dataVar::Instance().software_OtherInfo			= settings.value(QString("software_OtherInfo"), QString("")).toString();
	dataVar::Instance().software_CompanyName		= settings.value(QString("software_CompanyName"), QString("")).toString();
	dataVar::Instance().SetDlgSize.setWidth(settings.value(QString("spinBox_Width"), QString("-1")).toString().toDouble());
	dataVar::Instance().SetDlgSize.setHeight(settings.value(QString("spinBox_Height"), QString("-1")).toString().toDouble());
	settings.endGroup();

	//启动页面
	QtStartWidgets splash;
	splash.showAppText(dataVar::Instance().software_name);
	splash.showAppImg(dataVar::Instance().software_StartImg);
	splash.showAppOtherInfo(dataVar::Instance().software_OtherInfo);
	splash.showMessageText(QObject::tr("Process Start"));
	splash.setRange(0, 100);//进度条长度范围
	splash.show();
	splash.updateNum(0);

	SetSystem("width", 5120);
	SetSystem("height", 5120);
	ResetObjDb(5120, 5120, 3);

	splash.updateNum(10);
	splash.showMessageText(QObject::tr("Init System Data"));
	splash.showMessageText(QObject::tr("Plugins Load Camera"));

	MainWindow::getInstance()->hide();
	QFuture<void> future = QtConcurrent::run([=]() {
		PluginsManager::Instance().Init();
		// 可以在这里进行更多的初始化操作
		QString strError;
		if (MainWindow::getInstance()->initData(strError) != 0) {
			MyMessageBox::question(nullptr, QObject::tr("InitData System Error!") + strError, QObject::tr("Error"), MyMessageBox::Ok);
			MainWindow::getInstance()->Exit();
			return;
		}
	});
	while (!future.isFinished()) {
		QApplication::processEvents();
		//splash.raise();
	}
	MainWindow::getInstance()->Init();
	qInfo() << QObject::tr("Process Start");
	QString strError;
	if (MainWindow::getInstance()->initUIData(strError) != 0) {
		MyMessageBox::question(nullptr, QObject::tr("InitData System Error!") + strError, QObject::tr("Error"), MyMessageBox::Ok);
		MainWindow::getInstance()->Exit();
		return 0;
	}
	MainWindow::getInstance()->setAttribute(Qt::WA_DeleteOnClose);
	MainWindow::getInstance()->hide();

	splash.showMessageText(QObject::tr("Plugins Is Loading"));
	for (auto& iter : dataVar::Instance().m_DialogbaseLst) {
		splash.showMessageText(iter->Info() + QObject::tr(" Is Initing"));
		iter->Init();
	}
	splash.updateNum(10);
	for (auto& iter : dataVar::Instance().m_DialogbaseLst) {
		splash.showMessageText(iter->Info() + QObject::tr(" Is LoadingData"));
		iter->LoadData();
	}
	splash.updateNum(40);

	splash.showMessageText(QObject::tr("Plugins Load Completed"));
	splash.showMessageText(QObject::tr("Init Layout"));

	splash.updateNum(70);
	splash.showMessageText(QObject::tr("Init Layout Completed"));
	splash.repaint();

	//QFuture<bool> 	value = QtConcurrent::run([=] {		Sleep(500);	return true;	});
	//while (!value.isFinished()) { QApplication::processEvents(); }
	splash.updateNum(100);
	splash.showMessageText(QObject::tr("Load Completed"));

	splash.finish(MainWindow::getInstance());

	MainWindow::getInstance()->show();
	MainWindow::getInstance()->activateWindow();
	MainWindow::getInstance()->showMaximized();
	MainWindow::getInstance()->raise();

	int ret = 0;
	try {
		ret = app.exec(); // 事件循环结束
	}
	catch (const std::exception& ex) {
		qCritical() << ex.what();
	}
	catch (const HException& except) {
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				qCritical() << QObject::tr("SoftWare Abnormal Exit") << strError;
			}
		}
	}
	catch (...) {
		qCritical() << QObject::tr("SoftWare Abnormal Exit");
	}
	qInfo() << QObject::tr("Process Exit");
	try
	{
		SaveLog::Instance()->stop();
		CameraManager::Instance().Exit();
		PluginsManager::Instance().Exit();
		dataVar::Instance().Exit();
	}
	catch (...) {
		exit(0);
	}
	return ret;
}
