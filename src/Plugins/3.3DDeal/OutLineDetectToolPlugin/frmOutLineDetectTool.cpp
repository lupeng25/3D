#include "frmOutLineDetectTool.h"
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
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "frmAllVariableTool.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "databaseVar.h"
#include "XxwCustomPlot.h"
#include <QtoolTip>

QMap<EnumPortCheckType, QString> frmOutLineDetectTool::PortCheckTypeMap{
	{ EnumTypePoint,					"Point"					},
	{ EnumTypePointPointCenter,			"PointPointCenter"		},
	{ EnumTypeLineCross,				"LineCross"				},
	{ EnumTypeLineContourCross,			"LineContourCross"		},
	{ EnumTypeLine,						"Line"					},
	{ EnumTypeHorizontalLine,			"HorizontalLine"		},
	{ EnumTypeLineLineCross,			"LineLineCross"			},
	{ EnumTypeCircle,					"Circle"				},
	{ EnumTypePointPointLine,			"PointPointLine"		},
	{ EnumTypeverticalLine,				"verticalLine"			},
	{ EnumTypepointStroke,				"pointStroke"			},
	{ EnumTypeLineCircleCross,			"LineCircleCross"		},
	{ EnumTypeHeightDifference,			"HeightDifference"		},
	{ EnumTypeLineContourDistance,		"LineContourDistance"	},
	{ EnumTypeWidth,					"Width"					},
	{ EnumTypeSectionalArea,			"SectionalArea"			},
	{ EnumTypePointPointDistance,		"PointPointDistance"	},
	{ EnumTypePointLineDistance,		"PointLineDistance"		},
	{ EnumTypeLineLineAngle,			"LineLineAngle"			},
	{ EnumTypeLineContourCrossNum,		"LineContourCrossNum"	},
	{ EnumTypeConnect,					"Connect"				},
	{ EnumTypeLink,						"Link"					},
	{ EnumTypeUpLimit,					"UpLimit"				},
	{ EnumTypeDownLimit,				"DownLimit"				},
	{ EnumTypeCalculateMode,			"CalculateMode"			}
};

frmOutLineDetectTool::frmOutLineDetectTool(QWidget* parent)
	: frmBaseTool(parent),
	bPreventReaptUpdateMeasure(true)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Contour.png"));

	//绘图窗口
	m_view = new QGraphicsViews;
	connect(m_view,					&QGraphicsViews::sig_MouseMoveInImg,	this, &frmOutLineDetectTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	initConnect();
	initRoiShapeProperty();
	m_VecBaseItem.clear();
	initResultProperty();
	initParamProperty();
	initJudgeProperty();

	initTitleBar();
	initCustomplot();
	initContourParam();
	initData();
	//ChangeLanguage();
	//InitUI(this);
}

frmOutLineDetectTool::~frmOutLineDetectTool()
{
	if (btnGroupRadio != nullptr)			delete btnGroupRadio;				btnGroupRadio		= nullptr;
	if (m_titleBar != nullptr)				delete m_titleBar;					m_titleBar			= nullptr;
	if (m_view != nullptr)					delete m_view;						m_view				= nullptr;
	if (m_btnHintAction_buf != nullptr)		delete m_btnHintAction_buf;			m_btnHintAction_buf = nullptr;
	if (m_btnHintAction != nullptr)			delete m_btnHintAction;				m_btnHintAction		= nullptr;
	
	if (m_variantFactory != nullptr)		delete m_variantFactory;			m_variantFactory		= nullptr;
	if (m_variantManager != nullptr)		delete m_variantManager;			m_variantManager		= nullptr;
	if (m_propertyEditor != nullptr)		delete m_propertyEditor;			m_propertyEditor		= nullptr;

	this->deleteLater();
}

void frmOutLineDetectTool::initCustomplot()
{
	ui.widgetPlot->showTracer(false);

	ui.widgetPlot->rescaleAxes();
	ui.widgetPlot->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.widgetPlot->xAxis->setRange(-1, 1);
	ui.widgetPlot->yAxis->setRange(0, 1);
	//ui.widgetPlot->legend->setVisible(true);
	ui.widgetPlot->axisRect()->setupFullAxesBox();
	ui.widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iMultiSelect);
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->clearGraphs();
	connect(ui.widgetPlot, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_customContextMenuRequested(const QPoint &)));

	m_bRemovePoint = true;
}

void frmOutLineDetectTool::initContourParam()
{
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->horizontalHeader()->setVisible(true);
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)	ui.tableWidget->removeRow(0);  //设置行数

	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidget, &QTableWidget::customContextMenuRequested, this, &frmOutLineDetectTool::slot_OutLineContextMenuRequested);		//这个坐标一般Yes相对于控件左上角

}

void frmOutLineDetectTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Contour.png");

}

void frmOutLineDetectTool::initRoiShapeProperty()
{
#pragma region ROI属性
	ui.tableWidget->setColumnWidth(0, 50);
	ui.tableWidget->setColumnWidth(1, 200);
	//ui.tableWidget->setColumnWidth(2, 300);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidget->resizeColumnsToContents();

	m_variantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Roi_valueChanged(QtProperty *, const QVariant &)));
	m_variantFactory = new QtVariantEditorFactory(this);
	m_propertyEditor = new QtTreePropertyBrowser(ui.ROIwidget);
	m_propertyEditor->setFactoryForManager(m_variantManager, m_variantFactory);
	ui.ROILayout->addWidget(m_propertyEditor);
	m_idToProperty.clear();
	m_propertyToId.clear();
	//idToExpanded.clear();
#pragma endregion

	ui.tableROIWidget->setColumnWidth(0, 50);
	ui.tableROIWidget->setColumnWidth(1, 200);
	ui.tableROIWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行
	ui.tableROIWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置只能单选	
	ui.tableROIWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
	ui.tableROIWidget->resizeColumnsToContents();

	//ui.tableROIWidget->;
	ui.tableROIWidget->verticalHeader()->setVisible(false);
	ui.tableROIWidget->horizontalHeader()->setVisible(true);
	DeleteAllOutLineData();
}

void frmOutLineDetectTool::initResultProperty()
{
	initShowEditorProperty(ui.widget_Result, ui.ResultLayout);
	ui.gImage->setVisible(false);

	QtVariantProperty*	_ptrproperty = nullptr;
	QtVariantProperty*	_ptrSubproperty = nullptr;
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("showregion"));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showregion, tr("showregion"));
	}
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("showDrawData"));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showDrawData, tr("showDrawData"));
	}
}

void frmOutLineDetectTool::initParamProperty()
{
#pragma region 参数属性
	m_ParamvariantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_ParamvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Param_valueChanged(QtProperty *, const QVariant &)));
	m_ParamvariantFactory = new QtVariantEditorFactory(this);
	m_ParampropertyEditor = new QtTreePropertyBrowser(ui.widget_Result);
	m_ParampropertyEditor->setFactoryForManager(m_ParamvariantManager, m_ParamvariantFactory);
	ui.ParamLayout->addWidget(m_ParampropertyEditor);
	m_ParampropertyToId.clear();
	m_ParamidToProperty.clear();
	//m_ParamidToExpanded.clear();
#pragma endregion


}

void frmOutLineDetectTool::initJudgeProperty()
{
#pragma region 判定属性
	m_JudgeLimitvariantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_JudgeLimitvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_JudgeLimit_valueChanged(QtProperty *, const QVariant &)));
	m_ParamvariantFactory = new QtVariantEditorFactory(this);
	m_JudgeLimitpropertyEditor = new QtTreePropertyBrowser(ui.widget_Result);
	m_JudgeLimitpropertyEditor->setFactoryForManager(m_JudgeLimitvariantManager, m_ParamvariantFactory);
	ui.ParamLimitLayout->addWidget(m_JudgeLimitpropertyEditor);
	m_JudgeLimitpropertyToId.clear();
	m_JudgeLimitidToProperty.clear();
#pragma endregion

}

void frmOutLineDetectTool::initConnect()
{
	connect(ui.cbx_Camerachannal,	SIGNAL(activated(int)),									this, SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW,			SIGNAL(valueChanged(int)),								this, SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut,		SIGNAL(activated(int)),									this, SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view,					SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)),		this, SLOT(slot_MouseClicked(QList<QGraphicsItem*>&)));
	connect(m_view,					SIGNAL(sig_MouseChange(QList<QGraphicsItem*>&)),		this, SLOT(slot_MouseMouve(QList<QGraphicsItem*>&)));
	connect(ui.widgetPlot,			SIGNAL(sig_XxwTraceRect(XxwTraceRect*)),				this, SLOT(slot_customplot_XxwTraceRect(XxwTraceRect*)));
	//测量
	connect(ui.btnAdd,				SIGNAL(pressed()),										this, SLOT(slot_Contour_PopMenu()));
	connect(ui.tableWidget,			&QTableWidget::pressed,									this, &frmOutLineDetectTool::slot_TableWidget_pressed);
	connect(ui.tableWidget,			SIGNAL(itemEntered(QTableWidgetItem*)),					this, SLOT(slot_ItemEnter(QTableWidgetItem*)));
	connect(ui.widgetPlot,			SIGNAL(sig_ChangeTraceRect(XxwTraceRect*)),				this, SLOT(slot_TableWidget_ChangedTraceRect(XxwTraceRect*)));
	connect(ui.widgetPlot,			SIGNAL(sig_ChangeDetectItemLine(XxwDetectItemLine*)),	this, SLOT(slot_TableWidget_ChangedItemLine(XxwDetectItemLine*)));
	connect(ui.tableROIWidget,		SIGNAL(itemEntered(QTableWidgetItem*)),					this, SLOT(slot_ItemEnter(QTableWidgetItem*)));
	connect(ui.tableROIWidget,		&QTableWidget::pressed,									this, &frmOutLineDetectTool::slot_ROITableWidget_pressed);
	
	connect(ui.btnAddROI,			&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnAddROI_clicked);
	connect(ui.btnDeleteROISelect,	&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnDeleteROISelect_clicked);
	connect(ui.btnDeleteROIAll,		&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnDeleteROIAll_clicked);
	connect(ui.btnLinkContent,		&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,	&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnDelLinkContent_clicked);
	
	connect(ui.btnSub,				&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnSub_clicked);
	connect(ui.btnMoveUp,			&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnMoveUp_clicked);
	connect(ui.btnMoveDown,			&QPushButton::pressed,									this,	&frmOutLineDetectTool::on_btnMoveDown_clicked);

}

void frmOutLineDetectTool::initData()
{
	ui.tabWidget->setCurrentIndex(0);
	m_iChanal = 0;

	m_VecCheckOutLineData.clear();
	DeleteAllOutLineData();
}

void frmOutLineDetectTool::ChangeLanguage()
{
	frmBaseTool::ChangeLanguage();
	ui.retranslateUi(this);
}

//绘制ROI
void frmOutLineDetectTool::on_btnAddROI_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth = (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;

	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	int iCount = m_VecBaseItem.size();
	float fData[5] = { 0 };
	switch (ui.comboROIShape->currentIndex())	{
	case 10: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 5.0, iHeight / 5.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//椭圆
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new EllipseItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}   break;
	case 2: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}   break;
	case 3: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}   break;
	case 4: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}   break;
	case 5: {	//多边形
		_BaseItem = new PolygonItem();
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}   break;
	case 0: {	//线
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new LineArrowItem(fData[0] - fData[2], fData[1] - fData[3], fData[0] + fData[2], fData[1] + fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1

		LineData _data;
		((LineArrowItem*)_BaseItem)->GetLineData(_data);
		QString	_strName = QString::number(iCount) + "." + tr("Line");
		{	CheckOutLineData	_Linedata;
		_Linedata.strName = _strName;
		_Linedata.strData = QString("%1_%2_%3_%4@%5|%6")
			.arg(QString::number(_data.row))
			.arg(QString::number(_data.col))
			.arg(QString::number(_data.row1))
			.arg(QString::number(_data.col1))
			.arg(QString::number(ui.checkUseFollow->isChecked()))
			.arg(QString(ui.txtLinkFollow->text()));
		_Linedata.vecData.clear();
		m_VecCheckOutLineData.push_back(_Linedata);
		int count = ui.tableROIWidget->rowCount();
		ui.tableROIWidget->setRowCount(count + 1);
		{	//名称
			QTableWidgetItem*_pitem = new QTableWidgetItem();
			_pitem->setTextAlignment(Qt::AlignCenter);
			_pitem->setFlags(_pitem->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			_pitem->setText(_strName);
			ui.tableROIWidget->setItem(count, 0, _pitem);
		} {	//数据
			QString	_strName = _Linedata.strData;
			QTableWidgetItem* _pitem = new QTableWidgetItem();
			_pitem->setData(1, _strName);
			_pitem->setTextAlignment(Qt::AlignCenter);
			_pitem->setFlags(_pitem->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			_pitem->setText(_strName);
			ui.tableROIWidget->setItem(count, 1, _pitem);
		}
		}
	}   break;
	}
	for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
		if (((*mapIter).first).count(cstItem) > 0) {
			_ptrSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
			for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
				int				iEnable;
				EnumParamType	iType;
				QString			Param;
				QStringList		strEnumLst;
				QString			LowParam;
				QString			HighParam;
				QString			strLang;
				if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
					switch (iType) {
					case ParamType_Bool:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Bool, (*pIter).first);
						break;
					case ParamType_Int:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Int, (*pIter).first);
						break;
					case ParamType_Double:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					case ParamType_String:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::String, (*pIter).first);
						break;
					case ParamType_Enum:
						_ptrSubSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
						_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
						break;
					case ParamType_Other:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					default:		break;
					}
					_ptrSubSubproperty->setValue(Param);
					_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);
					_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
					if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
				}
				else {
					_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
					_ptrSubSubproperty->setValue((*pIter).second);
				}
				m_idToProperty[(*pIter).first] = _ptrSubSubproperty;
				m_idToProperty[(*pIter).first]->setPropertyName(strLang);
				m_propertyToId[_ptrSubSubproperty] = (*pIter).first;
				_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
			}
			addProperty(_ptrSubproperty, (*mapIter).first);
			_ptrSubproperty->setPropertyName(BaseItem::tr(cstItem.toStdString().c_str()));
		}
		else {
			_ptrSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), ((*mapIter).first));
			for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
				int				iEnable;
				EnumParamType	iType;
				QString			Param;
				QStringList		strEnumLst;
				QString			LowParam;
				QString			HighParam;
				QString			strLang;
				if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
					switch (iType) {
					case ParamType_Bool:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Bool, (*pIter).first);
						break;
					case ParamType_Int:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Int, (*pIter).first);
						break;
					case ParamType_Double:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					case ParamType_String:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::String, (*pIter).first);
						break;
					case ParamType_Enum:
						_ptrSubSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
						_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
						break;
					case ParamType_Other:
						_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					default:		break;
					}
					_ptrSubSubproperty->setValue(Param);
					_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);
					_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
					if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
				}
				else {
					_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
					_ptrSubSubproperty->setValue((*pIter).second);
				}

				m_idToProperty[(*pIter).first] = _ptrSubSubproperty;
				m_idToProperty[(*pIter).first]->setPropertyName(strLang);
				m_propertyToId[_ptrSubSubproperty] = (*pIter).first;

				_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
			}
			addProperty(_ptrSubproperty, (*mapIter).first);
			//_ptrproperty->addSubProperty(_ptrSubproperty);
		}
	}

	switch (ui.comboxROIType->currentIndex())	{
	case 0: {	_BaseItem->m_eRegionType = RegionType_Add;		}break;		//并集
	case 1: {	_BaseItem->m_eRegionType = RegionType_Differe;	}break;		//差集
	default:	break;
	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmOutLineDetectTool::on_btnDeleteROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)	{
		if (iter != nullptr)
			if (iter->type() == 10)		{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecBaseItem.size(); i++)	{
					if (m_VecBaseItem[i] == iter)	{
						_bFinded = true;

						//删除ROI形状
						{	if (i < m_VecCheckOutLineData.size())
								m_VecCheckOutLineData.erase(m_VecCheckOutLineData.begin() + i);
							if (i < ui.tableROIWidget->rowCount())	ui.tableROIWidget->removeRow(i);
						}

					}
				}
				if (_bFinded)	{
					(m_view->scene())->removeItem(iter);
					m_VecBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmOutLineDetectTool::on_btnDeleteROIAll_clicked()
{
	ClearROIProperty();
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecBaseItem.size(); i++)
			{
				if (m_VecBaseItem[i] == iter)
				{
					_bFinded = true;
				}
			}
			if (_bFinded)
			{
				m_VecBaseItem.removeOne((BaseItem*)iter);
				(m_view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
	m_VecCheckOutLineData.clear();
	DeleteAllOutLineData();
}

void frmOutLineDetectTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->m_strModelIndex		= m_strModelIndex;
	frm->m_strTypeVariable		= m_strTypeVariable;
	frm->m_strValueName			= m_strValueName;
	frm->SetLimitModel(QString::number(((OutLineDetectTool*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();
	if (frm->m_iRetn == 2)
	{
		//关联变量
		m_strModelIndex		= frm->m_strModelIndex;
		m_strTypeVariable	= frm->m_strTypeVariable;
		m_strValueName		= frm->m_strValueName;

		ui.txtLinkFollow->setText(QString("%1:%2:%3")
			.arg(m_strModelIndex)
			.arg(m_strTypeVariable)
			.arg(m_strValueName));
	}
	delete frm;

}

void frmOutLineDetectTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkFollow->clear();
	m_strModelIndex.clear();
	m_strTypeVariable.clear();
	m_strValueName.clear();
}

void frmOutLineDetectTool::onChannelClicked(int Index)
{
	if (Index < 0)	return;
	try
	{
		m_iChanal = Index;
		QImage img(Hal2QImage(m_dstImage));
		m_view->DispImage(img);
		m_view->update();
	}
	catch (...) {}
}

void frmOutLineDetectTool::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
}

void frmOutLineDetectTool::slot_OutLineContextMenuRequested(const QPoint & pos)
{
	QMenu*	m_menu = new QMenu(this);
	QPushButton* moveUpAction_buf = new QPushButton(m_menu);
	moveUpAction_buf->setObjectName("MoveUp");
	moveUpAction_buf->setText(tr("MoveUp"));
	moveUpAction_buf->setIconSize(QSize(20, 20));
	moveUpAction_buf->setMinimumSize(QSize(120, 40));
	//moveUpAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/previous.png"));
	QWidgetAction* moveUpAction = new QWidgetAction(this);
	moveUpAction->setDefaultWidget(moveUpAction_buf);
	connect(moveUpAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::on_btnMoveUp_clicked);
	connect(moveUpAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveUpAction);
	QPushButton* moveDownAction_buf = new QPushButton(m_menu);
	moveDownAction_buf->setObjectName("MoveDown");
	moveDownAction_buf->setText(tr("MoveDown"));
	moveDownAction_buf->setIconSize(QSize(20, 20));
	moveDownAction_buf->setMinimumSize(QSize(120, 40));
	//moveDownAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/next.png"));
	QWidgetAction* moveDownAction = new QWidgetAction(this);
	moveDownAction->setDefaultWidget(moveDownAction_buf);
	connect(moveDownAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::on_btnMoveDown_clicked);
	connect(moveDownAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(moveDownAction);
	QPushButton* deleteNameAction_buf = new QPushButton(m_menu);
	deleteNameAction_buf->setObjectName("Delete");
	deleteNameAction_buf->setText(tr("Delete"));
	deleteNameAction_buf->setIconSize(QSize(21, 21));
	deleteNameAction_buf->setMinimumSize(QSize(120, 40));
	//deleteNameAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/del.png"));
	QWidgetAction* deleteNameAction = new QWidgetAction(this);
	deleteNameAction->setDefaultWidget(deleteNameAction_buf);
	connect(deleteNameAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::on_btnSub_clicked);
	connect(deleteNameAction_buf, &QPushButton::clicked, m_menu, &QMenu::close);
	m_menu->addAction(deleteNameAction);
	m_menu->exec(QCursor::pos());

	if (moveUpAction_buf != nullptr)		delete moveUpAction_buf;		moveUpAction_buf = nullptr;
	if (moveUpAction != nullptr)			delete moveUpAction;			moveUpAction = nullptr;
	if (moveDownAction_buf != nullptr)		delete moveDownAction_buf;		moveDownAction_buf = nullptr;
	if (moveDownAction != nullptr)			delete moveDownAction;			moveDownAction = nullptr;
	if (deleteNameAction_buf != nullptr)	delete deleteNameAction_buf;	deleteNameAction_buf = nullptr;
	if (deleteNameAction != nullptr)		delete deleteNameAction;		deleteNameAction = nullptr;
	if (m_menu != nullptr)					delete m_menu;					m_menu = nullptr;
}

void frmOutLineDetectTool::slot_Contour_PopMenu()
{
	QMenu*	m_CheckMenu = new QMenu();
	{
		QMenu*_pMenuCreate = new QMenu(m_CheckMenu);
		//***********************点***********************
		{	//点
			QString	_strText = "Point";
			QAction*_pAction = _pMenuCreate->addAction(tr("Point"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("Point"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//点点中点
			QString	_strText = "PointPointCenter";
			QAction*_pAction = _pMenuCreate->addAction(tr("PointPointCenter"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("PointPointCenter"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//直线交点
			QString	_strText = "LineCross";
			QAction*_pAction = _pMenuCreate->addAction(tr("LineCross"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("LineCross"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		//{	//线和轮廓交点			QString	_strText = "直线";
		//	QString	_strText = "线和轮廓交点";
		//	QAction*_pAction = _pMenuCreate->addAction(_strText);
		//	_pAction->setData(_strText);
		//	_pAction->setToolTip(_strText);
		//	_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
		//	connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		//}
		_pMenuCreate->addSeparator();
		//***********************线***********************
		{	//直线
			QString	_strText = "Line";
			QAction*_pAction = _pMenuCreate->addAction(tr("Line"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("Line"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//水平直线
			QString	_strText = "HorizontalLine";
			QAction*_pAction = _pMenuCreate->addAction(tr("HorizontalLine"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("HorizontalLine"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		//{	//线线交点
		//	QString	_strText = "线线交点";
		//	QAction*_pAction = _pMenuCreate->addAction(_strText);
		//	_pAction->setData(_strText);
		//	_pAction->setToolTip(_strText);
		//	_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
		//	connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		//}
		_pMenuCreate->addSeparator();
		{	//园
			QString	_strText = "Circle";
			QAction*_pAction = _pMenuCreate->addAction(tr("Circle"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("Circle"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//点点直线
			QString	_strText = "PointPointLine";
			QAction*_pAction = _pMenuCreate->addAction(tr("PointPointLine"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("PointPointLine"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//垂线
			QString	_strText = "verticalLine";
			QAction*_pAction = _pMenuCreate->addAction(tr("verticalLine"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("verticalLine"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//垂点
			QString	_strText = "pointStroke";
			QAction*_pAction = _pMenuCreate->addAction(tr("pointStroke"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("pointStroke"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		//{	//线园交点
		//	QString	_strText = "线园交点";
		//	QAction*_pAction = _pMenuCreate->addAction(_strText);
		//	_pAction->setData(_strText);
		//	_pAction->setToolTip(_strText);
		//	_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
		//	connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		//}
		_pMenuCreate->addSeparator();
		//***********************线***********************
		QAction*_pActionCreate = m_CheckMenu->addMenu(_pMenuCreate);
		_pActionCreate->setIcon(QIcon(":/image/icon/images/seting.png"));
		_pActionCreate->setText(tr("Create"));
		_pActionCreate->setData("Create");
		_pActionCreate->setToolTip(tr("Create"));
		QMenu*	_pMenuMeasure = new QMenu(m_CheckMenu);
		{	//高度差
			QString	_strText = "HeightDifference";
			QAction*_pAction = _pMenuMeasure->addAction(tr("HeightDifference"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("HeightDifference"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		//{	//轮廓到线的距离
		//	QString	_strText = "轮廓到线的距离";
		//	QAction*_pAction = _pMenuMeasure->addAction(_strText);
		//	_pAction->setData(_strText);
		//	_pAction->setToolTip(_strText);
		//	_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
		//	connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		//}
		{	//宽度
			QString	_strText = "Width";
			QAction*_pAction = _pMenuMeasure->addAction(tr("Width"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("Width"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//截面积
			QString	_strText = "SectionalArea";
			QAction*_pAction = _pMenuMeasure->addAction(tr("SectionalArea"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("SectionalArea"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		QAction*_pActionMeasure = m_CheckMenu->addMenu(_pMenuMeasure);
		_pActionMeasure->setIcon(QIcon(":/image/icon/images/seting.png"));
		_pActionMeasure->setText(tr("MeaSure"));
		_pActionMeasure->setData("MeaSure");
		_pActionMeasure->setToolTip(tr("MeaSure"));
		QMenu*	_pMenuCalculated = new QMenu(m_CheckMenu);
		{	//点到点距离
			QString	_strText = "PointPointDistance";
			QAction*_pAction = _pMenuCalculated->addAction(tr("PointPointDistance"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("PointPointDistance"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//点到线距离
			QString	_strText = "PointLineDistance";
			QAction*_pAction = _pMenuCalculated->addAction(tr("PointLineDistance"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("PointLineDistance"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		{	//线到线夹角
			QString	_strText = "LineLineAngle";
			QAction*_pAction = _pMenuCalculated->addAction(tr("LineLineAngle"));
			_pAction->setData(_strText);
			_pAction->setToolTip(tr("LineLineAngle"));
			_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
			connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		}
		//{	//线和轮廓的交点总数
		//	QString	_strText = "线和轮廓的交点总数";
		//	QAction*_pAction = _pMenuCalculated->addAction(_strText);
		//	_pAction->setData(_strText);
		//	_pAction->setToolTip(_strText);
		//	_pAction->setIcon(QIcon(":/image/icon/images/seting.png"));
		//	connect(_pAction, &QAction::triggered, this, &frmOutLineDetectTool::slot_Menu_ADDAction);
		//}
		QAction*_pActionCalculate = m_CheckMenu->addMenu(_pMenuCalculated);
		_pActionCalculate->setIcon(QIcon(":/image/icon/images/seting.png"));
		_pActionCalculate->setText(tr("Calculate"));
		_pActionCalculate->setData("Calculate");
		_pActionCalculate->setToolTip(tr("Calculate"));
	}
	m_CheckMenu->exec(QCursor::pos());
}

void frmOutLineDetectTool::on_UpDateCheckType(int Index)
{

}

int frmOutLineDetectTool::Execute(const QString toolname)
{
	int _iCurrentRow = ui.tableROIWidget->currentRow();
	if (_iCurrentRow >= 0)	{	ShapeToRegion(m_VecBaseItem[_iCurrentRow], ModelRoiRegion);	}
	else return -1;
	QVector<sDrawText>	_VecText1D;
	QVector<QPointF>	_VecPt1D;
	m_view->ClearAllDrawItem();
	HObject _MetrixRegion = ModelRoiRegion;

	if (!_MetrixRegion.IsInitialized() || m_VecBaseItem.size() <= 0) {
		sDrawText			_strText;
		_strText.bControl	= true;
		_strText.DrawColor	= QColor(255, 0, 0);
		if(m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth	= m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText	= tr("Region Isn't IsInitialized");
		_VecText1D.push_back(_strText);
		m_view->DispTextList(_VecText1D);
		return -1;
	}
	if (ui.checkUseFollow->isChecked()) {
		HTuple	_Metrix;
		HTuple	Rows, Columns, _Lenth;
		HObject _MetrixContour;
		if (OutLineDetectTool::CheckMetrixType(ui.txtLinkFollow->text(), _Metrix) == 0) {
			AffineTransRegion(ModelRoiRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
			_MetrixContour.GenEmptyObj();
			GetRegionContour(_MetrixRegion, &Rows, &Columns);
			HalconCpp::TupleLength(Rows, &_Lenth);
			for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {	_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));		}
		} else {	
			sDrawText			_strText;	
			_strText.bControl	= true;
			_strText.DrawColor	= QColor(255, 0, 0);
			if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.strText	= tr(" The Link Matrix Failed!");
			_VecText1D.push_back(_strText);
			m_view->DispTextList(_VecText1D);
		}
	}
	int _iCheckMode = databaseVar::Instance().m_iCheckMode;
	UpdateCheckManager();
	QString	strError;
	m_CheckManager.PrExcute();
	MiddleParam _param;
	EnumErrorRetn retn = m_CheckManager.Excute(_iCurrentRow,false,_param,m_srcImage,strError);
	switch (retn)	{
	case ErrorRetn_NotContain:	{
		sDrawText _strText;
		_strText.bControl	= true;
		_strText.DrawColor	= QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText	= tr("Detection Param Error") + strError;
		_VecText1D.push_back(_strText);
		m_view->DispTextList(_VecText1D);
		return NodeResult_ParamError;
	}	break;
	case ErrorRetn_Error:	{
		sDrawText _strText;
		_strText.bControl	= true;
		_strText.DrawColor	= QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText	= tr("Detection Error") + strError;
		_VecText1D.push_back(_strText);
		m_view->DispTextList(_VecText1D);
		return NodeResult_Error;
	}	break;
	case ErrorRetn_NG:	{
		sDrawText _strText;
		_strText.bControl	= true;
		_strText.DrawColor	= QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText	= tr("Detection NG") + strError;
		_VecText1D.push_back(_strText);
		m_view->DispTextList(_VecText1D);
		return NodeResult_Error;
	}	break;
	case ErrorRetn_OK:	{
		sDrawText _strText;
		_strText.bControl	= true;
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor	= QColor(0, 255, 0);
		_strText.strText	= tr("Detection Complete!");
		_VecText1D.push_back(_strText);
	}	break;
	default:{	}	break;	}
	bool _bShowData = false;
	if (m_ShowidToProperty.count(showDrawData) > 0)		_bShowData = m_ShowidToProperty[showDrawData]->value().toInt();
	for (int i = 0; i < m_CheckManager.m_vecCheck2D[_iCurrentRow].size(); i++){
		QString	_strName		= QString("  %1.").arg(QString::number(i));
		auto&Check				= m_CheckManager.m_vecCheck2D[_iCurrentRow][i];
		sDrawText				_sDrawText;
		_sDrawText.bControl		= true;
		if (m_ShowidToProperty.count(FontSize) > 0)		_sDrawText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_sDrawText.DrawColor	= QColor(255, 0, 0);
		switch (Check->eCheckResult){
		case CheckResult_Error:	{
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection Error ")	+ (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_NG: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection NG ")		+ (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OverUp: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Upper Limit of Detection ")		+ (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OverDown: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Down Limit of Detection ")		+ (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_ParamError: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection Parameter Error ")   + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OK: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection OK ")			+ (_bShowData ? Check->GetCheckResult() : "");
			_sDrawText.DrawColor	= QColor(0, 255, 0);
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_NoCheck: {
			_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Created Successfully ")	+ (_bShowData ? Check->GetCheckResult() : "");
			_sDrawText.DrawColor	= QColor(0, 255, 0);
			_VecText1D.push_back(_sDrawText);
		}	break;
		default: {	}	break;}
	}
	m_view->DispTextList(_VecText1D);
	m_view->DispPoint(_VecPt1D, QColor(0, 255, 0));

	return 0;
}

EnumNodeResult frmOutLineDetectTool::Execute(MiddleParam & param, QString & strError)
{
	int _iCurrentRow = ui.tableROIWidget->currentRow();
	if (_iCurrentRow >= 0) { ShapeToRegion(m_VecBaseItem[_iCurrentRow], ModelRoiRegion); }
	else return NodeResult_Error;
	std::vector<sDrawText>	_VecText1D;
	std::vector<QPointF>	_VecPt1D;
	//m_view->ClearAllDrawItem();
	HObject _MetrixRegion = ModelRoiRegion;

	if (!_MetrixRegion.IsInitialized() || m_VecBaseItem.size() <= 0) {
		sDrawText			_strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText = tr("Region Not Inited! ");
		_VecText1D.push_back(_strText);
		param.VecDrawText = (_VecText1D);
		//m_view->DispTextList(_VecText1D);
		return NodeResult_ParamError;
	}
	if (ui.checkUseFollow->isChecked()) {
		HTuple	_Metrix;
		HTuple	Rows, Columns, _Lenth;
		HObject _MetrixContour;
		if (OutLineDetectTool::CheckMetrixType(ui.txtLinkFollow->text(), _Metrix) == 0) {
			AffineTransRegion(ModelRoiRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
			if (m_ShowidToProperty.value(showregion)->value().toBool()) {
				AffineShapeTrans(m_VecBaseItem, _Metrix, param, true);
			}
		}
		else {
			sDrawText			_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("The Link Matrix Failed! ");
			_VecText1D.push_back(_strText);
			param.VecShowQPointFs = (_VecPt1D);
			param.VecDrawText = (_VecText1D);
			if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
			else			return NodeResult_NG;

		}
	}
	else	{
		if (m_ShowidToProperty.value(showregion)->value().toBool()) {
			AffineShapeTrans(m_VecBaseItem, HTuple(), param);
		}
	}
	int _iCheckMode = databaseVar::Instance().m_iCheckMode;
	UpdateCheckManager();
	QString	_strError;
	m_CheckManager.PrExcute();
	MiddleParam _param;
	EnumErrorRetn retn = m_CheckManager.Excute(_iCurrentRow, false, _param, m_srcImage, _strError);
	switch (retn) {
	case ErrorRetn_NotContain: {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText = tr("Detection Parameter Error! ") + _strError;
		_VecText1D.push_back(_strText);
		param.VecDrawText = (_VecText1D);
		return NodeResult_ParamError;
	}	break;
	case ErrorRetn_Error: {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText = tr("Detection Error!") + _strError;
		_VecText1D.push_back(_strText);
		param.VecDrawText = (_VecText1D);
		return NodeResult_Error;
	}	break;
	case ErrorRetn_NG: {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.strText = tr("Detection NG") + _strError;
		_VecText1D.push_back(_strText);
		param.VecDrawText = (_VecText1D);
		return NodeResult_Error;
	}	break;
	case ErrorRetn_OK: {
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)		_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(0, 255, 0);
		_strText.strText = tr("Detection Complete!");
		_VecText1D.push_back(_strText);
		param.VecDrawText = (_VecText1D);
	}	break;
	default: {	}	break;
	}
	bool _bShowData = false;
	if (m_ShowidToProperty.count(showDrawData) > 0)		_bShowData = m_ShowidToProperty[showDrawData]->value().toInt();
	for (int i = 0; i < m_CheckManager.m_vecCheck2D[_iCurrentRow].size(); i++) {
		QString	_strName = QString("  %1.").arg(QString::number(i));
		auto&Check = m_CheckManager.m_vecCheck2D[_iCurrentRow][i];
		sDrawText				_sDrawText;
		_sDrawText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)		_sDrawText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_sDrawText.DrawColor = QColor(255, 0, 0);
		switch (Check->eCheckResult) {
		case CheckResult_Error: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Detection Error! ") + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_NG: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Detection NG ") + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OverUp: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Upper Limit of Detection ") + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OverDown: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Down Limit of Detection ") + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_ParamError: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Detection Parameter Error ") + (_bShowData ? Check->GetCheckResult() : "");
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_OK: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Detection OK ") + (_bShowData ? Check->GetCheckResult() : "");
			_sDrawText.DrawColor = QColor(0, 255, 0);
			_VecText1D.push_back(_sDrawText);
		}	break;
		case CheckResult_NoCheck: {
			_sDrawText.strText = _strName + Check->GetCheckName() + tr(" Created Successfully ") + (_bShowData ? Check->GetCheckResult() : "");
			_sDrawText.DrawColor = QColor(0, 255, 0);
			_VecText1D.push_back(_sDrawText);
		}	break;
		default: {	}	break;
		}
	}
	param.VecDrawText		= (_VecText1D);
	param.VecShowQPointFs	= (_VecPt1D);
	return EnumNodeResult(NodeResult_OK);
}

void frmOutLineDetectTool::ChangeLimitParam(int iIndex, XxwTraceRect::LimitType type, bool _iIsLimit, QStringList _strLimitArray)
{
	if (ui.widgetPlot->m_XxwLimitRectLst.size() <= 0)	ui.widgetPlot->m_XxwLimitRectLst.push_back(new XxwTraceRect(ui.widgetPlot, XxwTraceRect::LimitTypeUpAndDown));

	ui.widgetPlot->m_XxwLimitRectLst[iIndex]->ChangeLimitMode((XxwTraceRect::LimitType)type);
	if (_iIsLimit == 1)
	{
		ui.tableWidget->selectRow(ui.tableWidget->rowCount() - 1);
		ui.widgetPlot->m_XxwLimitRectLst[iIndex]->ResetLimit();
	}
	else
	{
		if (_strLimitArray.size() > 1)	ui.widgetPlot->m_XxwLimitRectLst[iIndex]->InitUpPosition(_strLimitArray[0].toDouble(), _strLimitArray[1].toDouble());
		if (_strLimitArray.size() > 3)	ui.widgetPlot->m_XxwLimitRectLst[iIndex]->InitDnPosition(_strLimitArray[2].toDouble(), _strLimitArray[3].toDouble());
	}
}

void frmOutLineDetectTool::GetLimitParam(int iIndex, int & iRegionType, bool & iIsLimit, QStringList & strLimitArray)
{
	iIsLimit = false;
	if (ui.widgetPlot->m_XxwLimitRectLst.size() > iIndex)
	{
		iRegionType = ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_type;
		strLimitArray.clear();
		if (ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_lineUp)
		{
			strLimitArray.append(QString::number(ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_RectUpRange.fValue, 'f', databaseVar::Instance().form_System_Precision));
			strLimitArray.append(QString::number(ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_RectUpRange.fRangeValue, 'f', databaseVar::Instance().form_System_Precision));
		}
		if (ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_lineDn)
		{
			strLimitArray.append(QString::number(ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_RectDnRange.fValue, 'f', databaseVar::Instance().form_System_Precision));
			strLimitArray.append(QString::number(ui.widgetPlot->m_XxwLimitRectLst[iIndex]->m_RectDnRange.fRangeValue, 'f', databaseVar::Instance().form_System_Precision));
		}
	}
}

void frmOutLineDetectTool::ChangeOutLineData(int irow)
{
	if (irow < m_VecCheckOutLineData.size())
	{
		ui.tableROIWidget->item(irow, 1)->setText( m_VecCheckOutLineData[irow].strData );
	}
}

void frmOutLineDetectTool::DeleteAllOutLineData()
{
	int	iRowCount = ui.tableROIWidget->rowCount();
	for (size_t i = 0; i < iRowCount; i++)
		ui.tableROIWidget->removeRow(0);
}

void frmOutLineDetectTool::DeleteSelectedOutLineData()
{
	int _iRow = ui.tableROIWidget->currentIndex().row();
	ui.tableROIWidget->removeRow(_iRow);
}

void frmOutLineDetectTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
	ResizeUI();
}

void frmOutLineDetectTool::showEvent(QShowEvent * ev)
{
	frmBaseTool::showEvent(ev);
	ResizeUI();
}

void frmOutLineDetectTool::hideEvent(QHideEvent * ev)
{
	frmBaseTool::hideEvent(ev);
}

void frmOutLineDetectTool::ResizeUI()
{
	int _iWidth = ui.tableWidget->width();
	ui.tableWidget->setColumnWidth(0, _iWidth / 5.0 - 8);
	ui.tableWidget->setColumnWidth(1, _iWidth / 5.0 * 4 - 8);
	_iWidth = ui.tableROIWidget->width();
	ui.tableROIWidget->setColumnWidth(0, _iWidth / 5.0 - 8);
	ui.tableROIWidget->setColumnWidth(1, _iWidth / 5.0 * 4 - 8);
}

void frmOutLineDetectTool::slot_MouseClicked(QList<QGraphicsItem*>& items)
{
	if (items.size() <= 0)	return;
	ClearROIProperty();
	int	iRowCount = ui.tableROIWidget->rowCount();
	for (int i = 0; i < m_VecBaseItem.size(); i++)	{
		for (int IK = 0; IK < items.size(); IK++)	{
			if (items[IK] == m_VecBaseItem[i])		{
				if (i < iRowCount)					{
					ui.tableROIWidget->selectRow(i);
				}
			}
		}
	}
	for (int iRow = 0; iRow < m_VecBaseItem.size(); iRow++)	{
		for (auto& iter : items)							{
			if (iter == m_VecBaseItem[iRow])				{
				slot_ROITableWidget_cellClicked(iRow, 0);

				bool bNeedClear = false;
				if (m_VecCheckOutLineData.size() > iRow)
					if (m_VecCheckOutLineData[iRow].vecData.size() > 0)	{
						bNeedClear = true;
						slot_TableWidget_cellClicked(0, 0);
					}
				if (!bNeedClear)	{
					ClearParamProperty();
				}
			}
		}
	}

}

void frmOutLineDetectTool::slot_MouseMouve(QList<QGraphicsItem*>& items)
{
	std::lock_guard<std::mutex> _lock(m_Lock);
	try
	{
		for (int i = 0; i < items.size(); i++) {
			QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
			((BaseItem*)items[i])->GetAllParam(_mapValue);

			for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
				for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
					QString	strName = (*pIter).first;
					if (m_idToProperty.count(strName) > 0) {
						int				iEnable;
						EnumParamType	iType;
						QString			Param;
						QStringList		strEnumLst;
						QString			LowParam;
						QString			HighParam;
						QString			strLang;
						if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
							m_idToProperty[strName]->setValue(Param);
							m_idToProperty[strName]->setEnabled(iEnable == 1 ? true : false);
							m_idToProperty[strName]->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
							if (!LowParam.isEmpty())	m_idToProperty[strName]->setAttribute(QLatin1String("minimum"), LowParam);
							if (!HighParam.isEmpty())	m_idToProperty[strName]->setAttribute(QLatin1String("maximum"), HighParam);
							m_idToProperty[strName]->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
						}
						else {
							m_idToProperty[strName]->setValue(Param);
						}
						m_idToProperty[strName]->setPropertyName(strLang);
					}
				}
			}
		}

	}
	catch (...)
	{
		int i = 0;
		i++;
	}
}

void frmOutLineDetectTool::ChangeGraphicsItemInFo(QList<QGraphicsItem*>& items)
{
	ClearROIProperty();

	QtVariantProperty *	_ptrproperty		= nullptr;
	QtVariantProperty *	_ptrSubproperty		= nullptr;
	QtVariantProperty *	_ptrSubSubproperty	= nullptr;

	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] != nullptr) {
			switch (((BaseItem*)items[i])->GetItemType()) 
			{
			case ItemCircle: {
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *Item = (LineArrowItem*)items[i];
				LineData cir;
				Item->GetLineData(cir);

				//修改相关的数据
				for (size_t iK = 0; iK < m_VecBaseItem.size(); iK++) {
					if (m_VecBaseItem[iK] == items[i]) {
						if (iK < m_VecCheckOutLineData.size()) {
							m_VecCheckOutLineData[iK].strData = QString("%1_%2_%3_%4@%5|%6")
								.arg(QString::number(cir.row))
								.arg(QString::number(cir.col))
								.arg(QString::number(cir.row1))
								.arg(QString::number(cir.col1))
								.arg(QString::number(ui.checkUseFollow->isChecked()))
								.arg(QString(ui.txtLinkFollow->text()));
							ChangeOutLineData(iK);
						}
					}
				}
			}	break;
			}
			((BaseItem*)items[i])->GetAllParam(_mapValue);
			for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
				if (((*mapIter).first).count(cstItem) > 0) {
					_ptrSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
					for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
						int				iEnable;
						EnumParamType	iType;
						QString			Param;
						QStringList		strEnumLst;
						QString			LowParam;
						QString			HighParam;
						QString			strLang;
						if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
							switch (iType) {
							case ParamType_Bool:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Bool, (*pIter).first);
								break;
							case ParamType_Int:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Int, (*pIter).first);
								break;
							case ParamType_Double:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
								break;
							case ParamType_String:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::String, (*pIter).first);
								break;
							case ParamType_Enum:
								_ptrSubSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
								_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
								break;
							case ParamType_Other:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
								break;
							default:		break;
							}
							_ptrSubSubproperty->setValue(Param);
							_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);
							_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
							if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
							if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
						}
						else {
							_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
							_ptrSubSubproperty->setValue(	(*pIter).second	);
						}
						m_idToProperty[(*mapIter).first + (*pIter).first] = _ptrSubSubproperty;
						m_idToProperty[(*mapIter).first + (*pIter).first]->setPropertyName(strLang);
						m_propertyToId[_ptrSubSubproperty] = (*mapIter).first + (*pIter).first;
						_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
					}
					addProperty(_ptrSubproperty, (*mapIter).first);
					_ptrSubproperty->setPropertyName(BaseItem::tr(cstItem.toStdString().c_str()));
				}
				else {
					_ptrSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
					for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
						int				iEnable;
						EnumParamType	iType;
						QString			Param;
						QStringList		strEnumLst;
						QString			LowParam;
						QString			HighParam;
						QString			strLang;
						if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
							switch (iType) {
							case ParamType_Bool:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Bool, (*pIter).first);
								break;
							case ParamType_Int:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Int, (*pIter).first);
								break;
							case ParamType_Double:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
								break;
							case ParamType_String:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::String, (*pIter).first);
								break;
							case ParamType_Enum:
								_ptrSubSubproperty = m_variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
								_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
								break;
							case ParamType_Other:
								_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
								break;
							default:		break;
							}
							_ptrSubSubproperty->setValue(Param);
							_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);
							_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
							if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
							if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
						}
						else {
							_ptrSubSubproperty = m_variantManager->addProperty(QVariant::Double, (*pIter).first);
							_ptrSubSubproperty->setValue((*pIter).second);
						}

						m_idToProperty[(*mapIter).first + (*pIter).first] = _ptrSubSubproperty;
						m_idToProperty[(*mapIter).first + (*pIter).first]->setPropertyName(strLang);
						m_propertyToId[_ptrSubSubproperty] = (*mapIter).first + (*pIter).first;

						_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
					}
					addProperty(_ptrSubproperty, (*mapIter).first);
					//_ptrproperty->addSubProperty(_ptrSubproperty);
				}
			}

		}

	}
	slot_UpdateImgContour(items);

}

int frmOutLineDetectTool::Load()
{
	m_titleBar->setTitleContent(((OutLineDetectTool*)m_pTool)->GetItemId());

	//设置图像输入
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((OutLineDetectTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((OutLineDetectTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((OutLineDetectTool*)m_pTool)->GetItemId())) {
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}

	ui.widgetPlot->DrawAllItemClear();
	ui.widgetPlot->SetSearchRectVisable();
	ui.widgetPlot->SetTraceItemLineVisable();

	//显示图像
	ui.cbx_ImgInPut->setCurrentText(((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_strInImg);
	if (ui.cbx_ImgInPut->currentIndex() >= 0)	slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());

	on_btnDeleteROIAll_clicked();
	DataToShape(((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_vecCommonData, m_VecBaseItem);
	for (auto pItem : m_VecBaseItem)	m_view->AddItems(pItem);

	if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains("spinRoiW"))	ui.spinRoiW->setValue(((OutLineDetectTool*)m_pTool)->m_mapParam["spinRoiW"].toInt());

	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		m_VecBaseItem[i]->setIndex(i);
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
	QList<QGraphicsItem*> _ItemLst;
	for (int i = 0; i < m_VecBaseItem.size(); i++)	_ItemLst.append(m_VecBaseItem[i]);

	ui.widgetPlot->InitPlot();
	ui.widgetPlot->RePlotALLTrace();

	//参数
	int _iSize = 0;
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)	ui.tableWidget->removeRow(0);  //设置行数
	if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains(QString("ParamCount"))) _iSize = ((OutLineDetectTool*)m_pTool)->m_mapParam[QString("ParamCount")].toDouble();
	m_MapVectorParam.clear();
	//m_VecData.clear();
	for (size_t irow = 0; irow < _iSize; irow++)
	{
		int count = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(count + 1);  //设置行数

		QString	_strName;
		if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains(QString("ParamName%1").arg(QString::number(irow))))
		{
			_strName = ((OutLineDetectTool*)m_pTool)->m_mapParam[QString("ParamName%1").arg(QString::number(irow))];
			QStringList _strNameArray =	_strName.split("_");
			m_MapVectorParam[_strNameArray[0]].push_back(_strNameArray[1].toInt());
			{
				QTableWidgetItem* _item = new QTableWidgetItem(_strName);
				_item->setTextAlignment(Qt::AlignCenter);
				_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
				ui.tableWidget->setItem(count, 0, _item);
			}
		}
		if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains(QString("ParamData%1").arg(QString::number(irow))))
		{
			_strName = ((OutLineDetectTool*)m_pTool)->m_mapParam[QString("ParamData%1").arg(QString::number(irow))];
			{
				QTableWidgetItem* _item = new QTableWidgetItem(_strName);
				_item->setTextAlignment(Qt::AlignCenter);
				//m_VecData.push_back(_strName);
				_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
				ui.tableWidget->setItem(count, 1, _item);
			}
		}
	}

	ui.tableWidget->selectRow(0);

	if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains("checkUseFollow"))			ui.checkUseFollow->setChecked(((OutLineDetectTool*)m_pTool)->m_mapParam["checkUseFollow"].toInt());
	if (((OutLineDetectTool*)m_pTool)->m_mapParam.contains("txtLinkFollow"))			ui.txtLinkFollow->setText(((OutLineDetectTool*)m_pTool)->m_mapParam["txtLinkFollow"]);

	//计算
	slot_UpdateImgContour(_ItemLst);
	slot_TableWidget_cellClicked(0, 0);
	slot_customplot_SetParamMode();

	//设置显示参数
	for (auto iter = ((OutLineDetectTool*)m_pTool)->m_mapShowParam.begin(); iter		!= ((OutLineDetectTool*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)			m_ShowidToProperty[iter.key()]->setValue(iter.value());

	m_VecCheckOutLineData										= ((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_vecCheckOutLineData;
	int count = ui.tableROIWidget->rowCount();
	for (int iRow = 0; iRow < count; iRow++)					ui.tableROIWidget->removeRow(iRow);
	for (int irow = 0; irow < m_VecCheckOutLineData.size(); irow++){
		count = ui.tableROIWidget->rowCount();
		ui.tableROIWidget->setRowCount(irow + 1);
		auto & _Linedata		= m_VecCheckOutLineData[irow];
		{	//名称
			QTableWidgetItem*_pitem = new QTableWidgetItem();
			_pitem->setTextAlignment(Qt::AlignCenter);
			_pitem->setFlags(_pitem->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			_pitem->setText(_Linedata.strName);
			ui.tableROIWidget->setItem(count, 0, _pitem);
		} {	//数据
			QString	_strName = _Linedata.strData;
			QTableWidgetItem* _pitem = new QTableWidgetItem();
			_pitem->setData(1, _strName);
			_pitem->setTextAlignment(Qt::AlignCenter);
			_pitem->setFlags(_pitem->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			_pitem->setText(_strName);
			ui.tableROIWidget->setItem(count, 1, _pitem);
		}
	}
	ui.tableROIWidget->selectRow(0);
	slot_ROITableWidget_cellClicked(0,0);
	ResizeUI();
	return 0;
}

int frmOutLineDetectTool::Save()
{
	((OutLineDetectTool*)m_pTool)->m_strInImg = ui.cbx_ImgInPut->currentText();

	((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_vecCommonData.clear();
	ShapeToData(m_VecBaseItem, ((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_vecCommonData);

	((OutLineDetectTool*)m_pTool)->m_mapParam.insert("spinRoiW",			QString::number(ui.spinRoiW->value()));
	((OutLineDetectTool*)m_pTool)->m_mapParam.insert("checkUseFollow",	QString::number(ui.checkUseFollow->isChecked()));
	((OutLineDetectTool*)m_pTool)->m_mapParam.insert("txtLinkFollow",		ui.txtLinkFollow->text());

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((OutLineDetectTool*)m_pTool)->m_mapShowParam[iter.key()]			= iter.value()->value().toString();

	((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_strLinkName							= ui.txtLinkFollow->text();
	((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_bEnableLinkFollow					= ui.checkUseFollow->isChecked();
	//参数
	((OutLineDetectTool*)m_pTool)->m_mapParam.insert(QString("ParamCount"), QString::number(	ui.tableWidget->rowCount()	));
	for (int row = 0; row <	ui.tableWidget->rowCount()	; row++) {
		QString	_strType, _strName, _strData;
		if (ui.tableWidget->item(row, 0) != nullptr)
			if (!ui.tableWidget->item(row, 0)->text().isEmpty())	{
				_strName = ui.tableWidget->item(row, 0)->text();
				((OutLineDetectTool*)m_pTool)->m_mapParam.insert(QString("ParamName%1").arg(QString::number(row)), _strName);
			}
		if (ui.tableWidget->item(row, 1) != nullptr)
			if (!ui.tableWidget->item(row, 1)->text().isEmpty())	{
				_strData = ui.tableWidget->item(row, 1)->text();
				((OutLineDetectTool*)m_pTool)->m_mapParam.insert(QString("ParamData%1").arg(QString::number(row)), _strData);
			}
	}

	((OutLineDetectTool*)((OutLineDetectTool*)m_pTool))->m_vecCheckOutLineData					= m_VecCheckOutLineData;
	return 0;
}

bool frmOutLineDetectTool::CheckPoint(double & fValue)
{
	if (m_bRemovePoint && (fValue > -9000))
	{
		return true;
	}
	return false;
}

void frmOutLineDetectTool::UpdateCheckManager()
{
	m_CheckManager.ClearAll();
	m_CheckManager.LoadData(m_VecCheckOutLineData);
}

void frmOutLineDetectTool::addJudgeLimitProperty(QtVariantProperty * property, const QString & id)
{
	m_JudgeLimitpropertyID.push_back(id);
	m_JudgeLimitpropertyToId[property] = id;
	m_JudgeLimitidToProperty[id] = property;
	QtBrowserItem *item = m_JudgeLimitpropertyEditor->addProperty(property);
	if (cstItem == id) {
		m_JudgeLimitpropertyEditor->setExpanded(item, true);
	}
	else {
		m_JudgeLimitpropertyEditor->setExpanded(item, false);
	}
}

void frmOutLineDetectTool::ChangeParamValue(QString strParam, QString strValue)
{
	for (auto iter = m_ParamidToProperty.begin(); iter != m_ParamidToProperty.end(); iter++)	{
		if (iter.key() == strParam)	{
			iter.value()->setValue(strValue);
			iter.value()->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		}
	}
}

void frmOutLineDetectTool::InsertParamValue(int index, QString strParam, QStringList strValues, QtProperty *proper)
{
	if(!m_VecParampropertyID.contains(strParam))
		m_VecParampropertyID.insert(index + 1, strParam);
	if (!m_ParamidToProperty.contains(strParam))
	{
		QtVariantProperty *	_ptrSubproperty = nullptr;
		_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), strParam);
		_ptrSubproperty->setAttribute(QLatin1String("enumNames"), strValues);
		_ptrSubproperty->setValue(0);
		//addParamProperty(_ptrSubproperty, "关联点1");
		m_ParampropertyToId[_ptrSubproperty] = strParam;
		m_ParamidToProperty[strParam] = _ptrSubproperty;
		QtBrowserItem *item = m_ParampropertyEditor->insertProperty(_ptrSubproperty,proper);
	}
}

void frmOutLineDetectTool::DeleteParamProperty(QString strParam)
{
	for (auto iter = m_VecParampropertyID.begin(); iter != m_VecParampropertyID.end(); iter++)	{
		if (iter == strParam)	{
			m_VecParampropertyID.erase(iter);	break;
		}
	}
	for (auto iter = m_ParamidToProperty.begin(); iter != m_ParamidToProperty.end(); iter++)
		if(iter.key() == strParam)
			if (iter.value() != nullptr)	{
				m_ParamidToProperty.erase(iter);
				delete iter.value();
				break;
			}
	for (auto iter = m_ParampropertyToId.begin(); iter != m_ParampropertyToId.end(); iter++)
		if (iter.value() == strParam)
			if (iter.value() != nullptr)	{
				m_ParampropertyEditor->removeProperty(iter.key());
				m_ParampropertyToId.erase(iter);
				break;
			}
}

void frmOutLineDetectTool::addParamProperty(QtVariantProperty * property, const QString & id)
{
	//std::lock_guard<std::mutex> _lock(m_Lock);
	m_VecParampropertyID.push_back(id);
	m_ParampropertyToId[property] = id;
	m_ParamidToProperty[id] = property;
	QtBrowserItem *item = m_ParampropertyEditor->addProperty(property);
	if (cstItem == id) {
		m_ParampropertyEditor->setExpanded(item, true);
	}
	else {
		m_ParampropertyEditor->setExpanded(item, false);
	}
}

void frmOutLineDetectTool::addProperty(QtVariantProperty * propertyName, const QString & id)
{
	auto iter = m_idToProperty.find(id);
	if (iter != m_idToProperty.end())
	{
		delete (m_idToProperty[id]);
		m_idToProperty.erase(iter);
	}
	m_propertyToId[propertyName] = id;
	m_idToProperty[id] = propertyName;
	QtBrowserItem *item = m_propertyEditor->addProperty(propertyName);
	if (cstItem == id) {
		m_propertyEditor->setExpanded(item, true);
	}
	else {
		m_propertyEditor->setExpanded(item, false);
	}

}

void frmOutLineDetectTool::slot_Roi_valueChanged(QtProperty *property, const QVariant &value)
{
	//std::lock_guard<std::mutex> _lock(m_Lock);
	if (property == nullptr)		return;
	QString strName = property->propertyName();
	if (m_propertyToId.count(property) > 0) {
		QString _strName = m_propertyToId[property];
		if (m_idToProperty.count(m_propertyToId[property]) > 0) {
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++) {
				((BaseItem*)items[i])->SetParam(m_propertyToId[property], value.toString());
			}
		}
	}

}

void frmOutLineDetectTool::slot_Param_valueChanged(QtProperty* property, const QVariant& value)
{
	if (property == nullptr)		return;

	QString strName = property->propertyName();
	if (m_ParampropertyToId.count(property) > 0)	{
		QString _strName = m_ParampropertyToId[property];
		if (m_ParamidToProperty.count(m_ParampropertyToId[property]) > 0)	{
			int _iRow = ui.tableWidget->currentRow();
			QString	_strData;

			if (CheckSafetableWidget(_iRow, 1))	{
				_strData					= ui.tableWidget->item(_iRow, 1)->text();

				QStringList					_strParamArray;
				QStringList					_strLimitArray;
				QStringList					_strLimitValues;
				QStringList					_strLimitParams;
				bool _iIsLimit				= false;
				int	_iRegionType			= XxwTraceRect::LimitTypeUpAndDown;
				{
					GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
					QString _strValue		=	m_ParamidToProperty[_strName]->valueText();
					int		_iValue			=	m_ParamidToProperty[_strName]->valueType();
					int		_ipropertyType	=	m_ParamidToProperty[_strName]->propertyType();
					for (int i = 0; i < m_VecParampropertyID.size(); i++)				{
						if (m_VecParampropertyID[i] == _strName)						{
							if (	m_ParamvariantManager->propertyType(m_ParamidToProperty[_strName])
								== QtVariantPropertyManager::enumTypeId()	
								&& m_ParamidToProperty[_strName]->value() == 5
								)	{
									QString	_strIndex = "Connect" + _strName.right(1);
									QStringList strValues = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iRow);
									bPreventReaptUpdateMeasure = false;
									InsertParamValue(i, _strIndex, strValues, property);
									bPreventReaptUpdateMeasure = true;
									break;
							}
						}
						else if (m_VecParampropertyID[i].contains("Connect"))				{
							QString	_strIndex = m_VecParampropertyID[i].right(1);
							for (size_t K = 0; K < m_VecParampropertyID.size(); K++)	{
								if (K != i)
									if(m_VecParampropertyID[K].contains(_strIndex))		{
										bPreventReaptUpdateMeasure = false;
										QString	_strCurrentName = m_VecParampropertyID[K];
										if (!(m_ParamvariantManager->propertyType(m_ParamidToProperty[_strCurrentName])
											== QtVariantPropertyManager::enumTypeId() 
											&& m_ParamidToProperty[_strCurrentName]->value() == 5	))	{
											DeleteParamProperty(m_VecParampropertyID[i]);
										}
										bPreventReaptUpdateMeasure = true;
									}
							}
						}
					}
					_strParamArray.clear();
					for (int i = 0; i	<	m_VecParampropertyID.size(); i++)			{
						if (i < _strParamArray.size())									{
							_strParamArray[i] = QString(m_ParamidToProperty[m_VecParampropertyID[i]]->value().toString());
						}
						else															{
							_strParamArray.push_back(QString(m_ParamidToProperty[m_VecParampropertyID[i]]->value().toString()));
						}
					}
				}
				ChangeCurrentTableWidgetText(_iRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
			}
		}
	}
	if (bPreventReaptUpdateMeasure)														{
		slot_TableWidget_cellClicked(ui.tableWidget->currentRow(),0);
		//UpdateMeasureResult();
	}
}

void frmOutLineDetectTool::slot_JudgeLimit_valueChanged(QtProperty * property, const QVariant & value)
{
	//std::lock_guard<std::mutex> _lock(m_Lock);
	if (property == nullptr)		return;
	QString strName = property->propertyName();
	if (m_JudgeLimitpropertyToId.count(property) > 0)
	{
		QString _strName = m_JudgeLimitpropertyToId[property];
		if (m_JudgeLimitidToProperty.count(m_JudgeLimitpropertyToId[property]) > 0)		{
			int _iRow = ui.tableWidget->currentRow();
			QString	_strData;

			if (CheckSafetableWidget(_iRow, 1))											{
				_strData = ui.tableWidget->item(_iRow, 1)->text();

				QStringList					_strParamArray;
				QStringList					_strLimitArray;
				QStringList					_strLimitValues;
				QStringList					_strLimitParams;
				bool _iIsLimit = false;
				int	_iRegionType = XxwTraceRect::LimitTypeUpAndDown;					{
					GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
					QString _strValue = m_JudgeLimitidToProperty[_strName]->valueText();

					_strLimitParams.clear();
					for (int i = 0; i < m_JudgeLimitpropertyID.size(); i++)
					{
						QVariant _value = m_JudgeLimitidToProperty[m_JudgeLimitpropertyID[i]]->value();
						QString	_strValue;
						if ( m_JudgeLimitidToProperty[	m_JudgeLimitpropertyID[i]	]->propertyType() == QVariant::Double )
						{
							_strValue = QString::number(_value.toDouble(), 'f', databaseVar::Instance().form_System_Precision);
						}
						else
						{
							_strValue = _value.toString();
						}
						if (i < _strLimitParams.size())
						{
							_strLimitParams[i] = _strValue;
						}
						else
						{
							_strLimitParams.push_back( _strValue );
						}
					}
				}
				ChangeCurrentTableWidgetText(_iRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
			}
		}
	}
}

void frmOutLineDetectTool::slot_ChangeMatchImg_Index(int Index)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((OutLineDetectTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((OutLineDetectTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ ui.cbx_ImgInPut->currentText() ]);
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
			for (int i = 0; i < hChannel.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));
		}
	}
}

QList<QPointF> lineRectIntersection(const QLineF &line, const QRectF &rect) {
	QList<QPointF> intersections;
	QPointF rectCorners[4] = {
		rect.topLeft(),
		rect.topRight(),
		rect.bottomRight(),
		rect.bottomLeft()
	};

	for (int i = 0; i < 4; ++i) {
		QLineF edge(rectCorners[i], rectCorners[(i + 1) % 4]);
		QPointF intersection;
		auto _Type = line.intersect(edge,&intersection);
		switch (_Type)	{
		case QLineF::NoIntersection: {
		
		}	break;
		case QLineF::BoundedIntersection: {
			intersections.append(intersection);
		}	break;
		case QLineF::UnboundedIntersection: {
			//intersections.append(intersection);
		}	break;
		default: {

		}	break;
		}
	}

	return intersections;
}

void frmOutLineDetectTool::slot_UpdateImgContour(QList<QGraphicsItem*>& items)
{
	if (items.size() > 0){
		ui.widgetPlot->clearGraphs();
		ui.widgetPlot->rescaleAxes();
		ui.widgetPlot->DrawAllItemClear();
	}
	int		xMinLimit = -1, xMaxLimit = 1;
	double	yMinLimit = -1, yMaxLimit = 1;

	QCPGraph *graph1 = ui.widgetPlot->addGraph();
	ui.widgetPlot->graph(0)->setPen(QPen(Qt::blue,2));
	ui.widgetPlot->graph(0)->setName("Measurement");
	ui.widgetPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
	ui.widgetPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 10));

	if (!m_srcImage.IsInitialized())	return;
	for (int i = 0;i < items.size(); i ++)
	{
		if (items[i] == nullptr)	continue;
		QCPGraph *graph1 = ui.widgetPlot->addGraph();
		graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 0.5), QBrush(Qt::white), 2));

		BaseItem* Item = (BaseItem*)items[i];
		if (Item != nullptr) if (Item->type() == 10) {
			switch (Item->GetItemType())	{
			case ItemCircle: {
				CircleItem *_pitem = (CircleItem*)Item;		MCircle Cir;
				_pitem->GetCircle(Cir);
			}	break;
			case ItemEllipse: {
				EllipseItem *_pitem = (EllipseItem*)Item;	MRotatedRect Cir;
				_pitem->GetRotatedRect(Cir);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_pitem = (ConcentricCircleItem*)Item;		CCircle Cir;
				_pitem->GetConcentricCircle(Cir);
			}	break;
			case ItemRectangle: {
				RectangleItem *_pitem = (RectangleItem*)Item;		MRectangle Cir;
				_pitem->GetRect(Cir);			
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_pitem = (RectangleRItem*)Item;		MRotatedRect Cir;
				_pitem->GetRotatedRect(Cir);
			}	break;
			case ItemSquare: {
			}	break;
			case ItemPolygon: {
				PolygonItem *_pitem = (PolygonItem*)Item;			MPolygon Pointlst;
				_pitem->GetPolygon(Pointlst);
			}	break;
			case ItemLineObj: {
				LineItem *_pitem = (LineItem*)Item;					LineData Cir;
				_pitem->GetLineData(Cir);
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *_pitem = (LineArrowItem*)Item;		LineData Cir;
				_pitem->GetLineData(Cir);
				QVector<double> x, y;
				QVector<double> vecX,vecY,vecZ;
				HObject	RegionLines, Contours;
				HTuple Row,Col, Grayval;

				HTuple  hv_Width, hv_Height, hv_Min, hv_Max, _Range;
				GetImageSize(m_srcImage, &hv_Width, &hv_Height);
				QRectF _rect;
				_rect.setLeft(0);
				_rect.setTop(0);
				_rect.setRight( hv_Width.D() - 1);
				_rect.setBottom( hv_Height.D() - 1);
				QLineF _line;
				_line.setP1(QPointF(Cir.col, Cir.row));
				_line.setP2(QPointF(Cir.col1,Cir.row1));
				
				auto _IntersectPos = lineRectIntersection(_line, _rect);
				if (_IntersectPos.size() == 0)	{
					if (_rect.contains(_line.p1()) || _rect.contains(_line.p2()))	{	} else { return;	}
				}
				else if(_IntersectPos.size() == 1) {
					if ( _rect.contains(_line.p1()) ) {
						Cir.col		= _line.p1().x();
						Cir.row		= _line.p1().y();
						Cir.col1	= _IntersectPos[0].x();
						Cir.row1	= _IntersectPos[0].y();
					}
					else if ( _rect.contains(_line.p2()) )		{
						Cir.col		= _IntersectPos[0].x();
						Cir.row		= _IntersectPos[0].y();
						Cir.col1	= _line.p2().x();
						Cir.row1	= _line.p2().y();
					}
					else {	return;		}
				}
				else if (_IntersectPos.size() == 2) {
					if (_rect.contains(_line.p1())) {
						Cir.col = _line.p1().x();
						Cir.row = _line.p1().y();
						Cir.col1 = _IntersectPos[0].x();
						Cir.row1 = _IntersectPos[0].y();
					}
					else if (_rect.contains(_line.p2())) {
						Cir.col = _IntersectPos[0].x();
						Cir.row = _IntersectPos[0].y();
						Cir.col1 = _line.p2().x();
						Cir.row1 = _line.p2().y();
					}
					else { return; }
				}


				HTuple  Phi, Length1, Length2, hMeasureHandle, Distance, hAngle;
				DistancePp(_line.p1().y(), _line.p1().x(), _line.p2().y(), _line.p2().x(), &Distance);
				AngleLx(_line.p2().y(), _line.p2().x(), _line.p1().y(), _line.p1().x(), &hAngle);
				HalconCpp::GenMeasureRectangle2(_line.center().y(), _line.center().x(), hAngle, Distance / 2.0,
					8 / 2.0, hv_Width, hv_Height, "nearest_neighbor", &hMeasureHandle);
				//测量出灰度
				MeasureProjection(m_srcImage, hMeasureHandle, &m_hGrayval);
				CloseMeasure(hMeasureHandle);
				vecX.clear(); vecY.clear(); vecZ.clear();
				int		iLenth = m_hGrayval.TupleLength().I();
				double	_dRow = (_line.p1().y() - _line.p2().y()) / iLenth;
				double	_dCol = (_line.p1().x() - _line.p2().x()) / iLenth;
				for (int i = 0; i < m_hGrayval.Length(); i++)
				{
					vecX.push_back( _line.p2().x()	+ i * _dCol	);
					vecY.push_back(	_line.p2().y()	+ i * _dRow	);
					vecZ.push_back(m_hGrayval[i].D());
				}

				//将数据转化为绘图要的
				bool _bFirst = true;
				int iGrayLenth = vecZ.size();
				for (int i = 0; i < vecZ.size(); i++)		{
					if (m_bRemovePoint) {	//去除杂点
						if (CheckPoint(vecZ[i]))	{
							x.push_back(i);
							y.push_back(vecZ[i]);
						}
					}
					else	{
						x.push_back(i);
						y.push_back(vecZ[i]);
					}
					if (_bFirst)		{
						if (i == 0)		{
							xMinLimit = i;
							xMaxLimit = iGrayLenth;
							yMinLimit = vecZ[i];
							yMaxLimit = yMinLimit;
						}
						else	{
							xMaxLimit = max(xMaxLimit, iGrayLenth);
							if (vecZ[i] >= -9000)	{
								yMinLimit = min(yMinLimit, vecZ[i]);
							}
							yMaxLimit = max(yMaxLimit, vecZ[i]);
						}
					}
					_bFirst = false;
				}

				QPen pen;
				pen.setColor(QColor(qSin(i * 0.3) * 100 + 100, qSin(i * 0.6 + 0.7) * 100 + 100, qSin(i * 0.4 + 0.6) * 100 + 100));				// generate data:
				pen.setWidth(2);
				ui.widgetPlot->graph(1)->setData(x, y);
				ui.widgetPlot->graph(1)->rescaleAxes(true);
				ui.widgetPlot->graph(1)->setPen(pen);
				ui.widgetPlot->replot();
			}	break;
			}
		}
	}
	int _iIndex = items.size();

	ui.widgetPlot->xAxis->setRange(xMinLimit, xMaxLimit);
	float _fDurLimit = abs(yMaxLimit - yMinLimit) / 10.0;
	ui.widgetPlot->yAxis->setRange(yMinLimit - _fDurLimit, yMaxLimit + _fDurLimit);
	slot_customplot_FitAxis();
	ui.widgetPlot->UpDatePlot();
	UpdateMeasureResult();
}

void frmOutLineDetectTool::slot_customContextMenuRequested(const QPoint & pos)
{
	QMenu*	m_QMenu = new QMenu();

	//设置参数
	QPushButton* moveSetAction_buf = new QPushButton(m_QMenu);
	moveSetAction_buf->setText(tr("Set Param"));
	moveSetAction_buf->setIconSize(QSize(20, 20));
	moveSetAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSetAction = new QWidgetAction(this);
	moveSetAction->setDefaultWidget(moveSetAction_buf);
	connect(moveSetAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_SetParamMode);
	connect(moveSetAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveSetAction);

	//重置区域限制
	QPushButton* moveSetLimitAction_buf = new QPushButton(m_QMenu);
	moveSetLimitAction_buf->setText(tr("Reset Limits"));
	moveSetLimitAction_buf->setIconSize(QSize(20, 20));
	moveSetLimitAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSetLimitAction = new QWidgetAction(this);
	moveSetLimitAction->setDefaultWidget(moveSetLimitAction_buf);
	connect(moveSetLimitAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_ResetLimit);
	connect(moveSetLimitAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveSetLimitAction);
	m_QMenu->addSeparator();

	//显示指示线
	if (m_btnHintAction_buf == nullptr) {
		m_btnHintAction_buf = new QPushButton(m_QMenu);
		m_btnHintAction_buf->setText(tr("Display Indicator"));
		m_btnHintAction_buf->setIconSize(QSize(20, 20));
		m_btnHintAction_buf->setMinimumSize(QSize(120, 40));
		m_btnHintAction_buf->setIcon(QIcon(":/image/icon/images/untick.png"));
		m_btnHintAction_buf->setCheckable(true);
	}

	if (m_btnHintAction == nullptr)		m_btnHintAction = new QWidgetAction(this);
	m_btnHintAction->setCheckable(true);
	//m_btnHintAction->setText("显示指示线");
	m_btnHintAction->setDefaultWidget(m_btnHintAction_buf);
	connect(m_btnHintAction_buf, &QPushButton::pressed, this, [this]() {
		if (m_btnHintAction != nullptr)		{
			if (m_btnHintAction->isChecked())			{
				m_btnHintAction->setChecked(false);
				ui.widgetPlot->showTracer(false);
				m_btnHintAction_buf->setIcon(QIcon(":/image/icon/images/untick.png"));
				ui.widgetPlot->RePlotALLTrace();
			}
			else
			{
				m_btnHintAction->setChecked(true);
				ui.widgetPlot->showTracer(true);
				m_btnHintAction_buf->setIcon(QIcon(":/image/icon/images/tick.png"));
				ui.widgetPlot->RePlotALLTrace();
			}
		}
	});
	connect(m_btnHintAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(m_btnHintAction);
	m_QMenu->addSeparator();

	//缩放
	QPushButton* moveScaleAction_buf = new QPushButton(m_QMenu);
	moveScaleAction_buf->setText(tr("Scale Mode"));
	moveScaleAction_buf->setIconSize(QSize(20, 20));
	moveScaleAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveScaleAction = new QWidgetAction(this);
	moveScaleAction->setDefaultWidget(moveScaleAction_buf);
	connect(moveScaleAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_ZoomMode);
	connect(moveScaleAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveScaleAction);

	//拖动
	QPushButton* moveDragAction_buf = new QPushButton(m_QMenu);
	moveDragAction_buf->setText(tr("Drag Mode"));
	moveDragAction_buf->setIconSize(QSize(20, 20));
	moveDragAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveDragAction = new QWidgetAction(this);
	moveDragAction->setDefaultWidget(moveDragAction_buf);
	connect(moveDragAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_DragMode);
	connect(moveDragAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveDragAction);

	QPushButton* movSelectAction_buf = new QPushButton(m_QMenu);
	movSelectAction_buf->setText(tr("Select Mode"));
	movSelectAction_buf->setIconSize(QSize(20, 20));
	movSelectAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSelectAction = new QWidgetAction(this);
	moveSelectAction->setDefaultWidget(movSelectAction_buf);
	connect(movSelectAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_SelectMode);
	connect(movSelectAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveSelectAction);

	QPushButton* moveFitAction_buf = new QPushButton(m_QMenu);
	moveFitAction_buf->setText(tr("AutoFit"));
	moveFitAction_buf->setIconSize(QSize(20, 20));
	moveFitAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveFitAction = new QWidgetAction(this);
	moveFitAction->setDefaultWidget(moveFitAction_buf);
	connect(moveFitAction_buf, &QPushButton::clicked, this, &frmOutLineDetectTool::slot_customplot_FitAxis);
	connect(moveFitAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveFitAction);

	m_QMenu->exec(	QCursor::pos()	);

	if (moveSetAction_buf != nullptr)		delete moveSetAction_buf;		moveSetAction_buf = nullptr;
	if (moveSetAction != nullptr)			delete moveSetAction;			moveSetAction = nullptr;
	if (moveSetLimitAction_buf != nullptr)	delete moveSetLimitAction_buf;	moveSetLimitAction_buf = nullptr;
	if (moveSetLimitAction != nullptr)		delete moveSetLimitAction;		moveSetLimitAction = nullptr;
	//if (m_btnHintAction_buf != nullptr)		delete m_btnHintAction_buf;		m_btnHintAction_buf = nullptr;
	if (moveScaleAction_buf != nullptr)		delete moveScaleAction_buf;		moveScaleAction_buf = nullptr;
	if (moveScaleAction != nullptr)			delete moveScaleAction;			moveScaleAction = nullptr;
	if (moveDragAction_buf != nullptr)		delete moveDragAction_buf;		moveDragAction_buf = nullptr;
	if (moveDragAction != nullptr)			delete moveDragAction;			moveDragAction = nullptr;
	if (movSelectAction_buf != nullptr)		delete movSelectAction_buf;		movSelectAction_buf = nullptr;
	if (moveSelectAction != nullptr)		delete moveSelectAction;		moveSelectAction = nullptr;
	if (moveFitAction_buf != nullptr)		delete moveFitAction_buf;		moveFitAction_buf = nullptr;
	if (moveFitAction != nullptr)			delete moveFitAction;			moveFitAction = nullptr;
	if (m_QMenu != nullptr)					delete m_QMenu;					m_QMenu = nullptr;
}

void frmOutLineDetectTool::slot_customplot_FitAxis()
{
	ui.widgetPlot->rescaleAxes(true);
	for (int i = 0; i < ui.widgetPlot->graphCount(); i++)
	{
		ui.widgetPlot->graph(i)->rescaleAxes(true);
	}
	ui.widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iMultiSelect);
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->RePlotALLTrace();
}

void frmOutLineDetectTool::slot_customplot_ZoomMode()
{
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
}

void frmOutLineDetectTool::slot_customplot_DragMode()
{
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, true);//使能拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
}

void frmOutLineDetectTool::slot_customplot_SelectMode()
{
	for (int i = 0; i < ui.widgetPlot->graphCount(); i++)
	{
		ui.widgetPlot->graph(i)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
	}
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);
}

void frmOutLineDetectTool::slot_customplot_SetParamMode()
{
	slot_customplot_FitAxis();
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
	ui.widgetPlot->UpDatePlot();
	ui.widgetPlot->RePlotALLTrace();
}

void frmOutLineDetectTool::slot_customplot_ResetLimit()
{
	ui.widgetPlot->ResetALLLimit();
	ui.widgetPlot->UpDatePlot();
	ui.widgetPlot->RePlotALLTrace();
}

void frmOutLineDetectTool::slot_customplot_XxwTraceRect(XxwTraceRect * traceRect)
{
	if (traceRect == Q_NULLPTR)			return;
	//ui.cbx_CheckPlotType->setCurrentIndex(traceRect->m_type);	//设置当前的参数



}

void frmOutLineDetectTool::on_btnSub_clicked()
{
	int rowIndex = ui.tableWidget->currentRow();
	if (rowIndex >= 0)	{
		{
			int _iROIRow	= ui.tableROIWidget->currentIndex().row();
			int _iCheckRow	= ui.tableWidget->currentIndex().row();
			if (_iROIRow	>= 0 && _iROIRow < m_VecCheckOutLineData.size())	{
				auto& iter = m_VecCheckOutLineData[_iROIRow];
				if (_iCheckRow < iter.vecData.size() && _iCheckRow >= 0)
					iter.vecData.erase(iter.vecData.begin() + _iCheckRow);
			}
		}
		QString	_strType, _strName, _strData;
		if (CheckSafetableWidget(rowIndex, 0))	{
			_strName = ui.tableWidget->item(rowIndex, 0)->text();
		}
		if (CheckSafetableWidget(rowIndex, 1))	{
			_strData = ui.tableWidget->item(rowIndex, 1)->text();
		}

		QStringList _strArray = _strName.split("_");
		int _iIndex = -1;
		if(_strArray.size() > 1) _iIndex = _strArray[1].toInt();
		_strType	= _strArray[0];
		for (size_t i = 0; i < m_MapVectorParam[_strType].size(); i++)	{
			if (_iIndex == m_MapVectorParam[_strType][i])	{
				m_MapVectorParam[_strType].erase(m_MapVectorParam[_strType].begin() + i); break;
			}
		}
		ui.tableWidget->removeRow(rowIndex);
		ui.tableWidget->selectRow(rowIndex - 1);
		slot_TableWidget_cellClicked( rowIndex - 1, 0 );

	}

}

void frmOutLineDetectTool::on_btnMoveUp_clicked()
{
	{
		int _iROIRow = ui.tableROIWidget->currentIndex().row();
		int _iCheckRow = ui.tableWidget->currentIndex().row();
		if (_iROIRow >= 0
			&& _iROIRow < m_VecCheckOutLineData.size())
		{
			auto& iter = m_VecCheckOutLineData[_iROIRow];
			if (_iCheckRow < iter.vecData.size() && _iCheckRow >=0)
			{
				auto RowValue = iter.vecData[_iCheckRow];
				iter.vecData.removeAt(_iCheckRow);
				if ((_iCheckRow ) > 0)
					iter.vecData.insert(_iCheckRow - 1 , RowValue);
				else if ((_iCheckRow ) == 0)
					iter.vecData.insert(_iCheckRow, RowValue);
			}

		}
	}

	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow - 1);
}

void frmOutLineDetectTool::on_btnMoveDown_clicked()
{
	{
		int _iROIRow = ui.tableROIWidget->currentIndex().row();
		int _iCheckRow = ui.tableWidget->currentIndex().row();

		if (   _iROIRow >= 0 
			&& _iROIRow < m_VecCheckOutLineData.size())
		{
			auto& iter = m_VecCheckOutLineData[_iROIRow];
			if (_iCheckRow < iter.vecData.size() && _iCheckRow >= 0)
			{
				auto RowValue = iter.vecData[_iCheckRow];

				int _iRowCount = iter.vecData.size();
				iter.vecData.removeAt(_iCheckRow);
				if (	_iCheckRow  <  (_iRowCount - 1)	 )
					iter.vecData.insert(_iCheckRow + 1, RowValue);
				else if(_iCheckRow == (_iRowCount - 1) )
					iter.vecData.insert(_iCheckRow, RowValue);
			}
		}
	}

	int nRow = ui.tableWidget->currentRow();
	moveRow(ui.tableWidget, nRow, nRow + 2);
}

void frmOutLineDetectTool::slot_Menu_ADDAction()
{
	QAction*_pAction	= (QAction*)sender();
	QString _strText	= _pAction->data().toString();
	int count			= ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(count + 1);  //设置行数

	//当前顺序
	QVector<int> vecLst = m_MapVectorParam[_strText];
	std::sort(vecLst.begin(), vecLst.end());     //去重前需要排序
	int _iIndex = m_MapVectorParam[_strText].size() + 1;
	for (size_t i = 0; i < m_MapVectorParam[_strText].size(); i++)	{
		if (vecLst[i] != (i + 1))		{
			_iIndex = (i + 1);
			break;
		}
	}
	m_MapVectorParam[_strText].push_back(_iIndex);
	QString	_strName	= _strText;
	QString	_strData	= _strText;

	{	//类型
		_strName		= QString("%1_%2").arg(_strText).arg(QString::number(_iIndex));
		QTableWidgetItem*	_item = new QTableWidgetItem();
		_item->setData(0, _strText);
		_item->setTextAlignment(Qt::AlignCenter);
		_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		_item->setText(_strName);
		ui.tableWidget->setItem(count, 0, _item);
	}

	QStringList			_strParamArray;	//参数
	QStringList			_strLimitArray;
	QStringList			_strLimitValues;
	QStringList			_strLimitParams;
	bool _iIsLimit		= false;
	int	_iRegionType	= XxwTraceRect::LimitTypeUpAndDown;

	if (_strText == (PortCheckTypeMap[EnumTypePoint]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeRange;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item	= new QTableWidgetItem(_strData);
			_item->setData(1,_strText);
			_item->setText(_strData1);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypePointPointCenter]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item	= new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineCross]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,	_strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineContourCross]))
	{
		QString _strData1;
		_strParamArray.append(QString::number(0));
		_iRegionType = XxwTraceRect::LimitTypeRange;
		_strLimitArray.append(QString::number(0));
		_strLimitArray.append(QString::number(0));
		_strLimitArray.append(QString::number(0));
		_strLimitArray.append(QString::number(0));
		_iIsLimit = false;
		SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
		_iIsLimit = true;
		SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

		QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
		_item->setData(1, _strText);
		//_item->setTextAlignment(Qt::AlignCenter);
		_item->setText(_strData1);
		_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
		ui.tableWidget->setItem(count, 1, _item);
		//m_VecData.push_back(_strData1);
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLine]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeRange;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeHorizontalLine]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeRange;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineLineCross]))
	{

	}
	else if (_strText == (PortCheckTypeMap[EnumTypeCircle]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeRange;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypePointPointLine]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,_strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeverticalLine]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType	= XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypepointStroke]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineCircleCross]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,  _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	//测量
	else if (_strText == (PortCheckTypeMap[EnumTypeHeightDifference]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));

			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));

			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineContourDistance]))
	{
		

	}
	else if (_strText == (PortCheckTypeMap[EnumTypeWidth]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			//_item->setTextAlignment(Qt::AlignCenter);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeSectionalArea]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);

			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	//计算
	else if (_strText == (PortCheckTypeMap[EnumTypePointPointDistance]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);
			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypePointLineDistance]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);
			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineLineAngle]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitParams.append(QString::number(0.5));
			_strLimitParams.append(QString::number(-0.5));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);
			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}
	else if (_strText == (PortCheckTypeMap[EnumTypeLineContourCrossNum]))
	{
		{	//参数
			QString _strData1;
			_strParamArray.append(QString::number(0));
			_strParamArray.append(QString::number(0));
			_iRegionType = XxwTraceRect::LimitTypeNone;
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitArray.append(QString::number(0));
			_strLimitValues.append(QString::number(0));
			_iIsLimit = false;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData1);
			_iIsLimit = true;
			SetTableWidgetText(_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, _strData);
			QTableWidgetItem*	_item = new QTableWidgetItem(_strData);
			_item->setData(1, _strText);
			_item->setText(_strData1);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			//m_VecData.push_back(_strData1);
		}
	}

	{
		int _iROIRow = ui.tableROIWidget->currentIndex().row();
		int _iCheckRow = ui.tableWidget->rowCount();
		if (_iROIRow >= 0 && _iROIRow < m_VecCheckOutLineData.size() && _iCheckRow >= 0)
		{
			auto& iter = m_VecCheckOutLineData[_iROIRow];
			if (_iCheckRow < iter.vecData.size())
			{
				iter.vecData[_iCheckRow].strData = _strData;
				iter.vecData[_iCheckRow].strName = _strName;
			}
			else if (_iCheckRow == (iter.vecData.size() + 1))
			{
				CheckString _LineData;
				_LineData.strData = _strData;
				_LineData.strName = _strName;
				iter.vecData.push_back(_LineData);
			}
			else
			{
				iter.vecData.erase(iter.vecData.begin() + _iCheckRow, iter.vecData.end());
			}
		}
	}

	ChangeParamProperty(_strName, _strData);
}

void frmOutLineDetectTool::slot_TableWidget_cellClicked(int row, int column)
{
	QString	 _strName, _strData;

	if (CheckSafetableWidget(row, 0))
	{
		_strName = ui.tableWidget->item(row, 0)->text();
	}
	if (CheckSafetableWidget(row, 1))
	{
		_strData = ui.tableWidget->item(row, 1)->text();
	}
	ChangeParamProperty(_strName, _strData);

	ChangeJudgeLimitProperty(_strName, _strData);
}

void frmOutLineDetectTool::slot_ROITableWidget_cellClicked(int row, int column)
{
	for (int iRow = 0; iRow < m_VecBaseItem.size(); iRow++)	{
		m_VecBaseItem[iRow]->SelectItem();
		m_VecBaseItem[iRow]->setSelected(false);
		m_VecBaseItem[iRow]->m_bSelected = false;
		m_VecBaseItem[iRow]->clearFocus();
	}
	if (row < m_VecBaseItem.size())		{
		m_VecBaseItem[row]->m_bSelected = true;
		m_VecBaseItem[row]->SelectItem(true);
		m_VecBaseItem[row]->setSelected(true);
		m_VecBaseItem[row]->setFocus();

		//参数
		int _iSize		= m_VecCheckOutLineData.size();
		int	iRowCount	= ui.tableWidget->rowCount();
		for (int i	= 0; i < iRowCount; i++)	ui.tableWidget->removeRow(0);  //设置行数

		{	CheckOutLineData& _RoiData	= m_VecCheckOutLineData[row];
			if (_RoiData.vecData.size() <= 0)	{
				ClearParamProperty();
			}
			for (size_t itemRow = 0; itemRow < _RoiData.vecData.size(); itemRow++)	{
				auto& item = _RoiData.vecData[itemRow];

				int count = ui.tableWidget->rowCount();
				ui.tableWidget->setRowCount(count + 1);  //设置行数

				m_MapVectorParam.clear();
				//m_VecData.clear();
				QString	_strName = item.strName;
				QStringList _strNameArray = _strName.split("_");
				m_MapVectorParam[_strNameArray[0]].push_back(_strNameArray[1].toInt());

				{	QTableWidgetItem* _item = new QTableWidgetItem(_strName);
					_item->setTextAlignment(Qt::AlignCenter);
					_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
					ui.tableWidget->setItem(count, 0, _item);
				} {	_strName = item.strData;
					QTableWidgetItem* _item = new QTableWidgetItem(_strName);
					//m_VecData.push_back(_strName);
					_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
					ui.tableWidget->setItem(count, 1, _item);
				}
			}
		}

		ui.tableWidget->selectRow(0);
		{	std::lock_guard<std::mutex> _lock(m_Lock);
			QList<QGraphicsItem*> _Items;
			_Items.append(m_VecBaseItem[row]);
			ChangeGraphicsItemInFo(_Items);
		}
		//刷新检测项目
	}
}

void frmOutLineDetectTool::slot_TableWidget_pressed(const QModelIndex & index)
{
	int row = index.row();
	QString	 _strName, _strData;
	if (CheckSafetableWidget(row, 0))
	{
		_strName = ui.tableWidget->item(row, 0)->text();
	}
	if (CheckSafetableWidget(row, 1))
	{
		_strData = ui.tableWidget->item(row, 1)->text();
	}
	ChangeParamProperty(_strName, _strData);

	ChangeJudgeLimitProperty(_strName, _strData);
}

void frmOutLineDetectTool::slot_ROITableWidget_pressed(const QModelIndex & index)
{
	int row = index.row();
	for (int iRow = 0; iRow < m_VecBaseItem.size(); iRow++) {
		m_VecBaseItem[iRow]->SelectItem();
		m_VecBaseItem[iRow]->setSelected(false);
		m_VecBaseItem[iRow]->m_bSelected = false;
		m_VecBaseItem[iRow]->clearFocus();
	}
	if (row < m_VecBaseItem.size() && row >= 0) {
		m_VecBaseItem[row]->m_bSelected = true;
		m_VecBaseItem[row]->SelectItem(true);
		m_VecBaseItem[row]->setSelected(true);
		m_VecBaseItem[row]->setFocus();

		//参数
		int _iSize = m_VecCheckOutLineData.size();
		int	iRowCount = ui.tableWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)	ui.tableWidget->removeRow(0);  //设置行数

		{	CheckOutLineData& _RoiData = m_VecCheckOutLineData[row];
		if (_RoiData.vecData.size() <= 0) {
			ClearParamProperty();
		}
		for (size_t itemRow = 0; itemRow < _RoiData.vecData.size(); itemRow++) {
			auto& item = _RoiData.vecData[itemRow];

			int count = ui.tableWidget->rowCount();
			ui.tableWidget->setRowCount(count + 1);  //设置行数

			m_MapVectorParam.clear();
			//m_VecData.clear();
			QString	_strName = item.strName;
			QStringList _strNameArray = _strName.split("_");
			m_MapVectorParam[_strNameArray[0]].push_back(_strNameArray[1].toInt());

			{	QTableWidgetItem* _item = new QTableWidgetItem(_strName);
			_item->setTextAlignment(Qt::AlignCenter);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 0, _item);
			} {	_strName = item.strData;
			QTableWidgetItem* _item = new QTableWidgetItem(_strName);
			//m_VecData.push_back(_strName);
			_item->setFlags(_item->flags() & (~Qt::ItemIsEditable));  //设置列不可编辑	
			ui.tableWidget->setItem(count, 1, _item);
			}
		}
		}

		ui.tableWidget->selectRow(0);
		{	std::lock_guard<std::mutex> _lock(m_Lock);
		QList<QGraphicsItem*> _Items;
		_Items.append(m_VecBaseItem[row]);
		ChangeGraphicsItemInFo(_Items);
		}
		//刷新检测项目
	}
}

void frmOutLineDetectTool::slot_TableWidget_ChangedTraceRect(XxwTraceRect * traceRect)
{
	int _iRow = ui.tableWidget->currentRow();
	QString	_strData;
	if(CheckSafetableWidget(_iRow, 1))
	{
		_strData = ui.tableWidget->item(_iRow, 1)->text();

		QStringList			_strParamArray;
		QStringList			_strLimitArray;
		QStringList			_strLimitValues;
		QStringList			_strLimitParams;
		bool _iIsLimit		= false;
		int	_iRegionType	= XxwTraceRect::LimitTypeUpAndDown;
		{
			GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);

			GetLimitParam(0, _iRegionType, _iIsLimit, _strLimitArray);
		}
		ChangeCurrentTableWidgetText(_iRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
	}
	if (bPreventReaptUpdateMeasure)
		UpdateMeasureResult();
}

void frmOutLineDetectTool::slot_TableWidget_ChangedItemLine(XxwDetectItemLine * ItemLine)
{
	int _iRow = ui.tableWidget->currentRow();
	QString	_strData;
	if (CheckSafetableWidget(_iRow, 1))
	{
		QString		_strName		= ui.tableWidget->item(_iRow, 0)->text();
		QStringList _strNameArray	= _strName.split("_");
		QString	_strCheckType		= _strNameArray[0];

		if (!( (_strCheckType == (PortCheckTypeMap[EnumTypeHorizontalLine])	) || (_strCheckType == (PortCheckTypeMap[EnumTypeLine])	) ))	return;

		_strData = ui.tableWidget->item(_iRow, 1)->text();

		QStringList			_strParamArray;
		QStringList			_strLimitArray;
		QStringList			_strLimitValues;
		QStringList			_strLimitParams;
		bool _iIsLimit		= false;
		int	_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		{
			GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
			_strParamArray.clear();

			switch (ItemLine->m_type)
			{
			case VerticalLine: //垂直线
			{
				if (ItemLine->m_pItemLine)
				{
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->start->coords().x(), 'f', databaseVar::Instance().form_System_Precision));
					ChangeParamValue("X", _strParamArray[0]);
				}
			}	break;
			case HorizonLine: //水平线
			{
				if (ItemLine->m_pItemLine)
				{
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->start->coords().y(), 'f', databaseVar::Instance().form_System_Precision));
					ChangeParamValue("Y", _strParamArray[0]);
				}
			}	break;
			case Both: 		//同时显示水平和垂直线
				break;
			case None: 		//无需任何
			default:
			{
				if (ItemLine->m_pItemLine)
				{
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->start->coords().x(),'f', databaseVar::Instance().form_System_Precision));
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->start->coords().y(), 'f', databaseVar::Instance().form_System_Precision));
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->end->coords().x(), 'f', databaseVar::Instance().form_System_Precision));
					_strParamArray.push_back(QString::number(ItemLine->m_pItemLine->end->coords().y(), 'f', databaseVar::Instance().form_System_Precision));

					ChangeParamValue("X1", _strParamArray[0]);
					ChangeParamValue("Y1", _strParamArray[1]);
					ChangeParamValue("X2", _strParamArray[2]);
					ChangeParamValue("Y2", _strParamArray[3]);
				}
			}	break;
			}
		}
		ChangeCurrentTableWidgetText(_iRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
	}

	UpdateMeasureResult();
}

void frmOutLineDetectTool::slot_ItemEnter(QTableWidgetItem * item)
{
	if (item == nullptr) return;
	QToolTip::showText(QCursor::pos(), item->text());
}

void frmOutLineDetectTool::ChangeJudgeLimitProperty(QString strName, QString strData)
{
	std::lock_guard<std::mutex> _lock(m_Lock);
	for (auto iter : m_JudgeLimitidToProperty)	if (iter != nullptr)	delete iter;	m_JudgeLimitidToProperty.clear();
	m_JudgeLimitpropertyToId.clear();
	m_JudgeLimitpropertyID.clear();
	int _iCurrentRow = ui.tableWidget->currentRow();
	//m_ParamidToExpanded.clear();
	m_JudgeLimitvariantManager->clear();
	m_JudgeLimitpropertyEditor->clear();
	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QStringList			_strArray = strName.split("_");
	QStringList			_strParamArray;
	QStringList			_strLimitArray;
	QStringList			_strLimitValues;
	QStringList			_strLimitParams;
	bool _iIsLimit = false;
	int	_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
	if (_strArray.size() != 2 || _strArray.size() < 0) return;
	int				_iShowSearchRect = 1;
	int				_iShowLimitType[2] = { 1,1 };

	{	//提取参数
		GetTableWidgetText(strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues,_strLimitParams);
	}
	if (_strArray[0] == (PortCheckTypeMap[EnumTypePoint]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointCenter]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineCross]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourCross]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLine]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeHorizontalLine]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineLineCross]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeCircle]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointLine]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeverticalLine]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypepointStroke]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineCircleCross]))
	{
		int _iParamIndex = 0;
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeHeightDifference]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(),'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		}	{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourDistance]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeWidth]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}

	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeSectionalArea]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointDistance]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointLineDistance]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineLineAngle]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(),'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourCrossNum]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
			_iParamIndex++;
		} {
			_ptrproperty = m_JudgeLimitvariantManager->addProperty(QVariant::Double);
			if (_strLimitParams.size() > _iParamIndex) _ptrproperty->setValue(QString::number(_strLimitParams[_iParamIndex].toDouble(), 'f', databaseVar::Instance().form_System_Precision)); else _ptrproperty->setValue(QString::number(-0.5));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			_ptrproperty->setAttribute(QLatin1String("singleStep"), 0.1);
			addJudgeLimitProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
			_iParamIndex++;
		}
	}
}

void frmOutLineDetectTool::ChangeParamProperty(QString strName,QString strData)
{
	//std::lock_guard<std::mutex> _lock(m_Lock);
	ClearParamProperty();
	int _iCurrentRow = ui.tableWidget->currentRow();
	QtVariantProperty *	_ptrproperty		= nullptr;
	QtVariantProperty *	_ptrSubproperty		= nullptr;
	QStringList			_strArray			= strName.split("_");
	QStringList			_strParamArray;
	QStringList			_strLimitArray;
	QStringList			_strLimitValues;
	QStringList			_strLimitParams;
	bool _iIsLimit = false;
	int	_iRegionType	= XxwTraceRect::LimitTypeUpAndDown;
	if (_strArray.size() != 2 || _strArray.size() < 0) return;
	int				_iShowSearchRect	= 1;
	int				_iShowLimitType[2] = {1,1};

	{	//提取参数
		GetTableWidgetText(strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
	}
	bPreventReaptUpdateMeasure = false;
	if (_strArray[0] == (PortCheckTypeMap[EnumTypePoint]))
	{
		_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
		_ptrproperty->setPropertyName(tr("PointType"));
		QStringList enumNames;
		enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);

		if (_strParamArray.size() > 0)	{	_ptrproperty->setValue(_strParamArray[0].toInt());	}
		addParamProperty(_ptrproperty, "PointType");
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointCenter]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			_ptrproperty->setPropertyName(tr("PointA"));
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "PointA");

			//关联点A的1
			if (_strParamArray[_iParamIndex].toInt() == 5)
			{
				_iParamIndex++;
				_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
				QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
				_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
				if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
				addParamProperty(_ptrSubproperty, "ConnectA");
				_ptrSubproperty->setPropertyName(tr("ConnectA"));
				_iShowLimitType[0] = 0;
				//ChangeCorrelationPoint("关联点A", _strParamArray[_iParamIndex]);
			}
			else
			{
				DeleteParamProperty("ConnectA");
			}
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点" << "关联";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "PointB");
			_ptrproperty->setPropertyName(tr("PointB"));

			if (_strParamArray.size() > _iParamIndex)
				if (_strParamArray[_iParamIndex].toInt() == 5)
				{
					_iParamIndex++;
					_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
					_ptrSubproperty->setPropertyName(tr("ConnectB"));
					QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
					_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
					if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
					addParamProperty(_ptrSubproperty, "ConnectB");
					_ptrSubproperty->setPropertyName(tr("ConnectB"));
					_iShowLimitType[1] = 0;
					//ChangeCorrelationPoint("关联点B", _strParamArray[_iParamIndex]);
				}
				else
				{
					DeleteParamProperty("ConnectB");
				}
		}
		if ((_iShowLimitType[0] == 1) && (_iShowLimitType[1] == 1))
		{
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineCross]))
	{
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << "tukey" << "huber" << "drop";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "LineAFitType");
			_ptrproperty->setPropertyName(tr("LineAFitType"));
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << "tukey" << "huber" << "drop";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "LineBFitType");
			_ptrproperty->setPropertyName(tr("LineBFitType"));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourCross]))
	{
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "上限");
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			_ptrproperty->setPropertyName(tr("DownLimit"));
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLine]))
	{
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "拟合方式");
			QStringList enumNames;
			enumNames << "tukey" << "huber" << "drop";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > 0) _ptrproperty->setValue(_strParamArray[0].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "FitType");
			_ptrproperty->setPropertyName(tr("FitType"));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeHorizontalLine]))
	{
		_iShowSearchRect = 0;
		{
			_ptrproperty	=	m_ParamvariantManager->addProperty(QVariant::Double, "Y");
			if (_strParamArray.size() > 0) _ptrproperty->setValue(QString::number(_strParamArray[0].toDouble())); else _ptrproperty->setValue(QString::number(0));
			_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
			addParamProperty(_ptrproperty, "Y");

			if (ui.widgetPlot->m_XxwTraceItemLineLst.size() <= 0)	ui.widgetPlot->m_XxwTraceItemLineLst.push_back(new XxwDetectItemLine(ui.widgetPlot));
			QLineF _line;
			_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _strParamArray[0].toDouble()));
			_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _strParamArray[0].toDouble()));
			ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line,true);
			ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = HorizonLine;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineLineCross]))
	{
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "上限");
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "UpLimit");
			_ptrproperty->setPropertyName(tr("UpLimit"));
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "下限");
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";;
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "DownLimit");
			_ptrproperty->setPropertyName(tr("DownLimit"));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeCircle]))
	{
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << "algebraic" << "ahuber" << "atukey" << "geometric" << "geohuber" << "geotukey";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "FitType");
			_ptrproperty->setPropertyName(tr("FitType"));

		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QVariant::Int);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, "ClipNumber");
			_ptrproperty->setPropertyName(tr("ClipNumber"));
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QVariant::Int);
			_ptrproperty->setValue(5);
			addParamProperty(_ptrproperty, "Iterations");
			_ptrproperty->setPropertyName(tr("Iterations"));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointLine]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "A");

			//关联点A的1
			if (_strParamArray[_iParamIndex].toInt() == 5)
			{
				_iParamIndex++;
				_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
				QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
				_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
				if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
				addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "A");
				_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "A");
				_iShowLimitType[0] = 0;
			}
			else	{
				DeleteParamProperty(PortCheckTypeMap[EnumTypeConnect] + "A");
			}
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "B");

			if (_strParamArray.size() > _iParamIndex)
				if (_strParamArray[_iParamIndex].toInt() == 5)	{
					_iParamIndex++;
					_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
					QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
					_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
					if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
					addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "B");
					_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "B");
					_iShowLimitType[1] = 0;
				}
				else	{
					DeleteParamProperty(	PortCheckTypeMap[EnumTypeConnect] + "B"	);
				}
		}
		if ((_iShowLimitType[0] == 1) && (_iShowLimitType[1] == 1))
		{
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		}

	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeverticalLine]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) + "A");

			//addParamProperty(_ptrproperty, "链接点A");
		}	{
			_iParamIndex++;
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + PortCheckTypeMap[EnumTypeLine] );
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) +
				tr(PortCheckTypeMap[EnumTypeLine].toStdString().c_str()));

		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypepointStroke]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) + "A");
			//addParamProperty(_ptrproperty, "链接点A");
		}	{
			_iParamIndex++;
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLine]	);
			_ptrproperty->setPropertyName(	tr(PortCheckTypeMap[EnumTypeLine].toStdString().c_str())	);
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineCircleCross]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty,PortCheckTypeMap[EnumTypeLine]);
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLine].toStdString().c_str()));
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(	QtVariantPropertyManager::enumTypeId()	);
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeCircle], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeCircle]);
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeCircle].toStdString().c_str()));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeHeightDifference]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "点A");
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点" << "关联";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "A");
			//关联点A的1
			if (_strParamArray[_iParamIndex].toInt() == 5)	{
				_iParamIndex++;
				_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
				QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
				_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
				if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
				addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "A");
				_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "A");
				_iShowLimitType[0] = 0;
				//ChangeCorrelationPoint("关联点A", _strParamArray[_iParamIndex]);
			}
			else	{
				DeleteParamProperty(	PortCheckTypeMap[EnumTypeConnect] + "A"	);
			}
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点" << "关联";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "B");

			if (_strParamArray.size() > _iParamIndex)
				if (_strParamArray[_iParamIndex].toInt() == 5)	{
					_iParamIndex++;
					_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
					QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
					_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
					if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
					addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "B");
					_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "B");
					_iShowLimitType[1] = 0;
				}
				else
				{
					DeleteParamProperty(PortCheckTypeMap[EnumTypeConnect] + "B");
				}
		}
		if ((_iShowLimitType[0] == 1) && (_iShowLimitType[1] == 1))
		{
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourDistance]))
	{


	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeWidth]))
	{
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点" << "关联";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "A");

			//关联点A的1
			if (_strParamArray[_iParamIndex].toInt() == 5)
			{
				_iParamIndex++;
				_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
				QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
				_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
				if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
				addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "A");
				_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "A");
				_iShowLimitType[0] = 0;
				//ChangeCorrelationPoint("关联点A", _strParamArray[_iParamIndex]);
			}
			else
			{
				DeleteParamProperty(PortCheckTypeMap[EnumTypeConnect] + "A");
			}
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点" << "关联";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypePoint] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str()) + "B");

			if (_strParamArray.size() > _iParamIndex)
				if (_strParamArray[_iParamIndex].toInt() == 5)				{
					_iParamIndex++;
					_ptrSubproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
					QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
					_ptrSubproperty->setAttribute(QLatin1String("enumNames"), enumNames);
					if (_strParamArray.size() > _iParamIndex) _ptrSubproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrSubproperty->setValue(0);
					addParamProperty(_ptrSubproperty, PortCheckTypeMap[EnumTypeConnect] + "B");
					_ptrSubproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeConnect].toStdString().c_str()) + "B");
					_iShowLimitType[1] = 0;
				}
				else
				{
					DeleteParamProperty(PortCheckTypeMap[EnumTypeConnect] + "B");
				}
		}
		if ((_iShowLimitType[0] == 1) && (_iShowLimitType[1] == 1))
		{
			_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeSectionalArea])) {
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeUpLimit]);
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeUpLimit].toStdString().c_str()));
		}
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("Maxinize") << tr("Mininize") << tr("Median") << tr("Mean") << tr("Turnpoint") << tr("AssociativePoint");
			//enumNames << "最大" << "最小" << "中值" << "均值" << "拐点";
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			_ptrproperty->setValue(0);

			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeDownLimit]);
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeDownLimit].toStdString().c_str()));
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointPointDistance])) {
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);

			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) + "A");

			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);

			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) + "B");

			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("AllDistance") << tr("XDistance") << tr("YDistance");
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeCalculateMode] );
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeCalculateMode].toStdString().c_str()) );
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypePointLineDistance])) {
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink]  + PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str())
				+ tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str())
				+ "A");
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + PortCheckTypeMap[EnumTypeLine] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str()) 
				+ tr(PortCheckTypeMap[EnumTypeLine].toStdString().c_str())
				+ "B");
			_iParamIndex++;
		} {
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames;
			enumNames << tr("AllDistance") << tr("XDistance") << tr("YDistance");
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeCalculateMode]);
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeCalculateMode].toStdString().c_str()));
			_iParamIndex++;
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineLineAngle])) {
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str())
				+ tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str())
				+ "A");
			_iParamIndex++;
		}	{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + PortCheckTypeMap[EnumTypePoint] + "B");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str())
				+ tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str())
				+ "B");
		}
	}
	else if (_strArray[0] == (PortCheckTypeMap[EnumTypeLineContourCrossNum])) {
		int _iParamIndex = 0;
		{
			_ptrproperty = m_ParamvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
			QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], _iCurrentRow);
			_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
			if (_strParamArray.size() > _iParamIndex) _ptrproperty->setValue(_strParamArray[_iParamIndex].toInt()); else _ptrproperty->setValue(0);
			addParamProperty(_ptrproperty, PortCheckTypeMap[EnumTypeLink] + PortCheckTypeMap[EnumTypePoint] + "A");
			_ptrproperty->setPropertyName(tr(PortCheckTypeMap[EnumTypeLink].toStdString().c_str())
				+ tr(PortCheckTypeMap[EnumTypePoint].toStdString().c_str())
				+ "A");
			_iParamIndex++;
		}
	}

	ui.widgetPlot->SetSearchRectVisable();
	ui.widgetPlot->SetTraceItemLineVisable();
	if ((_iShowLimitType[0] == 0) && (_iShowLimitType[1] == 0))
		_iRegionType = XxwTraceRect::LimitTypeNone;
	else if ((_iShowLimitType[0] == 1) && (_iShowLimitType[1] == 0))
		_iRegionType = XxwTraceRect::LimitTypeUp;
	else if ((_iShowLimitType[0] == 0) && (_iShowLimitType[1] == 1))
		_iRegionType = XxwTraceRect::LimitTypeDown;
	ChangeLimitParam(0,(XxwTraceRect::LimitType)_iRegionType, _iIsLimit,_strLimitArray);		//设置限制区域

	slot_TableWidget_ChangedTraceRect(ui.widgetPlot->m_XxwLimitRectLst[0]);
	switch (_iShowSearchRect){
	case 0: {	//用于显示直线
		if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
	}	break;
	case 1: {	//限制区域
		if (ui.widgetPlot->m_XxwLimitRectLst.size() > 0){
			ui.widgetPlot->m_XxwLimitRectLst[0]->setVisible(true);
		}
	}	break;
	default:	break;
	}
	bPreventReaptUpdateMeasure = true;
	if (bPreventReaptUpdateMeasure)
		UpdateMeasureResult();

	ui.widgetPlot->RePlotALLTrace();
}

void frmOutLineDetectTool::UpdateMeasureResult()
{
	int _iCurrentRow = ui.tableWidget->currentRow();
	QString	_strData;
	QString	_strCheckType;
	if(CheckSafetableWidget(_iCurrentRow,0) && CheckSafetableWidget(_iCurrentRow, 1)	)
	{
		QString		_strName				= ui.tableWidget->item(_iCurrentRow, 0)->text();
		QStringList _strNameArray			= _strName.split("_");
		_strCheckType						= _strNameArray[0];

		_strData							= GetTableWidgetText(_iCurrentRow);//ui.tableWidget->item(_iCurrentRow, 1)->text();
		QStringList							_strParamArray;	//参数
		QStringList							_strLimitArray;
		QStringList							_strLimitValues;
		QStringList							_strLimitParams;
		bool _iIsLimit = false;
		int	_iRegionType					= XxwTraceRect::LimitTypeUpAndDown;
		{
			GetTableWidgetText(_iCurrentRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);

			if (ui.widgetPlot->graph(0)		!= nullptr)
			{
				QVector<double>		x, y;
				QVector<QString>	strVecName;
				QVector<QPen>		strVecQPen;
				int					_iRetn;

				ui.widgetPlot->DrawAllItemClear();
				//点
				if (_strCheckType == (PortCheckTypeMap[EnumTypePoint]))
				{
					int		_iCheckType		= _strParamArray[0].toInt();
					double	_dCheckvalue	= _strLimitArray[0].toDouble();
					double	_dCheckRange	= _strLimitArray[1].toDouble();
					_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
					if (_iRetn < 0)		return;
					strVecName.push_back(QString("%1(%2,%3)")
						.arg(tr("Point"))
						.arg(QString::number(x[0], 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(y[0], 'f', databaseVar::Instance().form_System_Precision)));
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypePointPointCenter]))
				{
					int _iParamIndex = 0;
					{
						int		_iCheckType = _strParamArray[_iParamIndex].toInt();
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[0].toDouble();
							double	_dCheckRange = _strLimitArray[1].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}	{
						int		_iCheckType = _strParamArray.size() > 1 ? _strParamArray[_iParamIndex].toInt() : 0;
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[2].toDouble();
							double	_dCheckRange = _strLimitArray[3].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}
					QPointF _qPtCenter = QPointF((x[0] + x[1]) / 2.0, (y[0] + y[1]) / 2.0);

					x.push_back(_qPtCenter.x());
					y.push_back(_qPtCenter.y());

					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointA"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointB"));
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("%1(%2,%3)")
						.arg(tr("Center"))
						.arg(QString::number(_qPtCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_qPtCenter.y(),'f', databaseVar::Instance().form_System_Precision))	);
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineCross]))
				{
					QLineF	_qline1F, _qline2F;
					QVector<QPointF> _vecPos;
					QVector<QString> _vecString;
					QPointF _qPtCenter;
					{
						int		_iCheckType		= _strParamArray[0].toInt();
						double	_dCheckvalue	= _strLimitArray[0].toDouble();
						double	_dCheckRange	= _strLimitArray[1].toDouble();
						if (CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline1F) >= 0)
						{
							ui.widgetPlot->DrawItemLines(0, _qline1F, tr("LineA"));
						}
					}	
					{
						int		_iCheckType		= _strParamArray[1].toInt();
						double	_dCheckvalue	= _strLimitArray[2].toDouble();
						double	_dCheckRange	= _strLimitArray[3].toDouble();
						if (CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline2F) >= 0)
						{
							ui.widgetPlot->DrawItemLines(1, _qline2F, tr("LineB"));
						}
					}
					QLineF::IntersectType _type = _qline1F.intersect(_qline2F,&_qPtCenter);
					switch (_type)
					{
					case QLineF::NoIntersection: {	}	break;	//无交叉点
					case QLineF::BoundedIntersection:	//边界交叉点
					{
						_vecPos.push_back(_qPtCenter);
						_vecString.push_back(QString("%1(%2,%3)")
							.arg(tr("Cross"))
							.arg(QString::number(_qPtCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
							.arg(QString::number(_qPtCenter.y(), 'f', databaseVar::Instance().form_System_Precision)));
						strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					}	break;
					case QLineF::UnboundedIntersection:	//交点
					{
						_vecPos.push_back(_qPtCenter);
						_vecString.push_back(QString("%1(%2,%3)")
							.arg(tr("Cross"))
							.arg(QString::number(_qPtCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
							.arg(QString::number(_qPtCenter.y(), 'f', databaseVar::Instance().form_System_Precision)));
						strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					}	break;
					default:
						break;
					}
					ui.widgetPlot->DrawItemPoint(0, _vecPos, _vecString, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineContourCross]))
				{

				}
				//直线
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLine]))
				{
					QLineF	_qline1F, _qline2F;
					QVector<QPointF> _vecPos;
					QVector<QString> _vecString;
					QPointF _qPtCenter;
					{
						int		_iCheckType		= _strParamArray[0].toInt();
						double	_dCheckvalue	= _strLimitArray[0].toDouble();
						double	_dCheckRange	= _strLimitArray[1].toDouble();
						if (CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline1F) >= 0)
						{
							ui.widgetPlot->DrawItemLines(0, _qline1F, tr("Line"));
						}
					}
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeHorizontalLine]))
				{
					QLineF	_qline1F, _qline2F;
					QVector<QPointF> _vecPos;
					QVector<QString> _vecString;
					QPointF _qPtCenter;
					{
						QLineF _line;
						_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _strParamArray[0].toDouble()));
						_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _strParamArray[0].toDouble()));
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = HorizonLine;
						if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
					}
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineLineCross]))
				{

					
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeCircle]))
				{
					MCircle _Circle;
					int		_iCheckType		= _strParamArray[0].toInt();
					double	_dCheckvalue	= _strLimitArray[0].toDouble();
					double	_dCheckRange	= _strLimitArray[1].toDouble();
					if (CheckImgContourCircle(_iCheckType, _dCheckvalue, _dCheckRange, _Circle) >= 0)
					{
						ui.widgetPlot->DrawItemCircle(0, QPointF(_Circle.col, _Circle.row),_Circle.radius);
					}
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypePointPointLine]))
				{
					int _iParamIndex = 0;
					{
						int		_iCheckType		= _strParamArray[_iParamIndex].toInt();
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[0].toDouble();
							double	_dCheckRange = _strLimitArray[1].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}
					{
						int		_iCheckType		= _strParamArray.size() > 1 ? _strParamArray[_iParamIndex].toInt() : 0;
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[2].toDouble();
							double	_dCheckRange = _strLimitArray[3].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}

					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointA"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointB"));
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
					QLineF _line;
					_line.setP1(QPointF(x[0], y[0]));
					_line.setP2(QPointF(x[1], y[1]));
					ui.widgetPlot->DrawItemLines(0, _line, tr("Line"), QPen(Qt::green, 2, Qt::DashDotDotLine));
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeverticalLine]))
				{
					int _iParamIndex = 0;
					QLineF _Line, _OutLine;
					QPointF _qPt;
					QPointF _StokePt;
					{
						_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
						if (_iRetn < 0)		return;
						_iParamIndex++;
						x.push_back(_qPt.x());
						y.push_back(_qPt.y());
					}	
					{
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _Line);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}	
					{	//获取垂点
						_StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
					}
					if ( ( abs( _qPt.x() - _StokePt.x() ) <= 0.001 ) && ( abs( _qPt.y() - _StokePt.y() ) <= 0.001))
					{
						double k1 = (_Line.p1().y() - _Line.p2().y()) / (_Line.p1().x() - _Line.p2().x());
						// 计算过垂点的直线的斜率（垂直线的斜率Yes原斜率的负倒数）
						double k2 = -1.0 / k1;

						// 使用点斜式计算过垂点的直线上的一点
						double new_y = _StokePt.y() + k2 * (_StokePt.x() - _Line.p1().x());

						_qPt.setX(_Line.p1().x());
						_qPt.setY( new_y );

						_OutLine.setP1( _qPt	);
						_OutLine.setP2( _StokePt );
					}
					else
					{
						_OutLine.setP1(_qPt);
						_OutLine.setP2(_StokePt);
					}
					QRectF rect = ui.widgetPlot->GetQRectF();
					OutLineDetectTool::CalculateLineRectIntersections(_OutLine, rect, _OutLine);
					ui.widgetPlot->DrawItemLines(0, _Line, tr("Line"));
					ui.widgetPlot->DrawItemLines(1, _OutLine, tr("verticalLine"), QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("LinkA"));

					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypepointStroke]))
				{
					int _iParamIndex = 0;
					QLineF _Line, _OutLine;
					QPointF _qPt;
					QPointF _StokePt;
					{
						_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}	
					{
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex,_Line);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}
					{	//获取垂点
						_StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
					}
					QRectF rect = ui.widgetPlot->GetQRectF();
					OutLineDetectTool::CalculateLineRectIntersections(_Line, rect, _OutLine);
					ui.widgetPlot->DrawItemLines(0, _OutLine, tr("Line"));

					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("LinkA"));
					x.push_back(_qPt.x());
					y.push_back(_qPt.y());
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(tr("pointStroke"));
					x.push_back(_StokePt.x());
					y.push_back(_StokePt.y());
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineCircleCross]))
				{
					MCircle _circle;	
					int		_iParamIndex = 0;
					QLineF	_Line;
					{		//线
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _Line);
						if (_iRetn >= 0)
						{
							ui.widgetPlot->DrawItemLines(0, _Line, tr("Line"));
						}
						_iParamIndex++;
					}
					{
						_iRetn = GetReferenceCircleResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _circle);
						if (_iRetn >= 0)
						{
							ui.widgetPlot->DrawItemCircle(0, QPointF(_circle.col, _circle.row), _circle.radius);
						}
						_iParamIndex++;
					}
					//计算线圆交点

				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeHeightDifference]))
				{
					int _iParamIndex = 0;
					{
						int		_iCheckType = _strParamArray[_iParamIndex].toInt();
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[0].toDouble();
							double	_dCheckRange = _strLimitArray[1].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}	{
						int	_iCheckType = _strParamArray.size() > 1 ? _strParamArray[_iParamIndex].toInt() : 0;
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[2].toDouble();
							double	_dCheckRange = _strLimitArray[3].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointA"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointB"));

					double _dHeight = y[0] - y[1];
					QPointF _qPtCenter = QPointF((x[0] + x[1]) / 2.0, (y[0] + y[1]) / 2.0);
					x.push_back( MAX(x[0], x[1]) );
					y.push_back(_qPtCenter.y());
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("%1(%2)")
						.arg(tr("HeightDistance"))
						.arg(QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision))	);

					if (_strLimitValues.size() > 0)	_strLimitValues[0] = QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision);
					else _strLimitValues.push_back(QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision));

					QLineF	_Line1, _Line2;
					_Line1.setP1(QPointF(x[0], y[0]));
					_Line1.setP2(QPointF(x[0], y[1]));
					_Line2.setP1(QPointF(x[0], y[1]));
					_Line2.setP2(QPointF(x[1], y[1]));
					ui.widgetPlot->DrawItemLines(0, _Line1,"",QPen(Qt::blue, 1, Qt::DotLine));
					ui.widgetPlot->DrawItemLines(1, _Line2,"",QPen(Qt::blue, 1, Qt::DotLine));

					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineContourDistance]))
				{

				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeWidth]))
				{
					int _iParamIndex = 0;
					{
						int		_iCheckType = _strParamArray[_iParamIndex].toInt();
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[0].toDouble();
							double	_dCheckRange = _strLimitArray[1].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}
					{
						int	_iCheckType = _strParamArray.size() > 1 ? _strParamArray[_iParamIndex].toInt() : 0;
						if (_iCheckType != 5)
						{
							double	_dCheckvalue = _strLimitArray[2].toDouble();
							double	_dCheckRange = _strLimitArray[3].toDouble();
							_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
							if (_iRetn < 0)		return;
						}
						else  //关联
						{
							_iParamIndex++;
							QPointF _qPt;
							_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
							if (_iRetn < 0)		return;
							x.push_back(_qPt.x());
							y.push_back(_qPt.y());
						}
						_iParamIndex++;
					}
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointA"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointB"));

					double _dHeight = x[0] - x[1];
					QPointF _qPtCenter = QPointF((x[0] + x[1]) / 2.0, (y[0] + y[1]) / 2.0);
					x.push_back(_qPtCenter.x());
					y.push_back(_qPtCenter.y());
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("%1(%2)")
						.arg(tr("Height"))
						.arg(QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision)));

					if (_strLimitValues.size() > 0)	_strLimitValues[0] = QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision);
					else _strLimitValues.push_back(QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision));

					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeSectionalArea]))
				{

				}
				//测量
				else if (_strCheckType == (PortCheckTypeMap[EnumTypePointPointDistance]))
				{
					int		_iParamIndex = 0;
					QPointF _qPt1, _qPt2;
					QLineF	_qline1F;
					{
						_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt1);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}	{
						_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt2);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}
					x.push_back(_qPt1.x());
					y.push_back(_qPt1.y());
					x.push_back(_qPt2.x());
					y.push_back(_qPt2.y());
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointA"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("PointB"));
					{	
						double _dDistance;
						int	_iCheckType = _strParamArray[2].toInt();
						switch (_iCheckType)
						{
						case 0: {
							_dDistance = OutLineDetectTool::GetP2PDistance(_qPt1, _qPt2);
							x.push_back((_qPt2.x() + _qPt1.x()) / 2.0);
							y.push_back((_qPt2.y() + _qPt1.y()) / 2.0);
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("Distance"))
								.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));

							QLineF	_Line1;
							_Line1.setP1(	_qPt1	);
							_Line1.setP2(	_qPt2	);
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
						} break;
						case 1: {
							_dDistance = abs(_qPt1.x() - _qPt2.x());
							x.push_back(	(_qPt2.x() + _qPt1.x()) / 2.0	);
							y.push_back(	y[1] );
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("XDistance"))
								.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));
							QLineF	_Line1, _Line2;
							_Line1.setP1(QPointF(x[0], y[0]));
							_Line1.setP2(QPointF(x[0], y[1]));
							_Line2.setP1(QPointF(x[0], y[1]));
							_Line2.setP2(QPointF(x[1], y[1]));
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
							ui.widgetPlot->DrawItemLines(2, _Line2, "", QPen(Qt::blue, 1, Qt::DotLine));
						} break;
						case 2: {
							_dDistance = abs(_qPt1.y() - _qPt2.y());
							x.push_back(	x[0]	);
							y.push_back(	y[1]	);
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("YDistance"))
								.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));

							QLineF	_Line1, _Line2;
							_Line1.setP1(QPointF(x[0], y[0]));
							_Line1.setP2(QPointF(x[0], y[1]));
							_Line2.setP1(QPointF(x[0], y[1]));
							_Line2.setP2(QPointF(x[1], y[1]));
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
							ui.widgetPlot->DrawItemLines(2, _Line2, "", QPen(Qt::blue, 1, Qt::DotLine));
						} break;
						default:
							break;
						}

						if (_strLimitValues.size() > 0)	_strLimitValues[0] = QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision);
						else _strLimitValues.push_back(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
					}
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypePointLineDistance]))
				{
					int _iParamIndex = 0;
					QLineF _Line;
					QPointF _qPt;
					{
						_iRetn = GetReferencePointResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _qPt);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}	{
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex,_Line);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}
					ui.widgetPlot->DrawItemLines(0, _Line, tr("Line"));
					strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
					strVecName.push_back(tr("LinkA"));
					x.push_back(_qPt.x());
					y.push_back(_qPt.y());
					{
						//计算垂足
						QPointF _StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
						int	_iCheckType = _strParamArray[2].toInt();
						double _dDistance;
						switch (_iCheckType)
						{
						case 0: {
							QLineF	_Line1;
							_dDistance = OutLineDetectTool::GetP2PDistance(_qPt, _StokePt);
							x.push_back( _StokePt.x() );
							y.push_back( _StokePt.y() );

							_Line1.setP1(_qPt);
							_Line1.setP2(_StokePt);
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("Distance")).arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));
						} break;
						case 1: {
							_dDistance = abs(_qPt.x() - _StokePt.x());

							x.push_back(_StokePt.x());
							y.push_back(_StokePt.y());
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("XDistance")).arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));

							QLineF	_Line1, _Line2;
							_Line1.setP1(QPointF(x[0], y[0]));
							_Line1.setP2(QPointF(x[0], _StokePt.y()));
							_Line2.setP1(QPointF(x[0], _StokePt.y()));
							_Line2.setP2(QPointF(_StokePt.x(), _StokePt.y()));
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
							ui.widgetPlot->DrawItemLines(2, _Line2, "", QPen(Qt::blue, 1, Qt::DotLine));
						} break;
						case 2: {
							_dDistance = abs(_qPt.y() - _StokePt.y());

							x.push_back( _StokePt.x() );
							y.push_back( _StokePt.y() );
							strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
							strVecName.push_back(QString("%1(%2)")
								.arg(tr("YDistance"))
								.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));

							QLineF	_Line1, _Line2;
							_Line1.setP1(QPointF(x[0], y[0]));
							_Line1.setP2(QPointF(x[0], _StokePt.y()	));
							_Line2.setP1(QPointF(x[0], _StokePt.y()	));
							_Line2.setP2(QPointF(	_StokePt.x()	, _StokePt.y()	));
							ui.widgetPlot->DrawItemLines(1, _Line1, "", QPen(Qt::blue, 1, Qt::DotLine));
							ui.widgetPlot->DrawItemLines(2, _Line2, "", QPen(Qt::blue, 1, Qt::DotLine));
						} break;
						default:
							break;
						}

						if (_strLimitValues.size() > 0)	_strLimitValues[0] = QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision);
						else _strLimitValues.push_back(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
					}
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineLineAngle])) {
					int _iParamIndex = 0;
					QLineF _Line;
					QLineF _Line2;
					{
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _Line);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}	{
						_iRetn = GetReferenceLineResult(_strName, _iCurrentRow, _strParamArray, _iParamIndex, _Line2);
						if (_iRetn < 0)		return;
						_iParamIndex++;
					}

					HTuple _hdAngle;
					AngleLl(_Line.p1().y(),_Line.p1().x(), _Line.p2().y(), _Line.p2().x(), _Line2.p1().y(), _Line2.p1().x(), _Line2.p2().y(), _Line2.p2().x(),&_hdAngle);
					double _dDistance = _hdAngle.TupleDeg().D();

					if (_strLimitValues.size() > 0)	_strLimitValues[0] = QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision);
					else _strLimitValues.push_back(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));

					ui.widgetPlot->DrawItemLines(0, _Line, tr("LineA"));
					ui.widgetPlot->DrawItemLines(1, _Line2, tr("LineB"));

					QPointF _CrossPoint;
					if (_Line.intersect(_Line2, &_CrossPoint) != QLineF::NoIntersection)	{
						x.push_back(_CrossPoint.x());
						y.push_back(_CrossPoint.y());
						strVecQPen.push_back(QPen(Qt::blue, 2, Qt::SolidLine));
						strVecName.push_back(QString("%1(%2)")
							.arg(tr("Angle"))
							.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision)));
					}
					ui.widgetPlot->DrawItemPoint(0, x, y, strVecName, strVecQPen);
				}
				else if (_strCheckType == (PortCheckTypeMap[EnumTypeLineContourCrossNum])) {
					
				}

				ChangeCurrentTableWidgetText(_iCurrentRow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);
			}
		}
	}
	ui.widgetPlot->RePlotALLTrace();
}

void frmOutLineDetectTool::ClearROIProperty()
{
	for (auto iter : m_idToProperty)	if (iter != nullptr)	delete iter;
	m_idToProperty.clear();
	m_propertyToId.clear();
	m_variantManager->clear();
	m_propertyEditor->clear();
}

void frmOutLineDetectTool::ClearParamProperty()
{
	for (auto iter : m_ParamidToProperty)	if (iter != nullptr)	delete iter;	m_ParamidToProperty.clear();
	m_ParamidToProperty.clear();
	m_ParampropertyToId.clear();
	m_ParamvariantManager->clear();
	m_ParampropertyEditor->clear();
	m_VecParampropertyID.clear();
}

void frmOutLineDetectTool::ClearJudgeProperty()
{
	for (auto iter : m_JudgeLimitidToProperty)	if (iter != nullptr)	delete iter;	m_JudgeLimitidToProperty.clear();
	m_JudgeLimitidToProperty.clear();
	m_JudgeLimitpropertyToId.clear();
	m_JudgeLimitvariantManager->clear();
	m_JudgeLimitpropertyEditor->clear();
}

int frmOutLineDetectTool::CheckImgContourPoint(int iCheckType,double xValue,double xRange, QVector<double>& x, QVector<double>& y, int iIndex)
{
	int _iMin = MAX(xValue,0);
	int _iMax = MIN(xRange + xValue, m_hGrayval.Length() - 1);
	if (m_hGrayval.Length() <= 0) return -1;
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	switch (iCheckType)
	{
	case 0:	//最大值
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)
		{
			int _iIndex = _hIndices.Length() - 1;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			x.push_back(_hIndices[_iIndex] + (int)xValue);
			y.push_back(_hValue.D());
			return 0;
		}
	}	break;
	case 1:	//最小值
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)
		{
			int _iIndex = 0;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			x.push_back(_hIndices[_iIndex] + (int)xValue);
			y.push_back(_hValue.D());
			return 0;
		}
	}	break;
	case 2:	//中值点
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)
		{
			int _iIndex = _hIndices.Length() / 2.0;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			x.push_back(_hIndices[_iIndex] + (int)xValue);
			y.push_back(_hValue.D());
			return 0;
		}
	}	break;
	case 3:	//均值点
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		TupleMedian(_hSelected, &_hValue);		//绘制直线
		if (_hSelected.Length() > 0)
		{
			x.push_back( (_iMin + _iMax) / 2.0  );
			y.push_back(_hValue.D());
			return 0;
		}
	}	break;
	case 4:	//拐点
	{
	}	break;
	case 5:	//关联点
	{
	}	break;
	default:
		break;
	}
	return -1;
}

int frmOutLineDetectTool::CheckImgContourLine(int iCheckType, double xValue, double xRange,QLineF & line, int iIndex)
{
	int _iMin = MAX(xValue, 0);
	int _iMax = MIN(xRange + xValue, m_hGrayval.Length() - 1);
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	try
	{
		//if (ui.widgetPlot->m_lineTracers.size() <= iIndex) { ui.widgetPlot->m_lineTracers.push_back(new XxwTraceHintLine(ui.widgetPlot, HorizonLine)); }
		HTuple _hSelected, _hValueXIndex, _hValue;
		HObject _hContour;
		HTuple RowBegin, ColBegin, RowEnd, ColEnd, Nr, Nc, Dist;
		HalconCpp::TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		for (int i = _iMin; i <= _iMax; i++)	_hValueXIndex.Append(i);
		std::string	_strAlgorithm;
		switch (iCheckType)
		{
		case 0:	//最大值
		{
			_strAlgorithm = "tukey";
		}	break;
		case 1:	//最小值
		{
			_strAlgorithm = "tukey";
		}	break;
		case 2:	//中值点
		{
			_strAlgorithm = "drop";
		}	break;
		default:
			break;
		}
		HalconCpp::GenContourPolygonXld(&_hContour, _hSelected, _hValueXIndex);
		HalconCpp::FitLineContourXld(_hContour,		_strAlgorithm.c_str(), -1, 0, 5, 2, &RowBegin, &ColBegin, &RowEnd, &ColEnd, &Nr, &Nc, &Dist);

		line.setP1(QPointF(ColBegin.D(), RowBegin.D()));
		line.setP2(QPointF(ColEnd.D(), RowEnd.D()));
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

int frmOutLineDetectTool::CheckImgContourCircle(int iCheckType, double xValue, double xRange, MCircle & circle, int iIndex)
{
	int _iMin = MAX(xValue, 0);
	int _iMax = MIN(xRange + xValue, m_hGrayval.Length() - 1);
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	try
	{
		//if (ui.widgetPlot->m_lineTracers.size() <= iIndex) { ui.widgetPlot->m_lineTracers.push_back(new XxwTraceHintLine(ui.widgetPlot, HorizonLine)); }
		HTuple	_hSelected, _hValueXIndex, _hValue;
		HObject _hContour;
		HTuple  hv_Nr, hv_Nc, hv_Dist, hv_Row, hv_Column, hv_Radius;
		HTuple  hv_StartPhi, hv_EndPhi, hv_PointOrder;
		HalconCpp::TupleSelectRange(m_hGrayval, _iMin, _iMax, &_hSelected);
		for (int i = _iMin; i <= _iMax; i++)	_hValueXIndex.Append(i);
		std::string	_strAlgorithm;
		switch (iCheckType)
		{
		case 0:	//最大值
		{
			_strAlgorithm = "algebraic";
		}	break;
		case 1:	//最小值
		{
			_strAlgorithm = "ahuber";
		}	break;
		case 2:	//中值点
		{
			_strAlgorithm = "atukey";
		}	break;
		case 3:	//中值点
		{
			_strAlgorithm = "geometric";
		}	break;
		case 4:	//中值点
		{
			_strAlgorithm = "geohuber";
		}	break;
		case 5:	//中值点
		{
			_strAlgorithm = "geotukey";
		}	break;
		default:
			break;
		}
		HalconCpp::GenContourPolygonXld(&_hContour, _hSelected, _hValueXIndex);
		HalconCpp::FitCircleContourXld(_hContour, _strAlgorithm.c_str(),-1,0,0,5,2, &hv_Row,&hv_Column,&hv_Radius,&hv_StartPhi,&hv_EndPhi,&hv_PointOrder);
		circle.row		= hv_Row.D();
		circle.col		= hv_Column.D();
		circle.radius	= hv_Radius.D();
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

int frmOutLineDetectTool::GetReferencePointResult(QString strName, int Currentrow, QStringList strLst,int iParamIndex, QPointF & pt)
{
	QString		_strName;	
	QStringList _strNameArray = strName.split("_");
	int _iRetn	= -1;
	QStringList enumNames	= GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypePoint], Currentrow);
	if (enumNames.size() <= 0)			return -1;
	int _iIndex				= strLst[iParamIndex].toInt();
	if (enumNames.size() <= _iIndex)	return -1;
	_strName				= enumNames[_iIndex];

	int _iRow;
	QString _strData;
	if (GetTableWidgetNameToData(_strName, _iRow, _strData) >= 0)	{
		QStringList					_strParamArray;	//参数
		QStringList					_strLimitArray;
		QStringList					_strLimitValues;
		QStringList					_strLimitParams;
		bool _iIsLimit = false;
		int	_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues,_strLimitParams);		//获取关联点
		_iRetn = CheckImgDetectPoint(_strName, Currentrow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, pt);
	}
	return _iRetn;
}

int frmOutLineDetectTool::GetReferenceLineResult(QString strName, int Currentrow, QStringList strLst, int iParamIndex, QLineF & line)
{
	if (strLst.size() <= iParamIndex)	return -1;
	QString		_strName;
	QStringList _strNameArray = strName.split("_");
	int _iRetn = -1;
	QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeLine], Currentrow);
	if (enumNames.size() <= 0)	return -1;

	int _iIndex = strLst[iParamIndex].toInt();
	if (enumNames.size() <= _iIndex	)	return -1;

	_strName	= enumNames[_iIndex];

	int _iRow;
	QString _strData;
	if (GetTableWidgetNameToData(_strName, _iRow, _strData) >= 0)
	{
		QStringList					_strParamArray;	//参数
		QStringList					_strLimitArray;
		QStringList					_strLimitValues;
		QStringList					_strLimitParams;
		bool _iIsLimit = false;
		int	_iRegionType = XxwTraceRect::LimitTypeUpAndDown;
		GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams);		//获取关联点
		_iRetn = CheckImgDetectLine(_strName, Currentrow,_strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams,line);
	}
	return _iRetn;
}

int frmOutLineDetectTool::GetReferenceCircleResult(QString strName, int Currentrow, QStringList strLst, int iParamIndex, MCircle & ptCircle)
{
	if (strLst.size() <= iParamIndex)	return -1;
	QString		_strName;
	QStringList _strNameArray = strName.split("_");
	int _iRetn	= -1;
	QStringList enumNames = GetTableWidgetFilterNames(PortCheckTypeMap[EnumTypeCircle], Currentrow);
	if (enumNames.size() <= 0)			return -1;
	int _iIndex = strLst[iParamIndex].toInt();
	if (enumNames.size() <= _iIndex)	return -1;
	_strName	= enumNames[_iIndex];
	int _iRow;
	QString _strData;
	if (GetTableWidgetNameToData(_strName, _iRow, _strData) >= 0)
	{
		QStringList					_strParamArray;	//参数
		QStringList					_strLimitArray;
		QStringList					_strLimitValues;
		QStringList					_strLimitParams;
		bool _iIsLimit = false;
		int	_iRegionType			= XxwTraceRect::LimitTypeUpAndDown;
		GetTableWidgetText(_strData, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues,_strLimitParams);		//获取关联点
		_iRetn = CheckImgDetectCircle(_strName, Currentrow, _strParamArray, _iIsLimit, _iRegionType, _strLimitArray, _strLimitValues, _strLimitParams, ptCircle);
	}
	return _iRetn;
}

int frmOutLineDetectTool::CheckImgDetectPoint(QString strName, int Currentrow, QStringList& strParamArray,bool& iIsLimit,int& iRegionType,QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams,QPointF& pt)
{
	QString	_strData;
	QString	_strCheckType;
	QStringList _strNameArray	= strName.split("_");
	_strCheckType				= _strNameArray[0];
	int _iRetn = 0;
	QVector<double> x, y;
	if (_strCheckType == PortCheckTypeMap[EnumTypePoint] )
	{
		int		_iCheckType		= strParamArray[0].toInt();
		double	_dCheckvalue	= strLimitArray[0].toDouble();
		double	_dCheckRange	= strLimitArray[1].toDouble();
		_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
		if (_iRetn >= 0)
		{
			pt.setX(x[0]);
			pt.setY(y[0]);
		}
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypePointPointCenter] )
	{
		{
			int		_iCheckType		= strParamArray[0].toInt();
			double	_dCheckvalue	= strLimitArray[0].toDouble();
			double	_dCheckRange	= strLimitArray[1].toDouble();
			_iRetn					= CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
			if (_iRetn < 0)		goto End;
		}	{
			int		_iCheckType		= strParamArray.size() > 1 ? strParamArray[1].toInt() : 0;
			double	_dCheckvalue	= strLimitArray[2].toDouble();
			double	_dCheckRange	= strLimitArray[3].toDouble();
			_iRetn					= CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
			if (_iRetn < 0)		goto End;
		}
		if (_iRetn >= 0)
		{
			pt = QPointF((x[0] + x[1]) / 2.0, (y[0] + y[1]) / 2.0);
		}
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypeLineCross])
	{
		QLineF	_qline1F, _qline2F;
		QVector<QPointF> _vecPos;
		QVector<QString> _vecString;
		QPointF _qPtCenter;
		{
			int		_iCheckType = strParamArray[0].toInt();
			double	_dCheckvalue = strLimitArray[0].toDouble();
			double	_dCheckRange = strLimitArray[1].toDouble();
			_iRetn = CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline1F);
			if (_iRetn < 0)		goto End;
		}	{
			int		_iCheckType = strParamArray[1].toInt();
			double	_dCheckvalue = strLimitArray[2].toDouble();
			double	_dCheckRange = strLimitArray[3].toDouble();
			_iRetn = CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline2F);
			if (_iRetn < 0)		goto End;
		}
		QLineF::IntersectType _type = _qline1F.intersect(_qline2F, &_qPtCenter);
		switch (_type)
		{
		case QLineF::NoIntersection: {
			_iRetn = -1;
		}	break;	//无交叉点
		case QLineF::BoundedIntersection:	//边界交叉点
		case QLineF::UnboundedIntersection:	//交点
		{
			pt = _qPtCenter;
		}	break;
		default:
			break;
		}
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypepointStroke])
	{
		int _iParamIndex = 0;
		QLineF _Line;
		QPointF _qPt;
		{
			_iRetn = GetReferencePointResult(strName, Currentrow, strParamArray, _iParamIndex, _qPt);
			if (_iRetn < 0)		goto End;
			_iParamIndex++;
		}	{
			_iRetn = GetReferenceLineResult(strName, Currentrow, strParamArray, _iParamIndex, _Line);
			if (_iRetn < 0)		goto End;
			_iParamIndex++;
		}	{	//获取垂点
			pt = OutLineDetectTool::FindFoot(_Line, _qPt);
		}
	}

End:
	return _iRetn;
}

int frmOutLineDetectTool::CheckImgDetectLine(QString strName, int Currentrow, QStringList & strParamArray, bool & iIsLimit, int & iRegionType, QStringList & strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams, QLineF & line)
{
	QString	_strData;
	QString	_strCheckType;
	QStringList _strNameArray = strName.split("_");
	_strCheckType = _strNameArray[0];
	int _iRetn = 0;
	QVector<double> x, y;
	if (_strCheckType == PortCheckTypeMap[EnumTypeLine])
	{
		QLineF	_qline1F;
		QVector<QPointF> _vecPos;
		QVector<QString> _vecString;
		QPointF _qPtCenter;
		{
			int		_iCheckType		= strParamArray[0].toInt();
			double	_dCheckvalue	= strLimitArray[0].toDouble();
			double	_dCheckRange	= strLimitArray[1].toDouble();
			_iRetn = CheckImgContourLine(_iCheckType, _dCheckvalue, _dCheckRange, _qline1F);
		}
		line = _qline1F;
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypeHorizontalLine])
	{
		QLineF	_qline1F, _qline2F;
		QVector<QPointF> _vecPos;
		QVector<QString> _vecString;
		QPointF _qPtCenter;
		{
			line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, strParamArray[0].toDouble()));
			line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, strParamArray[0].toDouble()));
		}
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypePointPointLine])
	{
		int _iParamIndex = 0;
		{
			int		_iCheckType		= strParamArray[_iParamIndex].toInt();
			double	_dCheckvalue	= strLimitArray[0].toDouble();
			double	_dCheckRange	= strLimitArray[1].toDouble();
			if (_iCheckType != 5)
			{
				_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
				if (_iRetn < 0)		goto End;
			}
			else  //关联
			{
				_iParamIndex++;
				QPointF _qPt;
				_iRetn = GetReferencePointResult(strName, Currentrow, strParamArray, _iParamIndex, _qPt);
				if (_iRetn < 0)		goto End;
				x.push_back(_qPt.x());
				y.push_back(_qPt.y());
			}
			_iParamIndex++;
		}	
		{
			int		_iCheckType		= strParamArray.size() > 1 ? strParamArray[_iParamIndex].toInt() : 0;
			double	_dCheckvalue	= strLimitArray[2].toDouble();
			double	_dCheckRange	= strLimitArray[3].toDouble();
			if (_iCheckType != 5)
			{
				_iRetn = CheckImgContourPoint(_iCheckType, _dCheckvalue, _dCheckRange, x, y);
				if (_iRetn < 0)		goto End;
			}
			else  //关联
			{
				_iParamIndex++;
				QPointF _qPt;
				_iRetn = GetReferencePointResult(strName, Currentrow, strParamArray, _iParamIndex, _qPt);
				if (_iRetn < 0)		goto End;
				x.push_back(_qPt.x());
				y.push_back(_qPt.y());
			}
			_iParamIndex++;
		}
		line.setP1(QPointF(x[0], y[0]));
		line.setP2(QPointF(x[1], y[1]));
	}
	else if (_strCheckType == PortCheckTypeMap[EnumTypeverticalLine])
	{
		int		_iParamIndex = 0;
		QLineF	_Line;
		QPointF _qPt;
		QPointF _StokePt;
		{
			_iRetn = GetReferencePointResult(strName, Currentrow, strParamArray, _iParamIndex, _qPt);
			if (_iRetn < 0)		goto End;
			_iParamIndex++;
		}
		{
			_iRetn = GetReferenceLineResult(strName, Currentrow, strParamArray, _iParamIndex, _Line);
			if (_iRetn < 0)		goto End;
			_iParamIndex++;
		}
		{	//获取垂点
			_StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
		}
		if ((abs(_qPt.x() - _StokePt.x()) <= 0.001) && (abs(_qPt.y() - _StokePt.y()) <= 0.001))
		{
			double k1 = (_Line.p1().y() - _Line.p2().y()) / (_Line.p1().x() - _Line.p2().x());
			// 计算过垂点的直线的斜率（垂直线的斜率Yes原斜率的负倒数）
			double k2 = -1.0 / k1;

			// 使用点斜式计算过垂点的直线上的一点
			double new_y = _StokePt.y() + k2 * (_StokePt.x() - _Line.p1().x());

			_qPt.setX(_Line.p1().x());
			_qPt.setY(new_y);

			line.setP1(_qPt);
			line.setP2(_StokePt);
		}
		else
		{
			line.setP1(_qPt);
			line.setP2(_StokePt);
		}
	}
End:
	return _iRetn;
}

int frmOutLineDetectTool::CheckImgDetectCircle(QString strName, int Currentrow, QStringList & strParamArray, bool & iIsLimit, int & iRegionType, QStringList & strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams, MCircle & circle)
{
	QString	_strData;
	QString	_strCheckType;
	QStringList _strNameArray = strName.split("_");
	_strCheckType = _strNameArray[0];
	int _iRetn = 0;
	QVector<double> x, y;
	if (_strCheckType == PortCheckTypeMap[EnumTypeCircle])
	{
		MCircle	_qline1F;
		QVector<QPointF> _vecPos;
		QVector<QString> _vecString;
		QPointF _qPtCenter;
		{
			int		_iCheckType = strParamArray[0].toInt();
			double	_dCheckvalue = strLimitArray[0].toDouble();
			double	_dCheckRange = strLimitArray[1].toDouble();
			_iRetn = CheckImgContourCircle(_iCheckType, _dCheckvalue, _dCheckRange, circle);
		}
	}
End:
	return _iRetn;
}

bool frmOutLineDetectTool::CheckSafetableWidget(int row, int col)
{
	if (ui.tableWidget->rowCount() > row)
		if (ui.tableWidget->columnCount() > col)
			if (ui.tableWidget->item(row, col) != nullptr)
				if (!ui.tableWidget->item(row, col)->text().isEmpty())	{
					return true;
				}
	return false;
}

QStringList frmOutLineDetectTool::GetTableWidgetFilterNames(QString strFilterName, int Currentrow)
{
	int _iRowCount = ui.tableWidget->rowCount();
	QStringList _strNames;
	for (int iRow = 0; iRow < _iRowCount; iRow++)
	{
		QString	_strData			= ui.tableWidget->item(iRow, 0)->text();
		QStringList _strNameArray	= _strData.split("_");
		QString _strName			= _strNameArray[0];
		bool _bFind					= false;
		if (iRow >= Currentrow)
		{
			break;
		}
		if (strFilterName == PortCheckTypeMap[EnumTypePoint])
		{
			if ((_strName == PortCheckTypeMap[EnumTypePoint]) 
				|| (_strName == PortCheckTypeMap[EnumTypePointPointCenter]) 
				|| (_strName == PortCheckTypeMap[EnumTypeLineCross])
				|| (_strName == PortCheckTypeMap[EnumTypepointStroke])
				|| (_strName == PortCheckTypeMap[EnumTypeLineCircleCross]))
			{
				_bFind = true;
			}
		}
		else if(strFilterName == PortCheckTypeMap[EnumTypeLine])
		{
			if ( (_strName == PortCheckTypeMap[EnumTypeHorizontalLine])
				|| (_strName == PortCheckTypeMap[EnumTypeLine])
				|| (_strName == PortCheckTypeMap[EnumTypePointPointLine])
				|| (_strName == PortCheckTypeMap[EnumTypeverticalLine]))
			{
				_bFind = true;
			}
		}
		else if (strFilterName == PortCheckTypeMap[EnumTypeCircle])
		{
			if ((_strName == PortCheckTypeMap[EnumTypeCircle])	)
			{
				_bFind = true;
			}
		}
		if (_bFind)
		{
			_strNames.append(_strData);
		}
	}
	return _strNames;
}

int frmOutLineDetectTool::ChangeCurrentTableWidgetText(int irow, QStringList strParamArray, bool iIsLimit, int iRegionType, QStringList strLimitArray, QStringList&strLimitValues, QStringList strLimitParams)
{
	if (CheckSafetableWidget(irow, 1))
	{
		QString	_strData1 = "P:";
		for (size_t i = 0; i < strParamArray.size(); i++)
		{
			if (i == (strParamArray.size() - 1))
				_strData1.append(strParamArray[i]);
			else
				_strData1.append(strParamArray[i]).append("|");
		}
		_strData1.append(QString("&R%1%2:").arg(QString::number(iIsLimit ? 1 : 0)).arg(QString::number(iRegionType)));
		for (size_t i = 0; i < strLimitArray.size(); i++)
		{
			if (i == (strLimitArray.size() - 1))
				_strData1.append(strLimitArray[i]);
			else
				_strData1.append(strLimitArray[i]).append("|");
		}
		_strData1.append("@@");
		for (size_t i = 0; i < strLimitParams.size(); i++)
		{
			if (i == (strLimitParams.size() - 1))
				_strData1.append(strLimitParams[i]);
			else
				_strData1.append(strLimitParams[i]).append("|");
		}
		_strData1.append("&&");
		for (size_t i = 0; i < strLimitValues.size(); i++)
		{
			if (i == (strLimitValues.size() - 1))
				_strData1.append(strLimitValues[i]);
			else
				_strData1.append(strLimitValues[i]).append("|");
		}
		//m_VecData[irow] = _strData1;
		ui.tableWidget->item(irow, 1)->setText(_strData1);

		QModelIndex _current = ui.tableROIWidget->currentIndex();
		if (_current.isValid())	{
			int _iROIRow = ui.tableROIWidget->currentIndex().row();
			if (_iROIRow >= 0 && _iROIRow < m_VecCheckOutLineData.size())	{
				auto& iter = m_VecCheckOutLineData[_iROIRow];
				if (irow < iter.vecData.size())	{
					iter.vecData[irow].strData = _strData1;
				}
			}
		}
	}
	return 0;
}

int frmOutLineDetectTool::GetTableWidgetText(int irow,QStringList& strParamArray,bool& iIsLimit,int& iRegionType,QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams)
{
	if (CheckSafetableWidget(irow, 1))
	{
		QString	_strData = ui.tableWidget->item(irow, 1)->text();
		return GetTableWidgetText(_strData, strParamArray, iIsLimit, iRegionType, strLimitArray, strLimitValues, strLimitParams);
	}
	return -1;
}

QString frmOutLineDetectTool::GetTableWidgetText(int iRow)
{
	int _iROIRow	= ui.tableROIWidget->currentIndex().row();
	int _iCheckRow	= ui.tableWidget->currentIndex().row();
	if (_iROIRow < m_VecCheckOutLineData.size() && _iROIRow >= 0)	{
		auto& iter = m_VecCheckOutLineData[_iROIRow];
		if (_iCheckRow < iter.vecData.size() && _iCheckRow >= 0)		{
			return iter.vecData[_iCheckRow].strData;
		}
	}
	return QString();
}

int frmOutLineDetectTool::GetTableWidgetText(QString strText, QStringList & strParamArray,bool&iIsLimit,int&iRegionType,QStringList&strLimitArray, QStringList&strLimitValues,QStringList&strLimitParams)
{
	strLimitParams.clear();
	int _iLastIndex = strText.indexOf("&R");
	int _iFirstIndex = 2;
	QString _strParamData = strText.mid(_iFirstIndex, _iLastIndex - _iFirstIndex);
	strParamArray	= _strParamData.split("|");
	QStringList _strArray = strText.split("@@");
	_strParamData	= strText.mid(_iLastIndex + 5, _strArray[0].length() - ( _iLastIndex + 5 ));
	strLimitArray	= _strParamData.split("|");
	iIsLimit		= strText.mid(_iLastIndex + 2, 1).toInt();
	iRegionType		= strText.mid(_iLastIndex + 3, 1).toInt();

	_strArray		= strText.split("&&");
	if (_strArray.size() > 1)
	{
		if(!_strArray[1].isEmpty())
			strLimitValues = _strArray[1].split("|");
	}
	QStringList _ParamsArray = _strArray[0].split("@@");
	if (_ParamsArray.size() > 1)
	{
		if (!_ParamsArray[1].isEmpty())
			strLimitParams = _ParamsArray[1].split("|");
	}
	return 0;
}

int frmOutLineDetectTool::SetTableWidgetText(QStringList & strParamArray, bool & iIsLimit, int & iRegionType, QStringList & strLimitArray,QStringList&strLimitValues,QStringList&strLimitParams, QString & strText)
{
	QString	_strData1 = "P:";
	for (size_t i = 0; i < strParamArray.size(); i++)
	{
		if (i == (strParamArray.size() - 1))
			_strData1.append(strParamArray[i]);
		else
			_strData1.append(strParamArray[i]).append("|");
	}
	_strData1.append(QString("&R%1%2:").arg(QString::number(iIsLimit ? 1 : 0)).arg(QString::number(iRegionType)));
	for (size_t i = 0; i < strLimitArray.size(); i++)
	{
		if (i == (strLimitArray.size() - 1))
			_strData1.append(strLimitArray[i]);
		else
			_strData1.append(strLimitArray[i]).append("|");
	}
	_strData1.append("@@");
	for (size_t i = 0; i < strLimitParams.size(); i++)
	{
		if (i == (strLimitParams.size() - 1))
			_strData1.append(strLimitParams[i]);
		else
			_strData1.append(strLimitParams[i]).append("|");
	}
	_strData1.append("&&");
	for (size_t i = 0; i < strLimitValues.size(); i++)
	{
		if (i == (strLimitValues.size() - 1))
			_strData1.append(strLimitValues[i]);
		else
			_strData1.append(strLimitValues[i]).append("|");
	}
	strText = _strData1;
	return 0;
}

int frmOutLineDetectTool::GetTableWidgetNameToData(QString strName, int & iRow, QString & strData)
{
	if(strName.isEmpty())	return -1;

	int _iRowCount = ui.tableWidget->rowCount();
	QString	_strName, _strData;
	for (int _iRow = 0; _iRow < _iRowCount; _iRow++)	{
		if (CheckSafetableWidget(_iRow, 0))				{
			_strName = ui.tableWidget->item(_iRow, 0)->text();
		}
		if (_strName == strName)						{
			if (CheckSafetableWidget(_iRow, 1))			{
				_strData = ui.tableWidget->item(_iRow, 1)->text();
			}

			iRow	= _iRow;
			strData = _strData;
			return 0;
		}
	}
	return -1;
}
