#ifndef DADUMPCAPTURE_H
#define DADUMPCAPTURE_H
// stl
#include <functional>
// qt
#include <QtGlobal>
#include <QObject>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QDateTime>
#include <QTextStream>
#include <QSysInfo>
#include <QScreen>
#include "Common/MyMessageBox.h"
#include "Data/dataVar.h"

// win32
#ifdef Q_OS_WIN
#ifdef Q_CC_MSVC

// msvc下才有用
#include <Windows.h>
#include "DbgHelp.h"
#include <TCHAR.h>
#pragma comment(lib, "dbghelp.lib")

namespace DA
{
LONG WINAPI ApplicationCrashHandler(EXCEPTION_POINTERS *pException);
//程序异常捕获
LONG WINAPI ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
	//初始化dump文件夹
	QString logFilePath = QCoreApplication::applicationDirPath() + "/dumps/";
	QDir dstDir(logFilePath);
	if (!dstDir.exists())	{
		if (!dstDir.mkpath(logFilePath))	{
			qDebug() << __FILE__ << __LINE__ << "创建DumpCrashes文件夹失败！";
		}
	}
	//创建Dump文件
	QString dumpFileName = logFilePath + "dump" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
	HANDLE hDumpFile = CreateFile((LPCWSTR)((dumpFileName + ".dmp").toStdWString().c_str()), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE)	{
		//Dump信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;
		//写入Dump文件内容
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules), &dumpInfo, NULL, NULL);
	}

	// 获取屏幕对象
	QScreen *screen = QGuiApplication::primaryScreen();
	// 截取整个屏幕
	QPixmap pixmap = screen->grabWindow(0);
	// 保存图片到本地
	pixmap.save(dumpFileName + ".bmp");

	//这里弹出一个错误对话框并退出程序
	EXCEPTION_RECORD* record = pException->ExceptionRecord;
	QString errCode(QString::number(record->ExceptionCode, 16));
	QString errAddr(QString::number((DWORD)record->ExceptionAddress, 16));

	MyMessageBox::critical(nullptr, QObject::tr("Program exception crash capture!\nerrCode:") 
		+ errCode.toStdString().c_str() + QObject::tr("\nerrAddr:") + errAddr.toStdString().c_str(), QObject::tr("Error"), MyMessageBox::Ok);
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif
#endif

/** * @brief The DADumpCapture class */
class DADumpCapture
{
public:
public:
    static void initDump()
    {
#ifdef Q_OS_WIN
#ifdef Q_CC_MSVC
		QString strDumpFileDir = QApplication::applicationDirPath() + "/dumps";
		QDir dir(strDumpFileDir);
		if (!dir.exists()) {
			if (!dir.mkpath(strDumpFileDir))
				qDebug() << QStringLiteral("创建dump文件目录失败");
		}
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);   //捕获构造完成之后的异常
#endif
#endif
    }
};

}  // end DA
#endif  // DADUMPCAPTURE_H