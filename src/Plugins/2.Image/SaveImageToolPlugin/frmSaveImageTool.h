#pragma once

#include <QDialog>
#include "ui_frmSaveImageTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "SaveImageTool.h"
#include <QDateTime>

#include "frmBaseTool.h"
class frmSaveImageTool : public frmBaseTool
{
	Q_OBJECT

public:
	frmSaveImageTool(QWidget* parent = Q_NULLPTR);
	~frmSaveImageTool();
private:
	Ui::frmSaveImageToolClass ui;

public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	//virtual int Execute(const QString toolname = "");
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initConnect();

private slots:
	void on_btnSelectImage_clicked();
	void on_btnSelectPath_clicked();
private slots:	

private:		
	QButtonGroup* pBtnGroupRadio = nullptr;
	int tool_index = 0;
	int radio_index;
};
