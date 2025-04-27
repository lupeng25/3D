#pragma once

#include <QWidget>
#include "ui_QtUserWidgets.h"
#include "Main\dbpage.h"
#include "QtWidgetsBase.h"

class QtUserWidgets : public QtWidgetsBase
{
	Q_OBJECT
public:
	QtUserWidgets(QWidget *parent = Q_NULLPTR);
	~QtUserWidgets();

public:
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);
public:
	void initForm();
	void SetColumnNames();
Q_SIGNALS:

	void sigExcuteSql(QString strDatabase, QString strSql);
	private slots:
	void slotExcuteSql(QString strDatabase, QString strSql);
	void slotUpdate(QString strName);
	void slotActive(const QModelIndex &index);
	void on_btnADD_clicked();
	void on_btnModify_clicked();
	void on_btnDelete_clicked();
	public slots:
	void on_btnSelect_clicked();
protected:
	void resizeEvent(QResizeEvent * ev);
	void ResizeUI();
	void showEvent(QShowEvent *ev);
	void hideEvent(QHideEvent *ev);
	void changeEvent(QEvent * ev);
private:
	Ui::QtUserWidgetsClass ui;
	virtual void RetranslationUi();
private:
	QList<QString> columnNames; 	//	
	QList<int> columnWidths;    		//
	DbPage *pdbPage{ nullptr };             		//
	QString tableName;          		//
	QString countName;          		//
};
