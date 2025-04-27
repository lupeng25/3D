#include "frmToolFlow.h"
#include <QHeaderView>
#include <QStyleFactory>
#include <QDir>
#include <QPluginLoader>
#include <QDrag>
#include <QMimeData>
#include "ToolTree.h"
#include <QWidgetAction>
#include "Data/dataVar.h"
#include "databaseVar.h"

frmToolFlow::frmToolFlow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint);
	ToolsTreeWidgetInit();
}

frmToolFlow::~frmToolFlow()
{
	if (ToolTreeWidget != nullptr)	delete ToolTreeWidget;	ToolTreeWidget = nullptr;
	if (m_ptrItems != nullptr)		delete m_ptrItems;		m_ptrItems = nullptr;
}

int frmToolFlow::GetData(QJsonObject& strData)
{
	//二级数据
	QJsonArray frmToolFlow;
	for (int iTop = 0; iTop < m_ptrItems->rowCount(); iTop++)	{
		QStandardItem* itemParent = m_ptrItems->item(iTop);
		QJsonArray __TopArray;
		__TopArray.append(itemParent->data().toString());
		__TopArray.append(itemParent->text());
		__TopArray.append(QString::number( itemParent->checkState()	));
		for (int iother = 0; iother < itemParent->rowCount(); iother++)		{
			QJsonArray __IotherArray;
			__IotherArray.append(itemParent->child(iother)->data().toString());
			__IotherArray.append(itemParent->child(iother)->text());
			__IotherArray.append(QString::number(itemParent->child(iother)->checkState()));
			__TopArray.append(__IotherArray);
		}
		frmToolFlow.append(__TopArray);
	}
	strData.insert("frmToolFlow", frmToolFlow);
	return 0;
}

int frmToolFlow::SetData(QJsonObject& strData)
{
	m_ptrItems->clear();

	QJsonArray frmToolFlow = strData.find("frmToolFlow").value().toArray();
	for (size_t irow = 0; irow < frmToolFlow.count(); irow++)	{
		//其中一个流程
		QJsonArray __TopArray		= frmToolFlow.at(irow).toArray();
		QString toolDataName		= __TopArray.at(0).toString();
		QString toolName			= __TopArray.at(1).toString();
		Qt::CheckState	check		= (Qt::CheckState)__TopArray.at(2).toString().toInt();

		if (toolDataName.isEmpty())	continue;
		QStandardItem* itemParent = nullptr;
		QList<QStandardItem*> SeletItems = findItems(toolDataName); //m_ptrItems->findItems(toolBoxName, Qt::MatchExactly);
		if (SeletItems.count() <= 0)		{
			itemParent = new QStandardItem();
			itemParent->setText(toolName);
			itemParent->setData(toolDataName);
			itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			itemParent->setCheckable(true);
			itemParent->setCheckState(check);
			m_ptrItems->appendRow(itemParent);
		}
		else {	itemParent = SeletItems[0];	}

		int	_iCount = __TopArray.count();
		for (size_t iother = 3; iother < _iCount; iother++)		{
			QJsonArray __IotherArray	= __TopArray.at(iother).toArray();
			QString toolDataName		= __IotherArray.at(0).toString();
			QString toolName			= __IotherArray.at(1).toString();
			Qt::CheckState	check		= (Qt::CheckState)__IotherArray.at(2).toString().toInt();
			if (toolName.isEmpty())		continue;
			SeletItems					= findItems(toolDataName);
			QStandardItem*			itemother = nullptr;
			if (SeletItems.count() <= 0)		{
				itemother				= new QStandardItem();
				itemother->setText(toolName);
				itemother->setData(toolDataName);
				itemother->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				itemother->setCheckable(true);
				itemother->setCheckState(check);
				QString strNum = FindNum(toolDataName);
				if(!strNum.isEmpty())
					if(strNum.toInt() < databaseVar::Instance().m_vecPtr.size())
						itemother->setCheckState(databaseVar::Instance().m_vecPtr[strNum.toInt()]->m_bEnable ? Qt::Checked :Qt::Unchecked );
				itemParent->appendRow(itemother);
			}
			else		{
				itemother				= SeletItems[0];
			}
		}
	}
	ToolTreeWidget->expandAll();
	return 0;
}

int frmToolFlow::NewProject()
{	
	m_ptrItems->clear();
	return 0;
}

int frmToolFlow::AddToolFlow(QString strTop,QString strFlow)
{		
	//其中一个流程
	if (strTop.isEmpty())	return -1;
	if (strFlow.isEmpty())	return -1;
	QList<QStandardItem*> SeletItems = findItems(strTop);
	QStandardItem* itemParent = nullptr;
	if (SeletItems.count() <= 0)	{
		itemParent = new QStandardItem();
		itemParent->setText(tr(strTop.toStdString().c_str()));
		itemParent->setData(strTop);
		itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ptrItems->appendRow(itemParent);		
		itemParent->setCheckable(true);
	}
	else	{
		itemParent = SeletItems[0];
	}
	SeletItems = findItems(strFlow);
	QStandardItem*			itemother = nullptr;
	if (SeletItems.count() <= 0)	{
		itemother = new QStandardItem();
		itemother->setData( strFlow);
		QStringList strFlo  = strFlow.split(strTop);
		if (strFlo.size() > 0)
			itemother->setText(tr(strTop.toStdString().c_str()) + strFlo[strFlo.size() - 1]);
		itemother->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		itemother->setCheckable(true);
		itemother->setCheckState(Qt::Checked);
		itemParent->appendRow(itemother);
	}
	else	{
		itemother = SeletItems[0];
	}
	return 0;
}

int frmToolFlow::DeleteToolFlow(QString strTop,QString strFlow)
{
	if (strTop.isEmpty())	return -1;
	if (strFlow.isEmpty())	return -1;
	QList<QStandardItem*> SeletItems = findItems(strTop);
	QStandardItem* itemParent = nullptr;
	if (SeletItems.count() <= 0)	return -1;
	else	itemParent	= SeletItems[0];
	QList<QStandardItem*> Items = itemParent->takeColumn(0);
	itemParent->removeRows(0, Items.size());
	for (QList<QStandardItem*>::iterator iter = Items.begin(); iter != Items.end(); )	{
		if ((*iter) != nullptr)		{
			if ((*iter)->data().toString() == strFlow)			{
				Items.erase(iter);	
				break;
			}
			else { iter++; }
		}
		else { iter++; }
	}
	itemParent->insertColumn(0,Items);
	if (Items.size() <= 0)	{
		delete	m_ptrItems->item(0);
		m_ptrItems->removeRows(0, 1);
	}

	return 0;
}

void frmToolFlow::slot_btnFlowAdd_clicked()	//添加
{
	int	_iIndex = 0;
	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());

	QString	strTop = ("flow");
	QString	_strTop = tr("flow");

	QList<QStandardItem*> SeletItems = findItems(strTop);
	QStandardItem* itemParent = nullptr;
	if (SeletItems.count() <= 0) 	{
		QString	_strFlow	= strTop + QString::number(0);
		AddToolFlow(strTop, _strFlow);
		emit sig_AddFlow(_strFlow, _strTop + QString::number(0)	);
		ToolTreeWidget->expandAll();
		return;
	}
	else itemParent = SeletItems[0];
	SeletItems = itemParent->takeColumn(0);
	int	_Index = -1;
	for (int iSize = 0; iSize < (SeletItems.size() + 1); iSize++)	{
		bool	_bFind		= false;
		QString	_strFlow	= strTop + QString::number(iSize);
		for (int i = 0; i < SeletItems.size(); i++)		{
			QString	_strFlow1 = SeletItems[i]->data().toString();
			if (_strFlow1 == _strFlow)			{
				_bFind = true;	break;
			}
		}
		if (!_bFind)		{
			_Index = iSize;
			break;
		}
	}
	if (_Index != -1){
		QString	_strFlow = strTop + QString::number(_Index);
		itemParent->insertColumn(0,SeletItems);
		AddToolFlow(strTop, _strFlow);

		if (currentItem != nullptr)	currentItem->setSelectable(true);//m_ptrItems->set(currentItem,true);
		emit sig_AddFlow(_strFlow, _strTop + QString::number(_Index));
	}
	ToolTreeWidget->expandAll();
}

void frmToolFlow::slot_btnFlowDelete_clicked()	//删除
{
	QList<QStandardItem*> SeletItems;
	QString	strTop = ("flow");
	QString	_strTop = tr("flow");

	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());
	if (currentItem != nullptr)
	{
		QString	_strFlow1 = currentItem->data().toString();
		if (_strFlow1 == strTop)		{
			SeletItems = currentItem->takeColumn(0);
			currentItem->removeRows(0, SeletItems.size() > 1 ? SeletItems.size() : 1);
			for (int iRow = 0; iRow < SeletItems.size(); iRow++) 			{
				if (SeletItems[iRow] != nullptr) 				{
					_strFlow1 = SeletItems[iRow]->data().toString();
					emit sig_DeleteFlow(_strFlow1);
					delete SeletItems[iRow]; 
				} 
			}
			m_ptrItems->removeRows(0,1);
			delete	m_ptrItems->item(0);//takeTopLevelItem(0);
			emit sig_DeleteFlow(_strFlow1);
		}
		else		{
			DeleteToolFlow(strTop, _strFlow1);

			emit sig_DeleteFlow(_strFlow1);
		}
	}
	if (currentItem != nullptr)	currentItem->setSelectable(true);//ToolTreeWidget->setItemSelected(item, true);
}

void frmToolFlow::slot_btnMoveUp_clicked()		//上移动
{
	QString	strTop = ("flow");
	QList<QStandardItem*> SeletItems = findItems(strTop);// m_ptrItems->findItems(strTop, Qt::MatchExactly);
	QStandardItem* itemParent = nullptr;
	if (SeletItems.count() <= 0)	return ;
	else itemParent = SeletItems[0];

	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());
	if (currentItem != nullptr)								{
		QString	_strFlow	= currentItem->data().toString();
		if (_strFlow		!= strTop)						{
			QList<QStandardItem*> Items = itemParent->takeColumn(0); //takeChildren();
			itemParent->removeRows(0, Items.size() > 1 ? Items.size() : 1);
			for (int iRow = 0; iRow < Items.size(); iRow++)	{
				QString	_strIndex = Items[iRow]->data().toString();
				if (_strFlow == _strIndex)					{
					if ((iRow - 1) >= 0)					{
						Items.swap(iRow,(iRow - 1));
						break;
					}
				}
			}
			itemParent->insertColumn(0, Items); //addChildren(Items);
		}
	}
	if (currentItem != nullptr)	currentItem->setSelectable(true);//ToolTreeWidget->setItemSelected(currentItem, true);
}

void frmToolFlow::slot_btnMoveDown_clicked()	//下移
{
	QString	strTop = /*QString::fromLocal8Bit*/("flow");
	QList<QStandardItem*> SeletItems = findItems(strTop);//m_ptrItems->findItems(strTop, Qt::MatchExactly);
	QStandardItem* itemParent = nullptr;
	if (SeletItems.count() <= 0)	return;
	else itemParent = SeletItems[0];

	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());
	if (currentItem != nullptr)	{
		QString	_strFlow = currentItem->data().toString();
		if (_strFlow != strTop)		{
			QList<QStandardItem*> Items = itemParent->takeColumn(0);
			itemParent->removeRows(0, Items.size() > 1 ? Items.size() : 1);
			for (int iRow = 0; iRow < Items.size(); iRow++)			{
				QString	_strIndex = Items[iRow]->data().toString();
				if (_strFlow == _strIndex)				{
					if ((iRow + 1) < Items.size())					{
						Items.swap(iRow, (iRow + 1));
						break;
					}
				}
			}
			itemParent->insertColumn(0, Items); //addChildren(Items);
		}
	}
	if (currentItem != nullptr)	currentItem->setSelectable(true);//ToolTreeWidget->setItemSelected(currentItem, true);

}

//重命名
void frmToolFlow::slot_btnReName_clicked()
{
	QString strOld, strNew;
	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());

	if (currentItem != nullptr)	{
		m_plEdit->show();
		m_plEdit->setText(currentItem->text());    //写入选项卡当前的名字
		auto pRect = ToolTreeWidget->visualRect(ToolTreeWidget->currentIndex());
		m_plEdit->resize(QSize(pRect.width(), pRect.height()));
		m_plEdit->move(pRect.x(), pRect.y());
		m_plEdit->setFocus();        //设置焦点
		m_plEdit->selectAll();       //字符全选

		//emit sig_ReNameFlow(strOld, strNew);
	}

}

void frmToolFlow::slot_SetFlowName()
{
	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());
	if (currentItem != nullptr)	{
		QString	strKey = currentItem->data().toString();
		QString	strOld = currentItem->text();
		currentItem->setText(m_plEdit->text());
		emit sig_ReNameFlow(strKey, strOld, m_plEdit->text());
	}
	m_plEdit->hide();
}

void frmToolFlow::slot_FlowClick(const QModelIndex & index)
{
	QStandardItem * currentItem = m_ptrItems->itemFromIndex(index/*ToolTreeWidget->currentIndex()*/);
	if (currentItem != nullptr)	{
		QString	_strFlow = currentItem->data().toString();
		emit sig_SetFlowEnable(_strFlow, currentItem->checkState() == Qt::Checked ? true:false);
	}
}

QString frmToolFlow::FindNum(QString str)
{
	QString strNum;
	QRegExp rx("\\d+");  // 正则表达式匹配连续的数字
	int pos = 0;
	while ((pos = rx.indexIn(str, pos)) != -1) {
		strNum += rx.cap(0);  // 获取匹配到的数字
		pos += rx.matchedLength();
	}
	return strNum;
}

void frmToolFlow::changeEvent(QEvent * ev)
{
	QString	strTop = ("flow");
	switch (ev->type())	{
	case QEvent::LanguageChange: {
		for (int iRow = 0; iRow < m_ptrItems->rowCount(); iRow++) {
			auto pItem = m_ptrItems->item(iRow);
			if (pItem) {
				UpdateStandardItems(pItem);
			}
		}
	}	break;
	default:	break;
	}
}

void frmToolFlow::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu* menu = new QMenu(this);
	//增加
	QPushButton* _btn_Add = new QPushButton(menu);
	_btn_Add->setObjectName("Add");
	_btn_Add->setText(tr("添加"));
	_btn_Add->setIconSize(QSize(22, 22));
	_btn_Add->setMinimumSize(QSize(120, 40));
	QWidgetAction* _btn_AddAction = new QWidgetAction(this);
	_btn_AddAction->setDefaultWidget(_btn_Add);
	connect(_btn_Add, &QPushButton::clicked, this, &frmToolFlow::slot_btnFlowAdd_clicked);
	connect(_btn_Add, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(_btn_AddAction);

	QPushButton* _btn_Delete = new QPushButton(menu);
	_btn_Delete->setObjectName("Delete");
	_btn_Delete->setText(/*QString::fromLocal8Bit*/tr("删除"));
	_btn_Delete->setIconSize(QSize(22, 22));
	_btn_Delete->setMinimumSize(QSize(120, 40));
	QWidgetAction* _btn_DeleteAction = new QWidgetAction(this);
	_btn_DeleteAction->setDefaultWidget(_btn_Delete);
	connect(_btn_Delete, &QPushButton::clicked, this, &frmToolFlow::slot_btnFlowDelete_clicked);
	connect(_btn_Delete, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(_btn_DeleteAction);

	QPushButton* _btn_ReName = new QPushButton(menu);
	_btn_ReName->setObjectName("ReName");
	_btn_ReName->setText(/*QString::fromLocal8Bit*/tr("ReName"));
	_btn_ReName->setIconSize(QSize(22, 22));
	_btn_ReName->setMinimumSize(QSize(120, 40));
	_btn_ReName->setIcon(QIcon(":/QtApplication/Bitmaps/del.png"));
	QWidgetAction* _btn_ReNameAction = new QWidgetAction(this);
	_btn_ReNameAction->setDefaultWidget(_btn_ReName);
	connect(_btn_ReName, &QPushButton::clicked, this, &frmToolFlow::slot_btnReName_clicked);
	connect(_btn_ReName, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(_btn_ReNameAction);

	//上移
	QPushButton* _btn_MoveUp = new QPushButton(menu);
	_btn_MoveUp->setObjectName("MoveUp");
	_btn_MoveUp->setText(/*QString::fromLocal8Bit*/tr("上移"));
	_btn_MoveUp->setIconSize(QSize(22, 22));
	_btn_MoveUp->setMinimumSize(QSize(120, 40));
	QWidgetAction* _btn_MoveUpAction = new QWidgetAction(this);
	_btn_MoveUpAction->setDefaultWidget(_btn_MoveUp);
	connect(_btn_MoveUp, &QPushButton::clicked, this, &frmToolFlow::slot_btnMoveUp_clicked);
	connect(_btn_MoveUp, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(_btn_MoveUpAction);

	//下移
	QPushButton* _btn_MoveDown = new QPushButton(menu);
	_btn_MoveDown->setObjectName("MoveDown");
	_btn_MoveDown->setText(/*QString::fromLocal8Bit*/(tr("下移")));
	_btn_MoveDown->setIconSize(QSize(22, 22));
	_btn_MoveDown->setMinimumSize(QSize(120, 40));
	QWidgetAction* _btn_MoveDownAction = new QWidgetAction(this);
	_btn_MoveDownAction->setDefaultWidget(_btn_MoveDown);
	connect(_btn_MoveDown, &QPushButton::clicked, this, &frmToolFlow::slot_btnMoveDown_clicked);
	connect(_btn_MoveDown, &QPushButton::clicked, menu, &QMenu::close);
	menu->addAction(_btn_MoveDownAction);

	menu->exec(QCursor::pos());
	if (_btn_Add != nullptr)			delete _btn_Add;			_btn_Add = nullptr;
	if (_btn_AddAction != nullptr)		delete _btn_AddAction;		_btn_AddAction = nullptr;
	if (_btn_Delete != nullptr)			delete _btn_Delete;			_btn_Delete = nullptr;
	if (_btn_DeleteAction != nullptr)	delete _btn_DeleteAction;	_btn_DeleteAction = nullptr;
	if (_btn_MoveUp != nullptr)			delete _btn_MoveUp;			_btn_MoveUp = nullptr;
	if (_btn_MoveUpAction != nullptr)	delete _btn_MoveUpAction;	_btn_MoveUpAction = nullptr;
	if (_btn_MoveDown != nullptr)		delete _btn_MoveDown;		_btn_MoveDown = nullptr;
	if (_btn_MoveDownAction != nullptr)	delete _btn_MoveDownAction;	_btn_MoveDownAction = nullptr;
	if (_btn_ReName != nullptr)			delete _btn_ReName;			_btn_ReName = nullptr;
	if (_btn_ReNameAction != nullptr)	delete _btn_ReNameAction;	_btn_ReNameAction = nullptr;
	if (menu != nullptr)				delete menu;				menu = nullptr;

}

void frmToolFlow::ToolsTreeWidgetInit()	//工具列表初始化
{
	m_plEdit = new QLineEdit(this);
	m_plEdit->hide();
	m_plEdit->setToolTip(tr("输入工作表名字"));
	m_plEdit->setPlaceholderText(tr("输入工作表名字"));
	m_plEdit->setAlignment(Qt::AlignCenter);

	ToolTreeWidget = new QTreeView(this);
	m_ptrItems = new QStandardItemModel();
	ToolTreeWidget->setModel(m_ptrItems);
	QGridLayout* ToolGLayout = new QGridLayout(ui.ToolTreeFrame);
	ToolGLayout->setContentsMargins(0, 0, 0, 0);
	ToolGLayout->setSpacing(5);
	ToolGLayout->addWidget(ToolTreeWidget, 0, 0);
	ToolTreeWidget->setIconSize(QSize(24, 24));

	connect(ToolTreeWidget, &QTreeView::doubleClicked,	this, &frmToolFlow::slot_FlowDoubleClick);
	connect(ToolTreeWidget, &QTreeView::clicked,		this, &frmToolFlow::slot_FlowClick);

	ToolTreeWidget->header()->setVisible(false);
	m_ptrItems->clear();
	ToolTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::ContiguousSelection);
	ToolTreeWidget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	this->setAcceptDrops(true);

	connect(m_plEdit, SIGNAL(editingFinished()),	this, SLOT(slot_SetFlowName()));     //输入完成
	connect(m_plEdit, SIGNAL(returnPressed()),		this, SLOT(slot_SetFlowName()));
}

QList<QStandardItem*> frmToolFlow::findItems(QString strData)
{
	QList<QStandardItem*>	_SelectedLst;
	for (int iRow = 0; iRow < m_ptrItems->rowCount(); iRow++) {
		auto pItem = m_ptrItems->item(iRow);
		if (pItem) {
			if (pItem->data().toString() == strData) {
				_SelectedLst.push_back(pItem);
			}
			_SelectedLst.append(findStandardItems(pItem, strData));
		}
	}
	return _SelectedLst;
}

QList<QStandardItem*> frmToolFlow::findStandardItems(QStandardItem* pItem, QString strData)
{
	QList<QStandardItem*>	_SelectedLst;
	for (int i = 0; i < pItem->rowCount(); i++) {
		auto _pItem = pItem->child(i, 0);
		if (_pItem) {
			if (_pItem->data().toString() == strData) {
				_SelectedLst.push_back(_pItem);
			}
			_SelectedLst.append(findStandardItems(_pItem, strData));
		}
	}
	return _SelectedLst;
}

void frmToolFlow::UpdateStandardItems(QStandardItem * pItem)
{
	QString	strTop = ("flow");
	for (int i = 0; i < pItem->rowCount(); i++) {
		auto _pItem = pItem->child(i, 0);
		if (_pItem) {
			if (_pItem->text().contains(tr("flow"))){			
				QString	strNum = FindNum(_pItem->text());
				if (!strNum.isEmpty())
					_pItem->setText(tr(strTop.toStdString().c_str()) + strNum);
			}
		}
	}
}

void frmToolFlow::slot_FlowDoubleClick(const QModelIndex &index)
{
	QStandardItem * currentItem = m_ptrItems->itemFromIndex(ToolTreeWidget->currentIndex());
	if (currentItem != nullptr)	{
		QString	_strFlow = currentItem->data().toString();
		emit sig_ChangeFlow(_strFlow);
	}
}
