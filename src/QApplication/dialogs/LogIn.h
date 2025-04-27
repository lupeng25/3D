#pragma once

#include <QWidget>
#include "ui_LogIn.h"

class LogIn : public QWidget
{
	Q_OBJECT

public:
	LogIn(QWidget *parent = nullptr);
	virtual ~LogIn();
public:
	void initData();
	void initUI();
	void initConnect();
	virtual void RetranslationUi();
protected:
	void changeEvent(QEvent * event);
signals:
	void sigUserLogIn();
	void sigUserChange();
	void sigLogChanged(QString);
	void sigLogOut();

public slots:
	void on_btnLogin_clicked();
	void onLogOutClicked();
	void onRegisterClicked();
	void onModifyClicked();
	void slotLogOut();

protected:
	virtual void keyPressEvent(QKeyEvent *ev);
	virtual void showEvent(QShowEvent *ev);

private:
	void updatenames();
	QStringList getnames();
	void setTextAlignment(QComboBox* cbx,Qt::Alignment alignment = Qt::AlignCenter);
private:
	Ui::LogInClass ui;
	bool m_bFirstShow = true;
};
