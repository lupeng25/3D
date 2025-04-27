#pragma once

#include <QDialog>
#include "ui_frmGlobalVariable.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include <QItemDelegate>
#include "Port.h"
#include <QMutex>
#include "frmBase.h"

class frmGlobalVariable : public frmBase
{
	Q_OBJECT
public:
	frmGlobalVariable(QWidget* parent = Q_NULLPTR);
	~frmGlobalVariable();

public:
	int GetData(QJsonObject& strData);
	int SetData(QJsonObject& strData);
	int	NewProject();
public:
	void UpdateGlobalVar();
private:
	Ui::frmGlobalVariableClass ui;
protected:
	void changeEvent(QEvent * ev);
	void resizeEvent(QResizeEvent *ev);
	void showEvent(QShowEvent *ev);
private:
	void initTitleBar();
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);

public slots:
	void onButtonCloseClicked();

private slots:	
	void on_btnAdd_clicked();
	void on_btnSub_clicked();
	void on_btnMoveUp_clicked();
	void on_btnMoveDown_clicked();
	QString slot_GlobalValue(const QString strVar, const QString value, const int flowIndex);

public slots:
	QString slot_SetGlobalValue(const QString strVar, const QString value, const int flowIndex);

signals:
	QString sig_GlobalValue(const QString strVar, const QString value, const int flowIndex);

private:
	QMutex mutex;
};

class ReadOnlyDelegate : public QItemDelegate
{
public:
	ReadOnlyDelegate(QWidget* parent = NULL) :QItemDelegate(parent)
	{}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override
	{
		Q_UNUSED(parent)
			Q_UNUSED(option)
			Q_UNUSED(index)
			return NULL;
	}
};
