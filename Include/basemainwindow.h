#pragma once

#include <QtWidgets/QWidget>
#include <QDialog>
#include "mytitlebar.h"
//class MyTitleBar;
#include <QShowEvent>
#include <QCloseEvent>

//要继承QMainWindow，则需要把QWidget改为QMainWindow
class _MYTITLEBAR_API_ BaseWindow : public QDialog
{
	Q_OBJECT
public:
	BaseWindow(QWidget*parent = 0);
	~BaseWindow();
private:
	void initTitleBar();
	void loadStyleSheet(const QString &sheetName);

protected:
	void showEvent(QShowEvent * event);
	void closeEvent(QCloseEvent * event);
	void changeEvent(QEvent * ev);
protected:
	virtual void ChangeLanguage();
public slots:
	void onButtonMinClicked();
	void onButtonRestoreClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();

	void animationClose();
protected:
	MyTitleBar* m_titleBar = nullptr;	
};
