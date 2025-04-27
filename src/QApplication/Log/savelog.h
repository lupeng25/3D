#pragma once

#include <QObject>
#include "MyEvent.h"
class QFile;

class SaveLog : public QObject
{
	Q_OBJECT

public:
	static SaveLog* Instance();
	explicit SaveLog(QObject* parent = 0);
	~SaveLog();

private:
	static QScopedPointer<SaveLog> self;
	int maxRow, currentRow;
	int maxSize;
	bool useContext;
	QString path;
	MsgType msgType;
public:
	bool getUseContext();
	MsgType getMsgType();

Q_SIGNALS:
	void send(MsgType type,const QString& content);
	void sigUpdate();
public Q_SLOTS:
	void start();
	void stop();
	void clear();
	void setMaxRow(int maxRow);
	void setMaxSize(int maxSize);
	void LogInfo(MsgType type,QString str);
	void setUseContext(bool useContext);
	void setPath(const QString& path);
	void setMsgType(const MsgType& msgType);
};

