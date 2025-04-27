#include "savelog.h"
#include "qmutex.h"
#include "qdir.h"
#include "qfile.h"
#include "qdatetime.h"
#include "qapplication.h"
#include "qtimer.h"
#include "qtextstream.h"
#include "qstringlist.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/async.h"

#include "Log/operatelog.h"
#include "Data/dataVar.h"

#define QDATE qPrintable(QDate::currentDate().toString("yyyy-MM-dd"))
#define QDATETIMS qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))

void Log(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	static QMutex mutex;
	QMutexLocker locker(&mutex);
	QString content;
	MsgType __type;
	int msgType = SaveLog::Instance()->getMsgType();
	switch (type) {
	case QtDebugMsg:
		if ((msgType & MsgType_Debug) == MsgType_Debug){
			__type = MsgType_Debug;
			content = QString("%1").arg(msg);
		}
		break;
	case QtInfoMsg:
		if ((msgType & MsgType_Info) == MsgType_Info){
			__type = MsgType_Info;
			content = QString("%1").arg(msg);
		}
		break;
	case QtWarningMsg:
		if ((msgType & MsgType_Warning) == MsgType_Warning){
			__type = MsgType_Warning;
			content = QString("%1").arg(msg);
		}
		break;
	case QtCriticalMsg:
		if ((msgType & MsgType_Critical) == MsgType_Critical){
			__type = MsgType_Critical;
			content = QString("%1").arg(msg);
		}
		break;
	case QtFatalMsg:
		if ((msgType & MsgType_Fatal) == MsgType_Fatal){
			__type = MsgType_Fatal;
			content = QString("%1").arg(msg);
		}
		break;
	}
	if (content.isEmpty()){
		return;
	}
	if (SaveLog::Instance()->getUseContext()) {
		int line = context.line;
		QString file = context.file;
		QString function = context.function;
		if (line > 0) {
			content = QString("Line:%1 File:%2 Function:%3\n%4").arg(line).arg(file).arg(function).arg(content);
		}
	}	
	SaveLog::Instance()->LogInfo(__type,content);
}

QScopedPointer<SaveLog> SaveLog::self;
SaveLog* SaveLog::Instance()
{
	if (self.isNull()){
		static QMutex mutex;
		QMutexLocker locker(&mutex);
		if (self.isNull()){
			self.reset(new SaveLog);
		}
	}
	return self.data();
}

SaveLog::SaveLog(QObject* parent) : QObject(parent)
{	
	maxRow = currentRow = 0;
	maxSize = 0;
	useContext = false;
	path = qApp->applicationDirPath() + "/Log/";
	QDir dir(path);
	if (!dir.exists())	
		dir.mkdir(path);

	QString str = path + "log.log";
	msgType = MsgType(MsgType_Debug | MsgType_Info | MsgType_Warning | MsgType_Critical | MsgType_Fatal);
	auto m_logger = spdlog::daily_logger_mt("daily_logger", str.toStdString().c_str(), 0, 0);
	m_logger->set_level(spdlog::level::trace);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
	spdlog::flush_every(std::chrono::seconds(2));
}

SaveLog::~SaveLog()
{
}

bool SaveLog::getUseContext()
{
	return this->useContext;
}

MsgType SaveLog::getMsgType()
{
	return this->msgType;
}
void SaveLog::start()
{
	qInstallMessageHandler(Log);
}

void SaveLog::stop()
{
	this->clear();
	spdlog::drop_all();
	qInstallMessageHandler(0);
}
void SaveLog::clear()
{
	currentRow = 0;
}
void SaveLog::setMaxRow(int maxRow)
{
	if (maxRow >= 0) {
		this->maxRow = maxRow;
		this->clear();
	}
}

void SaveLog::setMaxSize(int maxSize)
{
	if (maxSize >= 0) {
		this->maxSize = maxSize;
		this->clear();
	}
}

void SaveLog::LogInfo(MsgType type, QString str)
{
	auto logger = spdlog::get("daily_logger");
	OPERATE_LOG_T _log;
	_log.sTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	_log.sUser = dataVar::Instance().strUserName;
	QString strLog;
	switch (type)
	{
	case MsgType_Debug: {
		_log.sType = /*tr*/("debug");
		logger->debug(QString("[" + _log.sUser + "] " + str).toStdString());
	}	break;
	case MsgType_Info: {
		_log.sType = /*tr*/("info");
		logger->info(QString("[" + _log.sUser + "] " + str).toStdString() );
	}	break;
	case MsgType_Warning: {
		_log.sType = /*tr*/("warning");
		logger->warn(QString("[" + _log.sUser + "] " + str).toStdString());
	}	break;
	case MsgType_Critical: {
		_log.sType = /*tr*/("critical");
		logger->critical(QString("[" + _log.sUser + "] " + str).toStdString());
	}	break;
	case MsgType_Fatal: {
		_log.sType = /*tr*/("fatal");
		logger->error(QString("[" + _log.sUser + "] " + str).toStdString());
		//logger->error("[" + _log.sUser.toStdString() + "] " + str.toStdString() );
	}	break;
	default: {
		_log.sType = /*tr*/("trace");
		logger->trace(QString("[" + _log.sUser + "] " + str).toStdString());
		//logger->trace("[" + _log.sUser.toStdString() + "] " + str.toStdString() );
	}	break;
	}
	strLog = QString(" [" + tr(_log.sType.toStdString().c_str()) + "] " + str);
	_log.sContent = str;
	OperateLog::getInstance().appendLog(_log);
	emit sigUpdate();
	emit send(type, QDateTime::currentDateTime().toString("MM-dd hh:mm:ss.zzz") + strLog);
}

void SaveLog::setUseContext(bool useContext)
{
	this->useContext = useContext;
}

void SaveLog::setPath(const QString& path)
{
	this->path = path;
	QDir dir(path);
	if (!dir.exists())
		dir.mkdir(path);
}

void SaveLog::setMsgType(const MsgType& msgType)
{
	this->msgType = msgType;
}
