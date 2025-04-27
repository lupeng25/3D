#pragma once

#include <QDialog>
#include "ui_frmScriptEditTool.h"
#include "mytitlebar.h"
#include <QTextCodec>
#include <QCompleter>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QMetaType>
#include <QGraphicsOpacityEffect>
#include "ScriptEditTool.h"

#include <Qsci\qsciscintilla.h>
#include <Qsci\qscilexercustom.h>
#include <Qsci/QsciAPIs.h>
#include <Qsci/qscilexerlua.h>
#include <functional>
#include <qvector.h>
#include "frmBaseTool.h"

#include "DockManager.h"
#include "DockAreaWidget.h"
#include "DockWidget.h"

class frmScriptEditTool : public frmBaseTool
{
	Q_OBJECT
public:	
	frmScriptEditTool(QWidget* parent = Q_NULLPTR);
	~frmScriptEditTool();

private:
	Ui::frmScriptEditToolClass ui;

public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	virtual int	ExecuteComplete();
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initSplitter();
	void initConnect();
	void createdock();
	void InitLayOut();

protected:
	virtual void showEvent(QShowEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);
private slots:
	void slot_ItemDoubleClicked(QTreeWidgetItem* item, int column);
	void slot_CreateList();
	void slot_DataModeChange(int iMode);
	void slot_DoubleClicked(int, int);
	void slot_WidgetDoubleClicked(int, int);
	void slot_Accepted();
	void set_Update_tableWidget(MiddleParam& param);
signals:
	void sig_UpdateQTreeWidget(QString, QString);
private:
	void form_load();
	void ToolsTreeWidgetInit();
	void DefaultCallTips();
	void ResizeUI();
private:	
	int tool_index = 0;
	typedef std::pair<QString, std::list<QString> > ToolsPair;
	QTreeWidget*			ToolTreeWidget;
	QVector<QString>		m_vecCallTips;
	std::vector<ToolsPair>	ToolNamesVec;
	QString					m_strShowMsg;

	QsciLexer*				m_textLexer;
	QsciAPIs *				m_apis;
	QGraphicsOpacityEffect* e;
	QGraphicsOpacityEffect* e2;
	lua_State*	m_LuaState;

	ads::CDockManager*							DockManager{ nullptr };
	QMap<QString, ads::CDockWidget*>			m_mapDockWidget;
	QMap<QString, ads::CDockAreaWidget*>		m_mapAreaDockWidget;
	ads::CDockWidget*							m_pMainDockWidget{ nullptr };
	QList<QAction*>								m_DockActionLst;

	ads::CDockWidget*							m_pCentralDock{ nullptr };
	ads::CDockWidget*							m_pModulDock{ nullptr };
	ads::CDockWidget*							m_pToolDock{ nullptr };
	ads::CDockWidget*							m_pTextTips{ nullptr };
	ads::CDockWidget*							m_pLogDock{ nullptr };
	ads::CDockWidget*							m_pVariableDock{ nullptr };

	ads::CDockAreaWidget*						m_pAreaCentralDock{ nullptr };
	ads::CDockAreaWidget*						m_pAreaModeDock{ nullptr };
	ads::CDockAreaWidget*						m_pAreaToolDock{ nullptr };
	ads::CDockAreaWidget*						m_pAreaTextTipsDock{ nullptr };
	ads::CDockAreaWidget*						m_pAreaLogDock{ nullptr };
	ads::CDockAreaWidget*						m_pAreaVariableDock{ nullptr };
};