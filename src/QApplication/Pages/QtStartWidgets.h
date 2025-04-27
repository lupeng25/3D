#pragma once

#include <QWidget>
#include "ui_QtStartWidgets.h"
#include <QObject>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTime>
#include <QCoreApplication>
#include <QLabel>

class QtStartWidgets : public QWidget
{
	Q_OBJECT
public:
	QtStartWidgets(QWidget *parent = Q_NULLPTR);
	~QtStartWidgets();
public:
	void  setRange(int min, int max);
	void  updateNum(int n);
	void  showAppText(QString str);
	void  showAppOtherInfo(QString str);
	void  showAppImg(QString strFileName);
	void  showMessageText(QString str);
	void  showVisionText(QString str);
	void  finish(QWidget *parent = Q_NULLPTR);
protected:
	void changeEvent(QEvent *);
private:
	Ui::QtStartWidgets ui;
};