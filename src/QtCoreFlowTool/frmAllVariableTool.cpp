#include "ui_frmAllVariableTool.h"
#include "frmAllVariableTool.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QPainter>
#include "qmutex.h"
#include <QThread>
#include <qdebug.h>
#include <QApplication>
#include <QAbstractItemView>
#include <QComboBox>
#include "Comm.h"
#include <QSpinBox>

frmAllVariableTool::frmAllVariableTool(QWidget* parent)
	: frmBase(parent)
	, m_strModelIndex("")
	, m_strTypeVariable("")
	, m_strValueName("")
	, ui(new Ui::frmAllVariableToolClass())
{
	ui->setupUi(this);
	m_eLimitType = EnumLimitType_NotAll;
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);

	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

	//设置窗口背景透明
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/appbar.network.port.png"));

	//初始化标题栏
	initTitleBar();

	//设置表格列宽	
	ui->tableWidget->setColumnWidth(0, 120);
	ui->tableWidget->setColumnWidth(1, 200);
	ui->tableWidget->setColumnWidth(2, 240);
	ui->tableWidget->setColumnWidth(3, 240);

	//隐藏水平header
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	connect(this, &frmAllVariableTool::sig_GlobalValue, this, &frmAllVariableTool::slot_GlobalValue, Qt::BlockingQueuedConnection);

	//隐藏水平header
	ui->tableModelWidget->verticalHeader()->setVisible(false);
	ui->tableModelWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui->tableModelWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	connect(ui->tableModelWidget,	SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slot_DoubleClicked(int, int)));
	connect(ui->tableWidget,		SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slot_VariableDoubleClicked(int, int)));

	ClearModelTable();

	ui->widget->setVisible(false);
}

frmAllVariableTool::~frmAllVariableTool()
{
	if (m_titleBar != nullptr)	delete	m_titleBar; m_titleBar = nullptr;
	this->deleteLater();
	if (ui != nullptr)			delete ui;		ui = nullptr;
}

void frmAllVariableTool::SetLimitModel(QString strModel, int Type)
{
	m_strLimitModel = strModel;
	m_eLimitType = Type;
}

void frmAllVariableTool::SetCurrentLinkContent(QString strLinkContent)
{
	m_strModelIndex.clear();
	m_strTypeVariable.clear();
	m_strValueName.clear();

	MiddleParam* _Params												= nullptr;
	int	_DataType														= -1;
	bool _bFind															= false;
	QStringList _Lst = strLinkContent.split(":");
	if (_Lst.size() > 0)												{
		if (!_Lst[0].isEmpty())											{
			if (_Lst[0] == "GlobalVar")									{
				_DataType				= 0;
				_Params					= MiddleParam::Instance();
				m_strModelIndex			= "GlobalVar";
			}
			else if (_Lst.size() > 2)									{
				_DataType = _Lst[0].toInt() + 1;
				if (MiddleParam::MapMiddle().size() >= (_DataType - 1))	{
					_Params				= &MiddleParam::MapMiddle()[_DataType - 1];
					m_strModelIndex		= QString::number(_DataType - 1);
				}
			}
		}
	}
	if (_Lst.size() > 2 && _Params != nullptr)							{
		if (_Lst[1] == "Bool")											{
			m_strTypeVariable = "Bool";
			if (_Params->MapBool.count(_Lst[2]) > 0)					{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
		else if (_Lst[1] == "Int")										{
			m_strTypeVariable			= "Int";
			if (_Params->MapInt.count(_Lst[2]) > 0)						{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
		else if (_Lst[1] == "Double")									{
			m_strTypeVariable			= "Double";
			if (_Params->MapDou.count(_Lst[2]) > 0)						{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
		else if (_Lst[1] == "QString")									{
			m_strTypeVariable			= "QString";
			if (_Params->MapStr.count(_Lst[2]) > 0)						{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
		else if (_Lst[1] == "QPoint")									{
			m_strTypeVariable			= "QPoint";
			if (_Params->MapPoint.count(_Lst[2]) > 0)					{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
		else if (_Lst[1] == "QPointF")									{
			m_strTypeVariable			= "QPointF";
			if (_Params->MapPointF.count(_Lst[2]) > 0)					{
				_bFind					= true;
				m_strValueName			= _Lst[2];
			}
		}
	}
}

void frmAllVariableTool::SetCurrentVariable(QString strModel, QString TypeVariable, QString strValueName)
{
	m_strModelIndex		= strModel;
	m_strTypeVariable	= TypeVariable;
	m_strValueName		= strValueName;
}

void frmAllVariableTool::GetCurrentVariable(QString & strModel, QString & TypeVariable, QString & strValueName)
{
	strModel			= m_strModelIndex;
	TypeVariable		= m_strTypeVariable;
	strValueName		= m_strValueName;
}

int frmAllVariableTool::Load()
{
	int count = ui->tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui->tableWidget->removeRow(0);  //设置行数
	int	_iCurrentModelRow = -1;
	if (m_strLimitModel == "" || m_strLimitModel == "GlobalVar")		{
		//GlobalVar
		count = ui->tableModelWidget->rowCount();
		ui->tableModelWidget->setRowCount(count + 1);  //设置行数
		QTableWidgetItem* item_type = new QTableWidgetItem(tr("GlobalVar"));
		item_type->setTextAlignment(Qt::AlignCenter);
		item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		item_type->setData(1, "GlobalVar");
		item_type->setText(tr("GlobalVar"));
		ui->tableModelWidget->setItem(count, 0, item_type);
		if (m_strModelIndex == "GlobalVar")	_iCurrentModelRow = count;
	}
	for (auto iter : (*MiddleParam::Instance()).MapMiddle())							{
		if (m_strLimitModel == "" || m_strLimitModel == QString::number(iter.m_iIndex))	{
			count = ui->tableModelWidget->rowCount();
			ui->tableModelWidget->setRowCount(count + 1);  //设置行数
			QTableWidgetItem* item_value = new QTableWidgetItem(QString::number(iter.m_iIndex));
			item_value->setTextAlignment(Qt::AlignCenter);
			item_value->setData(1, QString::number(iter.m_iIndex));
			item_value->setText(	QString::number(iter.m_iIndex)	);
			item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableModelWidget->setItem(count, 0, item_value);
			if (m_strModelIndex == QString::number(iter.m_iIndex))	_iCurrentModelRow = count;
		}
	}
	if (_iCurrentModelRow >= 0)	{	ui->tableModelWidget->selectRow(	_iCurrentModelRow	);	}
	else ui->tableModelWidget->selectRow(0);
	m_iCurrentIndex = -1;
	if (_iCurrentModelRow >= 0) { slot_DoubleClicked(_iCurrentModelRow,0); }
	else slot_DoubleClicked(0, 0);
	if (m_iCurrentIndex >= 0)	{	ui->tableWidget->selectRow(m_iCurrentIndex);	}
	else ui->tableWidget->selectRow(0);

	m_iRetn = 0;

	return 0;
}

int frmAllVariableTool::Save()
{

	return 0;
}

int frmAllVariableTool::SetData(QJsonArray & strData, MiddleParam& Param)
{

	return 0;
}

int frmAllVariableTool::NewProject()
{
	int count = ui->tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui->tableWidget->removeRow(0);  //设置行数

	count = ui->tableModelWidget->rowCount();
	for (int i = 0; i < count; i++)	ui->tableModelWidget->removeRow(0);  //设置行数
	return 0;
}

void frmAllVariableTool::initTitleBar()
{
	//m_titleBar = new MyTitleBar(this);
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/appbar.network.port.png");
	m_titleBar->setButtonType(MIN_MAX_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

void frmAllVariableTool::resizeEvent(QResizeEvent * ev)
{
	QWidget::resizeEvent(ev);
	int count = ui->tableWidget->columnCount();
	double iWidth = ui->tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++) {
		ui->tableWidget->setColumnWidth(i, iWidth);
	}

}

void frmAllVariableTool::onButtonCloseClicked()
{
	try
	{
		this->close();
	}
	catch (std::exception& ex)
	{
		qDebug() << ex.what();
	}
}

QString frmAllVariableTool::slot_SetGlobalValue(const QString strVar, const QString value, const int flowIndex)
{
	//加锁
	QMutexLocker locker(&mutex);
	return QString();
}

QString frmAllVariableTool::slot_GlobalValue(const QString strVar, const QString value, const int flowIndex)
{
	return QString();
}

void frmAllVariableTool::ClearModelTable()
{
	int count = ui->tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui->tableWidget->removeRow(0);  //设置行数
}

void frmAllVariableTool::on_btnAdd_clicked()
{

}

void frmAllVariableTool::slot_DoubleClicked(int row, int column)
{
	int count = ui->tableWidget->rowCount();
	for (int i = 0; i < count; i++)	ui->tableWidget->removeRow(0);  //设置行数
	if (ui->tableModelWidget->item(row, 0) == nullptr)	return;
	QString	strItem = ui->tableModelWidget->item(row,0)->data(1).toString();

	if (strItem == "GlobalVar" )	{
		set_Update_tableWidget(*MiddleParam::Instance());
	}
	else {
		int	_iRow	= strItem.toInt();
		if (MiddleParam::MapMiddle().size() > _iRow)	set_Update_tableWidget(MiddleParam::MapMiddle()[_iRow]);
	}
}

void frmAllVariableTool::slot_VariableDoubleClicked(int row, int column)
{
	if (ui->tableModelWidget->item(ui->tableModelWidget->currentRow(), 0) == nullptr)	return;
	m_strModelIndex		= ui->tableModelWidget->item(ui->tableModelWidget->currentRow(), 0)->data(1).toString();
	m_strValueName		= ui->tableWidget->item(row, 1)->text();
	m_strTypeVariable	= ui->tableWidget->item(row, 0)->text();
	m_iRetn = 2;
	onButtonCloseClicked();
}

void frmAllVariableTool::on_btnDelete_clicked()
{
	int rowIndex = ui->tableWidget->currentRow();
	if (rowIndex != -1)
	{
		ui->tableWidget->removeRow(rowIndex);
	}
}

void frmAllVariableTool::on_btnMoveUp_clicked()
{
	int nRow = ui->tableWidget->currentRow();
	moveRow(ui->tableWidget, nRow, nRow - 1);
}

void frmAllVariableTool::on_btnMoveDown_clicked()
{
	int nRow = ui->tableWidget->currentRow();
	moveRow(ui->tableWidget, nRow, nRow + 2);
}

void frmAllVariableTool::set_Update_tableWidget(MiddleParam & param)
{
	m_iCurrentIndex = -1;
	int count = ui->tableWidget->rowCount();
	for (int i = 0; i < count; i++)	ui->tableWidget->removeRow(0);  //设置行数
	QWidget* _pitem = nullptr;

	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Bool) == EnumLimitType_Bool)
	{
		for (auto iter : param.MapBool)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("Bool");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			_pitem = new QComboBox();
			((QComboBox*)_pitem)->addItem(tr("false"));
			((QComboBox*)_pitem)->addItem(tr("true"));

			((QComboBox*)_pitem)->setCurrentIndex(0);
			((QComboBox*)_pitem)->setEnabled(false);
			if (iter.second)	((QComboBox*)_pitem)->setCurrentIndex(1);
			if (_pitem != nullptr)	ui->tableWidget->setCellWidget(count, 2, _pitem);

			QTableWidgetItem* item_remark = new QTableWidgetItem(tr("false为假，true为真"));
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
			
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Int) == EnumLimitType_Int)
	{
		_pitem = nullptr;
		for (auto iter : param.MapInt)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("Int");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			_pitem = new QSpinBox();
			((QSpinBox*)_pitem)->setMinimum(-999999);
			((QSpinBox*)_pitem)->setMaximum(999999);
			((QSpinBox*)_pitem)->setValue(iter.second);
			((QSpinBox*)_pitem)->setEnabled(false);
			((QSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
			if (_pitem != nullptr)	ui->tableWidget->setCellWidget(count, 2, _pitem);

			QTableWidgetItem* item_remark = new QTableWidgetItem("");
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Dou) == EnumLimitType_Dou)
	{
		_pitem = nullptr;
		for (auto iter : param.MapDou)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("Double");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			_pitem = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pitem)->setMinimum(-999999);
			((QDoubleSpinBox*)_pitem)->setMaximum(999999);
			((QDoubleSpinBox*)_pitem)->setValue(iter.second);
			((QDoubleSpinBox*)_pitem)->setEnabled(false);
			((QDoubleSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
			if (_pitem != nullptr)	ui->tableWidget->setCellWidget(count, 2, _pitem);

			QTableWidgetItem* item_remark = new QTableWidgetItem("");
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Str) == EnumLimitType_Str)
	{
		_pitem = nullptr;
		for (auto iter : param.MapStr)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("QString");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			QTableWidgetItem* item_Text = new QTableWidgetItem(iter.second);
			item_Text->setTextAlignment(Qt::AlignCenter);
			item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 2, item_Text);

			QTableWidgetItem* item_remark = new QTableWidgetItem("");
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Point) == EnumLimitType_Point)
	{
		_pitem = nullptr;
		for (auto iter : param.MapPoint)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("QPoint");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			QTableWidgetItem* item_Text = new QTableWidgetItem("");
			item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_Text->setTextAlignment(Qt::AlignCenter);
			item_Text->setText(QString("(%1,%2)").arg(QString::number(iter.second.x())).arg(QString::number(iter.second.y())));
			ui->tableWidget->setItem(count, 2, item_Text);

			QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_PointF) == EnumLimitType_PointF)
	{
		_pitem = nullptr;
		for (auto iter : param.MapPointF)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("QPointF");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			QTableWidgetItem* item_Text = new QTableWidgetItem("");
			item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_Text->setTextAlignment(Qt::AlignCenter);
			item_Text->setText(QString("(%1,%2)").arg(QString::number(iter.second.x())).arg(QString::number(iter.second.y())));
			ui->tableWidget->setItem(count, 2, item_Text);

			QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_ResultPointF) == EnumLimitType_ResultPointF)
	{
		_pitem = nullptr;
		for (auto iter : param.MapCrossPointF)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("sResultCross");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			QTableWidgetItem* item_Text = new QTableWidgetItem("");
			item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_Text->setTextAlignment(Qt::AlignCenter);
			item_Text->setText(QString("(%1,%2,%3)").arg(QString::number(iter.second.LineCenterX)).arg(QString::number(iter.second.LineCenterY)).arg(QString::number(iter.second.LineRad / M_PI * 180.0 )));
			ui->tableWidget->setItem(count, 2, item_Text);

			QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含()"));
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
	if (m_eLimitType == EnumLimitType_NotAll || (m_eLimitType & EnumLimitType_Metrix) == EnumLimitType_Metrix)
	{
		_pitem = nullptr;
		for (auto iter : param.MapPtrMetrix)
		{
			int count = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(count + 1);  //设置行数

			QTableWidgetItem* item_type = new QTableWidgetItem("Metrix");
			item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_type->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 0, item_type);

			QTableWidgetItem* item_name = new QTableWidgetItem(iter.first);
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui->tableWidget->setItem(count, 1, item_name);

			QTableWidgetItem* item_Text = new QTableWidgetItem("");
			item_Text->setFlags(item_Text->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_Text->setTextAlignment(Qt::AlignCenter);
			item_Text->setText(QString("Metrix"));
			ui->tableWidget->setItem(count, 2, item_Text);

			QTableWidgetItem* item_remark = new QTableWidgetItem("");
			item_remark->setFlags(item_remark->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			item_remark->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(count, 3, item_remark);

			if (m_strValueName == iter.first)	m_iCurrentIndex = count;
		}
	}
}

void frmAllVariableTool::moveRow(QTableWidget* pTable, int nFrom, int nTo)
{
	if (pTable == NULL) return;
	pTable->setFocus();
	if (nFrom == nTo) return;
	if (nFrom < 0 || nTo < 0) return;
	int nRowCount = pTable->rowCount();
	if (nFrom >= nRowCount || nTo > nRowCount) return;
	if (nTo < nFrom) nFrom++; pTable->insertRow(nTo);
	int nCol = pTable->columnCount();
	for (int i = 0; i < nCol; i++)
	{
		if (pTable->item(nFrom, i) == nullptr)
		{
			pTable->setCellWidget(nTo, i, pTable->cellWidget(nFrom, i));
		}
		else
		{
			pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
		}
	}
	if (nFrom < nTo)
	{
		nTo--;
	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}
