#pragma once

#include <QDialog>
#include "ui_frmImageLayout.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "frmBase.h"

class frmImageLayout : public frmBase
{
	Q_OBJECT

public:
	frmImageLayout(QWidget* parent = Q_NULLPTR);
	~frmImageLayout();
public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
private:
	Ui::frmImageLayoutClass ui;

private:
	void initTitleBar();
	virtual void ChangeLanguage();
	void showEvent(QShowEvent *ev);
private slots:
	void onButtonCloseClicked();
	void slot_Image(bool);
public:
Q_SIGNALS:
	void sig_ChangeVideoCount(int iCount);
private:	
	QButtonGroup* btnGroupRadio;	
};
