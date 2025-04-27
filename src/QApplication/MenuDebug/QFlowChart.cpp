#include "QFlowChart.h"
#include "Data/dataVar.h"
#include "flowchart_graphics_item.h"
#include "NodeManager.h"
#include "PluginsManager.h"
#include "mainwindow.h"

#include "databaseVar.h"
#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
using namespace ads;

QFlowChart::QFlowChart(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	MiddleParam::MapMiddle().clear();
	databaseVar::Instance().m_vecPtr.clear();

	createdock();
	//initDock();
	if (m_pToolDock != nullptr)		m_pToolDock->setWidget(m_pfrmToolsTree = new frmToolsTree());
	if (m_pFlowDock != nullptr)		m_pFlowDock->setWidget(m_pfrmflow = new frmflow());
	if (m_pFlowManager != nullptr)	m_pFlowManager->setWidget(m_pfrmToolFlow = new frmToolFlow());
	if (m_pLogDock != nullptr)		m_pLogDock->setWidget(m_pFrmLog = new FrmLog());
	if (m_pCentralDock != nullptr)	m_pCentralDock->setWidget(m_pFrmImageView = new FrmImageView());
	ui.gridLayout->addWidget(DockManager);

	InitConnect();
}

QFlowChart::~QFlowChart()
{

}

void QFlowChart::initUI()
{

}

QList<QAction*> QFlowChart::findMeItems(QString strData)
{
	QList<QAction*> selectActLst;
	//auto menuLst = m_pMenuToolWidget->actions();
	//for (auto& menuLst : m_pMenuToolWidget->actions())	{
	//	if (menuLst->data() == strData)	{
	//		selectActLst.push_back(menuLst);
	//	}
	//	QMenu* pItem = menuLst->menu();
	//	selectActLst.append(findMenuItems(pItem, strData));
	//}
	return selectActLst;
}

QList<QAction*> QFlowChart::findMenuItems(QMenu* pItem, QString strData)
{
	QList<QAction*> selectActLst;
	if (pItem == nullptr)	return selectActLst;

	for (auto& menuLst : pItem->actions()) {
		if (menuLst->data() == strData) {
			selectActLst.push_back(menuLst);
		}
		QMenu* _pItem = menuLst->menu();
		selectActLst.append(findMenuItems(_pItem, strData));
	}
	return selectActLst;
}

int QFlowChart::AddMenuItems(QMenu * pItem, QVector<QPair<QString, QString>>& vecTools, int iIndex)
{
	if (pItem == nullptr)		return 0;
	if (vecTools.size() <= 0)	return 0;
	if (iIndex >= vecTools.size())	return -1;
	QList<QAction*> selectActLst = findMenuItems(pItem, vecTools[iIndex].first);
	if (selectActLst.size() <= 0) {
		QAction* _pAction = new QAction();
		_pAction->setData(vecTools[iIndex].first);
		_pAction->setText(vecTools[iIndex].second);
		_pAction->setToolTip(vecTools[iIndex].second);
		QMenu* _pMenu = new QMenu();
		_pAction->setMenu(_pMenu);
		pItem->addAction(_pAction);
		selectActLst.append(_pAction);
	}
	iIndex++;
	AddMenuItems(selectActLst[0]->menu(), vecTools, iIndex);


	return 0;
}

bool QFlowChart::CheckPageAvalible()
{
	return false;
}

int QFlowChart::CloseUI()
{
	return 0;
}

int QFlowChart::initData(QString & strError)
{
	//for (auto iter : PluginsManager::Instance().m_ToolPluginLst)
	//	if (iter != nullptr) {
	//		QVector<QPair<QString, QString>> vecToolName = iter->GetToolTreeName();
	//		for (int iRow = 0; iRow < vecToolName.size(); iRow++)	{
	//			QList<QAction*> selectActLst = findMeItems(vecToolName[iRow].first);
	//			AddMenuItems(m_pMenuToolWidget, vecToolName,0);
	//		}
	//	}
	return 0;
}

int QFlowChart::GetData(QJsonObject & strData)
{
	m_pfrmflow->GetData(strData);
	m_pfrmToolFlow->GetData(strData);
	m_pFrmImageView->GetData(strData);

	return 0;
}

int QFlowChart::SetData(QJsonObject & strData)
{
	m_pfrmflow->SetData(strData);
	m_pfrmToolFlow->SetData(strData);
	m_pFrmImageView->SetData(strData);
	return 0;
}

int QFlowChart::GetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam;

	//QJsonArray FlowChartsParam;
	//int	_iTabCount = ui.tabWidget->count();
	//for (int iTab = 0; iTab < _iTabCount; iTab++) {
	//	QJsonObject FlowParam;
	//	FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
	//	if (pAreaViews == nullptr) { FlowChartsParam.append(FlowParam);	continue; }
	//	FlowParam.insert("TabName", ui.tabWidget->tabBar()->tabText(iTab));
	//	pAreaViews->GetGraphData(FlowParam);
	//	FlowChartsParam.append(FlowParam);
	//}
	//CommonParam.insert("FlowChartsParam", FlowChartsParam);
	strData.insert("QFlowChart", CommonParam);
	return 0;
}

int QFlowChart::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QFlowChart").value().toObject();
	
	//int	_iRowCount = ui.tabWidget->count();
	//for (int i = 0; i < _iRowCount; i++)		ui.tabWidget->removeTab(0);
	//MiddleParam::MapMiddle().clear();
	//dataVar::Instance().m_vecpFlowView.clear();
	//QJsonArray FlowChartsParam = CommonParam.find("FlowChartsParam").value().toArray();
	//for (int iTab = 0; iTab < FlowChartsParam.count(); iTab++)	{
	//	QJsonObject FlowParam = FlowChartsParam.at(iTab).toObject();
	//	slotAddTabWidget();
	//	FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
	//	if (pAreaViews == nullptr) { continue; }
	//	if (FlowParam.contains("TabName")) {
	//		QString	strTabName = FlowParam["TabName"].toString();
	//		ui.tabWidget->setTabText(iTab, strTabName);
	//		pAreaViews->SetText(strTabName);
	//		if (iTab < MiddleParam::MapMiddle().size())
	//			MiddleParam::MapMiddle()[iTab].m_strName = strTabName;
	//	}
	//	pAreaViews->SetGraphData(FlowParam);
	//}
	//_iRowCount = ui.tabWidget->count();
	//if (_iRowCount <= 0) {
	//	slotAddTabWidget();
	//}
	return 0;
}

void QFlowChart::initDock()
{
	//CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
	//CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
	//CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
	//CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);
	CDockManager::setConfigFlag(CDockManager::FloatingContainerHasWidgetTitle, true);
	CDockManager::setConfigFlag(CDockManager::FloatingContainerHasWidgetIcon, true);
	CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
	CDockManager::setConfigFlag(CDockManager::DockAreaHasUndockButton, false);
	DockManager = new CDockManager(this);
	{
		m_pCentralDock = new CDockWidget(tr("main"));
		m_pAreaCentralDock = DockManager->setCentralWidget(m_pCentralDock);
		m_pCentralDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pAreaCentralDock->setAllowedAreas(DockWidgetArea::OuterDockAreas);
		m_pCentralDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pAreaCentralDock->resize(250, 150);
	}
	m_pFlowDock = new CDockWidget(tr("FlowDock"));
	m_pToolDock = new CDockWidget(tr("ToolDock"));
	m_pFlowManager = new CDockWidget(tr("FlowManager"));
	m_pLogDock = new CDockWidget(tr("LogDock"));

	{	// create other dock widgets
		m_pAreaFlowDock = DockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, m_pFlowDock, m_pAreaCentralDock);
		m_pFlowDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pFlowDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pFlowDock->setFeatures(ads::CDockWidget::AllDockWidgetFeatures);//可以停靠的位置
		m_pFlowDock->resize(1000, 150);
		m_pAreaFlowDock->resize(1000, 150);
	}
	{	// create other dock widgets
		m_pAreaToolDock = DockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, m_pToolDock);
		m_pToolDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pToolDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pToolDock->resize(250, 150);
		m_pAreaToolDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaFlowLstDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pFlowManager, m_pAreaToolDock);
		m_pFlowManager->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pFlowManager->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pFlowManager->resize(250, 150);
		m_pAreaFlowLstDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaLogDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pLogDock);
		m_pLogDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pLogDock->setMinimumSize(QSize(250, 200));
		m_pLogDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pLogDock->resize(250, 150);
		m_pAreaLogDock->resize(250, 200);
	}
}

void QFlowChart::createdock()
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
	{
		m_pCentralDock = new CDockWidget(tr("main"));
		m_pAreaCentralDock = DockManager->setCentralWidget(m_pCentralDock);
		m_pCentralDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pAreaCentralDock->setAllowedAreas(DockWidgetArea::OuterDockAreas);
		m_pCentralDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pAreaCentralDock->resize(250, 150);
	}
	m_pFlowDock		= new CDockWidget(tr("FlowDock"));
	m_pToolDock		= new CDockWidget(tr("ToolDock"));
	m_pFlowManager	= new CDockWidget(tr("FlowManager"));
	m_pLogDock		= new CDockWidget(tr("LogDock"));

	{	// create other dock widgets
		m_pAreaFlowDock = DockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, m_pFlowDock, m_pAreaCentralDock);
		m_pFlowDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pFlowDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pFlowDock->setFeatures(ads::CDockWidget::AllDockWidgetFeatures);//可以停靠的位置
		m_pFlowDock->resize(1000, 150);
		m_pAreaFlowDock->resize(1000, 150);
	}
	{	// create other dock widgets
		m_pAreaToolDock = DockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, m_pToolDock);
		m_pToolDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pToolDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pToolDock->resize(250, 150);
		m_pAreaToolDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaFlowLstDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pFlowManager, m_pAreaToolDock);
		m_pFlowManager->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pFlowManager->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pFlowManager->resize(250, 150);
		m_pAreaFlowLstDock->resize(250, 150);
	}
	{	// create other dock widgets
		m_pAreaLogDock = DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, m_pLogDock);
		m_pLogDock->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
		m_pLogDock->setMinimumSize(QSize(250, 200));
		m_pLogDock->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
		m_pLogDock->resize(250, 150);
		m_pAreaLogDock->resize(250, 200);
	}
}

void QFlowChart::InitConnect()
{
	connect(m_pfrmToolFlow, &frmToolFlow::sig_ChangeFlow,		m_pfrmflow, &frmflow::slot_ChangeFlow);
	connect(m_pfrmToolFlow, &frmToolFlow::sig_ReNameFlow,		m_pfrmflow, &frmflow::slot_ReNameFlow);
	connect(m_pfrmToolFlow, &frmToolFlow::sig_AddFlow,			m_pfrmflow, &frmflow::slot_AddFlow);
	connect(m_pfrmToolFlow, &frmToolFlow::sig_DeleteFlow,		m_pfrmflow, &frmflow::slot_DeleteFlow);
	connect(m_pfrmToolFlow, &frmToolFlow::sig_SetFlowEnable,	m_pfrmflow, &frmflow::slot_SetFlowEnable);
	connect(dataVar::Instance().pfrmImageLayout, &frmImageLayout::sig_ChangeVideoCount, m_pFrmImageView, &FrmImageView::slot_ChangeVideoCount);
}

void QFlowChart::ChangeLanguage()
{
	m_pFlowDock->setToolTip(tr("FlowDock"));
	m_pToolDock->setToolTip(tr("ToolDock"));
	m_pFlowManager->setToolTip(tr("FlowManager"));
	m_pLogDock->setToolTip(tr("LogDock"));

	m_pFlowDock->setWindowTitle(tr("FlowDock"));
	m_pToolDock->setWindowTitle(tr("ToolDock"));
	m_pFlowManager->setWindowTitle(tr("FlowManager"));
	m_pLogDock->setWindowTitle(tr("LogDock"));

	m_pFlowDock->toggleViewAction()->setText(tr("FlowDock"));
	m_pToolDock->toggleViewAction()->setText(tr("ToolDock"));
	m_pFlowManager->toggleViewAction()->setText(tr("FlowManager"));
	m_pLogDock->toggleViewAction()->setText(tr("LogDock"));
}

void QFlowChart::UpDateTab()
{

}

void QFlowChart::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev); 
}

void QFlowChart::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
}

void QFlowChart::hideEvent(QHideEvent * ev)
{
	QtWidgetsBase::hideEvent(ev);
}
