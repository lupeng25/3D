#pragma once

#include <QWidget>
#include <QTimer>
#include "ui_QtMotionIOWidgets.h"
#include "Pages/QtWidgetsBase.h"
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include "Motion\MOtionIO.h"
//#include "IOC0640.H"

class QtMotionIOWidgets : public QtWidgetsBase
{
	Q_OBJECT
public:
	QtMotionIOWidgets(QWidget *parent = Q_NULLPTR);
	~QtMotionIOWidgets();
public:
	virtual void initUI();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);
	int Load();
	int Save();
	virtual int	NewProject();
signals:

public slots :
	void slotHomeAxis();
protected:
	virtual void showEvent(QShowEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);

private:
	void ResizeUI();
	int InitAxis(QString& strError);
private slots :
	void slotTimeOut();
	void slotPressIOOutPut();
	void slotContextMenuRequested(const QPoint &pos);

	void AddAxis();
	void DeleteAxis();
	void ModifyAxis();

private:
	QMap<int, QVector<QCheckBox*>>	m_mapInPutCheckBox;
	QMap<int, QVector<QCheckBox*>>	m_mapOutPutCheckBox;
	QMap<int, QTableWidget*>		m_mapAxisTab;
	QMap<int, AxisParam>			m_mapAxisParam;


private:
	Ui::QtMotionIOWidgetsClass ui;
	QTimer *				m_pTimer{ nullptr };
	QWidget*				m_pTabMode{ nullptr };
	QLabel *				m_pTextlabel{ nullptr };
	QComboBox *				m_pQComboBox{ nullptr };
};
