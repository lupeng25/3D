#include "frmLog.h"
#include <QDateTime>
#include "Data/dataVar.h"
#include "qmutex.h"

FrmLog::FrmLog(QWidget* parent) :
	QWidget(parent)
{
	ui.setupUi(this);
	connect(SaveLog::Instance(), &SaveLog::send, this, &FrmLog::Slot_Log, Qt::DirectConnection);
	ui.textEdit->document()->setMaximumBlockCount(200);
}

FrmLog::~FrmLog()
{
}

bool FrmLog::event(QEvent * ev)
{
	if (ev->type() == LogEvent::EventType)
	{
		LogEvent* MyEvent = (LogEvent*)ev;
		if (MyEvent != nullptr)	OnActionLogOut(MyEvent->type, MyEvent->Message);
	}
	return QWidget::event(ev);
}

void FrmLog::Slot_Log(MsgType type, const QString & msg)
{
	QApplication::postEvent(this,new LogEvent(type, msg));
}

void FrmLog::OnActionLogOut(MsgType type, const QString & msg)
{
	m_QMutex.lock();
	switch (type)
	{
	default:
	case MsgType_Debug:
		ui.textEdit->append(QString("<span style=\"color:black\">%1</span>").arg(msg));
		break;
	case MsgType_Warning:
		ui.textEdit->append(QString("<span style=\"color:darkRed\">%1</span>").arg(msg));
		break;
	case MsgType_Critical:
		ui.textEdit->append(QString("<span style=\"color:red\">%1</span>").arg(msg));
		break;
	case MsgType_Fatal:
		ui.textEdit->append(QString("<span style=\"color:red\">%1</span>").arg(msg));
		break;
	case MsgType_Info:
		ui.textEdit->append(QString("<span style=\"color:blue\">%1</span>").arg(msg));
		break;
	}
	ui.textEdit->moveCursor(QTextCursor::End); //set to start
	m_QMutex.unlock();

}
