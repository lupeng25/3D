#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QScrollArea>
#include "ui_QFlowChart.h"
#include "Pages/QtWidgetsBase.h"
#include "FlowGraphicsViews.h"

#include "DockManager.h"
#include "DockAreaWidget.h"
#include "DockWidget.h"

#include "frmflow.h"
#include "frmToolFlow.h"
#include "frmImageView.h"
#include "frmLog.h"
#include "frmToolsTree.h"

class QFlowChart : public QtWidgetsBase
{
	Q_OBJECT
public:
	QFlowChart(QWidget *parent = Q_NULLPTR);
	~QFlowChart();
public:
	virtual void initUI();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);

	void initDock();
	void createdock();
	void InitConnect();

public:
	virtual void ChangeLanguage();
private:
	void UpDateTab();
	QList<QAction*> findMeItems(QString strData);
	QList<QAction*> findMenuItems(QMenu* pItem, QString strData);
	int AddMenuItems(QMenu* pItem,QVector<QPair<QString, QString>>& strData,int iIndex);
private slots:

public slots:
	
protected:
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
private:
	Ui::QFlowChart	ui;
public:
	ads::CDockManager*							DockManager			{ nullptr };
	QMap<QString, ads::CDockWidget*>			m_mapDockWidget;
	QMap<QString, ads::CDockAreaWidget*>		m_mapAreaDockWidget;
	ads::CDockWidget*							m_pMainDockWidget	{ nullptr };
	QList<QAction*>								m_DockActionLst;

	ads::CDockWidget*							m_pCentralDock	{ nullptr };
	ads::CDockWidget*							m_pFlowDock		{ nullptr };
	ads::CDockWidget*							m_pToolDock		{ nullptr };
	ads::CDockWidget*							m_pFlowManager	{ nullptr };
	ads::CDockWidget*							m_pLogDock		{ nullptr };

	ads::CDockAreaWidget*						m_pAreaCentralDock	{ nullptr };
	ads::CDockAreaWidget*						m_pAreaFlowDock		{ nullptr };
	ads::CDockAreaWidget*						m_pAreaToolDock		{ nullptr };
	ads::CDockAreaWidget*						m_pAreaFlowLstDock	{ nullptr };
	ads::CDockAreaWidget*						m_pAreaLogDock		{ nullptr };

	frmToolsTree*								m_pfrmToolsTree{ nullptr };
	frmflow*									m_pfrmflow{ nullptr };
	frmToolFlow*								m_pfrmToolFlow{ nullptr };
	FrmLog*										m_pFrmLog{ nullptr };
	FrmImageView*								m_pFrmImageView{ nullptr };
};