#pragma once

#include <QDialog>
#include "ui_frmForTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "ForTool.h"
//#include "ComSocket.h"

#include "frmBaseTool.h"
class frmForTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmForTool(QWidget* parent = Q_NULLPTR);
	~frmForTool();

private:
	Ui::frmForToolClass ui;

public:
	virtual int Execute(const QString toolname = "");
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initConnect();
private slots:
	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();

private:		
	QButtonGroup* btnGroupRadio = nullptr;
	int tool_index = 0;
	int radio_index;

public:
	QString m_strModelIndex		= "";
	QString m_strTypeVariable	= "";
	QString	m_strValueName		= "";

};