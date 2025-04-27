#include "frmImageShow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QThread>
#include <QElapsedTimer>
#include <QPainter>
#include <QtConcurrent>
#include <QWidgetAction>
#include <QToolBar>
#include <QToolButton>
#include <QFileDialog>
#include "databaseVar.h"
#include "MyEvent.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmImageShow::frmImageShow(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	//this->setWindowIcon(QIcon(":/QtApplication/Bitmaps/image_source.png"));

	//初始化标题栏
	initTitleBar();

	initConnect();
	//选项按钮
	btnGroupRadio = new QButtonGroup(this);

	variantManager		= new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),this, SLOT(valueChanged(QtProperty *, const QVariant &)));
	variantFactory		= new QtVariantEditorFactory(this);
	propertyEditor		= new QtTreePropertyBrowser(ui.tab_Camera);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.gridLayout_camera->addWidget(propertyEditor);

	ui.listWidget->setSelectionMode(QAbstractItemView::SelectionMode::ContiguousSelection);
	ui.listWidget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

frmImageShow::~frmImageShow()
{
	if (m_titleBar != nullptr)				delete m_titleBar;			m_titleBar = nullptr;
	if (btnGroupRadio != nullptr)			delete btnGroupRadio;		btnGroupRadio = nullptr;
	if (propertyEditor != nullptr)			delete propertyEditor;		propertyEditor = nullptr;
	if (variantManager != nullptr)			delete variantManager;		variantManager = nullptr;
	if (variantFactory != nullptr)			delete variantFactory;		variantFactory = nullptr;
	this->deleteLater();
}

void frmImageShow::initTitleBar()
{
	//m_titleBar->setTitleIcon(":/QtApplication/Bitmaps/image_source.png");
	//m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
}

void frmImageShow::initConnect()
{
	connect(ui.btnAdd,				&QPushButton::pressed,								this,&frmImageShow::on_btnAdd_clicked);
	connect(ui.btnDelete,			&QPushButton::pressed,								this,&frmImageShow::on_btnDelete_clicked);
}

void frmImageShow::ClearParam1D()
{
	QMap<QtProperty *, QString>::ConstIterator itProp = propertyToId.constBegin();
	while (itProp != propertyToId.constEnd()) {
		delete itProp.key();
		itProp++;
	}
	propertyToId.clear();
	idToProperty.clear();
}

EnumNodeResult frmImageShow::Execute(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

int frmImageShow::Load()
{
	variantManager->clear();
	propertyEditor->clear();
	QtVariantProperty *_ptrproperty = nullptr;
	QString Param;
	ClearParam1D();

	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((ShowImage*)m_pTool)->m_iFlowIndex)						{
		for (auto iter : MiddleParam::MapMiddle()[((ShowImage*)m_pTool)->m_iFlowIndex].MapPImgVoid)	{	//图像
			if (!iter.first.contains(((ShowImage*)m_pTool)->GetItemId()))								{
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}

	Param = QString::number(((ShowImage*)m_pTool)->m_iShowIndex);
	_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "ShowIndex");
	QStringList enumNames;
	for (int i = 0; i < (databaseVar::Instance().form_Layout_Number  ); i++)	enumNames.push_back( QString::number( i ) );
	_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
	_ptrproperty->setValue(Param);
	addProperty(_ptrproperty, "ShowIndex");

	//connect(ui.listWidget, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(slot_TreeItemPressed(QListWidgetItem *)));
	ui.listWidget->clear();
	for (auto iter : ((ShowImage*)m_pTool)->m_vecCamParam){
		QListWidgetItem* itemParent = new QListWidgetItem(ui.listWidget);
		itemParent->setText(iter);
		//itemParent->setIcon(QIcon(":/QtApplication/Bitmaps/tool.ico"));
		itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
	}
	int _iCount = ui.listWidget->count();

	return 0;
}

int frmImageShow::Save()
{
	((ShowImage*)m_pTool)->m_iShowIndex = idToProperty["ShowIndex"]->value().toInt();

	((ShowImage*)m_pTool)->m_vecCamParam.clear();
	for (int i = 0; i < ui.listWidget->count(); i++){
		if (ui.listWidget->item(i) == nullptr)	continue;
		auto iteme = ui.listWidget->item(i);
		((ShowImage*)m_pTool)->m_vecCamParam.push_back(iteme->text());
	}

	return 0;
}

void frmImageShow::ShowImg()
{
	QApplication::postEvent(this, new MsgEvent(1, 0, ""));
}

void frmImageShow::valueChanged(QtProperty *property, const QVariant &value)
{
	if (!propertyToId.contains(property))	return;
	QString id = propertyToId[property];
}

void frmImageShow::slot_btnGroupClicked(int value)
{

}

void frmImageShow::UpDataCameraParams(bool bCameraopened)
{
	variantManager->clear();
	propertyEditor->clear();
}

void frmImageShow::on_btnAdd_clicked()
{
	for (int i = 0; i < ui.listWidget->count(); i++) {
		if (ui.listWidget->item(i) == nullptr)	continue;
		auto iteme = ui.listWidget->item(i);
		if (iteme->text() == ui.cbx_ImgInPut->currentText()){return;}
	}
	QListWidgetItem* itemParent = new QListWidgetItem(ui.listWidget);
	itemParent->setText(ui.cbx_ImgInPut->currentText());
	//itemParent->setIcon(QIcon(":/QtApplication/Bitmaps/tool.ico"));
	itemParent->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
}

void frmImageShow::on_btnDelete_clicked()
{
	auto pItem = ui.listWidget->takeItem(ui.listWidget->currentIndex().row());
	if (pItem != nullptr)	delete pItem;	pItem = nullptr;
}

//void frmImageShow::on_btnExecute_clicked()
//{
//	//view->update();
//}

void frmImageShow::addProperty(QtVariantProperty * property, const QString & id)
{
	propertyToId[property] = id;
	idToProperty[id] = property;
	QtBrowserItem *item		= propertyEditor->addProperty(property);

}