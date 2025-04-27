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

	void createdock();
private:
	void UpDateTab();
	QList<QAction*> findMeItems(QString strData);
	QList<QAction*> findMenuItems(QMenu* pItem, QString strData);
	int AddMenuItems(QMenu* pItem,QVector<QPair<QString, QString>>& strData,int iIndex);
private slots:
	void on_btnRunOnce_clicked();
	void on_btnRunCycle_clicked();
	void on_btnStop_clicked();
public slots:
	void slotAddTabWidget();						//添加页面
	void slotCloseTabWidget(int i);					//关闭选项卡
	void slotSetTabName(int id);					//设置选项卡的名字
	void slotGetTabName();							//输入框中获取选项卡的名字
	void slotSetTabCurrentIndex(int index);         //输入框中获取选项卡的名字
	void slotStopStatus();
	void slotCheckFlowTool();
protected:
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
private:
	Ui::QFlowChart	ui;
	QLineEdit *		m_pLEditName{ nullptr };     //选项卡名输入框
	QToolButton *	m_pBtnTabWidget{ nullptr };
	QToolButton *	m_pBtnToolWidget{ nullptr };
	QMenu *			m_pMenuToolWidget{ nullptr };
	int				m_iTempTabId = -1;

public:
	ads::CDockManager*						DockManager = nullptr;
	QMap<QString, ads::CDockWidget*>		m_mapDockWidget;
	QMap<QString, ads::CDockAreaWidget*>	m_mapAreaDockWidget;
	ads::CDockWidget*						m_pMainDockWidget{ nullptr };
	QList<QAction*>							m_DockActionLst;
};