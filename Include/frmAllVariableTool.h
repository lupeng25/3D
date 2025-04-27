#pragma once

#include <QDialog>
#include <QButtonGroup>
#include "mytitlebar.h"
#include <QItemDelegate>
#include "Port.h"
#include <QMutex>
#include <QTableWidget>
#include "basemainwindow.h"
#include "frmBase.h"

//class frmAllVariableToolClass;
namespace Ui {
	class frmAllVariableToolClass;
} // namespace Ui

enum EnumLimitType
{
	EnumLimitType_NotAll		= 0x00000000,
	EnumLimitType_Bool			= 0x00000400,
	EnumLimitType_Int			= 0x00000800,
	EnumLimitType_Dou			= 0x00001000,
	EnumLimitType_Str			= 0x00002000,
	EnumLimitType_Point			= 0x00004000,
	EnumLimitType_PointF		= 0x00008000,
	EnumLimitType_ResultPointF	= 0x00010000,
	EnumLimitType_Metrix		= 0x00020000
};

//局部变量窗口
class _MYTITLEBAR_API_ frmAllVariableTool : public frmBase
{
	Q_OBJECT
public:
	frmAllVariableTool(QWidget* parent = Q_NULLPTR);
	~frmAllVariableTool();
public:
	void SetLimitModel(QString strModel = "", int Type = EnumLimitType_NotAll);
public:
	void SetCurrentLinkContent(QString strLinkContent);
	void SetCurrentVariable(QString strModel, QString TypeVariable, QString strValueName);
	void GetCurrentVariable(QString& strModel,QString &TypeVariable,QString& strValueName);
public:
	int	m_iRetn;
public:
	int Load();
	int Save();
	static int SetData(QJsonArray& strData, MiddleParam& Param);
	int	NewProject();

	QString m_strModelIndex		= "";
	QString m_strTypeVariable	= "";
	QString	m_strValueName		= "";
	int		m_iCurrentIndex		= -1;
private:
	Ui::frmAllVariableToolClass* ui;

private:
	void initTitleBar();
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);

	virtual void resizeEvent(QResizeEvent *ev);
public slots:
	void onButtonCloseClicked();

	void slot_DoubleClicked(int row, int column);
	void slot_VariableDoubleClicked(int row, int column);
private slots:	
	void on_btnAdd_clicked();
	void on_btnDelete_clicked();
	void on_btnMoveUp_clicked();
	void on_btnMoveDown_clicked();
	void set_Update_tableWidget(MiddleParam& param);
	QString slot_GlobalValue(const QString strVar, const QString value, const int flowIndex);

public slots:
	QString slot_SetGlobalValue(const QString strVar, const QString value, const int flowIndex);

signals:
	QString sig_GlobalValue(const QString strVar, const QString value, const int flowIndex);

private:
	void ClearModelTable();
private:
	QMutex mutex;

	QString	m_strLimitModel;
	int		m_eLimitType;
};