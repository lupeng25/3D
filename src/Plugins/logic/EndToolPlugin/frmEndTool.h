#pragma once

#include <QDialog>
#include "ui_frmEndTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "EndTool.h"
//#include "ComSocket.h"

#include "frmBaseTool.h"
class frmEndTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmEndTool(QWidget* parent = Q_NULLPTR);
	~frmEndTool();

private:
	Ui::frmIFToolClass ui;

public:
	virtual int Execute(const QString toolname = "");
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
private:
	void initTitleBar();

private:		
	QButtonGroup* btnGroupRadio = nullptr;
	int tool_index = 0;
	int radio_index;

public:
	QString m_strModelIndex		= "";
	QString m_strTypeVariable	= "";
	QString	m_strValueName		= "";

};