#include "frmToolsTree.h"
#include <QHeaderView>
#include <QStyleFactory>
#include <QDir>
#include <QPluginLoader>
#include <QDrag>
#include <QMimeData>
#include "ToolTree.h"
#include "PluginsManager.h"

frmToolsTree::frmToolsTree(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	ToolsTreeWidgetInit();
}

//工具列表初始化
void frmToolsTree::ToolsTreeWidgetInit()
{
	if (ToolTreeWidget != nullptr)	delete ToolTreeWidget; ToolTreeWidget = nullptr;
	ToolTreeWidget = new QtTreeWidget(this);
	QGridLayout* ToolGLayout = new QGridLayout(ui.ToolTreeFrame);
	ToolGLayout->setContentsMargins(0, 0, 0, 0);
	//ToolGLayout->setSpacing(5);
	ToolGLayout->addWidget(ToolTreeWidget, 0, 0);
	//ToolTreeWidget->setIconSize(QSize(24, 24));
	ToolTreeWidget->header()->setVisible(false);
	ToolTreeWidget->clear();
	this->setAcceptDrops(true);
	connect(ToolTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(FlowButtonSlot(QTreeWidgetItem *, int)));
	for (auto iter : PluginsManager::Instance().m_ToolPluginLst){
		if (iter != nullptr)	{
			switch (iter->getPluginType()){
			case ENUM_PLUGIN_TYPE_CAMERA:	{			}	break;
			case ENUM_PLUGIN_TYPE_TOOL:		{
				QVector<QPair<QString, QString>> vecPair = iter->GetLanguageToolName();
				if (vecPair.size() > 0)				{
					QList<QTreeWidgetItem*> _selects = findItems(vecPair[0].first);
					QTreeWidgetItem* itemParent = nullptr;
					if (_selects.size() > 0)	{
						itemParent = _selects[0];
					}
					else					{
						itemParent = new QTreeWidgetItem(ToolTreeWidget);
						itemParent->setText(0, vecPair[0].second);
						itemParent->setData(1, 1, vecPair[0].first);
						itemParent->setIcon(0, QIcon(":/image/icon/images/File.png"));
						itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
					}
					for (int i = 1; i < vecPair.size(); i++)					{
						QTreeWidgetItem* treeItem = new QTreeWidgetItem(itemParent);
						treeItem->setData(1, 1, vecPair[i].first);
						treeItem->setText(0, vecPair[i].second);
						//treeItem->setIcon(0, iter->getQIcon());
						treeItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
						treeItem->setToolTip(0,iter->getDescription());
						itemParent = treeItem;
					}
				}
			}	break;
			case ENUM_PLUGIN_TYPE_STD:	{		}	break;
			default: {	}	break;
			}
		}
	}

}

QList<QTreeWidgetItem*> frmToolsTree::findItems(QString strData)
{
	QList<QTreeWidgetItem*>	_SelectedLst;
	for (int iIndex = 0; iIndex < ToolTreeWidget->topLevelItemCount(); iIndex++) {
		QTreeWidgetItem* pItem = ToolTreeWidget->topLevelItem(iIndex);
		if (pItem) {
			if (pItem->data(1,1).toString() == strData) {
				_SelectedLst.push_back(pItem);
			}
			_SelectedLst.append(findStandardItems(pItem, strData));
		}
	}
	return _SelectedLst;
}

QList<QTreeWidgetItem*> frmToolsTree::findStandardItems(QTreeWidgetItem* pItem, QString strData)
{
	QList<QTreeWidgetItem*>	_SelectedLst;
	for (int i = 0; i < pItem->childCount(); i++) {
		auto _pItem = pItem->child(i);
		if (_pItem) {
			if (_pItem->data(1,1).toString() == strData) {
				_SelectedLst.push_back(_pItem);
			}
			_SelectedLst.append(findStandardItems(_pItem, strData));
		}
	}
	return _SelectedLst;
}

void frmToolsTree::changeEvent(QEvent * ev)
{
	switch (ev->type()) {
	case QEvent::LanguageChange: {
		for (auto iter : PluginsManager::Instance().m_ToolPluginLst) {
			if (iter != nullptr) {
				switch (iter->getPluginType()) {
				case ENUM_PLUGIN_TYPE_TOOL: {
					QVector<QPair<QString, QString>> vecPair = iter->GetLanguageToolName();
					if (vecPair.size() > 0) {
						for (auto Pair : vecPair)		{
							QList<QTreeWidgetItem*>	_SelectedLst = findItems(Pair.first);
							if (_SelectedLst.size() > 0)							{
								_SelectedLst[0]->setText(0,Pair.second);
							}
						}
					}
				}	break;
				case ENUM_PLUGIN_TYPE_STD: {	}	break;
				default: {		}	break;
				}
			}
		}
	}	break;
	default:	break;
	}
}

void frmToolsTree::FlowButtonSlot(QTreeWidgetItem *item, int column)
{
	QDrag *drag = new QDrag(this);
	QMimeData *data = new QMimeData();
	data->clear();
	data->setText(item->data(1, 1).toString());
	drag->setMimeData(data);
	drag->exec(Qt::MoveAction);
}