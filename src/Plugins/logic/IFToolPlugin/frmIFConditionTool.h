#pragma once

#include <QDialog>
#include "ui_frmIFConditionTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "IFTool.h"
//#include "ComSocket.h"
#include "frmBaseTool.h"

class frmIFConditionTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmIFConditionTool(QWidget* parent = Q_NULLPTR);
	~frmIFConditionTool();

private:
	Ui::frmIFConditionToolClass ui;

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

	void on_btnLinkSecondContent_clicked();
	void on_btnDelLinkSecondContent_clicked();
private:		
	void UpdateOperator();

public:
	QString m_strModelIndex		= "";
	QString m_strTypeVariable	= "";
	QString	m_strValueName		= "";
};