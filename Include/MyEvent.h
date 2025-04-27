#pragma once
#ifndef MYEVENT_H
#define MYEVENT_H

#include <QEvent>
#include <QObject>
//

#ifndef _MYEVENT_EXPORT_
#define _MYEVENT_API_ _declspec(dllexport)
#else
#define _MYEVENT_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

//消息类型
enum MsgType {
	MsgType_Debug = 0x0001,
	MsgType_Info = 0x0002,
	MsgType_Warning = 0x0004,
	MsgType_Critical = 0x0008,
	MsgType_Fatal = 0x0010,
};
class _MYEVENT_API_ LogEvent : public QEvent
{
public:
	static const QEvent::Type EventType;
	explicit LogEvent(MsgType, const QString& message);
	~LogEvent();
public:
	const QString Message;
	MsgType type;
};
class _MYEVENT_API_ MsgEvent : public QEvent
{
public:
	static const QEvent::Type EventType;
	explicit MsgEvent(int index, int subindex, const QString& message);
	~MsgEvent();
public:
	const QString Message;
	int iIndex;
	int isubIndex;
};

//事件
class _MYEVENT_API_ QCreateEvent
{
public:
	explicit QCreateEvent(bool bManualReset = true, bool bInitialState = false);
	~QCreateEvent();
public:
	const int EventRetnTimeOut = 258L;
public:
	void SetEvent();
	void RstEvent();

	// timeout < 0 表示无限时间
	//返回值为EventRetnTimeOut表示事件超时 其他为其他事件 >= 0 
	int  WaitEvent(int timeout = -1);

private:
	void*			m_hTrrigerHand;
};

#endif // MOD_H