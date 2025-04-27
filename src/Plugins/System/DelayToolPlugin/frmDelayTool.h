#pragma once

#include <QDialog>
#include "ui_frmDelayTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "DelayTool.h"
#include "frmBaseTool.h"

class frmDelayTool : public frmBaseTool
{
	Q_OBJECT

public:
	frmDelayTool(QWidget* parent = Q_NULLPTR);
	~frmDelayTool();

private:
	Ui::frmDelayToolClass ui;

public:
	virtual int Execute(const QString toolname = "");
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
private:
	void initTitleBar();

private:		
	QButtonGroup* btnGroupRadio;
	int tool_index = 0;
	int radio_index;
};
