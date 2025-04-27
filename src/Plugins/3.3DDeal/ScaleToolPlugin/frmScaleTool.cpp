#include "frmScaleTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QElapsedTimer>
#include "databaseVar.h"
#include <QColorDialog>
#include "ImageShow/ImageItem.h"
#include "ImageShow/QGraphicsScenes.h"
#include <fstream>
#include <QWidgetAction>

#include "databaseVar.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmScaleTool::frmScaleTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/RightScale.png"));

	initTitleBar();
	initConnect();
	//图像显示
	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmScaleTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	connect(ui.cbx_Camerachannal,	SIGNAL(activated(int)), this, SLOT(onChannelClicked(int)));
	connect(ui.cbx_ImgInPut,		SIGNAL(activated(int)), this, SLOT(slot_ChangeMatchImg_Index(int)));
	connect(ui.tableWidget,			&QTableWidget::customContextMenuRequested, this, &frmScaleTool::slot_CustomContextMenuRequested);		//这个坐标一般是相对于控件左上角

	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	//添加右键菜单
	//AddRightButtonMenu();

	ui.tableWidget->horizontalHeader()->setVisible(true);
	ui.tableWidget->verticalHeader()->setVisible(true);
	ui.tabWidget->setCurrentIndex(0);
}

frmScaleTool::~frmScaleTool()
{
	if (m_view != nullptr)					delete m_view;						m_view		= nullptr;
	if (m_titleBar != nullptr)				delete m_titleBar;					m_titleBar	= nullptr;

	this->deleteLater();
}

void frmScaleTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/RightScale.png");

}

void frmScaleTool::initConnect()
{
	connect(ui.btnAddROI, &QPushButton::pressed, this, &frmScaleTool::on_btnAddROI_clicked);
}

void frmScaleTool::ResizeUI()
{
	int count = ui.tableWidget->columnCount();
	int	iWidth = ui.tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++) {
		ui.tableWidget->setColumnWidth(i, iWidth);
	}
}

//绘制ROI
void frmScaleTool::on_btnAddROI_clicked()
{
	int count = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(count + 1);  //设置行数
	QWidget*			_pQWidget = nullptr;
	QTableWidgetItem*	item_value = nullptr;

	//item_value = new QTableWidgetItem(QString::number(count));
	//item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
	//item_value->setTextAlignment(Qt::AlignCenter);
	//ui.tableWidget->setItem(count, 0, item_value);

	item_value = new QTableWidgetItem(ui.cbx_ImgInPut->currentText());
	item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
	item_value->setTextAlignment(Qt::AlignCenter);
	ui.tableWidget->setItem(count, 0, item_value);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setStyleSheet(ui.spBx_Scale->styleSheet());
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-10000000);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(10000000);
	((QDoubleSpinBox*)_pQWidget)->setSingleStep(0.100000000000000);
	((QDoubleSpinBox*)_pQWidget)->setDecimals(ui.spBx_Scale->decimals());
	((QDoubleSpinBox*)_pQWidget)->setValue(ui.spBx_Scale->value());
	if (_pQWidget != nullptr)	ui.tableWidget->setCellWidget(count, 1, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setStyleSheet(ui.spBx_Scale->styleSheet());
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setDecimals(ui.spBx_Scale->decimals());
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-10000000);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(10000000);
	((QDoubleSpinBox*)_pQWidget)->setSingleStep(0.100000000000000);
	((QDoubleSpinBox*)_pQWidget)->setValue(ui.spBx_Offset->value());
	if (_pQWidget != nullptr)	ui.tableWidget->setCellWidget(count, 2, _pQWidget);
}

void frmScaleTool::onChannelClicked(int Index)
{
	if (Index < 0)	return;
	try
	{
		m_iChanal = Index;
		QImage img(Hal2QImage(m_dstImage));
		m_view->DispImage(img);
		m_view->update();
		//ui.btnExecute->setEnabled(true);
	}
	catch (...) {}
}

void frmScaleTool::slot_ChangeMatchImg_Index(int Index)
{
	m_srcImage.Clear();
	if (MiddleParam::MapMiddle()[((ScaleTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((ScaleTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ ui.cbx_ImgInPut->currentText() ]);
		if (pImg.IsInitialized()) {
			m_srcImage = pImg.Clone();
			m_dstImage = m_srcImage.Clone();
			QImage img(Hal2QImage(m_dstImage));
			m_view->DispImage(img);

			ui.cbx_Camerachannal->clear();
			HTuple	hType, hChannel;
			GetImageType(m_dstImage, &hType);
			CountChannels(m_dstImage, &hChannel);
			ui.cbx_Camerachannal->addItem("All");
			for (int i = 0; i < hChannel.I(); i++) {
				ui.cbx_Camerachannal->addItem(QString::number(i + 1));
			}
		}
	}
}

EnumNodeResult frmScaleTool::Execute(MiddleParam & param, QString & strError)
{
	if (!m_srcImage.IsInitialized()) { return NodeResult_Error; }

	HTuple	hType, hChannel;
	GetImageType(m_srcImage, &hType);

	QVector<sScale> m_vecScale;
	for (int row = 0; row < ui.tableWidget->rowCount(); row++) {
		QVector<QString> _Data;
		sScale _sCaleData;
		if (ui.tableWidget->item(row, 0) != nullptr)		_sCaleData.strImg = ui.tableWidget->item(row, 0)->text();
		if (ui.tableWidget->cellWidget(row, 1) != nullptr)	_sCaleData.fScale = ((QDoubleSpinBox*)ui.tableWidget->cellWidget(row, 1))->value();
		if (ui.tableWidget->cellWidget(row, 2) != nullptr)	_sCaleData.fOffset = ((QDoubleSpinBox*)ui.tableWidget->cellWidget(row, 2))->value();
		m_vecScale.push_back(_sCaleData);
	}
	HObject _dstImage;
	if (hType.S() == HString(HString("byte"))) {
		_dstImage = m_srcImage.Clone();
		ConvertImageType(m_dstImage, &_dstImage, HTuple("real"));
	}
	else if (hType.S() == HString("real") || hType.S() == HString("uint2") || hType.S() == HString("int4")) {
		ConvertImageType(m_srcImage, &_dstImage, HTuple("real"));
	}

	for (size_t i = 0; i < m_vecScale.size(); i++) {
		ScaleImage(_dstImage, &_dstImage, m_vecScale[i].fScale, m_vecScale[i].fOffset);
	}
	m_dstImage.Clear();
	m_dstImage = _dstImage.Clone();
	QImage img(Hal2QImage(m_dstImage));
	//m_view->DispImage(img);

	ui.cbx_Camerachannal->clear();
	GetImageType(m_dstImage, &hType);
	CountChannels(m_dstImage, &hChannel);
	ui.cbx_Camerachannal->addItem("All");
	for (int i = 0; i < hChannel.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));
	return NodeResult_OK;
}

void frmScaleTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
	ResizeUI();
}

int frmScaleTool::Load()
{
	m_titleBar->setTitleContent(((ScaleTool*)m_pTool)->GetItemId());

	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((ScaleTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((ScaleTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((ScaleTool*)m_pTool)->GetItemId())) {
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((ScaleTool*)m_pTool)->m_strInImg);
	if (ui.cbx_ImgInPut->currentIndex() >= 0) {
		slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	}

	if (MiddleParam::MapMiddle()[((ScaleTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ((ScaleTool*)m_pTool)->m_strInImg) > 0)	{
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((ScaleTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
		if (pImg.IsInitialized()) {
			m_srcImage = pImg.Clone();
			m_dstImage = m_srcImage.Clone();
			QImage img(Hal2QImage(m_dstImage));
			m_view->DispImage(img);

			ui.cbx_Camerachannal->clear();
			HTuple	hType, hChannel;
			GetImageType(m_dstImage, &hType);
			CountChannels(m_dstImage, &hChannel);
			ui.cbx_Camerachannal->addItem("All");
			for (int i = 0; i < hChannel.I(); i++) {
				ui.cbx_Camerachannal->addItem(QString::number(i + 1));
			}
		}
	}
	int count = ui.tableWidget->rowCount();
	for (int row = 0; row < count; row++) ui.tableWidget->removeRow(row);
	for (int irow = 0; irow < ((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale.size(); irow++)	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数
		QWidget*			_pQWidget = nullptr;
		QTableWidgetItem*	item_value = nullptr;

		item_value = new QTableWidgetItem(((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale[irow].strImg);
		item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_value->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(irow, 0, item_value);

		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setStyleSheet(ui.spBx_Scale->styleSheet());
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setDecimals(ui.spBx_Scale->decimals());
		((QDoubleSpinBox*)_pQWidget)->setMinimum(-10000000000000000);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(100000000000000000);
		((QDoubleSpinBox*)_pQWidget)->setSingleStep(0.100000000000000);
		((QDoubleSpinBox*)_pQWidget)->setValue(((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale[irow].fScale);
		if (_pQWidget != nullptr)	ui.tableWidget->setCellWidget(irow, 1, _pQWidget);

		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setStyleSheet(ui.spBx_Scale->styleSheet());
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setDecimals(ui.spBx_Scale->decimals());
		((QDoubleSpinBox*)_pQWidget)->setMinimum(-10000000000000000);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(100000000000000000);
		((QDoubleSpinBox*)_pQWidget)->setSingleStep(0.100000000000000);
		((QDoubleSpinBox*)_pQWidget)->setValue(((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale[irow].fOffset);
		if (_pQWidget != nullptr)	ui.tableWidget->setCellWidget(irow, 2, _pQWidget);
	}
	return 0;
}

int frmScaleTool::Save()
{
	((ScaleTool*)m_pTool)->m_strInImg = ui.cbx_ImgInPut->currentText();
	((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale.clear();
	for (int row = 0; row < ui.tableWidget->rowCount(); row++)	{
		QVector<QString> _Data;
		sScale _sCaleData;
		if (ui.tableWidget->item(row, 0) != nullptr)		_sCaleData.strImg	= ui.tableWidget->item(row, 0)->text();
		if (ui.tableWidget->cellWidget(row, 1) != nullptr)	_sCaleData.fScale	= ((QDoubleSpinBox*)ui.tableWidget->cellWidget(row, 1))->value();
		if (ui.tableWidget->cellWidget(row, 2) != nullptr)	_sCaleData.fOffset	= ((QDoubleSpinBox*)ui.tableWidget->cellWidget(row, 2))->value();
		((ScaleTool*)((ScaleTool*)m_pTool))->m_vecScale.push_back(_sCaleData);
	}
	return 0;
}

void frmScaleTool::slot_MoveUp()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmScaleTool::slot_MoveDown()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmScaleTool::slot_DeleteName()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex >= 0)	{
		ui.tableWidget->removeRow(rowIndex);
	}
}

void frmScaleTool::slot_CustomContextMenuRequested(const QPoint & pos)
{
	QMenu*	m_menu = new QMenu(this);
	QPushButton* deleteNameAction_buf = new QPushButton(m_menu);
	deleteNameAction_buf->setObjectName("Delete");
	deleteNameAction_buf->setMinimumSize(QSize(120, 40));
	deleteNameAction_buf->setText(tr("Delete"));
	//deleteNameAction_buf->setIconSize(QSize(30, 21));
	QWidgetAction* deleteNameAction = new QWidgetAction(this);
	deleteNameAction->setDefaultWidget(deleteNameAction_buf);
	connect(deleteNameAction_buf, &QPushButton::clicked, this, &frmScaleTool::slot_DeleteName);
	connect(deleteNameAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(deleteNameAction);
	m_menu->exec(QCursor::pos());

	if (deleteNameAction_buf != nullptr)	delete deleteNameAction_buf;		deleteNameAction_buf = nullptr;
	if (deleteNameAction != nullptr)		delete deleteNameAction;			deleteNameAction = nullptr;
	if (m_menu != nullptr)					delete m_menu;						m_menu = nullptr;

}
