#pragma once

#include <QDialog>
#include "ui_frmRunFlowTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "RunFlowTool.h"
//#include "ComSocket.h"

#include "frmBaseTool.h"
class frmRunFlowTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmRunFlowTool(QWidget* parent = Q_NULLPTR);
	~frmRunFlowTool();

private:
	Ui::frmRunFlowToolClass ui;

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

public:
	QString m_strModelIndex		= "";
	QString m_strTypeVariable	= "";
	QString	m_strValueName		= "";


};