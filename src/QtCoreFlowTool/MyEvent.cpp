#include <qdebug.h>
#include <Windows.h>
#include "MyEvent.h"
const QEvent::Type LogEvent::EventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 1));
const QEvent::Type MsgEvent::EventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 2));

LogEvent::LogEvent(MsgType type, const QString & message)
	:QEvent(EventType)
	,type(type)
	,Message(message)
{
}

LogEvent::~LogEvent()
{
}

MsgEvent::MsgEvent(int index, int subindex, const QString& message)
	:QEvent(EventType)
	, iIndex(index)
	, isubIndex(subindex)
	, Message(message)
{
}

MsgEvent::~MsgEvent()
{
}

QCreateEvent::QCreateEvent(bool bManualReset, bool bInitialState)
{
	m_hTrrigerHand = ::CreateEventA(NULL, bManualReset, bInitialState, NULL);
}

QCreateEvent::~QCreateEvent()
{
	if(m_hTrrigerHand != nullptr)
		CloseHandle(m_hTrrigerHand);
	m_hTrrigerHand = nullptr;
}

void QCreateEvent::SetEvent()
{
	::SetEvent(m_hTrrigerHand);
}

void QCreateEvent::RstEvent()
{
	::ResetEvent(m_hTrrigerHand);
}

int QCreateEvent::WaitEvent(int timeout)
{
	DWORD dwRetn = WAIT_OBJECT_0;
	if (timeout < 0)
		dwRetn = WaitForSingleObject(m_hTrrigerHand,INFINITE);
	else
	{
		dwRetn = WaitForSingleObject(m_hTrrigerHand, timeout);
		if (dwRetn == WAIT_TIMEOUT)
			return EventRetnTimeOut;
		else
		{
			return dwRetn - WAIT_OBJECT_0;
		}
	}
	return dwRetn;
}
