#include "QFlowChart.h"
#include "Data/dataVar.h"
#include "flowchart_graphics_item.h"
#include "NodeManager.h"
#include "PluginsManager.h"

#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
using namespace ads;

QFlowChart::QFlowChart(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	ui.btnStop->setEnabled(false);
	int	_iRowCount = ui.tabWidget->count();
	for (size_t i = 0; i < _iRowCount; i++)		ui.tabWidget->removeTab(0);
	ui.tabWidget->setTabsClosable(true);//设置有关闭按钮
	ui.tabWidget->setMovable(true);//可拖动
	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseTabWidget(int)));//关闭选项卡信号
	connect(ui.tabWidget, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(slotSetTabName(int)));//双击选项卡信号
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotSetTabCurrentIndex(int)));//双击选项卡信号
	m_pBtnTabWidget = new QToolButton(this);//新建页面的Button
	m_pBtnTabWidget->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.add.png"));
	m_pBtnTabWidget->show();
	ui.tabWidget->setCornerWidget(m_pBtnTabWidget, Qt::TopRightCorner);        //添加到选项卡栏里
	connect(m_pBtnTabWidget, SIGNAL(clicked(bool)), this, SLOT(slotAddTabWidget()));

	m_pBtnToolWidget = new QToolButton(this);
	m_pBtnToolWidget->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.add.png"));
	m_pBtnToolWidget->show();
	ui.tabWidget->setCornerWidget(m_pBtnToolWidget, Qt::TopLeftCorner);        //添加到选项卡栏里
	connect(m_pBtnToolWidget, SIGNAL(clicked(bool)), this, SLOT(slotCheckFlowTool()));

	m_pMenuToolWidget = new QMenu(this);

	m_pLEditName = new QLineEdit(this);   //添加输入框
	m_pLEditName->hide();            //隐藏
	m_pLEditName->setToolTip(tr("输入工作表名字"));
	connect(m_pLEditName, SIGNAL(editingFinished()),	this, SLOT(slotGetTabName()));     //输入完成
	connect(m_pLEditName, SIGNAL(returnPressed()),		this, SLOT(slotGetTabName()));
	connect(m_pLEditName, SIGNAL(inputRejected()),		this, SLOT(slotGetTabName()));
	MiddleParam::MapMiddle().clear();
	dataVar::Instance().m_vecpFlowView.clear();

	createdock();

	slotAddTabWidget();

}

QFlowChart::~QFlowChart()
{
	if (m_pLEditName != nullptr)		delete m_pLEditName;		m_pLEditName = nullptr;
	if (m_pBtnTabWidget != nullptr)		delete m_pBtnTabWidget;		m_pBtnTabWidget = nullptr;
	if (m_pBtnToolWidget != nullptr)	delete m_pBtnToolWidget;	m_pBtnToolWidget = nullptr;
	if (m_pMenuToolWidget != nullptr)	delete m_pMenuToolWidget;	m_pMenuToolWidget = nullptr;
}

void QFlowChart::initUI()
{

}

QList<QAction*> QFlowChart::findMeItems(QString strData)
{
	QList<QAction*> selectActLst;
	auto menuLst = m_pMenuToolWidget->actions();
	for (auto& menuLst : m_pMenuToolWidget->actions())	{
		if (menuLst->data() == strData)	{
			selectActLst.push_back(menuLst);
		}
		QMenu* pItem = menuLst->menu();
		selectActLst.append(findMenuItems(pItem, strData));
	}
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
	for (auto iter : PluginsManager::Instance().m_ToolPluginLst)
		if (iter != nullptr) {
			QVector<QPair<QString, QString>> vecToolName = iter->GetToolTreeName();
			for (int iRow = 0; iRow < vecToolName.size(); iRow++)	{
				QList<QAction*> selectActLst = findMeItems(vecToolName[iRow].first);
				AddMenuItems(m_pMenuToolWidget, vecToolName,0);
			}
		}
	return 0;
}

int QFlowChart::GetData(QJsonObject & strData)
{
	return 0;
}

int QFlowChart::SetData(QJsonObject & strData)
{
	return 0;
}

int QFlowChart::GetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam;

	QJsonArray FlowChartsParam;
	int	_iTabCount = ui.tabWidget->count();
	for (int iTab = 0; iTab < _iTabCount; iTab++) {
		QJsonObject FlowParam;
		FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
		if (pAreaViews == nullptr) { FlowChartsParam.append(FlowParam);	continue; }
		FlowParam.insert("TabName", ui.tabWidget->tabBar()->tabText(iTab));
		pAreaViews->GetGraphData(FlowParam);
		FlowChartsParam.append(FlowParam);
	}
	CommonParam.insert("FlowChartsParam", FlowChartsParam);

	strData.insert("QFlowChart", CommonParam);
	return 0;
}

int QFlowChart::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QFlowChart").value().toObject();
	
	int	_iRowCount = ui.tabWidget->count();
	for (int i = 0; i < _iRowCount; i++)		ui.tabWidget->removeTab(0);
	MiddleParam::MapMiddle().clear();
	dataVar::Instance().m_vecpFlowView.clear();

	QJsonArray FlowChartsParam = CommonParam.find("FlowChartsParam").value().toArray();
	for (int iTab = 0; iTab < FlowChartsParam.count(); iTab++)	{
		QJsonObject FlowParam = FlowChartsParam.at(iTab).toObject();
		slotAddTabWidget();
		FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
		if (pAreaViews == nullptr) { continue; }
		if (FlowParam.contains("TabName")) {
			QString	strTabName = FlowParam["TabName"].toString();
			ui.tabWidget->setTabText(iTab, strTabName);
			pAreaViews->SetText(strTabName);
			if (iTab < MiddleParam::MapMiddle().size())
				MiddleParam::MapMiddle()[iTab].m_strName = strTabName;
		}
		pAreaViews->SetGraphData(FlowParam);
	}
	_iRowCount = ui.tabWidget->count();
	if (_iRowCount <= 0) {
		slotAddTabWidget();
	}
	return 0;
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
}

void QFlowChart::UpDateTab()
{
	int	_iTabCount = ui.tabWidget->count();
	for (int iTab = 0; iTab < _iTabCount; iTab++) {
		FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
		if (pAreaViews != nullptr) { pAreaViews->UpdateIndex(iTab); }

		if (iTab < MiddleParam::MapMiddle().size())
			MiddleParam::MapMiddle()[iTab].m_strName = ui.tabWidget->tabBar()->tabText(iTab);

	}
	for (size_t i = 0; i < MiddleParam::MapMiddle().size(); i++)	MiddleParam::MapMiddle()[i].m_iIndex = i;
}

void QFlowChart::on_btnStop_clicked()
{
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = true;
	int iTab = ui.tabWidget->currentIndex();
	//if (_iIndex >= dataVar::Instance().m_vecPtr.size())	return;
	FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
	if (pAreaViews != nullptr) { pAreaViews->Stop(); }
	QTimer::singleShot(500, this, SLOT(slotStopStatus()));

}

void QFlowChart::on_btnRunOnce_clicked()
{
	int iTab = ui.tabWidget->currentIndex();
	//if (_iIndex >= dataVar::Instance().m_vecPtr.size() || _iIndex < 0)	return;
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(true);
	ui.tabWidget->setEnabled(false);
	m_pLEditName->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 1;
	FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
	if (pAreaViews != nullptr) { pAreaViews->Start();	return;	}
	//FlowchartScene*	_ptrgraph = dataVar::Instance().m_vecPtr[_iIndex];
	//if (_ptrgraph != nullptr) { _ptrgraph->Start(); return; }
	QTimer::singleShot(500, this, SLOT(slotStopStatus()));
}

void QFlowChart::on_btnRunCycle_clicked()
{
	int iTab = ui.tabWidget->currentIndex();
	ui.btnRunOnce->setEnabled(false);
	ui.btnRunCycle->setEnabled(false);
	ui.btnStop->setEnabled(true);
	m_pLEditName->setEnabled(false);
	ui.tabWidget->setEnabled(false);

	FlowchartGraphicsItem::m_bVisionThreadRun = true;
	FlowchartGraphicsItem::m_bVisionThreadNeedStop = false;
	FlowchartGraphicsItem::m_iVisionThreadRunTime = 0;

	FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
	if (pAreaViews != nullptr) { pAreaViews->Start(); return;	}
	QTimer::singleShot(500, this, SLOT(slotStopStatus()));
}

void QFlowChart::slotAddTabWidget()      //新建选项卡
{
	int	_iTabCount = ui.tabWidget->count();
	int	_Index = -1;
	for (int iRow = 0; iRow < _iTabCount + 1; iRow++) {
		bool	_bFind = false;
		QWidget * pArea = qobject_cast<QWidget*>(ui.tabWidget->widget(iRow));
		if (pArea == nullptr) {
			_Index = iRow;
			break;
		}
		for (int iTab = 0; iTab < _iTabCount; iTab++) {
			QWidget * pTabArea = qobject_cast<QWidget*>(ui.tabWidget->widget(iTab));
			if (pTabArea == nullptr)	continue;
			int _iTabIndex = pTabArea->property("strKey").toString().toInt();
			if (iRow == _iTabIndex) {
				_bFind = true;	break;
			}
			QString strName = ui.tabWidget->tabBar()->tabText(iTab);
			if (strName == (tr("Tab") + QString::number(_iTabIndex)))		{
				_bFind = true;	break;
			}
		}
		if (!_bFind)	{
			_Index = iRow;
			break;
		}
	}
	if (_Index != -1)	{
		QWidget *tab = new QWidget();
		FlowGraphicsViews*	m_pView = new FlowGraphicsViews(tab);
		connect(m_pView, SIGNAL(sigAutoRunFinished()), this, SLOT(slotStopStatus()));
		dataVar::Instance().m_vecpFlowView.push_back(m_pView);

		QGridLayout *gridLayout_2 = new QGridLayout(tab);
		gridLayout_2->setSpacing(0);
		gridLayout_2->setContentsMargins(0, 0, 0, 0);
		gridLayout_2->addWidget(m_pView);
		tab->setProperty("strKey", QString::number(_Index));
		QString	_strName = tr("Tab") + QString::number(_Index);
		m_pView->SetText(_strName);
		ui.tabWidget->addTab(tab, _strName);
		ui.tabWidget->setCurrentWidget(tab); //添加页面
		MiddleParam::MapMiddle().push_back(MiddleParam());
		if (_Index < MiddleParam::MapMiddle().size())
			MiddleParam::MapMiddle()[_Index].m_strName = _strName;
	}
	UpDateTab();

}

void QFlowChart::slotCloseTabWidget(int i)//关闭选项卡
{
	if (ui.tabWidget->count() == 1) { //判断选显卡数量，不能少于1
		dataVar::Instance().pManager->notify(tr("Cann't Delete Only Tab"), NotifyType_Error);
	}
	else {
		QWidget * Area = qobject_cast<QWidget*>(ui.tabWidget->widget(i));
		ui.tabWidget->removeTab(i);
		delete Area;
		if (MiddleParam::MapMiddle().size() > i)
		{
			MiddleParam::MapMiddle()[i].ClearAll();
			MiddleParam::MapMiddle().erase(MiddleParam::MapMiddle().begin() + i);
		}
		if (dataVar::Instance().m_vecpFlowView.size() > i)
			dataVar::Instance().m_vecpFlowView.erase(dataVar::Instance().m_vecpFlowView.begin() + i);

	}
	UpDateTab();
}

void QFlowChart::slotSetTabName(int id)           //设置选项卡的名字
{
	if (!m_pLEditName->isEnabled()){
		return;
	}
	m_pLEditName->show();
	m_pLEditName->setText(tr("%1").arg(ui.tabWidget->tabBar()->tabText(id)));
	m_pLEditName->resize(ui.tabWidget->tabBar()->tabRect(id).size());
	m_pLEditName->move(ui.tabWidget->x() + ui.tabWidget->tabBar()->tabRect(id).x() , ui.tabWidget->y());
	m_pLEditName->setFocus();
	m_pLEditName->selectAll();
	m_iTempTabId = id;
}

void QFlowChart::slotGetTabName()
{
	QString strName = m_pLEditName->text();
	if (m_iTempTabId != -1)	{
		QWidget * pArea = qobject_cast<QWidget*>(ui.tabWidget->widget(m_iTempTabId));
		if (pArea == nullptr)	return;
		QGridLayout * pQGridLayoutView = qobject_cast<QGridLayout*>(pArea->layout());
		if (pQGridLayoutView == nullptr)	return;
		FlowGraphicsViews * pAreaView = qobject_cast<FlowGraphicsViews*>(pQGridLayoutView->itemAt(0)->widget());
		if (pAreaView == nullptr)	return;
		pAreaView->SetText(strName);
		ui.tabWidget->setTabText(m_iTempTabId,strName);

		if(m_iTempTabId < MiddleParam::MapMiddle().size())
			MiddleParam::MapMiddle()[m_iTempTabId].m_strName = strName;

		m_iTempTabId = -1;
	}
	m_pLEditName->hide();
}

void QFlowChart::slotSetTabCurrentIndex(int index)
{

}

void QFlowChart::slotStopStatus()
{
	ui.btnRunOnce->setEnabled(true);
	ui.btnRunCycle->setEnabled(true);
	ui.btnStop->setEnabled(false);
	ui.tabWidget->setEnabled(true);
	m_pLEditName->setEnabled(true);
	bool _bAllStop = true;

	int	_iTabCount = ui.tabWidget->count();
	for (int iTab = 0; iTab < _iTabCount; iTab++) {
		FlowGraphicsViews * pAreaViews = qobject_cast<FlowGraphicsViews*>(ui.tabWidget->widget(iTab)->layout()->itemAt(0)->widget());
		_bAllStop &= !pAreaViews->m_pNodeManager->m_bThreadRun;
	}

	if (_bAllStop)	FlowchartGraphicsItem::m_bVisionThreadRun = false;

	QTimer::singleShot(100, this, SIGNAL(sig_AllCycleStop()));
}

void QFlowChart::slotCheckFlowTool()
{
	if(m_pMenuToolWidget != nullptr)
		m_pMenuToolWidget->exec(QCursor::pos());
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
	m_pLEditName->hide();
}
