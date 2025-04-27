#include "frmVariableTool.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QPainter>
#include "databaseVar.h"
#include "gvariable.h"
#include "qmutex.h"
#include <QThread>
#include <qdebug.h>
#include <QApplication>
#include <QAbstractItemView>
#include <QComboBox>

frmVariableTool::frmVariableTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Var.png"));

	//初始化标题栏
	initTitleBar();
	initConnect();
	//设置表格列宽	
	ui.tableWidget->setColumnWidth(0, 120);
	ui.tableWidget->setColumnWidth(1, 200);
	ui.tableWidget->setColumnWidth(2, 280);
	ui.tableWidget->setColumnWidth(3, 240);

	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	connect(this, &frmVariableTool::sig_GlobalValue, this, &frmVariableTool::slot_GlobalValue, Qt::BlockingQueuedConnection);
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数

	//隐藏水平header
	ui.tableModelWidget->setVisible(false);
	ui.tableModelWidget->verticalHeader()->setVisible(false);
	ui.tableModelWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableModelWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	count = ui.tableModelWidget->rowCount();
	for (int i = 0; i < count; i++)	ui.tableModelWidget->removeRow(0);  //设置行数
}

frmVariableTool::~frmVariableTool()
{
	if (m_titleBar != nullptr)	delete	m_titleBar; m_titleBar = nullptr;
	this->deleteLater();
}

int frmVariableTool::Load()
{
	m_titleBar->setTitleContent(((VariableTool*)m_pTool)-> GetItemId());

	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数

	QJsonArray CameraListInfo	= ((VariableTool*)m_pTool)->m_Variable;

	for (size_t irow = 0; irow < CameraListInfo.count(); irow++)
	{
		QJsonArray object_value = CameraListInfo.at(irow).toArray();

		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QWidget*			_pQWidget = nullptr;
		QTableWidgetItem*	item_value = nullptr;
		QString				strName = "";
		for (size_t icol = 0; icol < object_value.size(); icol++)
		{
			QString str = object_value.at(icol).toString();
			if (icol == 0)
			{
				strName = str;
			}
			if (icol == 2)
			{
				if (strName == "Int")
				{
					_pQWidget = new QSpinBox();
					((QSpinBox*)_pQWidget)->setValue(str.toInt());
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					item_value = nullptr;
				}
				else if (strName == "Double")
				{
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setValue(str.toDouble());
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					item_value = nullptr;
				}
				else if (strName == "QString")
				{
					_pQWidget = nullptr;
					item_value = new QTableWidgetItem(str);
				}
				else if (strName == "Bool")
				{
					item_value = nullptr;
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem("false");
					((QComboBox*)_pQWidget)->addItem("true");
					((QComboBox*)_pQWidget)->setCurrentIndex(str.toInt());

					QTableWidgetItem* item_remark = new QTableWidgetItem(tr("false为假，true为真"));
					item_remark->setTextAlignment(Qt::AlignCenter);
					ui.tableWidget->setItem(count, 3, item_remark);
				}
				else if (strName == "QPoint")
				{
					_pQWidget = nullptr;
					item_value = new QTableWidgetItem(str);
					QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
					item_remark->setTextAlignment(Qt::AlignCenter);
					ui.tableWidget->setItem(count, 3, item_remark);
				}
				else if (strName == "QPointF")
				{
					_pQWidget = nullptr;
					item_value = new QTableWidgetItem(str);
					QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
					item_remark->setTextAlignment(Qt::AlignCenter);
					ui.tableWidget->setItem(count, 3, item_remark);
				}
				else
				{
					_pQWidget = nullptr;
					item_value = new QTableWidgetItem();
				}

				if (_pQWidget != nullptr)
				{
					ui.tableWidget->setCellWidget(count, 2, _pQWidget);
				}
			}
			else
			{
				item_value = new QTableWidgetItem();
				if (icol == 0 || icol == 1)	item_value->setFlags(item_value->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
				item_value->setTextAlignment(Qt::AlignCenter);
				item_value->setText(str);
			}
			if (item_value != nullptr)
			{
				item_value->setTextAlignment(Qt::AlignCenter);
				ui.tableWidget->setItem(irow, icol, item_value);
			}
		}
	}

	return 0;
}

int frmVariableTool::Save()
{
	QJsonArray IoArray;
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		QJsonArray __IoArray;
		for (int j = 0; j < ui.tableWidget->columnCount(); j++)
		{
			if (ui.tableWidget->item(i, j) != nullptr)
				__IoArray.append(ui.tableWidget->item(i, j)->text());
			else if (ui.tableWidget->cellWidget(i, j) != nullptr)
			{
				if (ui.tableWidget->item(i, 0)->text() == "Int")
				{
					QSpinBox* _pBox = ((QSpinBox*)ui.tableWidget->cellWidget(i, 2));
					__IoArray.append(QString::number(_pBox->value()));
				}
				else if (ui.tableWidget->item(i, 0)->text() == "Double")
				{
					QDoubleSpinBox* _pBox = ((QDoubleSpinBox*)ui.tableWidget->cellWidget(i, 2));
					__IoArray.append(QString::number(_pBox->value()));
				}
				else if (ui.tableWidget->item(i, 0)->text() == "Bool")
				{
					QComboBox* _pBox = ((QComboBox*)ui.tableWidget->cellWidget(i, 2));
					__IoArray.append(QString::number(_pBox->currentIndex()));
				}
				else
				{
					__IoArray.append("");
				}
			}
			else
			{
				__IoArray.append("");
			}
		}
		IoArray.append(__IoArray);
	}
	((VariableTool*)m_pTool)->m_Variable = IoArray;
	return 0;
}

EnumNodeResult frmVariableTool::Execute(MiddleParam & param, QString & strError)
{
	return NodeResult_Idle;
}

int frmVariableTool::SetData(QJsonArray & strData, MiddleParam& Param)
{
	for (size_t irow = 0; irow < strData.count(); irow++)
	{
		QJsonArray object_value = strData.at(irow).toArray();

		QString	strType = "";
		QString	strName = "";
		for (size_t icol = 0; icol < object_value.size(); icol++)
		{
			QString str = object_value.at(icol).toString();
			if (icol == 0)
			{
				strType = str;
			}
			else if (icol == 1)
			{
				strName = str;
			}
			else if (icol == 2)
			{
				if (strType == "Int")
				{
					Param.MapInt[strName] = str.toInt();
				}
				else if (strType == "Double")
				{
					Param.MapDou[strName] = str.toDouble();
				}
				else if (strType == "QString")
				{
					Param.MapStr[strName] = str;
				}
				else if (strType == "Bool")
				{
					Param.MapBool[strName] = str.toInt();
				}
				else if (strType == "QPoint")
				{
					QStringList list = str.split(QRegExp("[(,)]"));
					if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
					{
						//databaseVar::Instance().m_NotifyManager->notify(tr("QPointF输入条件错误！"), Notify_Error);

						return NodeResult_Error;
					}
					QPoint qpointf;
					qpointf.setX(list[1].toFloat());
					qpointf.setY(list[2].toFloat());
					Param.MapPoint[strName] = qpointf;
				}
				else if (strType == "QPointF")
				{
					QStringList list = str.split(QRegExp("[(,)]"));
					if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
					{
						//databaseVar::Instance().m_NotifyManager->notify(tr("QPointF输入条件错误！"), Notify_Error);
						return NodeResult_Error;
					}
					QPointF qpointf;
					qpointf.setX(list[1].toFloat());
					qpointf.setY(list[2].toFloat());
					Param.MapPointF[strName] = qpointf;
				}

			}
		}
	}
	return 0;
}

int frmVariableTool::NewProject()
{
	int count = ui.tableWidget->rowCount();
	for (int i = 0; i < count; i++)		ui.tableWidget->removeRow(0);  //设置行数

	count = ui.tableModelWidget->rowCount();
	for (int i = 0; i < count; i++)	ui.tableModelWidget->removeRow(0);  //设置行数
	return 0;
}


void frmVariableTool::showEvent(QShowEvent * ev)
{
	frmBaseTool::showEvent(ev);
	ResizeUI();
	QTimer::singleShot(100, this, [this]() {	ResizeUI();	});
}

void frmVariableTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);

	ResizeUI();
}

void frmVariableTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Var.png");

}

void frmVariableTool::initConnect()
{	
	connect(ui.btnAdd,					&QPushButton::pressed,								this,&frmVariableTool::on_btnAdd_clicked);
	connect(ui.btnSub,				&QPushButton::pressed,								this,&frmVariableTool::on_btnSub_clicked);
	connect(ui.btnMoveUp,				&QPushButton::pressed,								this,&frmVariableTool::on_btnMoveUp_clicked);
	connect(ui.btnMoveDown,				&QPushButton::pressed,								this,&frmVariableTool::on_btnMoveDown_clicked);
	
}

QString frmVariableTool::slot_GlobalValue(const QString strVar, const QString value, const int flowIndex)
{
	return QString();
}

void frmVariableTool::on_btnAdd_clicked()
{
	int count = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(count + 1);  //设置行数
	QVector<int> vecLst;
	for (int iIndex = 0; iIndex < count; iIndex++) {
		QString	strName = ui.tableWidget->item(iIndex, 1)->text();
		if (strName.isEmpty())	continue;
		QString	strNum = strName.mid(3);
		vecLst.push_back(strNum.toInt());
	}

	std::sort(vecLst.begin(), vecLst.end());     //去重前需要排序
	int	_iIndex = vecLst.size() + 1;
	for (int i = 0; i < vecLst.size(); i++)
		if (vecLst[i] != (i + 1)) {
			_iIndex = (i + 1);
			break;
		}

	QTableWidgetItem* item_type = new QTableWidgetItem(ui.comboType->currentText());
	item_type->setFlags(item_type->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
	item_type->setTextAlignment(Qt::AlignCenter);
	ui.tableWidget->setItem(count, 0, item_type);
	QTableWidgetItem* item_name = new QTableWidgetItem("Var" + QString::number(_iIndex));
	item_name->setTextAlignment(Qt::AlignCenter);
	item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
	ui.tableWidget->setItem(count, 1, item_name);
	QTableWidgetItem* item_value = nullptr;
	QWidget* _pitem = nullptr;
	if (ui.comboType->currentText() == "Int")
	{
		_pitem = new QSpinBox();
		((QSpinBox*)_pitem)->setValue(0);
		((QSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
	}
	else if (ui.comboType->currentText() == "Double")
	{
		_pitem = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pitem)->setValue(0);
		((QDoubleSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
	}
	else if (ui.comboType->currentText() == "QString")
	{
		item_value = new QTableWidgetItem(QString());
		item_value->setTextAlignment(Qt::AlignCenter);
	}
	else if (ui.comboType->currentText() == "Bool")
	{
		_pitem = new QComboBox();
		((QComboBox*)_pitem)->addItem("false");
		((QComboBox*)_pitem)->addItem("true");

		QTableWidgetItem* item_remark = new QTableWidgetItem(tr("false为假，true为真"));
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	else if (ui.comboType->currentText() == "QPoint")
	{
		item_value = new QTableWidgetItem("(0,0)");
		QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	else if (ui.comboType->currentText() == "QPointF")
	{
		item_value = new QTableWidgetItem("(0.0000,0.0000)");
		QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
		item_remark->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 3, item_remark);
	}
	if (item_value != nullptr)
	{
		item_value->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget->setItem(count, 2, item_value);
	}
	if(_pitem != nullptr)	ui.tableWidget->setCellWidget(count,2, _pitem);
}

void frmVariableTool::on_btnSub_clicked()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex != -1)
	{
		ui.tableWidget->removeRow(rowIndex);
	}
}

void frmVariableTool::on_btnMoveUp_clicked()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmVariableTool::on_btnMoveDown_clicked()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmVariableTool::moveRow(QTableWidget* pTable, int nFrom, int nTo)
{
	if (pTable == NULL) return;
	pTable->setFocus();
	if (nFrom == nTo) return;
	if (nFrom < 0 || nTo < 0) return;
	int nRowCount = pTable->rowCount();
	if (nFrom >= nRowCount || nTo > nRowCount) return;
	if (nTo < nFrom) nFrom++; pTable->insertRow(nTo);
	int nCol = pTable->columnCount();
	for (int i = 0; i < nCol; i++)	{
		if (pTable->item(nFrom, i) == nullptr)		{
			pTable->setCellWidget(nTo, i, pTable->cellWidget(nFrom, i));
		}
		else	{
			pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
		}
	}
	if (nFrom < nTo)
	{
		nTo--;
	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}

void frmVariableTool::ResizeUI()
{
	int count = ui.tableWidget->columnCount();
	int	iWidth = ui.tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++) {
		ui.tableWidget->setColumnWidth(i, iWidth);
	}
}
