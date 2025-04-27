/****************************************************************************
**
** 
****************************************************************************/

#include "settingdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QLabel>
#include <QTime>
#include "dialogs/System/frmNormalSet.h"
#include "dialogs/System/frmLogSet.h"
#include "dialogs/System/frmStartSet.h"
#include "dialogs/System/frmPasswordSet.h"
#include "dialogs/System/frm3DSet.h"
#include "QFramer/ftoolbutton.h"
#include "qdebug.h"
#include <QApplication>
#include "Data/dataVar.h"
#include <QScreen>

SettingDialog::SettingDialog(QWidget *parent) :
    FBaseDialog(parent)
{
	getTitleBar()->setMaxButtonVisible(true);
	initData();
	initUI();
	ToolsTreeWidgetInit();
	initConnect();
	LanguageChange();
}

SettingDialog::~SettingDialog()
{
	for (auto& iter : m_mapfrmSetBase)	{
		if (iter != nullptr)	
			delete iter;	
		iter = nullptr;
	}
	m_mapfrmSetBase.clear();
	this->deleteLater();
}

void SettingDialog::closeEvent(QCloseEvent * ev)
{
	ev->ignore();
	this->hide();
}

void SettingDialog::resizeEvent(QResizeEvent * ev)
{
	QDialog::resizeEvent(ev);
}

int SettingDialog::GetData(QJsonObject & strData)
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->GetData(strData);
	}
	return 0;
}

int SettingDialog::SetData(QJsonObject & strData)
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->SetData(strData);
	}
	UpdateUI();
	return 0;
}

int SettingDialog::NewProject()
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->NewProject();
	}
	return 0;
}

int SettingDialog::UpdateUI()
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->UpdateUI();
	}
	// 获取主屏幕
	QScreen *screen = QGuiApplication::primaryScreen();
	// 获取屏幕的尺寸
	QSize screenSize = screen->size();
	int screenWidth = screenSize.width() / 2.0;
	int screenHeight = screenSize.height() / 3.0 * 2;
	if (dataVar::Instance().SetDlgSize.width() >= 640) {
		screenWidth = dataVar::Instance().SetDlgSize.width();
	}
	if (dataVar::Instance().SetDlgSize.height() >= 480) {
		screenHeight = dataVar::Instance().SetDlgSize.height();
	}
	normalSize = QSize(screenWidth, screenHeight);
	return 0;
}

int SettingDialog::ScanTime()
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->ScanTime();
	}
	return 0;
}

void SettingDialog::RetranslationUi()
{
	//getTitleBar()->getTitleLabel()->setText(tr("SetDialog"));
	getTitleBar()->getContentLabel()->setText(tr("SetDialog"));
	lineEditSearch->setPlaceholderText(tr("lEditSearch"));
	for (auto iter : m_BtnLst){
		iter->setToolTip(tr(iter->objectName().toStdString().c_str()));
		iter->setText(tr(iter->objectName().toStdString().c_str()));
	}
	bool _bNeedBreak = false;
	for (int i = 0; i < m_pModel->rowCount(); i++){
		QStandardItem *item = m_pModel->item(i);
		if (item) {
			QString	_str = item->data().toString();
			auto _strLst = _str.split("_");
			if (_strLst.size() > 0) {
				for (auto iter = m_mapfrmSetBase.begin(); iter != m_mapfrmSetBase.end(); iter++){
					if (iter.key().contains(_str)){
						QVector<QPair<QString, QString>> vecNames = iter.value()->GetVecSetName();
						for (auto it = vecNames.begin(); it != vecNames.end(); it++){
							if ((*it).first == _strLst[_strLst.size() - 1]){
								item->setText((*it).second);
								_bNeedBreak = true;	break;
							}
						}
						if (_bNeedBreak)break;
					}
				}
			}
		}
		_bNeedBreak = false;
		if (item->hasChildren()){
			for (int i = 0; i < item->rowCount(); i++){
				QStandardItem * childitem = item->child(i);
				if (childitem != nullptr){
					QString	_str = childitem->data().toString();
					auto _strLst = _str.split("_");
					if (_strLst.size() > 1) {
						for (auto iter = m_mapfrmSetBase.begin(); iter != m_mapfrmSetBase.end(); iter++) {
							if (iter.key().contains(_str)) {
								QVector<QPair<QString, QString>> vecNames = iter.value()->GetVecSetName();
								for (auto it = vecNames.begin(); it != vecNames.end(); it++) {
									if ((*it).first == _strLst[_strLst.size() - 1]) {
										childitem->setText((*it).second);
										_bNeedBreak = true;	break;
									}
								}
								if (_bNeedBreak)break;
							}
						}
					}
				}
			}
		}
	}

}

void SettingDialog::LanguageChange()
{
	RetranslationUi();
}

QList<QStandardItem*> SettingDialog::findItems(QString strData)
{
	QList<QStandardItem*>	_SelectedLst;
	for (int iRow = 0; iRow < m_pModel->rowCount(); iRow++) {
		auto pItem = m_pModel->item(iRow);
		if (pItem) {
			if (pItem->data() == strData) {
				_SelectedLst.push_back(pItem);
			}
			_SelectedLst.append(findStandardItems(pItem, strData));
		}
	}
	return _SelectedLst;
}

QList<QStandardItem*> SettingDialog::findStandardItems(QStandardItem* pItem,QString strData)
{
	QList<QStandardItem*>	_SelectedLst;
	for (int i = 0; i < pItem->rowCount(); i++) {
		auto _pItem = pItem->child(i,0);
		if (_pItem) {
			if (_pItem->data() == strData) {
				_SelectedLst.push_back(_pItem);
			}
			_SelectedLst.append(findStandardItems(_pItem, strData));
		}
	}
	return _SelectedLst;
}

void SettingDialog::showEvent(QShowEvent * ev)
{
	FBaseDialog::showEvent(ev);
}

void SettingDialog::hideEvent(QHideEvent * ev)
{
	FBaseDialog::hideEvent(ev);
}

void SettingDialog::LoadData()
{
	QString strPath = QApplication::applicationDirPath();
	QString strFilePath = strPath + "/config/Systemconfig.json";
	QString strData;
	QFile file(strFilePath);
	if (!file.exists())	{
		qCritical() << tr("Not Exist") << strFilePath;
		return;
	}
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QByteArray t = file.readAll();
	strData = QString(t);
	file.close();
	// 开始解析 解析成功返回QJsonDocument对象否则返回null
	QJsonParseError err_rpt;
	QJsonDocument root_document = QJsonDocument::fromJson(strData.toUtf8(), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError && !root_document.isNull())	{
		qCritical() << tr("Not Exist") << strFilePath;
		return;
	}
	// 获取根节点
	QJsonObject root = root_document.object();
	SetData(root);
}

void SettingDialog::ResetData()
{
	for (auto& iter : m_mapfrmSetBase)	{
		iter->ResetData();
	}
}

void SettingDialog::SaveData()
{
	QString strPath = QApplication::applicationDirPath();
	QString strFilePath = strPath + "/config/Systemconfig.json";
	QJsonObject root;
	GetData(root);
	QJsonParseError err_rpt;
	QJsonDocument	root_document;

	root_document.setObject(root);
	QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
	QString	strData = QString(root_string_compact);
	QFile file(strFilePath);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	file.write(strData.toUtf8());
	file.close();
	UpdateUI();
	emit sigUpDateParam();
}

void SettingDialog::on_btnSave_clicked()
{
	SaveData();
	this->hide();
}

void SettingDialog::on_btnApply_clicked()
{
	SaveData();
}

void SettingDialog::on_btnCancer_clicked()
{
	this->hide();
}

void SettingDialog::initUI()
{
	// 获取主屏幕
	QScreen *screen = QGuiApplication::primaryScreen();
	// 获取屏幕的尺寸
	QSize screenSize = screen->size();
	int screenWidth = screenSize.width() / 2.0;
	int screenHeight = screenSize.height() / 3.0 * 2;
	if (dataVar::Instance().SetDlgSize.width() >= 640)	{
		screenWidth = dataVar::Instance().SetDlgSize.width();
	}
	if (dataVar::Instance().SetDlgSize.height() >= 480)	{
		screenHeight = dataVar::Instance().SetDlgSize.height();
	}
    normalSize = QSize(screenWidth, screenHeight);
    getTitleBar()->getContentLabel()->setText(tr("SetDialog"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	mainLayout->setContentsMargins(1, 1, 1, 1);
	mainLayout->setSpacing(0);
	setObjectName(QString("SetDialog"));
	stackWidget = new QStackedWidget();
	QVBoxLayout*verticalLayout_2 = new QVBoxLayout();
	verticalLayout_2->setSpacing(4);
	verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
	QHBoxLayout*horizontalLayout = new QHBoxLayout();
	horizontalLayout->setSpacing(0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

	lineEditSearch = new QLineEdit(this);
	lineEditSearch->setObjectName(QString::fromUtf8("lEditSearch"));
	lineEditSearch->setMinimumSize(QSize(50, 30));
	lineEditSearch->setPlaceholderText(tr("lEditSearch"));
	horizontalLayout->addWidget(lineEditSearch);

	FToolButton*btnSearch = new FToolButton(this);
	btnSearch->setObjectName(QString::fromUtf8("BtnSearch"));
	btnSearch->setToolTip(tr("BtnSearch"));
	btnSearch->setMinimumSize(QSize(30, 30));
	btnSearch->setIcon(QIcon(":/skin/icon/icon/icons/dark/search.png"));
	horizontalLayout->addWidget(btnSearch);
	m_BtnLst.append(btnSearch);

	verticalLayout_2->addLayout(horizontalLayout);
	verticalLayout_2->setContentsMargins(0, 5, 0, 0);
	treeViewSearch = new QTreeView(this);
	treeViewSearch->setObjectName(QString::fromUtf8("treeViewSearch"));
	treeViewSearch->setMinimumSize(QSize(250, 100));
	verticalLayout_2->addWidget(treeViewSearch);

	QHBoxLayout*horizontalLayout_2 = new QHBoxLayout();
	horizontalLayout_2->setSpacing(2);
	horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
	horizontalLayout_2->setContentsMargins(0, 0, 0, 0);

	horizontalLayout_2->addLayout(verticalLayout_2);
	horizontalLayout_2->addWidget(stackWidget);
	horizontalLayout_2->setStretch(0, 1);
	horizontalLayout_2->setStretch(1, 4);

	QHBoxLayout*horizontalLayout_4 = new QHBoxLayout();
	horizontalLayout_4->setSpacing(6);
	horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
	horizontalLayout_4->setContentsMargins(0, 4, 10, 4);

	QSpacerItem*horizontalSpacer_3 = new QSpacerItem(5, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
	horizontalLayout_4->addSpacerItem(horizontalSpacer_3);

	FToolButton* btnOK = new FToolButton(this);
	btnOK->setObjectName(QString::fromUtf8("btnOK"));
	btnOK->setText(tr("btnOK"));
	btnOK->setMinimumSize(QSize(100, 40));
	connect(btnOK, &QPushButton::clicked, this, &SettingDialog::on_btnSave_clicked);
	horizontalLayout_4->addWidget(btnOK);
	m_BtnLst.append(btnOK);

	FToolButton* btnApply = new FToolButton(this);
	btnApply->setObjectName(QString::fromUtf8("btnApply"));
	btnApply->setText(tr("btnApply"));
	btnApply->setMinimumSize(QSize(100, 40));
	connect(btnApply, &QPushButton::clicked, this, &SettingDialog::on_btnApply_clicked);
	horizontalLayout_4->addWidget(btnApply);
	m_BtnLst.append(btnApply);

	FToolButton*btnCancer = new FToolButton(this);
	btnCancer->setObjectName(QString::fromUtf8("btnCancer"));
	btnCancer->setMinimumSize(QSize(100, 40));
	btnCancer->setText(tr("btnCancer"));
	connect(btnCancer, &QPushButton::clicked, this, &SettingDialog::on_btnCancer_clicked);
	horizontalLayout_4->addWidget(btnCancer);
	m_BtnLst.append(btnCancer);

	QVBoxLayout*verticalLayout21 = new QVBoxLayout();
	verticalLayout21->addLayout(horizontalLayout_2);
	verticalLayout21->addLayout(horizontalLayout_4);
	verticalLayout21->setSpacing(2);
	verticalLayout21->setContentsMargins(0, 0, 0, 0);
	mainLayout->addLayout(verticalLayout21);
}

void SettingDialog::initData()
{
}

void SettingDialog::initConnect()
{
}

void SettingDialog::addWidget(const QString &tile, const QString &obejctName, frmSetBase* widget)
{
	//navagationBar->addNavgationTile(tile, obejctName);
	widget->setObjectName(obejctName);
	stackWidget->addWidget(widget);
}

int SettingDialog::AddWndSet(frmSetBase * widget)
{
	if (widget == nullptr) return -1;
	int index = m_pModel->rowCount();

	QVector<QPair<QString, QString>> vecNames = widget->GetVecSetName();

	//设置
	QStandardItem *			_TopItem = nullptr;
	QStandardItem *			_ChildItem = nullptr;
	QList<QStandardItem*>	_SelectedLst;
	QString	_TopName		= "Normal";
	if (index <= 0 ) {
		if (vecNames.size() > 0)	{
			_TopItem = new QStandardItem((const QString&)vecNames[0]);
			_TopItem->setData(vecNames[0].first);
			_TopItem->setText(vecNames[0].second);
			m_pModel->appendRow(_TopItem);
		}
		else return -1;
	}
	else	{
		if (vecNames.size() > 0) {
			_SelectedLst.clear();
			for (size_t i = 0; i < m_pModel->rowCount(); i++) {
				auto pItem = m_pModel->item(i);
				if (pItem) {
					if (pItem->data() == vecNames[0].first) {
						_SelectedLst.push_back(pItem);
					}
				}
			}
			if (_SelectedLst.size() <= 0) {
				_TopItem = new QStandardItem((const QString&)vecNames[0]);
				_TopItem->setData(vecNames[0].first);
				_TopItem->setText(vecNames[0].second);
				m_pModel->appendRow(_TopItem);
				_SelectedLst.push_back(_TopItem);
			}
			_TopItem = _SelectedLst[0];
		}
		else return -1;
	}

	QString	_strName = vecNames[0].first;
	for (size_t i = 1; i < vecNames.size(); i++)	{
		_TopName = vecNames[i].first;
		_strName.append("_").append(_TopName);
		_SelectedLst.clear();
		for (size_t i = 0; i < m_pModel->rowCount(); i++) {
			auto pItem = m_pModel->item(i);
			if (pItem) {
				if (pItem->data() == _TopName) {
					_SelectedLst.push_back(pItem);
				}
			}
		}

		if (_SelectedLst.size() > 0)		{
			_ChildItem = _SelectedLst[0];
		}
		else		{
			_ChildItem = new QStandardItem(_TopName);
			_ChildItem->setText(vecNames[i].second);
			_ChildItem->setData(_strName);
			_TopItem->appendRow(_ChildItem);
		}
		if (i < (vecNames.size() - 1))		{
			_TopItem = _ChildItem;
		}
		else		{
			_ChildItem->setData(_strName);
		}
	}
	m_mapfrmSetBase.insert(_strName, widget);
	m_mapfrmSetBaseIndex.insert(_strName, stackWidget->count());
	stackWidget->addWidget(widget);
	return 0;
}

void SettingDialog::ToolsTreeWidgetInit()
{
	connect(treeViewSearch, &QTreeView::clicked, this, &SettingDialog::slotSearchTreeClick);
	m_pModel = new QStandardItemModel(treeViewSearch);
	treeViewSearch->setModel(m_pModel);
	treeViewSearch->header()->setSectionResizeMode(QHeaderView::Stretch);
	treeViewSearch->header()->setVisible(false);
	treeViewSearch->setEditTriggers(QAbstractItemView::NoEditTriggers);

	frmSetBase::AddWndSet(	new frmStartSet()		);
	frmSetBase::AddWndSet(	new frmNormalSet()		);
	frmSetBase::AddWndSet(	new frmLogSet()			);
	frmSetBase::AddWndSet(	new frmPasswordSet()	);
	frmSetBase::AddWndSet(	new frm3DSet()	);

	for (auto iter = frmSetBase::m_vecFrm.begin();
		iter != frmSetBase::m_vecFrm.end(); iter++) {
		AddWndSet(*iter);
	}

	for (auto iter = m_mapfrmSetBaseIndex.begin(); iter != m_mapfrmSetBaseIndex.end(); iter++) {
		if (0 == iter.value()) {
			QList<QStandardItem*> itemsLst = findItems(iter.key());
			if (itemsLst.size() > 0)
				treeViewSearch->setCurrentIndex(itemsLst[0]->index());
			break;
		}
	}

}

void SettingDialog::cloudAntimation(animation_Direction direction)
{
	QLabel* circle = new QLabel(stackWidget->currentWidget());
	QLabel* line = new QLabel(this);
	line->setObjectName(QString("AntimationLine"));
	line->resize(0, 2);
	line->show();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	circle->setPixmap(QPixmap::grabWidget(stackWidget->widget(preindex), stackWidget->widget(preindex)->geometry()));
#else
	//circle->setPixmap(stackWidget->widget(preindex)->grab());
#endif

	//	circle->setScaledContents(true);
	circle->show();
	circle->resize(stackWidget->currentWidget()->size());
	QPropertyAnimation *animation = new QPropertyAnimation(circle, "geometry");

	animation->setDuration(1000);
	animation->setStartValue(circle->geometry());

	QPropertyAnimation* animation_line = new QPropertyAnimation(line, "size");
	animation_line->setDuration(1000);
	animation_line->setEasingCurve(QEasingCurve::OutCubic);

	switch (direction) {
	case animationTop:
		animation->setEndValue(QRect(circle->x(), circle->y() - 10, circle->width(), 0));
		break;
	case animationTopRight:
		animation->setEndValue(QRect(circle->width(), 0, 0, 0));
		break;
	case animationRight:
		line->move(0, stackWidget->y() - 2);
		animation->setEndValue(QRect(circle->width() + 3, 0, 0, circle->height()));
		animation_line->setStartValue(QSize(0, 2));
		animation_line->setEndValue(QSize(stackWidget->width(), 2));
		break;
	case animationBottomRight:
		animation->setEndValue(QRect(circle->width(), circle->height(), 0, 0));
		break;
	case animationBottom:
		animation->setEndValue(QRect(0, circle->height() + 10, circle->width(), 0));
		break;
	case animationBottomLeft:
		animation->setEndValue(QRect(0, circle->height(), 0, 0));
		break;
	case animationLeft:
		animation->setEndValue(QRect(-3, 0, 0, circle->height()));
		line->move(stackWidget->x(), stackWidget->y() - 2);
		animation_line->setStartValue(QSize(0, 2));
		animation_line->setEndValue(QSize(stackWidget->width(), 2));
		break;
	case animationTopLeft:
		animation->setEndValue(QRect(0, 0, 0, 0));
		break;
	case animationCenter:
		animation->setEndValue(QRect(circle->width() / 2, circle->height() / 2, 0, 0));
		break;
	default:
		break;
	}
	animation->setEasingCurve(QEasingCurve::OutCubic);

	QPropertyAnimation* animation_opacity = new QPropertyAnimation(circle, "windowOpacity");
	animation_opacity->setDuration(1000);
	animation_opacity->setStartValue(1);
	animation_opacity->setEndValue(0);
	animation_opacity->setEasingCurve(QEasingCurve::OutCubic);

	QParallelAnimationGroup *group = new QParallelAnimationGroup;

	connect(group, SIGNAL(finished()), circle, SLOT(hide()));
	connect(group, SIGNAL(finished()), circle, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), line, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation_opacity, SLOT(deleteLater()));
	connect(group, SIGNAL(finished()), animation_line, SLOT(deleteLater()));
	group->addAnimation(animation);
	group->addAnimation(animation_opacity);
	group->addAnimation(animation_line);
	group->start();
}

void SettingDialog::slotSearchTreeClick(const QModelIndex & index)
{
	QStandardItem* itemToMove = m_pModel->itemFromIndex(index);
	if (itemToMove)	{
		QString	_Item = itemToMove->data().toString();
		if (m_mapfrmSetBaseIndex.contains(_Item))	{
			if (m_mapfrmSetBase.count(_Item) > 0)
				if(m_mapfrmSetBase[_Item]->CheckPageAvalible())		{
					if (stackWidget->currentIndex() != m_mapfrmSetBaseIndex[_Item])
						stackWidget->setCurrentIndex(m_mapfrmSetBaseIndex[_Item]);
				}
				else {
					int _iIndex = stackWidget->currentIndex();
					for (auto iter = m_mapfrmSetBaseIndex.begin(); iter != m_mapfrmSetBaseIndex.end(); iter++)	{
						if (_iIndex == iter.value())		{
							QList<QStandardItem*> itemsLst = findItems(iter.key());
							if (itemsLst.size() > 0)
								treeViewSearch->setCurrentIndex(itemsLst[0]->index());
							break;
						}
					}
				}
		}
	}
}