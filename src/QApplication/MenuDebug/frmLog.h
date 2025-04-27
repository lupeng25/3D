#pragma once

#include <QtWidgets/QWidget>
#include "ui_frmLog.h"
#include <QWidget>
#include "QPlainTextEdit"
#include "Log/savelog.h"
#include <QMutex>

class FrmLog : public QWidget
{
    Q_OBJECT

public:
    explicit FrmLog(QWidget *parent = Q_NULLPTR);   
    ~FrmLog();
public slots:
	void Slot_Log(MsgType type,const QString & msg);
protected:
	bool event(QEvent *ev) override;
public:
	void OnActionLogOut(MsgType type, const QString &msg);

private:
    Ui::frmLogClass		ui;
	QMutex				m_QMutex;
};