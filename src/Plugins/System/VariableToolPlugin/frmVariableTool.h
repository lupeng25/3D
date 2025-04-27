#pragma once

#include <QDialog>
#include "ui_frmVariableTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include <QItemDelegate>
#include "Port.h"
#include <QMutex>
#include <QTableWidget>
#include "VariableTool.h"
#include "frmBaseTool.h"

//局部变量窗口
class frmVariableTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmVariableTool(QWidget* parent = Q_NULLPTR);
	~frmVariableTool();

public:
	int Load();
	int Save();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	static int SetData(QJsonArray& strData, MiddleParam& Param);
	int	NewProject();
private:
	Ui::frmVariableToolClass ui;

protected:	//图像
	virtual void showEvent(QShowEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);
private:
	void initTitleBar();
	void initConnect();
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);
	void ResizeUI();
private slots:	
	void on_btnAdd_clicked();
	void on_btnSub_clicked();
	void on_btnMoveUp_clicked();
	void on_btnMoveDown_clicked();
	QString slot_GlobalValue(const QString strVar, const QString value, const int flowIndex);

signals:
	QString sig_GlobalValue(const QString strVar, const QString value, const int flowIndex);

private:
	QMutex mutex;
};

