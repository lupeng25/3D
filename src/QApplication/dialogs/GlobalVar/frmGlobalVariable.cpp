#include "frmGlobalVariable.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDoubleSpinBox>
#include <QDoubleSpinBox>
#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QPainter>
#include "Data/dataVar.h"
#include "gvariable.h"
#include "qmutex.h"
#include <QThread>
#include <qdebug.h>

frmGlobalVariable::frmGlobalVariable(QWidget* parent)
	: frmBase(parent)
{
	ui.setupUi(this);
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/GlobalVar.png"));
	//初始化标题栏
	initTitleBar();
	//h1 = QThread::currentThread();
	//设置表格列宽	
	ui.tableWidget->setColumnWidth(0, 120);
	ui.tableWidget->setColumnWidth(1, 200);
	ui.tableWidget->setColumnWidth(2, 273);
	ui.tableWidget->setColumnWidth(3, 237);
	//隐藏水平header
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	connect(this, &frmGlobalVariable::sig_GlobalValue, this, &frmGlobalVariable::slot_GlobalValue, Qt::BlockingQueuedConnection);
	ReadOnlyDelegate* readOnlyDelegate = new ReadOnlyDelegate(this);
	ui.tableWidget->setItemDelegateForColumn(0, readOnlyDelegate); //设置第1列只读	
}

frmGlobalVariable::~frmGlobalVariable()
{
	if (m_titleBar != nullptr)	delete	m_titleBar; m_titleBar = nullptr;
	this->deleteLater();
}

int frmGlobalVariable::GetData(QJsonObject & strData)
{
	{	//Instrumentation
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
		strData.insert("GlobalVariable", IoArray);
	}
	return 0;
}

int frmGlobalVariable::SetData(QJsonObject & strData)
{
	int count = ui.tableWidget->rowCount();
	for( int i = 0; i < count;i++)
		ui.tableWidget->removeRow(0);  //设置行数
	QJsonArray CameraListInfo = strData.find("GlobalVariable").value().toArray();
	for (size_t irow = 0; irow < CameraListInfo.count(); irow++)
	{
		QJsonArray object_value = CameraListInfo.at(irow).toArray();
		//QString	_strData = QString(strData.);

		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QWidget* _pQWidget				= nullptr;
		QTableWidgetItem* item_value	= nullptr;
		QString		strName = "";
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
					((QSpinBox*)_pQWidget)->setMinimum(-999999);
					((QSpinBox*)_pQWidget)->setMaximum(999999);
					((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QSpinBox*)_pQWidget)->setValue(str.toInt());
					item_value = nullptr;
				}
				else if (strName == "Double")
				{
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setValue(str.toDouble());
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
					_pQWidget	= nullptr;
					item_value	= new QTableWidgetItem(str);
					QTableWidgetItem* item_remark = new QTableWidgetItem(tr("必须包含( )"));
					item_remark->setTextAlignment(Qt::AlignCenter);
					ui.tableWidget->setItem(count, 3, item_remark);
				}
				else
				{
					_pQWidget	= nullptr;
					item_value	= new QTableWidgetItem();
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

	UpdateGlobalVar();

	return 0;
}

int frmGlobalVariable::NewProject()
{
	return 0;
}

void frmGlobalVariable::UpdateGlobalVar()
{
	MiddleParam		_MiddleParam;
	int row_count = ui.tableWidget->rowCount();
	for (int i = 0; i < row_count; i++)
	{
		if (ui.tableWidget->item(i, 0)->text() == "Int")
		{
			QSpinBox* _pBox		= ((QSpinBox*)ui.tableWidget->cellWidget(i, 2));
			QString	strName		= ui.tableWidget->item(i, 1)->text();
			int	iValue			= /*ui.tableWidget->item(i, 2)->text().toInt()*/_pBox->value();
			_MiddleParam.MapInt[strName] = iValue;
		}
		else if (ui.tableWidget->item(i, 0)->text() == "Double")
		{
			QDoubleSpinBox* _pBox	= ((QDoubleSpinBox*)ui.tableWidget->cellWidget(i, 2));
			QString	strName			= ui.tableWidget->item(i, 1)->text();
			double	dValue			= /*ui.tableWidget->item(i, 2)->text().toDouble()*/_pBox->value();
			_MiddleParam.MapDou[strName] = dValue;
		}
		else if (ui.tableWidget->item(i, 0)->text() == "QString")
		{
			if (ui.tableWidget->item(i, 1)->text().contains(QRegExp("[.]")) == true)	{
				dataVar::Instance().pManager->notify(tr("不允许输入“.”符号！"), NotifyType_Error);
				return;
			}

			QString	strName			= ui.tableWidget->item(i, 1)->text();
			QString	strValue		= ui.tableWidget->item(i, 2)->text();
			_MiddleParam.MapStr[strName] = strValue;
		}
		else if (ui.tableWidget->item(i, 0)->text() == "Bool")
		{
			QComboBox* _pBox		= ((QComboBox*)ui.tableWidget->cellWidget(i, 2));
			bool state				= _pBox->currentIndex();
			QString	strName			= ui.tableWidget->item(i, 1)->text();
			_MiddleParam.MapBool[strName] = state;
		}
		else if (ui.tableWidget->item(i, 0)->text() == "QPoint")
		{
			if (ui.tableWidget->item(i, 1)->text().contains(QRegExp("[.]")) == true)
			{
				dataVar::Instance().pManager->notify(tr("不允许输入“.”符号！"), NotifyType_Error);
				//QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "不允许输入“.”符号！");
				//msgBox.setWindowIcon(QIcon(NotifyType_Error));
				//msgBox.exec();
				return;
			}
			QStringList list = ui.tableWidget->item(i, 2)->text().split(QRegExp("[(,)]"));
			if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
			{
				dataVar::Instance().pManager->notify(tr("QPointF输入条件错误！"), NotifyType_Error);
				//QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "QPointF输入条件错误！");
				//msgBox.setWindowIcon(QIcon(NotifyType_Error));
				//msgBox.exec();
				return;
			}
			QPoint qpointf;
			qpointf.setX(list[1].toFloat());
			qpointf.setY(list[2].toFloat());
			QString	strName = ui.tableWidget->item(i, 1)->text();
			_MiddleParam.MapPoint[strName] = qpointf;
		}
		else if (ui.tableWidget->item(i, 0)->text() == "QPointF")
		{
			if (ui.tableWidget->item(i, 1)->text().contains(QRegExp("[.]")) == true)
			{
				dataVar::Instance().pManager->notify(tr("不允许输入“.”符号！"), NotifyType_Error);
				//QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "不允许输入“.”符号！");
				//msgBox.setWindowIcon(QIcon(NotifyType_Error));
				//msgBox.exec();
				return;
			}
			QStringList list = ui.tableWidget->item(i, 2)->text().split(QRegExp("[(,)]"));
			if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
			{
				dataVar::Instance().pManager->notify(tr("QPointF输入条件错误！"), NotifyType_Error);

				//QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", "QPointF输入条件错误！");
				//msgBox.setWindowIcon(QIcon(NotifyType_Error));
				//msgBox.exec();
				return;
			}
			QPointF qpointf;
			qpointf.setX(list[1].toFloat());
			qpointf.setY(list[2].toFloat());
			QString	strName		= ui.tableWidget->item(i, 1)->text();
			_MiddleParam.MapPointF[strName] = qpointf;
		}
	}

	(*MiddleParam::Instance()).MapInt.clear();
	(*MiddleParam::Instance()).MapDou.clear();
	(*MiddleParam::Instance()).MapStr.clear();
	(*MiddleParam::Instance()).MapBool.clear();
	(*MiddleParam::Instance()).MapPoint.clear();
	(*MiddleParam::Instance()).MapPointF.clear();
	_MiddleParam.CloneTo((*MiddleParam::Instance()));
	//(*MiddleParam::Instance()) = _MiddleParam;

}


void frmGlobalVariable::changeEvent(QEvent * ev)
{
	frmBase::changeEvent(ev);
	ui.retranslateUi(this);
}

void frmGlobalVariable::resizeEvent(QResizeEvent * ev)
{
	frmBase::resizeEvent(ev);
	int count = ui.tableWidget->columnCount();
	double iWidth = ui.tableWidget->width() / (double)count;
	for (int i = 0; i < count; i++) {
		ui.tableWidget->setColumnWidth(i, iWidth);
	}
}

void frmGlobalVariable::showEvent(QShowEvent * ev)
{
	frmBase::showEvent(ev);
	emit m_titleBar->getRestoreButton()->clicked();
}

void frmGlobalVariable::initTitleBar()
{
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/GlobalVar.png");
	m_titleBar->setButtonType(MIN_MAX_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

void frmGlobalVariable::onButtonCloseClicked()
{
	try
	{
		UpdateGlobalVar();

		this->close();
	}
	catch (std::exception& ex)
	{
		qDebug() << ex.what();

		dataVar::Instance().pManager->notify(ex.what(), NotifyType_Error);

		//QMessageBox msgBox(QMessageBox::Icon::NoIcon, "错误", ex.what());
		//msgBox.setWindowIcon(QIcon(NotifyType_Error));
		//msgBox.exec();
	}
}

QString frmGlobalVariable::slot_SetGlobalValue(const QString strVar, const QString value, const int flowIndex)
{
	//加锁
	QMutexLocker locker(&mutex);
	//h2 = QThread::currentThread();
	//if (h1 != h2)
	//{
	//	QString result_msg = emit sig_GlobalValue(strVar, value, flowIndex);
	//	if (result_msg == "p_error")
	//	{
	//		return result_msg;
	//	}
	//}
	//else
	//{
	//	//获取GlobalVar值
	//	QList<QString> global_keys;
	//	global_keys.reserve(300);
	//	global_keys.clear();
	//	for (map<QString, gVariable::Global_Var>::iterator it = gVariable::Instance().global_variable_link.begin(); 
	//		it != gVariable::Instance().global_variable_link.end(); ++it) {	global_keys.push_back(it->first);	}
	//	for (int k = 0; k < global_keys.length(); k++)
	//	{
	//		QString key = global_keys[k];
	//		if (gVariable::Instance().global_variable_link.at(key).global_type == "Int")
	//		{
	//			if (strVar == key)
	//			{
	//				if (value.contains(QRegExp("[0-9]+$")) == false || value.contains(QRegExp("[a-zA-Z{()}]")) == true)
	//				{
	//					//emit dataVar::fProItemTab->sig_ErrorClick();
	//					//emit dataVar::fProItemTab->sig_Log("Int类型只能输入数字！");
	//					return "p_error";
	//				}
	//				gVariable::Instance().GlobalVar.global_type = "Int";
	//				gVariable::Instance().GlobalVar.global_int_value = value.toInt();
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "Double")
	//		{
	//			if (strVar == key)
	//			{
	//				if (value.contains(QRegExp("[0-9]+$")) == false || value.contains(QRegExp("[a-zA-Z{()}]")) == true)
	//				{
	//					////   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   //   emit dataVar::fProItemTab->sig_Log("Double类型只能输入数字！");
	//					return "p_error";
	//				}
	//				gVariable::Instance().GlobalVar.global_type = "Double";
	//				gVariable::Instance().GlobalVar.global_double_value = value.toDouble();
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "QString")
	//		{
	//			if (strVar == key)
	//			{
	//				gVariable::Instance().GlobalVar.global_type = "QString";
	//				gVariable::Instance().GlobalVar.global_qstring_value = value;
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "Bool")
	//		{
	//			if (strVar == key)
	//			{
	//				bool state;
	//				if (value == "false")
	//				{
	//					state = false;
	//				}
	//				else if (value == "true")
	//				{
	//					state = true;
	//				}
	//				else
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("Bool类型输入条件错误！");
	//					return "p_error";
	//				}
	//				gVariable::Instance().GlobalVar.global_type = "Bool";
	//				gVariable::Instance().GlobalVar.global_bool_value = state;
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "QPoint")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,)]"));
	//				if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("QPoint输入条件错误！");
	//					return "p_error";
	//				}
	//				QPoint qpoint;
	//				qpoint.setX(list[1].toInt());
	//				qpoint.setY(list[2].toInt());
	//				gVariable::Instance().GlobalVar.global_type = "QPoint";
	//				gVariable::Instance().GlobalVar.global_qpoint_value = qpoint;
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "QPointF")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,)]"));
	//				if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("QPointF输入条件错误！");
	//					return "p_error";
	//				}
	//				QPointF qpointf;
	//				qpointf.setX(list[1].toFloat());
	//				qpointf.setY(list[2].toFloat());
	//				gVariable::Instance().GlobalVar.global_type = "QPointF";
	//				gVariable::Instance().GlobalVar.global_qpointf_value = qpointf;
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "cv::Point")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,)]"));
	//				if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("cv::Point输入条件错误！");
	//					return "p_error";
	//				}
	//				//cv::Point cvpoint;
	//				//cvpoint.x = list[1].toInt();
	//				//cvpoint.y = list[2].toInt();
	//				//gVariable::Instance().GlobalVar.global_type = "cv::Point";
	//				//gVariable::Instance().GlobalVar.global_cvpoint_value = cvpoint;
	//				//gVariable::Instance().global_variable_link.insert(key, gVariable::Instance().GlobalVar);
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "cv::Point2f")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,)]"));
	//				if (list.size() != 4 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("cv::Point2f输入条件错误！");
	//					return "p_error";
	//				}
	//				//cv::Point2f cvpoint2f;
	//				//cvpoint2f.x = list[1].toFloat();
	//				//cvpoint2f.y = list[2].toFloat();
	//				//gVariable::Instance().GlobalVar.global_type = "cv::Point2f";
	//				//gVariable::Instance().GlobalVar.global_cvpoint2f_value = cvpoint2f;
	//				//gVariable::Instance().global_variable_link.insert(key, gVariable::Instance().GlobalVar);
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "cv::Point3f")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,,)]"));
	//				if (list.size() != 5 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true || list[4].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("cv::Point3f输入条件错误！");
	//					return "p_error";
	//				}
	//				//cv::Point3f cvpoint3f;
	//				//cvpoint3f.x = list[1].toFloat();
	//				//cvpoint3f.y = list[2].toFloat();
	//				//cvpoint3f.z = list[3].toFloat();
	//				//gVariable::Instance().GlobalVar.global_type = "cv::Point3f";
	//				//gVariable::Instance().GlobalVar.global_cvpoint3f_value = cvpoint3f;
	//				//gVariable::Instance().global_variable_link.insert(key, gVariable::Instance().GlobalVar);
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "cv::Point3d")
	//		{
	//			if (strVar == key)
	//			{
	//				QStringList list = value.split(QRegExp("[(,,)]"));
	//				if (list.size() != 5 || list[0].contains(QRegExp("[a-zA-Z]")) == true || list[1].contains(QRegExp("[a-zA-Z]")) == true || list[2].contains(QRegExp("[a-zA-Z]")) == true || list[3].contains(QRegExp("[a-zA-Z]")) == true || list[4].contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("cv::Point3d输入条件错误！");
	//					return "p_error";
	//				}
	//				//cv::Point3d cvpoint3d;
	//				//cvpoint3d.x = list[1].toDouble();
	//				//cvpoint3d.y = list[2].toDouble();
	//				//cvpoint3d.z = list[3].toDouble();
	//				//gVariable::Instance().GlobalVar.global_type = "cv::Point3d";
	//				//gVariable::Instance().GlobalVar.global_cvpoint3d_value = cvpoint3d;
	//				//gVariable::Instance().global_variable_link.insert(key, gVariable::Instance().GlobalVar);
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "Float[]")
	//		{
	//			if (strVar == key)
	//			{
	//				if (value.contains(QRegExp("[0-9{,}]+$")) == false || value.contains(QRegExp("[，a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("Float[]类型只能输入数字！");
	//					return "p_error";
	//				}
	//				gVariable::Instance().GlobalVar.global_type = "Float[]";
	//				gVariable::Instance().GlobalVar.global_array_float_value.reserve(1000);
	//				gVariable::Instance().GlobalVar.global_array_float_value.clear();
	//				if (value.mid(0, 1) != "{" || value.mid(value.length() - 1, 1) != "}")
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("Float[]类型未包含{}！");
	//					return "p_error";
	//				}
	//				QString s_float = value.mid(1, value.length() - 2);
	//				QStringList list_float = s_float.split(",", QString::SkipEmptyParts);
	//				for (int m = 0; m < list_float.count(); m++)
	//				{
	//					gVariable::Instance().GlobalVar.global_array_float_value.push_back(list_float[m].toFloat());
	//				}
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//		else if (gVariable::Instance().global_variable_link.at(key).global_type == "Double[]")
	//		{
	//			if (strVar == key)
	//			{
	//				if (value.contains(QRegExp("[0-9{,}]+$")) == false || value.contains(QRegExp("[a-zA-Z]")) == true)
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("Double[]类型只能输入数字！");
	//					return "p_error";
	//				}
	//				gVariable::Instance().GlobalVar.global_type = "Double[]";
	//				gVariable::Instance().GlobalVar.global_array_double_value.reserve(1000);
	//				gVariable::Instance().GlobalVar.global_array_double_value.clear();
	//				if (value.mid(0, 1) != "{" || value.mid(value.length() - 1, 1) != "}")
	//				{
	//					//   emit dataVar::fProItemTab->sig_ErrorClick();
	//					//   emit dataVar::fProItemTab->sig_Log("Double[]类型未包含{}！");
	//					return "p_error";
	//				}
	//				QString s_double = value.mid(1, value.length() - 2);
	//				QStringList list_double = s_double.split(",", QString::SkipEmptyParts);
	//				for (int m = 0; m < list_double.count(); m++)
	//				{
	//					gVariable::Instance().GlobalVar.global_array_double_value.push_back(list_double[m].toDouble());
	//				}
	//				gVariable::Instance().global_variable_link.insert(std::pair<QString, gVariable::Global_Var>(key, gVariable::Instance().GlobalVar));
	//			}
	//		}
	//	}
	//	////更新GlobalVar
	//	//QConfig::ToolBase[flowIndex]->RunAllToolLink();
	//	////更新列表
	//	//int row_count = ui.tableWidget->rowCount();
	//	//int intVarState = 0;
	//	//for (int i = 0; i < row_count; i++)
	//	//{
	//	//	if (ui.tableWidget->item(i, 1)->text() == strVar)
	//	//	{
	//	//		ui.tableWidget->item(i, 2)->setText(value);
	//	//		intVarState = 1;
	//	//	}
	//	//}
	//	//if (intVarState == 0)
	//	//{
	//	//	return "p_error";
	//	//}
	//}
	return QString();
}

QString frmGlobalVariable::slot_GlobalValue(const QString strVar, const QString value, const int flowIndex)
{
	return QString();
}

void frmGlobalVariable::on_btnAdd_clicked()
{
	int count = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(count + 1);  //设置行数
	QVector<int> vecLst;
	for (int iIndex = 0; iIndex < count; iIndex++) {
		QString	strName = ui.tableWidget->item(iIndex, 1)->text();
		if (strName.isEmpty())	continue;
		QString	strNum = strName.mid(4);
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

	QTableWidgetItem* item_name = new QTableWidgetItem("gVar" + QString::number(_iIndex));
	item_name->setTextAlignment(Qt::AlignCenter);
	item_name->setFlags(item_name->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	

	ui.tableWidget->setItem(count, 1, item_name);
	QTableWidgetItem* item_value = nullptr;
	QWidget* _pitem = nullptr;
	if (ui.comboType->currentText() == "Int")
	{
		_pitem = new QSpinBox();

		((QSpinBox*)_pitem)->setMinimum(-999999);
		((QSpinBox*)_pitem)->setMaximum(999999);
		((QSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
		((QSpinBox*)_pitem)->setValue(0);
	}
	else if (ui.comboType->currentText() == "Double")
	{
		_pitem = new QDoubleSpinBox();

		((QDoubleSpinBox*)_pitem)->setMinimum(-999999);
		((QDoubleSpinBox*)_pitem)->setMaximum(999999);
		((QDoubleSpinBox*)_pitem)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pitem)->setValue(0);
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

void frmGlobalVariable::on_btnSub_clicked()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex != -1)
	{
		ui.tableWidget->removeRow(rowIndex);
	}
}

void frmGlobalVariable::on_btnMoveUp_clicked()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmGlobalVariable::on_btnMoveDown_clicked()
{
	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmGlobalVariable::moveRow(QTableWidget* pTable, int nFrom, int nTo)
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
