#include "frmScriptEditTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleFactory>
#include <QHeaderView>
#include "qmutex.h"
#include <QDesktopWidget>
#include <QApplication>
#include "databaseVar.h"
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "gvariable.h"

#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
using namespace ads;

frmScriptEditTool::frmScriptEditTool(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	DefaultCallTips();
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);

	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Script.png"));
	//初始化标题栏
	initTitleBar();
	initConnect();
	ToolsTreeWidgetInit();
	//父类如果有样式表，背景色设置方法
	e = new QGraphicsOpacityEffect(this);
	e->setOpacity(0.75);
	ui.plainTextExample->setGraphicsEffect(e);
	e2 = new QGraphicsOpacityEffect(this);
	e2->setOpacity(0.75);
	ui.txtMsg->setGraphicsEffect(e2);
	form_load();

	m_textLexer = new QsciLexerLua;
	ui.widget->setLexer(m_textLexer);//给QsciScintilla设置词法分析器
	m_apis = new QsciAPIs(m_textLexer);
	for (auto iter : m_vecCallTips)
	{
		m_apis->add(iter);
		//进行单个查找
		QString _str;
		for (size_t i = 0; i < iter.length(); i++)
		{
			if (isalpha(iter.at(i).toLatin1()) || iter.at(i).toLatin1() == '_')
				_str.append(iter.at(i).toLatin1());
			else	break;
		}
		ui.widget->addLexerString(1, _str.append(" "));
	}

	ui.widget->lexer()->setColor(QColor(0x00, 0x7f, 0x00), QsciLexerLua::LineComment);
	ui.widget->lexer()->setPaper(QColor(0xff, 0xff, 0xff), QsciLexerLua::LineComment);
	ui.widget->lexer()->setColor(QColor(0x00, 0x7f, 0x7f), QsciLexerLua::BasicFunctions);
	ui.widget->lexer()->setPaper(QColor(0xff, 0xff, 0xff), QsciLexerLua::BasicFunctions);
	ui.widget->lexer()->setColor(QColor(0xff, 0x00, 0x00), QsciLexerLua::KeywordSet8);
	ui.widget->lexer()->setPaper(QColor(0x00, 0x00, 0x00), QsciLexerLua::KeywordSet8);
	ui.widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	//ui.widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//加载基本关键字
	m_apis->prepare();

	//设置表格列宽	
	ui.tableWidget->setColumnWidth(0, 120);
	ui.tableWidget->setColumnWidth(1, 200);
	ui.tableWidget->setColumnWidth(2, 240);
	ui.tableWidget->setColumnWidth(3, 240);

	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	//connect(this, &frmAllVariableTool::sig_GlobalValue, this, &frmAllVariableTool::slot_GlobalValue, Qt::BlockingQueuedConnection);
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数

	//隐藏水平header
	ui.tableModelWidget->verticalHeader()->setVisible(false);
	ui.tableModelWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableModelWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	count = ui.tableModelWidget->rowCount();
	for (int i = 0; i < count; i++)	ui.tableModelWidget->removeRow(0);  //设置行数

	initSplitter();
	createdock();
	InitLayOut();

}

frmScriptEditTool::~frmScriptEditTool()
{
	if (m_titleBar != nullptr)		delete m_titleBar;		m_titleBar = nullptr;
	if (m_apis != nullptr)			delete m_apis;			m_apis = nullptr;
	if (m_textLexer != nullptr)		delete m_textLexer;		m_textLexer = nullptr;
	if (ToolTreeWidget != nullptr)	delete ToolTreeWidget;	ToolTreeWidget = nullptr;
	if (e != nullptr)				delete e;				e = nullptr;
	if (e2 != nullptr)				delete e2;				e2 = nullptr;

	this->deleteLater();
}

EnumNodeResult frmScriptEditTool::Execute(MiddleParam & param, QString & strError)
{
	QString _strError;
	_strError.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz: "));
	if (((ScriptEditTool*)m_pTool)->ExcuteString(ui.widget->text(), _strError) != NodeResult_OK) {
		m_strShowMsg = QString("<span style=\"color:red\">%1</span>").arg(_strError);
	}
	else {
		_strError.append("  Execute Sucessed!");
		m_strShowMsg = QString("<span style=\"color:green\">%1</span>").arg(_strError);
	}

	return NodeResult_OK;
}

int frmScriptEditTool::ExecuteComplete()
{
	ui.txtMsg->setText(m_strShowMsg);
	return 0;
}

int frmScriptEditTool::Load()
{
	m_titleBar->setTitleContent(((ScriptEditTool*)m_pTool)->GetItemId());

	ui.widget->setText(((ScriptEditTool*)m_pTool)->m_strScriptData);

	slot_DataModeChange(ui.cBx_Mode->currentIndex());
	return 0;
}

int frmScriptEditTool::Save()
{
	((ScriptEditTool*)m_pTool)->m_strScriptData = ui.widget->text();
	return 0;
}

void frmScriptEditTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Script.png");
	m_titleBar->setFocusPolicy(Qt::NoFocus);
}

void frmScriptEditTool::initSplitter()
{

}

void frmScriptEditTool::initConnect()
{
	connect(ui.cBx_Mode,			SIGNAL(activated(int)),					this, SLOT(slot_DataModeChange(int)));
	connect(ui.tableModelWidget,	SIGNAL(cellDoubleClicked(int, int)),	this, SLOT(slot_DoubleClicked(int, int)));
	connect(ui.tableWidget,			SIGNAL(cellDoubleClicked(int, int)),	this, SLOT(slot_WidgetDoubleClicked(int, int)));

}

void frmScriptEditTool::createdock()
{
	CDockManager::setConfigFlag(CDockManager::FloatingContainerHasWidgetTitle, true);
	CDockManager::setConfigFlag(CDockManager::FloatingContainerHasWidgetIcon, true);
	CDockManager::setConfigFlag(CDockManager::ActiveTabHasCloseButton, false);
	CDockManager::setConfigFlag(CDockManager::DockAreaHasCloseButton, false);
	CDockManager::setConfigFlag(CDockManager::DockAreaCloseButtonClosesTab, false);
	CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, false);
	CDockManager::setConfigFlag(CDockManager::DockAreaHasUndockButton, false);
	CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
	CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
	CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);

	DockManager = new CDockManager(this);
	connect(DockManager, &ads::CDockManager::focusedDockWidgetChanged, [](ads::CDockWidget* old, ads::CDockWidget* now) {
		if (now->widget() != nullptr)	now->widget()->setFocus();
	});

	m_pCentralDock = new CDockWidget(tr("main"));
	m_pModulDock = new CDockWidget(tr("ModulDock"));
	m_pToolDock = new CDockWidget(tr("ToolDock"));
	m_pTextTips = new CDockWidget(tr("TextTips"));
	m_pLogDock = new CDockWidget(tr("LogDock"));
	m_pVariableDock = new CDockWidget(tr("VariableDock"));


	{
		m_pAreaCentralDock = DockManager->setCentralWidget(m_pCentralDock);
		m_pCentralDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pAreaCentralDock->setAllowedAreas(DockWidgetArea::OuterDockAreas);
		m_pCentralDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pAreaCentralDock->resize(1000, 1000);
	}

	{	// create other dock widgets
		m_pAreaModeDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pModulDock);
		m_pModulDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pModulDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pModulDock->setFeatures(ads::CDockWidget::AllDockWidgetFeatures);//可以停靠的位置
		m_pModulDock->resize(250, 150);
		m_pAreaModeDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaVariableDock = DockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea, m_pVariableDock, m_pAreaModeDock);
		m_pVariableDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pVariableDock->setMinimumSize(QSize(250, 200));
		m_pVariableDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pVariableDock->resize(250, 150);
		m_pAreaVariableDock->resize(250, 200);
	}
	{	// create other dock widgets
		m_pAreaToolDock = DockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea, m_pToolDock);
		m_pToolDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pToolDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pToolDock->resize(250, 150);
		m_pAreaToolDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaTextTipsDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pTextTips, m_pAreaToolDock);
		m_pTextTips->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pTextTips->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pTextTips->resize(250, 150);
		m_pAreaTextTipsDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaLogDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pLogDock, m_pAreaTextTipsDock);
		m_pLogDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pLogDock->setMinimumSize(QSize(250, 200));
		m_pLogDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pLogDock->resize(250, 150);
		m_pAreaLogDock->resize(250, 200);
	}

	m_pCentralDock->setFocusPolicy(Qt::NoFocus);
	m_pModulDock->setFocusPolicy(Qt::NoFocus);
	m_pToolDock->setFocusPolicy(Qt::NoFocus);
	m_pTextTips->setFocusPolicy(Qt::NoFocus);
	m_pLogDock->setFocusPolicy(Qt::NoFocus);
	m_pVariableDock->setFocusPolicy(Qt::NoFocus);

	m_pAreaCentralDock->setFocusPolicy(Qt::NoFocus);
	m_pAreaModeDock->setFocusPolicy(Qt::NoFocus);
	m_pAreaToolDock->setFocusPolicy(Qt::NoFocus);
	m_pAreaTextTipsDock->setFocusPolicy(Qt::NoFocus);
	m_pAreaLogDock->setFocusPolicy(Qt::NoFocus);
	m_pAreaVariableDock->setFocusPolicy(Qt::NoFocus);
}

void frmScriptEditTool::InitLayOut()
{
	QLayoutItem *child;
	while ((child = ui.gridLayout->takeAt(0)) != nullptr) {
		if (child->widget()) {
			child->widget()->setParent(nullptr);//setParent(nullptr),防止删除之后界面不消失
			//delete child->widget();//释放
		}
		delete child;
	}

	if (m_pCentralDock != nullptr)	m_pCentralDock->setWidget(ui.widget );
	if (m_pToolDock != nullptr)		m_pToolDock->setWidget(ui.ToolTreeFrame	);
	if (m_pModulDock != nullptr)	m_pModulDock->setWidget(ui.widgetVar );
	if (m_pTextTips != nullptr)		m_pTextTips->setWidget(ui.plainTextExample );
	if (m_pLogDock != nullptr)		m_pLogDock->setWidget(ui.txtMsg );
	if (m_pVariableDock != nullptr)	m_pVariableDock->setWidget(ui.tableWidget);
	ui.gridLayout->addWidget(DockManager);

}

void frmScriptEditTool::showEvent(QShowEvent * ev)
{
	frmBaseTool::showEvent(ev);
	ResizeUI();
	QTimer::singleShot(100, this, [this]() {	ResizeUI();	});
}

void frmScriptEditTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
	ResizeUI();
}

void frmScriptEditTool::form_load()
{
	//设置表格列宽	
	ui.tableWidget->setColumnWidth(0, 45);
	ui.tableWidget->setColumnWidth(1, 202);
	ui.tableWidget->setColumnWidth(2, 310);
	ui.tableWidget->setColumnWidth(3, 242);

	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);

	//设置整行选中
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->horizontalHeader()->setDisabled(true);

	//添加List	 	
	//ui.listProcess->setEditTriggers(QAbstractItemView::NoEditTriggers);  //禁止编辑
}

void frmScriptEditTool::ToolsTreeWidgetInit()
{
	ToolTreeWidget = new QTreeWidget(this);
	ToolTreeWidget->setFocusPolicy(Qt::NoFocus);
	QGridLayout* ToolGLayout = new QGridLayout(ui.ToolTreeFrame);
	ToolGLayout->setContentsMargins(0, 0, 0, 0);
	ToolGLayout->setSpacing(5);
	ToolGLayout->addWidget(ToolTreeWidget, 0, 0);
	ToolTreeWidget->setIconSize(QSize(24, 24));
	ToolTreeWidget->header()->setVisible(false);
	ToolTreeWidget->clear();

	ToolsPair toolPair;
	toolPair.first = tr("Variable");
	toolPair.second.clear();
	toolPair.second.push_back("setglobal");
	toolPair.second.push_back("getglobal");
	toolPair.second.push_back("setlocal");
	toolPair.second.push_back("getlocal");
	ToolNamesVec.push_back(toolPair);

	toolPair.first = tr("Flow");
	toolPair.second.clear();
	toolPair.second.push_back("RunCall");
	ToolNamesVec.push_back(toolPair);

	toolPair.first = tr("Char");
	toolPair.second.clear();
	toolPair.second.push_back("getdatetime");
	toolPair.second.push_back("print");
	toolPair.second.push_back("split");
	ToolNamesVec.push_back(toolPair);

	toolPair.first = tr("Communicate");
	toolPair.second.clear();
	toolPair.second.push_back("SendData");
	toolPair.second.push_back("RecieveData");
	ToolNamesVec.push_back(toolPair);

	toolPair.first = tr("Log");
	toolPair.second.clear();
	toolPair.second.push_back("loginfo");
	toolPair.second.push_back("logwarn");
	toolPair.second.push_back("logerror");
	ToolNamesVec.push_back(toolPair);

	//循环处理图标
	for (int i = 0; i < ToolNamesVec.size(); i++)		{
		QString toolBoxName				= ToolNamesVec[i].first;
		QStringList toolList			= QStringList::fromStdList(ToolNamesVec[i].second);
		QTreeWidgetItem* itemParent		= new QTreeWidgetItem(ToolTreeWidget);
		itemParent->setText(0, toolBoxName);
		itemParent->setData(0,0, toolBoxName);
		itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
		for (int j = 0; j < toolList.count(); j++)		{
			QTreeWidgetItem* treeItem	= new QTreeWidgetItem(itemParent, QStringList(toolList[j]));
			treeItem->setData(0, 0, toolList[j]);
			treeItem->setText(0,toolList[j]);

			//QPixmap pixmap				= GetIcon(GetIconName(toolList[j]));
			//treeItem->setIcon(0, pixmap);
		}
	}

	//ToolTreeWidget->expandAll();
	this->setAcceptDrops(false);
	connect(ToolTreeWidget, &QTreeWidget::itemDoubleClicked, this, &frmScriptEditTool::slot_ItemDoubleClicked);
}

void frmScriptEditTool::slot_Accepted()
{
	slot_CreateList();
}

void frmScriptEditTool::set_Update_tableWidget(MiddleParam & param)
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)	ui.tableWidget->removeRow(0);  //设置行数

	QWidget* _pitem = nullptr;
	for (auto iter : param.MapBool)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("Bool");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		_pitem = new QComboBox();
		((QComboBox*)_pitem)->addItem("false");
		((QComboBox*)_pitem)->addItem("true");

		((QComboBox*)_pitem)->setCurrentIndex(0);
		((QComboBox*)_pitem)->setEnabled(false);
		if (iter.second)	((QComboBox*)_pitem)->setCurrentIndex(1);
		if (_pitem != nullptr)	ui.tableWidget->setCellWidget(count, 2, _pitem);

		QTableWidgetItem* item_remark = new QTableWidgetItem("false为假，true为真");
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	_pitem = nullptr;
	for (auto iter : param.MapInt)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("Int");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		_pitem = new QSpinBox();
		((QSpinBox*)_pitem)->setMinimum(-999999);
		((QSpinBox*)_pitem)->setMaximum(999999);
		((QSpinBox*)_pitem)->setValue(iter.second);
		((QSpinBox*)_pitem)->setEnabled(false);
		((QSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
		if (_pitem != nullptr)	ui.tableWidget->setCellWidget(count, 2, _pitem);

		QTableWidgetItem* item_remark = new QTableWidgetItem("");
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	_pitem = nullptr;
	for (auto iter : param.MapDou)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("Double");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		_pitem = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pitem)->setMinimum(-999999);
		((QDoubleSpinBox*)_pitem)->setMaximum(999999);
		((QDoubleSpinBox*)_pitem)->setValue(iter.second);
		((QDoubleSpinBox*)_pitem)->setEnabled(false);
		((QDoubleSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
		if (_pitem != nullptr)	ui.tableWidget->setCellWidget(count, 2, _pitem);

		QTableWidgetItem* item_remark = new QTableWidgetItem("");
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	_pitem = nullptr;
	for (auto iter : param.MapStr)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("QString");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		QTableWidgetItem* item_Text = new QTableWidgetItem(iter.second);
		item_Text->setTextAlignment(Qt::AlignCenter);
		item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 2, item_Text);

		QTableWidgetItem* item_remark = new QTableWidgetItem("");
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	_pitem = nullptr;
	for (auto iter : param.MapPoint)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("QPoint");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		QTableWidgetItem* item_Text = new QTableWidgetItem("");
		item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_Text->setTextAlignment(Qt::AlignCenter);
		item_Text->setText(QString("(%1,%2)").arg(QString::number(iter.second.x())).arg(QString::number(iter.second.y())));
		ui.tableWidget->setItem(count, 2, item_Text);

		QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	_pitem = nullptr;
	for (auto iter : param.MapPointF)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem("QPointF");
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 0, item_type);

		QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
		item_name->setTextAlignment(Qt::AlignCenter);
		item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, item_name);

		QTableWidgetItem* item_Text = new QTableWidgetItem("");
		item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_Text->setTextAlignment(Qt::AlignCenter);
		item_Text->setText(QString("(%1,%2)").arg(QString::number(iter.second.x())).arg(QString::number(iter.second.y())));
		ui.tableWidget->setItem(count, 2, item_Text);

		QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
		item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
}

void frmScriptEditTool::slot_CreateList()
{

}

void frmScriptEditTool::slot_DoubleClicked(int row, int column)
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)	ui.tableWidget->removeRow(0);  //设置行数
	switch (ui.cBx_Mode->currentIndex()) {
	case 0: {	//数据
		if (row == 0) { set_Update_tableWidget(*MiddleParam::Instance()); }
		else { if (MiddleParam::MapMiddle().size() >= row)	set_Update_tableWidget(MiddleParam::MapMiddle()[row - 1]); }
	}	break;
	case 1: {
		if (ui.tableModelWidget->item(row, column) != nullptr)		{
			QString str_fun = ui.tableModelWidget->item(row, 0)->text();
			ui.widget->insert(str_fun);
		}
	}break;
	case 2: {
		if (ui.tableModelWidget->item(row, column) != nullptr)		{
			QString str_fun = ui.tableModelWidget->item(row, 0)->text();
			ui.widget->insert(str_fun);
		}
	}break;
	default:
		break;
	}
}

void frmScriptEditTool::slot_WidgetDoubleClicked(int row, int column)
{
	if (ui.tableWidget->item(row, column) != nullptr)
	{
		QString str_fun = ui.tableWidget->item(row, 1)->text();
		ui.widget->insert(str_fun);
	}
}

void frmScriptEditTool::slot_DataModeChange(int iMode)
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数
	count = ui.tableModelWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableModelWidget->removeRow(0);  //设置行数

	switch (ui.cBx_Mode->currentIndex())	{
	case 0: {	//数据
		//GlobalVar
		count = ui.tableModelWidget->rowCount();
		ui.tableModelWidget->setRowCount(count + 1);  //设置行数

		QTableWidgetItem* item_type = new QTableWidgetItem(tr("GlobalVar"));
		item_type->setTextAlignment(Qt::AlignCenter);
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setData(0, "GlobalVar");
		item_type->setText(tr("GlobalVar"));
		ui.tableModelWidget->setItem(count, 0, item_type);

		for (auto iter : (*MiddleParam::Instance()).MapMiddle()) {
			count = ui.tableModelWidget->rowCount();
			ui.tableModelWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_value = new QTableWidgetItem(QString::number(iter.m_iIndex));
			item_value->setTextAlignment(Qt::AlignCenter);
			item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableModelWidget->setItem(count, 0, item_value);
		}

		slot_DoubleClicked(((ScriptEditTool*)m_pTool)->iToolIndex - 1, 0);
		ui.tableModelWidget->selectRow(((ScriptEditTool*)m_pTool)->iToolIndex - 1);
	}	break;
	case 1: {	//通讯
		for (size_t i = 0; i < gVariable::Instance().m_vecNames.size(); i++)	{
			auto strName = gVariable::Instance().m_vecNames[i];
			count = ui.tableModelWidget->rowCount();
			ui.tableModelWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_value = new QTableWidgetItem(strName);
			item_value->setTextAlignment(Qt::AlignCenter);
			item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableModelWidget->setItem(count, 0, item_value);
		}
	}	break;
	case 2: {	//通讯
		for (size_t i = 0; i < databaseVar::Instance().m_vecPtr.size(); i++) {
			auto strName = databaseVar::Instance().m_vecPtr[i]->m_strName;
			count = ui.tableModelWidget->rowCount();
			ui.tableModelWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_value = new QTableWidgetItem(strName.second);
			item_value->setTextAlignment(Qt::AlignCenter);
			item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableModelWidget->setItem(count, 0, item_value);
		}
	}	break;
	default:
		break;
	}


}

void frmScriptEditTool::slot_ItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	if (item->data(0,0).toString() == "setglobal")
	{		
		QString str_fun = "setglobal(\"gVar0\", \"1\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "getdatetime")
	{
		QString str_fun = "getdatetime('yyyy-MM-dd hh:mm:ss.zzz');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "print")
	{
		QString str_fun = "print('test');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "getglobal")
	{
		QString str_fun = "getglobal(\"gVar0\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "setlocal")
	{
		QString str_fun = "setlocal(\"Var0\", \"1\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "getlocal")
	{
		QString str_fun = "getlocal(\"Var0\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "split")
	{
		QString str_fun = "split(\"gVar0\",\'.\');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "loginfo")
	{
		QString str_fun = "loginfo('123');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "logwarn")
	{
		QString str_fun = "logwarn('123');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "logerror")
	{
		QString str_fun = "logerror('123');\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToPoint")
	{
		QString str_fun = "ConvertToPoint(Point, \"x\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToPoint2f")
	{
		QString str_fun = "ConvertToPoint2f(Point2f, \"x\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToPoint3f")
	{
		QString str_fun = "ConvertToPoint3f(Point3f, \"x\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToPoint3d")
	{
		QString str_fun = "ConvertToPoint3d(Point3d, \"x\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayInt")
	{
		QString str_fun = "ConvertToArrayInt(ArrayInt, 0);\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayBool")
	{
		QString str_fun = "ConvertToArrayBool(ArrayBool, 0);\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayFloat")
	{
		QString str_fun = "ConvertToArrayFloat(ArrayFloat, 0);\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayDouble")
	{
		QString str_fun = "ConvertToArrayDouble(ArrayDouble, 0);\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayQString")
	{
		QString str_fun = "ConvertToArrayQString(ArrayQString, 0);\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "ConvertToArrayCvPoint2f")
	{
		QString str_fun = "ConvertToArrayCvPoint2f(ArrayCvPoint2f, 0, \"x\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "LogInfo")
	{		
		QString str_fun = "LogInfo(\"内容\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "LogWarn")
	{		
		QString str_fun = "LogWarn(\"内容\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "LogError")
	{		
		QString str_fun = "LogError(\"内容\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "SendData")
	{
		QString str_fun = "SendData(\"EthernetTcp_1\",\"123\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "RecieveData")
	{
		QString str_fun = "RecieveData(\"EthernetTcp_1\",\"1000\");\n";
		ui.widget->insert(str_fun);
	}
	else if (item->data(0, 0).toString() == "RunCall")
	{
		QString str_fun = "RunCall(\"123\");\n";
		ui.widget->insert(str_fun);
	}
	ui.widget->GotoNextLine();
}

void frmScriptEditTool::DefaultCallTips()
{
	m_vecCallTips.clear();
	m_vecCallTips.push_back("function");
	m_vecCallTips.push_back("end");
	m_vecCallTips.push_back("while");
	m_vecCallTips.push_back("and");
	m_vecCallTips.push_back("do");
	m_vecCallTips.push_back("or");
	m_vecCallTips.push_back("not");
	m_vecCallTips.push_back("for");
	m_vecCallTips.push_back("break");
	m_vecCallTips.push_back("in");
	m_vecCallTips.push_back("return");
	m_vecCallTips.push_back("until");
	m_vecCallTips.push_back("goto");
	m_vecCallTips.push_back("repeat");
	m_vecCallTips.push_back("true");
	m_vecCallTips.push_back("false");
	m_vecCallTips.push_back("if");
	m_vecCallTips.push_back("then");
	m_vecCallTips.push_back("else");
	m_vecCallTips.push_back("elseif");
	m_vecCallTips.push_back("local");
	m_vecCallTips.push_back("print");
	m_vecCallTips.push_back("setParam");
	m_vecCallTips.push_back("getValue");
	m_vecCallTips.push_back("switchTab");
	//单个函数
	m_vecCallTips.push_back("string");
	m_vecCallTips.push_back("upper");
	m_vecCallTips.push_back("lower");
	m_vecCallTips.push_back("sub");
	m_vecCallTips.push_back("gsub");
	m_vecCallTips.push_back("dump");
	m_vecCallTips.push_back("find");
	m_vecCallTips.push_back("reverse");
	m_vecCallTips.push_back("format");
	m_vecCallTips.push_back("char");
	m_vecCallTips.push_back(("byte"));
	m_vecCallTips.push_back("len");
	m_vecCallTips.push_back("rep");
	m_vecCallTips.push_back("gmatch");
	//迭代器
	m_vecCallTips.push_back("pairs");
	m_vecCallTips.push_back("ipairs");
	m_vecCallTips.push_back("in");
	//table
	m_vecCallTips.push_back("table");
	m_vecCallTips.push_back("concat");
	m_vecCallTips.push_back("insert");
	m_vecCallTips.push_back("maxn");
	m_vecCallTips.push_back("remove");
	m_vecCallTips.push_back("sort");
	//模块
	m_vecCallTips.push_back("require");
	m_vecCallTips.push_back("package");
	m_vecCallTips.push_back("package.loaded");
	m_vecCallTips.push_back("package.seeall");
	m_vecCallTips.push_back("_G");
	//元素
	m_vecCallTips.push_back("setmetatable");
	m_vecCallTips.push_back("__index");
	m_vecCallTips.push_back("__newindex");
	//协程
	m_vecCallTips.push_back("coroutine.create");
	m_vecCallTips.push_back("coroutine.resume");
	m_vecCallTips.push_back("coroutine.yield");
	m_vecCallTips.push_back("coroutine.status");
	m_vecCallTips.push_back("coroutine.wrap");
	m_vecCallTips.push_back("coroutine.running");
	//文件读写
	m_vecCallTips.push_back("io.input");
	m_vecCallTips.push_back("io.output");
	m_vecCallTips.push_back("io.close");
	m_vecCallTips.push_back("io.read");
	m_vecCallTips.push_back("io.lines");
	m_vecCallTips.push_back("io.write");
	m_vecCallTips.push_back("io.flush");
	//错误和调试
	m_vecCallTips.push_back("error");
	m_vecCallTips.push_back("assert");
	m_vecCallTips.push_back("pcall");
	m_vecCallTips.push_back("debug");

	//添加的函数
	m_vecCallTips.push_back("print");
	m_vecCallTips.push_back("getdatetime");
	m_vecCallTips.push_back("setglobal");
	m_vecCallTips.push_back("getglobal");
	m_vecCallTips.push_back("setlocal");
	m_vecCallTips.push_back("getlocal");
	m_vecCallTips.push_back("split");

	m_vecCallTips.push_back("loginfo");
	m_vecCallTips.push_back("logwarn");
	m_vecCallTips.push_back("logerror");

	m_vecCallTips.push_back("SendData");
	m_vecCallTips.push_back("RecieveData");

	m_vecCallTips.push_back("RunCall");
}

void frmScriptEditTool::ResizeUI()
{
	int count = ui.tableWidget->columnCount();
	int	iWidth = ui.tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++) {
		ui.tableWidget->setColumnWidth(i, iWidth);
	}
}
