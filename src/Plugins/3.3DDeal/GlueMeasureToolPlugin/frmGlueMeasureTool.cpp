#include "frmGlueMeasureTool.h"
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
#include "frmAllVariableTool.h"
#include <QWidgetAction>

#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmGlueMeasureTool::frmGlueMeasureTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Contour.png"));
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Contour.png");

	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmGlueMeasureTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	initConnect();
	initCustomplot();
	initShapeProperty();
	initResultProperty();
	initShowProperty();
	initData();

}

frmGlueMeasureTool::~frmGlueMeasureTool()
{
	if (btnGroupRadio != nullptr)			delete btnGroupRadio;				btnGroupRadio = nullptr;
	if (m_view != nullptr)					delete m_view;						m_view = nullptr;

	if (variantFactory != nullptr)			delete variantFactory;				variantFactory = nullptr;
	if (variantManager != nullptr)			delete variantManager;				variantManager = nullptr;
	if (propertyEditor != nullptr)			delete propertyEditor;				propertyEditor = nullptr;

	if (m_ResultvariantFactory != nullptr)	delete m_ResultvariantFactory;		m_ResultvariantFactory = nullptr;
	if (m_ResultvariantManager != nullptr)	delete m_ResultvariantManager;		m_ResultvariantManager = nullptr;
	if (m_ResultpropertyEditor != nullptr)	delete m_ResultpropertyEditor;		m_ResultpropertyEditor = nullptr;
	this->deleteLater();
}

void frmGlueMeasureTool::initCustomplot()
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
	ui.widgetPlot->DrawClearAllItem();
	ui.widgetPlot->update();
}

void frmGlueMeasureTool::initShapeProperty()
{
	variantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	variantFactory = new QtVariantEditorFactory(this);
	propertyEditor = new QtTreePropertyBrowser(ui.ROIwidget);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.ROILayout->addWidget(propertyEditor);

	idToProperty.clear();
	propertyToId.clear();
	m_VecBaseItem.clear();
}

void frmGlueMeasureTool::initResultProperty()
{
	m_ResultvariantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_ResultvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	m_ResultvariantFactory = new QtVariantEditorFactory(this);
	m_ResultpropertyEditor = new QtTreePropertyBrowser(ui.widget_Result);
	m_ResultpropertyEditor->setFactoryForManager(m_ResultvariantManager, m_ResultvariantFactory);
	ui.ResultLayout->addWidget(m_ResultpropertyEditor);
}

void frmGlueMeasureTool::initShowProperty()
{
	initShowEditorProperty(ui.widget_Show, ui.ShowLayout);
	//connect(m_ShowvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Result_valueChanged(QtProperty *, const QVariant &)));
	ui.gImage->setVisible(false);

	QtVariantProperty*	_ptrproperty = nullptr;
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "显示绘制区域");
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showregion, tr("showregion"));
	}	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("显示绘制结果"));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showResult, tr("showResult"));
	} {
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("显示文字结果"));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showDrawResult, tr("showDrawResult"));
	}
}

void frmGlueMeasureTool::initConnect()
{
	connect(ui.cbx_Camerachannal,		SIGNAL(activated(int)),								this,			SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW,				SIGNAL(valueChanged(int)),							this,			SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut,			SIGNAL(activated(int)),								this,			SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view,						SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)),	this,			SLOT(slot_MouseClicked(QList<QGraphicsItem*>&)));
	connect(m_view,						SIGNAL(sig_MouseMouve(QList<QGraphicsItem*>&)),		this,			SLOT(slot_MouseMouve(QList<QGraphicsItem*>&)));
	connect(m_view,						&QGraphicsViews::sig_mousePressEvent,				this,			&frmGlueMeasureTool::slot_mousePressEvent);
	connect(m_view,						&QGraphicsViews::sig_mouseMoveEvent,				this,			&frmGlueMeasureTool::slot_mouseMoveEvent);
	connect(m_view,						&QGraphicsViews::sig_mouseReleaseEvent,				this,			&frmGlueMeasureTool::slot_mouseReleaseEvent);
	connect(ui.cbx_ALargrimCheckType,	SIGNAL(activated(int)),								this,			SLOT(onCheckTypeChanged(int)));
	connect(ui.SliderPercent,			SIGNAL(lowerValueChanged(int)),						ui.lbl_Low,		SLOT(setNum(int)));
	connect(ui.SliderPercent,			SIGNAL(upperValueChanged(int)),						ui.lbl_High,	SLOT(setNum(int)));
	connect(ui.cbx_CheckIndex,			SIGNAL(activated(int)),								this,			SLOT(onROIRectChanged(int)));

	connect(ui.btnAddROI,				&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnAddROI_clicked);
	connect(ui.btnDeleteROISelect,		&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnDeleteROISelect_clicked);
	connect(ui.btnDeleteROIAll,			&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnDeleteROIAll_clicked);

	connect(ui.btnLinkContent,			&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,		&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnDelLinkContent_clicked);
	
	//connect(ui.btnLinkContent,			&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnLinkContent_clicked);
	//connect(ui.btnDelLinkContent,		&QPushButton::pressed,								this,			&frmGlueMeasureTool::on_btnDelLinkContent_clicked);

}

void frmGlueMeasureTool::initData()
{
	ui.tabWidget->setCurrentIndex(0);
	m_iChanal = 0;
}

void frmGlueMeasureTool::ClearROIProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

//绘制ROI
void frmGlueMeasureTool::on_btnAddROI_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth	= (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty		= nullptr;
	QtVariantProperty *	_ptrSubproperty		= nullptr;
	QtVariantProperty *	_ptrSubSubproperty	= nullptr;

	float fData[5] = { 0 };
	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	switch (ui.comboROIShape->currentIndex())	{
	case 6: {	//园
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
		m_view->AddItems(_BaseItem);
	}   break;
	case 2: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 3: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 4: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 5: {	//多边形
		_BaseItem = new PolygonItem();
		m_view->AddItems(_BaseItem);
		_BaseItem->GetAllParam(_mapValue);
	}   break;
	case 0: {	//贝塞尔曲线
		_BaseItem = new BezierLineItem();
		m_view->AddItems(_BaseItem);
		_BaseItem->GetAllParam(_mapValue);

	}   break;
	}

	for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
		if (((*mapIter).first).count(cstItem) > 0) {
			_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
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
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Bool, (*pIter).first);
						break;
					case ParamType_Int:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Int, (*pIter).first);
						break;
					case ParamType_Double:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					case ParamType_String:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::String, (*pIter).first);
						break;
					case ParamType_Enum:
						_ptrSubSubproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
						_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
						break;
					case ParamType_Other:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					default:		break;
					}
					_ptrSubSubproperty->setValue(Param);
					_ptrSubSubproperty->setToolTip(Param);
					_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);

					_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
					if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
				}
				else {
					_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
					_ptrSubSubproperty->setValue((*pIter).second);
				}
				idToProperty[(*pIter).first] = _ptrSubSubproperty;
				idToProperty[(*pIter).first]->setPropertyName(strLang);
				propertyToId[_ptrSubSubproperty] = (*pIter).first;
				_ptrproperty->addSubProperty(_ptrSubSubproperty);
			}

			addProperty(_ptrproperty, ((*mapIter).first));
			_ptrproperty->setPropertyName(BaseItem::tr(cstItem.toStdString().c_str()));
		}
		else
		{
			_ptrSubproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), ((*mapIter).first));
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
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Bool, (*pIter).first);
						break;
					case ParamType_Int:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Int, (*pIter).first);
						break;
					case ParamType_Double:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
						break;
					case ParamType_String:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::String, (*pIter).first);
						break;
					case ParamType_Enum:
						_ptrSubSubproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
						_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
						break;
					case ParamType_Other:
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
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
					_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
					_ptrSubSubproperty->setValue((*pIter).second);
				}
				idToProperty[(*pIter).first] = _ptrSubSubproperty;
				idToProperty[(*pIter).first]->setPropertyName(strLang);
				propertyToId[_ptrSubSubproperty] = (*pIter).first;
				_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
			}
			addProperty(_ptrSubproperty, (*mapIter).first);
		}
	}
	switch (ui.comboxROIType->currentIndex())	{
	case 0:	{	_BaseItem->m_eRegionType = RegionType_Add;		}break;
	case 1:	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default:	break;	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmGlueMeasureTool::on_btnDeleteROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr)
			if (iter->type() == 10)
			{
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
					(m_view->scene())->removeItem(iter);
					m_VecBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmGlueMeasureTool::on_btnDeleteROIAll_clicked()
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
}

void frmGlueMeasureTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->m_strModelIndex		= m_strModelIndex;
	frm->m_strTypeVariable		= m_strTypeVariable;
	frm->m_strValueName			= m_strValueName;
	frm->SetLimitModel(QString::number(((GlueMeasureTool*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();

	if (frm->m_iRetn == 2)	{
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

void frmGlueMeasureTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkFollow->clear();
}

void frmGlueMeasureTool::onChannelClicked(int Index)
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

void frmGlueMeasureTool::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
}

void frmGlueMeasureTool::onCheckTypeChanged(int Index)
{
	switch (Index)	{
	case 0: {	
		ui.stackedCheckWidget->setCurrentIndex(0);
	}break;
	default: {
		ui.stackedCheckWidget->setCurrentIndex(1);
	}	break;
	}

}

void frmGlueMeasureTool::onROIRectChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) {
		if (iter->isSelected() || iter->m_bSelected || iter->m_bShowMoveSelected)	{
			((BezierLineItem*)iter)->SetParam(cstItemNormalSelectedIndex, QString::number(Index));
			QVector<MeasureRect2> _lines = ((BezierLineItem*)iter)->GetDrawLines();
			onChangeCustomplotContour(Index, _lines);
		}
	}
	m_view->update();

}

void frmGlueMeasureTool::on_UpDateCheckType(int Index)
{

}

EnumNodeResult frmGlueMeasureTool::Execute(MiddleParam & param, QString & strError)
{
	std::vector<QPointF>	_VecPt1D;
	std::vector<sDrawText>	_VecText1D;
	std::vector<sDrawText>	_Vec1Text1D;
	std::vector<sDrawText>	_Vec2Text1D;
	std::vector<sDrawCross> _PointLst;

	QtVariantProperty*	_ptrproperty = nullptr;
	QtVariantProperty*	_ptrSubproperty = nullptr;
	_ptrproperty = m_ResultvariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");
	QString strParent = ((GlueMeasureTool*)m_pTool)->GetToolName().first;
	_ptrproperty->setPropertyName(strParent);
	_ptrproperty->setAttribute("Parent", strParent);

	ShapeToRegion(m_VecBaseItem, ModelROIRegion);
	QVector<QVector<MeasureRect2>> _vecLines = ShapeToLine(m_VecBaseItem);
	QVector<QVector<MeasureRect2>> _vecResultLines = _vecLines;
	//m_view->ClearAllDrawItem();

	int	m_iInvalidValue = -9000;
	if (databaseVar::Instance().m_mapParam.contains("spinInvalidateValue")) {
		m_iInvalidValue = databaseVar::Instance().m_mapParam["spinInvalidateValue"].toDouble();
	}
	HObject _MetrixRegion = ModelROIRegion;
	if (!_MetrixRegion.IsInitialized() || m_VecBaseItem.size() <= 0) {
		sDrawText			_strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr(" Region Isn't IsInitialized");
		_VecText1D.push_back(_strText);

		param.VecShowQPointFs = (_VecPt1D);
		param.VecDrawText = (_VecText1D);
		for (int i = 0; i < _PointLst.size(); i++) {
			param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
		}
		return NodeResult_ParamError;
	}

	if (ui.checkUseFollow->isChecked()) {
		HTuple	_Metrix;
		HTuple	Rows, Columns, _Lenth;
		HObject _MetrixContour;
		if (GlueMeasureTool::CheckMetrixType(ui.txtLinkFollow->text(), _Metrix) == 0) {
			_vecResultLines.clear();
			for (auto& _Lines : _vecLines) {
				QVector<MeasureRect2> _lines;
				for (auto& _oneLine : _Lines) {
					MeasureRect2 _line = _oneLine;
					HTuple	_Rows, _Columns;
					AffineTransPixel(_Metrix, _oneLine.line.p1().y(), _oneLine.line.p1().x(), &_Rows, &_Columns);
					_line.line.setP1(QPointF(_Columns.D(), _Rows.D()));
					AffineTransPixel(_Metrix, _oneLine.line.p2().y(), _oneLine.line.p2().x(), &_Rows, &_Columns);
					_line.line.setP2(QPointF(_Columns.D(), _Rows.D()));
					_lines.push_back(_line);
				}
				_vecResultLines.push_back(_lines);
			}
			AffineTransRegion(ModelROIRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
			_MetrixContour.GenEmptyObj();
			GetRegionContour(_MetrixRegion, &Rows, &Columns);
			HalconCpp::TupleLength(Rows, &_Lenth);
			for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
				_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
			}
		}
		else { 
			sDrawText			_strText;
			_strText.bControl	= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor	= QColor(255, 0, 0);
			_strText.strText	= tr(" The Link Matrix Failed!");
			_VecText1D.push_back(_strText);

			param.VecShowQPointFs = (_VecPt1D);
			param.VecDrawText = (_VecText1D);
			for (int i = 0; i < _PointLst.size(); i++) {
				param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
			}
			return NodeResult_ParamError; 
		}
	}
	try
	{
		int _iCheckMode = databaseVar::Instance().m_iCheckMode;
		switch (_iCheckMode) {
		case 0: {	//2.5D检测
			HTuple Width, Height;
			HalconCpp::GetImageSize(m_srcImage, &Width, &Height);
			HTuple _iNumber;
			CountObj(_MetrixRegion, &_iNumber);
			HTuple		hv_FirstRow, hv_FirstCol, hv_SecondRow, hv_SecondCol;
			HObject		ho_FirstContour, ho_SecondContour, ho_ContourOut, ho_Rectangle;
			HTuple		hv_Distance, hv_Gray, hv_Result;
			HTuple		hv_DistanceMin, hv_DistanceMax;
			HTuple		hv_ResultRow, hv_ResultCol, hv_ResultZ;
			hv_FirstRow = HTuple();
			hv_FirstCol = HTuple();
			hv_SecondRow = HTuple();
			hv_SecondCol = HTuple();
			hv_ResultRow = HTuple();
			hv_ResultCol = HTuple();
			hv_ResultZ	= HTuple();
			for (auto& _Lines : _vecResultLines) {
				for (int i = 0; i < _Lines.size(); i++) {
					HTuple RowEdgeFirst, ColumnEdgeFirst, AmplitudeFirst, RowEdgeSecond, ColumnEdgeSecond, AmplitudeSecond, IntraDistance, InterDistance;
					{
						HTuple Row, Column, Phi, Length1, Length2, hMeasureHandle, Distance, hAngle;
						DistancePp(_Lines[i].line.p1().y(), _Lines[i].line.p1().x(), _Lines[i].line.p2().y(), _Lines[i].line.p2().x(), &Distance);
						AngleLx(_Lines[i].line.p2().y(), _Lines[i].line.p2().x(), _Lines[i].line.p1().y(), _Lines[i].line.p1().x(), &hAngle);
						HalconCpp::GenMeasureRectangle2(
							_Lines[i].line.center().y(), _Lines[i].line.center().x(),
							hAngle, Distance / 2.0, _Lines[i].dNormalLineWidth / 2.0,
							Width, Height, "nearest_neighbor", &hMeasureHandle);

						HTuple hv_AmplitudeFirst, hv_AmplitudeSecond, hv_IntraDistance, hv_InterDistance;
						if (_Lines[i].bNeedCheckDistance)		{
							//测量边缘对
							MeasurePairs(m_srcImage, hMeasureHandle, _Lines[i].dSigma, _Lines[i].dThreshold,
								_Lines[i].strTransition.toLower().toStdString().c_str(), 
								_Lines[i].strSelect.toLower().toStdString().c_str(),
								&RowEdgeFirst,&ColumnEdgeFirst,
								&hv_AmplitudeFirst,&RowEdgeSecond, 
								&ColumnEdgeSecond,&hv_AmplitudeSecond, 
								&hv_IntraDistance, &hv_InterDistance);
						}
						MeasureProjection(m_srcImage, hMeasureHandle, &hv_Gray);
						CloseMeasure(hMeasureHandle);
					}

					QPointF ptMeasureCenter;
					HTuple hv_line1Row = _Lines[i].line.p1().y();
					HTuple hv_line1Col = _Lines[i].line.p1().x();
					HTuple hv_line2Row = _Lines[i].line.p2().y();
					HTuple hv_line2Col = _Lines[i].line.p2().x();
					HTuple hv_lineCenterRow = (hv_line1Row + hv_line2Row) / 2.0;
					HTuple hv_lineCenterCol = (hv_line1Col + hv_line2Col) / 2.0;
					ptMeasureCenter.setX(	hv_lineCenterCol.D()	);
					ptMeasureCenter.setY(	hv_lineCenterRow.D()	);
					if (_Lines[i].bNeedCheckDistance)		{
						HTuple hFirstNumber, hSecondNumber;
						TupleLength(RowEdgeFirst, &hFirstNumber);
						TupleLength(RowEdgeSecond, &hSecondNumber);
						if (hFirstNumber.I() > 0 && hSecondNumber.I() > 0) {
							hv_FirstRow = hv_FirstRow.TupleConcat(RowEdgeFirst[0]);
							hv_FirstCol = hv_FirstCol.TupleConcat(ColumnEdgeFirst[0]);
							hv_SecondRow = hv_SecondRow.TupleConcat(RowEdgeSecond[0]);
							hv_SecondCol = hv_SecondCol.TupleConcat(ColumnEdgeSecond[0]);
							if (m_ShowidToProperty.count(showResult) > 0)
								if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
									{
										sDrawCross _Cross;
										_Cross.Center.setX(ColumnEdgeFirst[0].D());
										_Cross.Center.setY(RowEdgeFirst[0].D());
										_Cross.fAngle = 0;
										if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_Cross.DrawColor = QColor(0, 0, 255);
										_PointLst.push_back(_Cross);
									} {
										sDrawCross _Cross;
										_Cross.Center.setX(ColumnEdgeSecond[0].D());
										_Cross.Center.setY(RowEdgeSecond[0].D());
										_Cross.fAngle = 0;
										if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_Cross.DrawColor = QColor(0, 0, 255);
										_PointLst.push_back(_Cross);
									}
								}
						}
						else {
							if (hFirstNumber.I() > 0)
								if (m_ShowidToProperty.count(showResult) > 0)
									if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
										sDrawCross _Cross;
										_Cross.Center.setX(ColumnEdgeFirst[0].D());
										_Cross.Center.setY(RowEdgeFirst[0].D());
										_Cross.fAngle = 0;
										if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_Cross.DrawColor = QColor(255, 0, 0);
										_PointLst.push_back(_Cross);
									}
							if (hSecondNumber.I() > 0)
								if (m_ShowidToProperty.count(showResult) > 0)
									if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
										sDrawCross _Cross;
										_Cross.Center.setX(ColumnEdgeSecond[0].D());
										_Cross.Center.setY(RowEdgeSecond[0].D());
										_Cross.fAngle = 0;
										if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_Cross.DrawColor = QColor(255, 0, 0);
										_PointLst.push_back(_Cross);
									}
						}

						if (hFirstNumber.I() > 0 && hSecondNumber.I() > 0) {
							hv_line1Row			= RowEdgeFirst[0];
							hv_line1Col			= ColumnEdgeFirst[0];
							hv_line2Row			= RowEdgeSecond[0];
							hv_line2Col			= ColumnEdgeSecond[0];
							hv_lineCenterRow	= (hv_line1Row + hv_line2Row) / 2.0;
							hv_lineCenterCol	= (hv_line1Col + hv_line2Col) / 2.0;

							HTuple Row, Column, Phi, Length1, Length2, hMeasureHandle, Distance, hAngle;
							DistancePp(hv_line1Row, hv_line1Col, hv_line2Row, hv_line2Col, &Distance);
							AngleLx(hv_line1Row, hv_line1Col, hv_line2Row, hv_line2Col, &hAngle);
							HalconCpp::GenMeasureRectangle2(hv_lineCenterRow, hv_lineCenterCol, hAngle,
								Distance / 2.0, _Lines[i].dNormalLineWidth / 2.0,
								Width, Height, "nearest_neighbor", &hMeasureHandle);
							//测量边缘对
							MeasureProjection(m_srcImage, hMeasureHandle, &hv_Gray);
							CloseMeasure(hMeasureHandle);
							QPointF	ptMeasureCenter;
							ptMeasureCenter.setX((ColumnEdgeSecond[0].D() + ColumnEdgeFirst[0].D()) / 2.0);
							ptMeasureCenter.setY((RowEdgeSecond[0].D() + RowEdgeFirst[0].D()) / 2.0);

							HTuple hv_Sorted, hv_SortedIndex;

							TupleSort(hv_Gray, &hv_Sorted);
							TupleSortIndex(hv_Gray, &hv_SortedIndex);
							int iLenth = hv_Gray.TupleLength().I();
							double	_dRow = (_Lines[i].line.p1().y() - _Lines[i].line.p2().y()) / iLenth;
							double	_dCol = (_Lines[i].line.p1().x() - _Lines[i].line.p2().x()) / iLenth;
							double	_dZValue = 0;
							int		_iIndex = 0;
							double	_dPosY = 0;

							switch (ui.cbx_ALargrimCheckType->currentIndex()) {
							case 0: {	//百分比
								int	_iCurrentIndex = -1;
								double	_dValue = 0;
								int	_iLowerIndex = ui.SliderPercent->lowerValue() / 100.0 * iLenth;
								int	_iUpperIndex = ui.SliderPercent->upperValue() / 100.0 * iLenth;
								int	_iUseIndex = 0;
								int _iTotalIndex = 0;
								QVector<int>	vecIndex;
								QVector<int>	vecValue;

								for (int K = _iLowerIndex; K < _iUpperIndex; K++) {
									int _iTempIndex = hv_SortedIndex[K].I();
									_dValue = _dValue + hv_Sorted[K].D();
									vecValue.push_back(hv_Sorted[K].D());
									_iUseIndex++;
									vecIndex.push_back(_iTempIndex);
									_iTotalIndex = _iTotalIndex + _iTempIndex;
								}
								_dZValue = _dValue / _iUseIndex;
								_iIndex = _iTotalIndex / _iUseIndex;
								_dPosY = hv_Gray[_iIndex];
								{
									sDrawCross _Cross;

									_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

									hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
									hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
									hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
									_Cross.fAngle = 0;
									if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 1: {	//最大值				
								int	_iCurrentIndex = hv_SortedIndex[iLenth - 1];
								_dZValue = hv_Sorted[iLenth - 1].D();
								_iIndex = _iCurrentIndex;
								_dPosY = _dZValue;

								sDrawCross _Cross;
								_Cross.Center = ptMeasureCenter;

								_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

								hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								_Cross.fAngle = 0;

								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);

							}	break;
							case 2: {	//最小值
								int	_iCurrentIndex = hv_SortedIndex[0];
								_dZValue = hv_Sorted[0].D();
								//需要过滤无效值

								_iIndex = _iCurrentIndex;
								_dPosY = _dZValue;

								sDrawCross _Cross;
								_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

								hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								_Cross.fAngle = 0;

								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}	break;
							case 3: {	//中值
								int	_iCurrentIndex = hv_SortedIndex[iLenth / 2.0];

								_dZValue = hv_Sorted[iLenth / 2.0].D();
								_iIndex = _iCurrentIndex;
								_dPosY = hv_Gray[abs(_iIndex)].D();

								sDrawCross _Cross;
								_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

								hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);

								_Cross.fAngle = 0;
								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}	break;
							case 4: {	//均值
								int	_iCurrentIndex = -1;
								HTuple	hv_Median;
								TupleMedian(hv_Gray, &hv_Median);
								for (int i = 0; i < iLenth; i++) {
									if (hv_SortedIndex[i] > hv_Median.D()) {
										_iCurrentIndex = i;	break;
									}
								}
								_dZValue = hv_Median.D();
								if (_iCurrentIndex > -1) {
									sDrawCross _Cross;
									_Cross.Center = ptMeasureCenter;

									hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
									hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
									hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);

									_Cross.fAngle = 0;
									if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 5: {	//中线
								_dZValue = hv_Gray[iLenth / 2.0].D();
								_iIndex = iLenth / 2.0;
								_dPosY = hv_Gray[abs(_iIndex)].D();

								sDrawCross _Cross;
								_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

								hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);

								_Cross.fAngle = 0;
								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}	break;
							default: {		}	break;
							}

							if (m_ShowidToProperty.count(showDrawResult) > 0)
								if (m_ShowidToProperty[showDrawResult]->value().toInt() > 0) {
									{
										sDrawText				_strText;
										_strText.bControl = false;
										_strText.Center = ptMeasureCenter;
										if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_strText.DrawColor = QColor(0, 255, 0);
										_strText.strText = QString("%1").arg(QString::number(i));
										_Vec1Text1D.push_back(_strText);
									} {
										sDrawText				_strText;
										_strText.bControl = true;
										//_strText.Center			= ptMeasureCenter;
										if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
										_strText.DrawColor = QColor(0, 255, 0);

										_strText.strText = QString("%1.%2")
											.arg(QString::number(i))
											.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision))
											;
										_Vec2Text1D.push_back(_strText);
									}
								}
						}
					}
					else	{
						HTuple	hv_Sorted,		hv_SortedIndex;
						TupleSort(hv_Gray,		&hv_Sorted);
						TupleSortIndex(hv_Gray, &hv_SortedIndex);
						int iLenth = hv_Gray.TupleLength().I();
						double	_dRow = (	_Lines[i].line.p1().y() - _Lines[i].line.p2().y()	) / iLenth;
						double	_dCol = (	_Lines[i].line.p1().x() - _Lines[i].line.p2().x()	) / iLenth;
						double	_dZValue	= 0;
						int		_iIndex		= 0;
						double	_dPosY		= 0;

						switch (ui.cbx_ALargrimCheckType->currentIndex()) {
						case 0: {	//百分比
							int	_iCurrentIndex	= -1;
							double	_dValue		= 0;
							int	_iLowerIndex	= ui.SliderPercent->lowerValue() / 100.0 * iLenth;
							int	_iUpperIndex	= ui.SliderPercent->upperValue() / 100.0 * iLenth;
							int	_iUseIndex		= 0;
							int _iTotalIndex	= 0;
							QVector<int>		vecIndex;
							QVector<int>		vecValue;
							for (int K = _iLowerIndex; K < _iUpperIndex; K++) {
								int _iTempIndex = hv_SortedIndex[K].I();
								_dValue			= _dValue + hv_Sorted[K].D();
								vecValue.push_back(hv_Sorted[K].D());
								_iUseIndex++;
								vecIndex.push_back(_iTempIndex);
								_iTotalIndex = _iTotalIndex + _iTempIndex;
							}
							_dZValue	= _dValue / _iUseIndex;
							_iIndex		= _iTotalIndex / _iUseIndex;
							_dPosY		= hv_Gray[_iIndex];
							{
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x()	+ (_iIndex) *	_dCol	);
								_Cross.Center.setY(_Lines[i].line.p2().y()	+ ( _iIndex) *	_dRow	);

								hv_ResultRow	= hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol	= hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ		= hv_ResultZ.TupleConcat(	_dZValue	);
								_Cross.fAngle = 0;
								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						case 1: {	//最大值				
							int	_iCurrentIndex = hv_SortedIndex[iLenth - 1];
							_dZValue	= hv_Sorted[iLenth - 1].D();
							_iIndex		= _iCurrentIndex;
							_dPosY		= _dZValue;

							sDrawCross _Cross;
							_Cross.Center = ptMeasureCenter;

							_Cross.Center.setX(_Lines[i].line.p2().x() + ( _iIndex) *	_dCol);
							_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex) *	_dRow);

							hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
							hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
							_Cross.fAngle = 0;

							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(0, 255, 0);
							_PointLst.push_back(_Cross);

						}	break;
						case 2: {	//最小值
							int	_iCurrentIndex = hv_SortedIndex[0];
							_dZValue	= hv_Sorted[0].D();
							//需要过滤无效值
							for (int i = 0; i < hv_Sorted.Length(); i++) {
								if (hv_Sorted[i] >= m_iInvalidValue) {
									_iCurrentIndex = hv_SortedIndex[i];
									_dZValue = hv_Sorted[i].D();
									break;
								}
							}

							_iIndex		= _iCurrentIndex;
							_dPosY		= _dZValue;

							sDrawCross _Cross;
							_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
							_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

							hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
							hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
							_Cross.fAngle = 0;

							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(0, 255, 0);
							_PointLst.push_back(_Cross);
						}	break;
						case 3: {	//中值
							int	_iCurrentIndex = hv_SortedIndex[iLenth / 2.0];

							_dZValue	= hv_Sorted[iLenth / 2.0].D();
							_iIndex		= _iCurrentIndex;
							_dPosY		= hv_Gray[abs(_iIndex)].D();

							sDrawCross _Cross;
							_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
							_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

							hv_ResultRow	= hv_ResultRow.TupleConcat(_Cross.Center.y());
							hv_ResultCol	= hv_ResultCol.TupleConcat(_Cross.Center.x());
							hv_ResultZ		= hv_ResultZ.TupleConcat(_dZValue);

							_Cross.fAngle = 0;
							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(0, 255, 0);
							_PointLst.push_back(_Cross);
						}	break;
						case 4: {	//均值
							int	_iCurrentIndex = -1;
							HTuple	hv_Median;
							TupleMedian(hv_Gray, &hv_Median);
							for (int i = 0; i < iLenth; i++) {
								if (hv_SortedIndex[i] > hv_Median.D()) {
									_iCurrentIndex = i;	break;
								}
							}
							_dZValue = hv_Median.D();
							if (_iCurrentIndex > -1) {
								sDrawCross _Cross;
								_Cross.Center = ptMeasureCenter;

								hv_ResultRow	= hv_ResultRow.TupleConcat(_Cross.Center.y());
								hv_ResultCol	= hv_ResultCol.TupleConcat(_Cross.Center.x());
								hv_ResultZ		= hv_ResultZ.TupleConcat(_dZValue);

								_Cross.fAngle = 0;
								if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						case 5: {	//中线
							_dZValue = hv_Gray[iLenth / 2.0].D();
							_iIndex = iLenth / 2.0;
							_dPosY = hv_Gray[abs(_iIndex)].D();

							sDrawCross _Cross;

							_Cross.Center.setX(_Lines[i].line.p2().x() + _iIndex *	_dCol);
							_Cross.Center.setY(_Lines[i].line.p2().y() + _iIndex *	_dRow);

							hv_ResultRow = hv_ResultRow.TupleConcat(_Cross.Center.y());
							hv_ResultCol = hv_ResultCol.TupleConcat(_Cross.Center.x());
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);

							_Cross.fAngle = 0;
							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(0, 255, 0);
							_PointLst.push_back(_Cross);
						}	break;
						default: {		}	break;
						}

						if (m_ShowidToProperty.count(showDrawResult) > 0) {
							if (m_ShowidToProperty[showDrawResult]->value().toInt() > 0) {
								{
									sDrawText				_strText;
									_strText.bControl = false;
									_strText.Center = ptMeasureCenter;
									if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
									_strText.DrawColor = QColor(0, 255, 0);
									_strText.strText = QString("%1").arg(QString::number(i));
									_Vec1Text1D.push_back(_strText);
								} {
									sDrawText				_strText;
									_strText.bControl = true;
									if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
									_strText.DrawColor = QColor(0, 255, 0);

									_strText.strText = QString("%1.%2")
										.arg(QString::number(i))
										.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision))
										;
									_Vec2Text1D.push_back(_strText);
								}
							}
						}
					}

				}
			}

			{
				HTuple hSortNumber, hSorted;
				TupleSort(hv_Distance, &hSorted);
				TupleLength(hSorted, &hSortNumber);
				for (int i = 0; i < hSortNumber.I(); i++) {
					//显示结果
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, "Height" + QString::number(i));
					_ptrSubproperty->setValue(QString::number(hSorted[i], 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				}
			}
			if (m_ShowidToProperty.count(showregion) > 0)
				if (m_ShowidToProperty[showregion]->value().toInt() > 0) {
					for (int i = 0; i < _vecResultLines.size(); i++) {
						for (int j = 0; j < _vecResultLines[i].size(); j++) {
							sColorLineRect _LineRect;
							_LineRect.Dline = _vecResultLines[i][j].line;
							_LineRect.DWidth = _vecResultLines[i][j].dNormalLineWidth;
							_LineRect.DrawQColor = QColor(0, 0, 255);;
							_LineRect.LineWidth = -1;
							param.VecShowColorLineRect.push_back(_LineRect);
						}
					}
				}

		}	break;
		case 1: { //3D检测

		}	break;
		default: {	}break;
		}
		sDrawText				_strText;
		_strText.bControl		= true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(0, 255, 0);
		_strText.strText = tr("Successed In Glue Detection");
		_VecText1D.push_back(_strText);
		for (int i = 0; i < _Vec2Text1D.size(); i++)	_VecText1D.push_back(_Vec2Text1D[i]);
		for (int i = 0; i < _Vec1Text1D.size(); i++)	_VecText1D.push_back(_Vec1Text1D[i]);
	}
	catch (const HException& except) {
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_strText.strText = tr("Error In Glue Detection") + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...) {
		sDrawText			_strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("Error In Glue Detection");
		_VecText1D.push_back(_strText);
	}

	param.VecShowQPointFs = (_VecPt1D);
	param.VecDrawText = (_VecText1D);
	for (int i = 0; i < _PointLst.size(); i++) {
		param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
	}
	return NodeResult_OK;
}

void frmGlueMeasureTool::slot_MouseClicked(QList<QGraphicsItem*>& items)
{
	if (items.size() < 0)	return;

	ClearROIProperty();

	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;
	QtVariantProperty *_ptrSubSubproperty = nullptr;

	for (int i = 0; i < items.size(); i++) {
		QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
		((BaseItem*)items[i])->GetAllParam(_mapValue);

		for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
			if (((*mapIter).first).count(cstItem) > 0) {
				_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
				for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
					int				iEnable;
					EnumParamType	iType;
					QString			Param;
					QStringList strEnumLst;
					QString			LowParam;
					QString			HighParam;
					QString			strLang;
					if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
						switch (iType) {
						case ParamType_Bool:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Bool, (*pIter).first);
							break;
						case ParamType_Int:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Int, (*pIter).first);
							break;
						case ParamType_Double:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
							break;
						case ParamType_String:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::String, (*pIter).first);
							break;
						case ParamType_Enum:
							_ptrSubSubproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
							_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
							break;
						case ParamType_Other:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
							break;
						default:		break;
						}
						_ptrSubSubproperty->setValue(Param);
						_ptrSubSubproperty->setToolTip(Param);
						_ptrSubSubproperty->setEnabled(iEnable == 1 ? true : false);
						_ptrSubSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
						if (!LowParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("minimum"), LowParam);
						if (!HighParam.isEmpty())	_ptrSubSubproperty->setAttribute(QLatin1String("maximum"), HighParam);
					}
					else {
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
						_ptrSubSubproperty->setValue((*pIter).second);
					}
					idToProperty[(*pIter).first] = _ptrSubSubproperty;
					idToProperty[(*pIter).first]->setPropertyName(strLang);
					propertyToId[_ptrSubSubproperty] = (*pIter).first;
					_ptrproperty->addSubProperty(_ptrSubSubproperty);
				}
				addProperty(_ptrproperty, ((*mapIter).first));
				_ptrproperty->setPropertyName(BaseItem::tr(cstItem.toStdString().c_str()));
			}
			else
			{
				_ptrSubproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), ((*mapIter).first));
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
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Bool, (*pIter).first);
							break;
						case ParamType_Int:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Int, (*pIter).first);
							break;
						case ParamType_Double:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
							break;
						case ParamType_String:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::String, (*pIter).first);
							break;
						case ParamType_Enum:
							_ptrSubSubproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*pIter).first);
							_ptrSubSubproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
							break;
						case ParamType_Other:
							_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
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
						_ptrSubSubproperty = variantManager->addProperty(QVariant::Double, (*pIter).first);
						_ptrSubSubproperty->setValue((*pIter).second);
					}
					idToProperty[(*pIter).first] = _ptrSubSubproperty;
					idToProperty[(*pIter).first]->setPropertyName(strLang);
					propertyToId[_ptrSubSubproperty] = (*pIter).first;
					_ptrSubproperty->addSubProperty(_ptrSubSubproperty);
				}
				addProperty(_ptrSubproperty, (*mapIter).first);
			}
		}
	}


}

void frmGlueMeasureTool::slot_MouseMouve(QList<QGraphicsItem*>& items)
{
	for (int i = 0; i < items.size(); i++) {
		bool bFind = false;

		QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
		float fData[5] = { 0 };
		switch (((BaseItem*)items[i])->GetItemType()) {
		case ItemPoint: {
			PointItem *pValueItem = (PointItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemCircle: {
			CircleItem *pValueItem = (CircleItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemEllipse: {
			EllipseItem *pValueItem = (EllipseItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *pValueItem = (ConcentricCircleItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemRectangle: {
			RectangleItem *pValueItem = (RectangleItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemRectangleR: {
			RectangleRItem *pValueItem = (RectangleRItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemSquare: {
			QString strParent = QString::number(i) + ".方形";

		}; break;
		case ItemPolygon: {
			PolygonItem *pValueItem = (PolygonItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemBezierLineObj: {
			BezierLineItem *pValueItem = (BezierLineItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemLineObj: {
			LineItem *pValueItem = (LineItem*)items[i];
			if (pValueItem != nullptr)		pValueItem->GetAllParam(_mapValue);
		}; break;
		case ItemArrowLineObj: {
			LineArrowItem *pValueItem = (LineArrowItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		case LineCaliperObj: {
			LineCaliperItem *pValueItem = (LineCaliperItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		case ArcCaliperObj: {
			ArcCaliperItem *pValueItem = (ArcCaliperItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}; break;
		default:	break;
		}
		for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
			for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
				QString	strName = (*pIter).first;
				if (idToProperty.count(strName) > 0) {
					int				iEnable;
					EnumParamType	iType;
					QString			Param;
					QStringList		strEnumLst;
					QString			LowParam;
					QString			HighParam;
					QString			strLang;
					if (ValueToParam((*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
						idToProperty[strName]->setValue(Param);
						idToProperty[strName]->setEnabled(iEnable == 1 ? true : false);
						if (!LowParam.isEmpty())	idToProperty[strName]->setAttribute(QLatin1String("minimum"), LowParam);
						if (!HighParam.isEmpty())	idToProperty[strName]->setAttribute(QLatin1String("maximum"), HighParam);

						idToProperty[strName]->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					}
					else {
						idToProperty[strName]->setValue(Param);
					}
					idToProperty[strName]->setPropertyName(strLang);
				}
			}
		}
	}

}

void frmGlueMeasureTool::slot_mousePressEvent(QMouseEvent * ev)
{
	if (ev->button() == Qt::LeftButton)
	{
		auto _Pos = m_view->mapToScene(ev->pos());
		if (m_VecBaseItem.size() > 0){
			auto _pItem = m_VecBaseItem[m_VecBaseItem.size() - 1];
			if (_pItem->GetItemType() == ItemPolygon ){
				if (!_pItem->IsDrawEnd()){
					((PolygonItem*)_pItem)->PushPos(_Pos);
					((PolygonItem*)_pItem)->m_bSelected = true;
				}
			}
			else if (_pItem->GetItemType() == ItemBezierLineObj) {
				if (!_pItem->IsDrawEnd()) {
					((BezierLineItem*)_pItem)->PushPos(_Pos);
					((BezierLineItem*)_pItem)->m_bSelected = true;
				}
			}

		}
	}
	else if (ev->button() == Qt::RightButton)
	{

	}

}

void frmGlueMeasureTool::slot_mouseMoveEvent(QMouseEvent * ev)
{

}

void frmGlueMeasureTool::slot_mouseReleaseEvent(QMouseEvent * ev)
{
	bool _bAllSelected = true;
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) {
		if (iter->isSelected() || iter->m_bSelected || iter->m_bShowMoveSelected) {
			_bAllSelected &= true;
		}
		else	{
			_bAllSelected &= false;
			((BezierLineItem*)iter)->SetParam(cstItemNormalSelectedIndex, QString::number(-1));
		}
	}
	if (!_bAllSelected)	{
		ui.widgetPlot->clearGraphs();
		ui.widgetPlot->rescaleAxes();
		ui.widgetPlot->DrawClearAllItem();
		ui.cbx_CheckIndex->clear();

		//ui.widgetPlot->UpDatePlot();
		ui.widgetPlot->replot();
	}
	m_view->update();
}

QVector<QVector<MeasureRect2>>  frmGlueMeasureTool::ShapeToLine(QVector<BaseItem*>& vecItems)
{
	QVector<QVector<MeasureRect2>> veclines;
	for (auto iter : vecItems) {
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			switch (Item->GetItemType()) {
			case ItemPoint: {
			}	break;
			case ItemCircle: {
			}	break;
			case ItemEllipse: {
			}	break;
			case ItemConcentric_Circle: {
			}	break;
			case ItemRectangle: {
			}	break;
			case ItemRectangleR: {
			}	break;
			case ItemPolygon: {
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *_pItem = (BezierLineItem*)Item;
				QVector<MeasureRect2>	_lines;
				//QList<QPointF> points;
				_lines = _pItem->GetDrawLines();
				veclines.push_back(_lines);
			}	break;
			case ItemLineObj: {
			}	break;
			case ItemArrowLineObj: {
			}	break;
			}
			switch (Item->m_eRegionType) {
			case RegionType_Add: {
			} break;
			case RegionType_Differe: {
			} break;
			}
		}
	}
	return veclines;
}

int frmGlueMeasureTool::Load()
{
	m_titleBar->setTitleContent(((GlueMeasureTool*)m_pTool)->GetItemId());

	//设置图像输入
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((GlueMeasureTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((GlueMeasureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((GlueMeasureTool*)m_pTool)->GetItemId()))	{
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((GlueMeasureTool*)m_pTool)->m_strInImg);
	if (ui.cbx_ImgInPut->currentIndex() >= 0) {
		slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	}

	if (MiddleParam::MapMiddle()[((GlueMeasureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ((GlueMeasureTool*)m_pTool)->m_strInImg ) > 0)	{
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((GlueMeasureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

	on_btnDeleteROIAll_clicked();
	//DataToShape(((GlueMeasureTool*)m_pTool)->m_vecCommonData, m_VecBaseItem);
	QJsonToShape(	((GlueMeasureTool*)m_pTool)	->m_vecROIData, m_VecBaseItem);

	for(auto pItem : m_VecBaseItem)	m_view->AddItems(pItem);

	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("spinRoiW"))	ui.spinRoiW->setValue(((GlueMeasureTool*)m_pTool)->m_mapParam["spinRoiW"].toInt());
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		m_VecBaseItem[i]->setIndex(i);
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}

	//设置默认参数
	if (ui.cbx_ImgInPut->currentIndex() >= 0)					slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("checkUseFollow"))			ui.checkUseFollow->setChecked(((GlueMeasureTool*)m_pTool)->m_mapParam["checkUseFollow"].toInt());
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("txtLinkFollow"))			ui.txtLinkFollow->setText(((GlueMeasureTool*)m_pTool)->m_mapParam["txtLinkFollow"]);

	//检测结果
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("spinMinValue"))			ui.spinMinValue->setValue(((GlueMeasureTool*)m_pTool)->m_mapParam["spinMinValue"].toDouble());
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("spinMaxValue"))			ui.spinMaxValue->setValue(((GlueMeasureTool*)m_pTool)->m_mapParam["spinMaxValue"].toDouble());

	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("SliderLowPercent"))		ui.SliderPercent->setLowerValue(((GlueMeasureTool*)m_pTool)->m_mapParam["SliderLowPercent"].toDouble());
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("SliderHighPercent"))		ui.SliderPercent->setUpperValue(((GlueMeasureTool*)m_pTool)->m_mapParam["SliderHighPercent"].toDouble());
	if (((GlueMeasureTool*)m_pTool)->m_mapParam.contains("cbx_ALargrimCheckType"))	ui.cbx_ALargrimCheckType->setCurrentIndex(((GlueMeasureTool*)m_pTool)->m_mapParam["cbx_ALargrimCheckType"].toDouble());
	
	//设置显示参数
	for (auto iter = ((GlueMeasureTool*)m_pTool)->m_mapShowParam.begin(); iter != ((GlueMeasureTool*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)	m_ShowidToProperty[iter.key()]->setValue(iter.value());

	onCheckTypeChanged(ui.cbx_ALargrimCheckType->currentIndex());

	return 0;
}

int frmGlueMeasureTool::Save()
{
	((GlueMeasureTool*)m_pTool)->m_strInImg		= ui.cbx_ImgInPut->currentText();

	((GlueMeasureTool*)m_pTool)->m_vecROIData	= QJsonArray();
	ShapeToQJson(m_VecBaseItem, ((GlueMeasureTool*)m_pTool)->m_vecROIData);

	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("spinRoiW",				QString::number(ui.spinRoiW->value()));
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("checkUseFollow",		QString::number(ui.checkUseFollow->isChecked()));
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("txtLinkFollow",			ui.txtLinkFollow->text());
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("spinRoiW",				QString::number(ui.spinRoiW->value()));

	((GlueMeasureTool*)m_pTool)->m_strLinkName								= ui.txtLinkFollow->text();
	((GlueMeasureTool*)m_pTool)->m_bEnableCom								= ui.checkUseFollow->isChecked();
	
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("cbx_ALargrimCheckType",	QString::number(ui.cbx_ALargrimCheckType->currentIndex()));
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("SliderLowPercent",		QString::number(ui.SliderPercent->lowerValue()));
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("SliderHighPercent",		QString::number(ui.SliderPercent->upperValue()));

	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("spinMinValue",			QString::number(ui.spinMinValue->value()));
	((GlueMeasureTool*)m_pTool)->m_mapParam.insert("spinMaxValue",			QString::number(ui.spinMaxValue->value()));

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((GlueMeasureTool*)m_pTool)->m_mapShowParam[iter.key()] = iter.value()->value().toString();

	return 0;
}

int frmGlueMeasureTool::PrExecute()
{
	for (auto iter : m_ResultidToProperty)	if (iter != nullptr)	delete iter;	m_ResultidToProperty.clear();
	m_ResultpropertyToId.clear();
	return 0;
}

int frmGlueMeasureTool::ExecuteComplete()
{
	for (auto iter = m_ResultidToProperty.begin(); iter != m_ResultidToProperty.end(); iter++)	{
		QtBrowserItem *item = m_ResultpropertyEditor->addProperty(*iter);
		m_ResultpropertyEditor->setExpanded(item, true);
	}
	return 0;
}

void frmGlueMeasureTool::addProperty(QtVariantProperty * property, const QString & id)
{
	propertyToId[property] = id;
	idToProperty[id] = property;
	QtBrowserItem *item = propertyEditor->addProperty(property);
	if (cstItem == id)	{
		propertyEditor->setExpanded(item, true);
	}
	else {
		propertyEditor->setExpanded(item, false);
	}
}

void frmGlueMeasureTool::slot_valueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)		return;
	QString strName = property->propertyName();
	if (propertyToId.count(property) > 0){
		QString _strName = propertyToId[property];
		if (idToProperty.count(propertyToId[property]) > 0)	{
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++)			{
				((BaseItem*)items[i])->SetParam(propertyToId[property], value.toString());
				((BaseItem*)items[i])->update();

			}
		}
	}
}

void frmGlueMeasureTool::slot_Result_valueChanged(QtProperty * property, const QVariant & value)
{
	if (property == nullptr)		return;


}

void frmGlueMeasureTool::slot_ChangeMatchImg_Index(int Index)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((GlueMeasureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((GlueMeasureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

void frmGlueMeasureTool::slot_customContextMenuRequested(const QPoint & pos)
{
	QMenu*	m_QMenu = new QMenu();

	//设置参数
	QPushButton* moveSetAction_buf = new QPushButton(m_QMenu);
	moveSetAction_buf->setText(tr("Set Param"));
	moveSetAction_buf->setIconSize(QSize(20, 20));
	moveSetAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSetAction = new QWidgetAction(this);
	moveSetAction->setDefaultWidget(moveSetAction_buf);
	connect(moveSetAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_SetParamMode);
	connect(moveSetAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveSetAction);

	//重置区域限制
	QPushButton* moveSetLimitAction_buf = new QPushButton(m_QMenu);
	moveSetLimitAction_buf->setText(tr("Reset Limits"));
	moveSetLimitAction_buf->setIconSize(QSize(20, 20));
	moveSetLimitAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSetLimitAction = new QWidgetAction(this);
	moveSetLimitAction->setDefaultWidget(moveSetLimitAction_buf);
	connect(moveSetLimitAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_ResetLimit);
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
		if (m_btnHintAction != nullptr)
		{
			if (m_btnHintAction->isChecked()) {
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
	connect(moveScaleAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_ZoomMode);
	connect(moveScaleAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveScaleAction);

	//拖动
	QPushButton* moveDragAction_buf = new QPushButton(m_QMenu);
	moveDragAction_buf->setText(tr("Drag Mode"));
	moveDragAction_buf->setIconSize(QSize(20, 20));
	moveDragAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveDragAction = new QWidgetAction(this);
	moveDragAction->setDefaultWidget(moveDragAction_buf);
	connect(moveDragAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_DragMode);
	connect(moveDragAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveDragAction);

	QPushButton* movSelectAction_buf = new QPushButton(m_QMenu);
	movSelectAction_buf->setText(tr("Select Mode"));
	movSelectAction_buf->setIconSize(QSize(20, 20));
	movSelectAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveSelectAction = new QWidgetAction(this);
	moveSelectAction->setDefaultWidget(movSelectAction_buf);
	connect(movSelectAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_SelectMode);
	connect(movSelectAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveSelectAction);

	QPushButton* moveFitAction_buf = new QPushButton(m_QMenu);
	moveFitAction_buf->setText(tr("AutoFit"));
	moveFitAction_buf->setIconSize(QSize(20, 20));
	moveFitAction_buf->setMinimumSize(QSize(120, 40));
	QWidgetAction* moveFitAction = new QWidgetAction(this);
	moveFitAction->setDefaultWidget(moveFitAction_buf);
	connect(moveFitAction_buf, &QPushButton::clicked, this, &frmGlueMeasureTool::slot_customplot_FitAxis);
	connect(moveFitAction_buf, &QPushButton::clicked, m_QMenu, &QMenu::close);
	m_QMenu->addAction(moveFitAction);

	m_QMenu->exec(QCursor::pos());

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

void frmGlueMeasureTool::slot_customplot_FitAxis()
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

void frmGlueMeasureTool::slot_customplot_ZoomMode()
{
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
}

void frmGlueMeasureTool::slot_customplot_DragMode()
{
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, true);//使能拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
}

void frmGlueMeasureTool::slot_customplot_SelectMode()
{
	for (int i = 0; i < ui.widgetPlot->graphCount(); i++)
	{
		ui.widgetPlot->graph(i)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
	}
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);
}

void frmGlueMeasureTool::slot_customplot_SetParamMode()
{
	slot_customplot_FitAxis();
	ui.widgetPlot->setInteraction(QCP::iRangeDrag, false);//取消拖动
	ui.widgetPlot->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
	ui.widgetPlot->UpDatePlot();
	ui.widgetPlot->RePlotALLTrace();
}

void frmGlueMeasureTool::slot_customplot_ResetLimit()
{
	ui.widgetPlot->ResetALLLimit();
	ui.widgetPlot->UpDatePlot();
	ui.widgetPlot->RePlotALLTrace();
}

void frmGlueMeasureTool::onChangeCustomplotContour(int iIndex, QVector<MeasureRect2>&_lines)
{
	if (iIndex >= _lines.size())	{
		iIndex = _lines.size() - 1;
	}
	if (_lines.size() <= 0)	{
		//iIndex = -1;
		return;
	}
	else if (iIndex < 0){
		iIndex = 0;
	}
	ui.cbx_CheckIndex->setCurrentIndex(iIndex);
	auto _LineItem = _lines[iIndex];
	m_view->ClearAllDrawItem();
	QVector<QPointF>	_VecPt1D;
	QVector<sDrawText>	_VecText1D;
	QVector<sDrawCross> _PointLst;

	ui.widgetPlot->clearGraphs();
	ui.widgetPlot->rescaleAxes();
	ui.widgetPlot->DrawClearAllItem();

	QCPGraph *graph1 = ui.widgetPlot->addGraph();
	ui.widgetPlot->graph(0)->setPen(QPen(Qt::blue, 2));
	ui.widgetPlot->graph(0)->setName("Measurement");
	ui.widgetPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
	graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 0.5), QBrush(Qt::white), 2));

	QVector<double> x, y;
	QVector<double> vecX, vecY, vecZ;
	HObject	RegionLines, Contours;
	HTuple Row, Col, Grayval;
	int	m_iInvalidValue = -9000;
	if (databaseVar::Instance().m_mapParam.contains("spinInvalidateValue")) {
		m_iInvalidValue = databaseVar::Instance().m_mapParam["spinInvalidateValue"].toDouble();
	}
	vecX.clear(); vecY.clear(); vecZ.clear();
	HTuple  hv_Width, hv_Height, hv_Min, hv_Max, _Range;
	GetImageSize(m_srcImage, &hv_Width, &hv_Height);
	QRectF _rect;
	_rect.setLeft(0);
	_rect.setTop(0);
	_rect.setRight(hv_Width.D() - 1);
	_rect.setBottom(hv_Height.D() - 1);
	QLineF _line;
	
	_line.setP1(QPointF(_LineItem.line.x1(), _LineItem.line.y1()));
	_line.setP2(QPointF(_LineItem.line.x2(), _LineItem.line.y2()));

	auto _IntersectPos = lineRectIntersection(_line, _rect);
	if (_IntersectPos.size() == 0) {
		if (_rect.contains(_line.p1()) || _rect.contains(_line.p2())) {}
		else { return; }
	}
	else if (_IntersectPos.size() == 1) {
		if (_rect.contains(_line.p1())) {
			_LineItem.line.setP1(_line.p1());
			_LineItem.line.setP2(_IntersectPos[0]);
		}
		else if (_rect.contains(_line.p2())) {
			_LineItem.line.setP2(_line.p2());
			_LineItem.line.setP1(_IntersectPos[0]);
		}
		else { return; }
	}
	else if (_IntersectPos.size() == 2) {
		if (_rect.contains(_line.p1())) {
			_LineItem.line.setP1(_line.p1());
			_LineItem.line.setP2(_IntersectPos[0]);
		}
		else if (_rect.contains(_line.p2())) {
			_LineItem.line.setP1(_IntersectPos[0]);
			_LineItem.line.setP2(_line.p2());
		}
		else { return; }
	}

	QVector<double>		_dx, _dy;
	QVector<QString>	strVecName;
	QVector<QPen>		strVecQPen;
	HTuple RowEdgeFirst, ColumnEdgeFirst, AmplitudeFirst, RowEdgeSecond, ColumnEdgeSecond, AmplitudeSecond, IntraDistance, InterDistance;
	try
	{
		HTuple Row, Column, Phi, Length1, Length2, hMeasureHandle, Distance, hAngle;
		DistancePp(_LineItem.line.p1().y(), _LineItem.line.p1().x(), _LineItem.line.p2().y(), _LineItem.line.p2().x(), &Distance);
		AngleLx(_LineItem.line.p2().y(), _LineItem.line.p2().x(), _LineItem.line.p1().y(), _LineItem.line.p1().x(), &hAngle);
		HalconCpp::GenMeasureRectangle2(_LineItem.line.center().y(), _LineItem.line.center().x(), hAngle, Distance / 2.0,
			_LineItem.dNormalLineWidth / 2.0, hv_Width, hv_Height, "nearest_neighbor", &hMeasureHandle);
		//显示
		HTuple hv_AmplitudeFirst, hv_AmplitudeSecond, hv_IntraDistance, hv_InterDistance;
		//测量出灰度
		MeasureProjection(m_srcImage, hMeasureHandle, &m_hGrayval);
		//测量边缘对
		if (_LineItem.bNeedCheckDistance)	{
			MeasurePairs(m_srcImage, hMeasureHandle, _LineItem.dSigma, _LineItem.dThreshold,
				_LineItem.strTransition.toLower().toStdString().c_str(), _LineItem.strSelect.toLower().toStdString().c_str(),
				&RowEdgeFirst, &ColumnEdgeFirst, &hv_AmplitudeFirst, &RowEdgeSecond, &ColumnEdgeSecond,
				&hv_AmplitudeSecond, &hv_IntraDistance, &hv_InterDistance);
		}
		CloseMeasure(hMeasureHandle);

		QPointF ptMeasureCenter;
		HTuple hv_line1Row		= _LineItem.line.p1().y();
		HTuple hv_line1Col		= _LineItem.line.p1().x();
		HTuple hv_line2Row		= _LineItem.line.p2().y();
		HTuple hv_line2Col		= _LineItem.line.p2().x();
		HTuple hv_lineCenterRow = (hv_line1Row + hv_line2Row) / 2.0;
		HTuple hv_lineCenterCol = (hv_line1Col + hv_line2Col) / 2.0;
		ptMeasureCenter.setX(hv_lineCenterCol.D());
		ptMeasureCenter.setY(hv_lineCenterRow.D());

		int		iLenth	= m_hGrayval.TupleLength().I();
		double	_dRow	= (_LineItem.line.p1().y() - _LineItem.line.p2().y()) / iLenth;
		double	_dCol	= (_LineItem.line.p1().x() - _LineItem.line.p2().x()) / iLenth;

		if (_LineItem.bNeedCheckDistance)
		{
			HTuple hFirstNumber, hSecondNumber, hGray;
			TupleLength(RowEdgeFirst, &hFirstNumber);
			TupleLength(RowEdgeSecond, &hSecondNumber);
			if (hFirstNumber.I() > 0 && hSecondNumber.I() > 0) {
				{
					sDrawCross _Cross;
					_Cross.Center.setX(ColumnEdgeFirst[0].D());
					_Cross.Center.setY(RowEdgeFirst[0].D());
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 0, 255);
					_PointLst.push_back(_Cross);
				} 
				{
					sDrawCross _Cross;
					_Cross.Center.setX(ColumnEdgeSecond[0].D());
					_Cross.Center.setY(RowEdgeSecond[0].D());
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 0, 255);
					_PointLst.push_back(_Cross);
				}

				double hv_line1Row		= RowEdgeFirst[0].D();
				double hv_line1Col		= ColumnEdgeFirst[0].D();
				double hv_line2Row		= RowEdgeSecond[0].D();
				double hv_line2Col		= ColumnEdgeSecond[0].D();
				int		_iFirstIndex	= ((RowEdgeFirst[0].D() - _LineItem.line.p2().y()) / _dRow);
				double	_dPosX			= ColumnEdgeFirst[0].D();
				double	_dPosY			= m_hGrayval[abs(_iFirstIndex)].D();
				_dx.push_back(abs(_iFirstIndex));
				_dy.push_back(_dPosY);
				_dPosX = ColumnEdgeSecond[0].D();
				int	_iEndIndex = abs((RowEdgeSecond[0].D() - _LineItem.line.p2().y()) / _dRow);
				_dPosY = m_hGrayval[abs(_iEndIndex)].D();

				_dx.push_back(_iEndIndex);
				_dy.push_back(_dPosY);

				strVecName.push_back(QString("%1(%2,%3)").arg(tr("Point")).arg(QString::number(_dx[0], 'f', databaseVar::Instance().form_System_Precision)).arg(QString::number(_dy[0], 'f', databaseVar::Instance().form_System_Precision)));
				strVecQPen.push_back(QPen(Qt::blue, 2, Qt::DashDotDotLine));
				strVecName.push_back(QString("%1(%2,%3)").arg(tr("Point")).arg(QString::number(_dx[1], 'f', databaseVar::Instance().form_System_Precision)).arg(QString::number(_dy[1], 'f', databaseVar::Instance().form_System_Precision)));
				strVecQPen.push_back(QPen(Qt::blue, 2, Qt::DashDotDotLine));
				ptMeasureCenter.setX((ColumnEdgeSecond[0].D() + ColumnEdgeFirst[0].D()) / 2.0);
				ptMeasureCenter.setY((RowEdgeSecond[0].D() + RowEdgeFirst[0].D()) / 2.0);
				QLineF _lineF;
				_lineF.setP1(QPointF(_LineItem.line.p1()));
				_lineF.setP2(QPointF(_LineItem.line.p2()));
				int	_iIndex = 0;
				TupleSelectRange(m_hGrayval, abs(_iFirstIndex), abs(_iEndIndex), &hGray);
				int	ihvLenth = hGray.TupleLength().I();

				HTuple hv_Sorted, hv_SortedIndex;
				TupleSort(hGray, &hv_Sorted);
				TupleSortIndex(hGray, &hv_SortedIndex);
				double	_dZValue = 0;

				bool	_bEnableShowPoint = true;

				switch (ui.cbx_ALargrimCheckType->currentIndex()) {
				case 0: {	//百分比
					int		_iCurrentIndex = -1;
					double	_dValue = 0;
					int		_iLowerIndex = ui.SliderPercent->lowerValue() / 100.0 * ihvLenth;
					int		_iUpperIndex = ui.SliderPercent->upperValue() / 100.0 * ihvLenth;
					int		_iUseIndex = 0;
					for (int i = 0; i < ihvLenth; i++) {
						if ((i >= _iLowerIndex) && (i <= _iUpperIndex)) {
							_dValue = _dValue + hv_Sorted[i];
							_iUseIndex++;
						}
					}
					_dZValue = _dValue / _iUseIndex;
					_iIndex = ((ptMeasureCenter.y() - _lineF.p2().y()) / _dRow);
					_dPosY = m_hGrayval[abs(_iIndex)].D();
					_bEnableShowPoint = false;

					{
						QLineF _line;
						_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
						_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = LineType::HorizonLine;
						if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
					}
				}		break;
				case 1: {	//最大值
					int	_iCurrentIndex = hv_SortedIndex[ihvLenth - 1];
					_dZValue = hv_Sorted[ihvLenth - 1].D();
					sDrawCross				_Cross;
					//_Cross.Center			= ptMeasureCenter;
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_iIndex = _iCurrentIndex + _iFirstIndex;
					_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex * _dCol);
					_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex * _dRow);
					_dPosY = _dZValue;
					//_iIndex				= abs(_iCurrentIndex) + abs(_iFirstIndex);
					_PointLst.push_back(_Cross);
				}	break;
				case 2: {	//最小值
					int	_iCurrentIndex = hv_SortedIndex[0];
					_dZValue = hv_Sorted[0].D();
					//需要过滤无效值
					for (int i = 0; i < hv_Sorted.Length(); i++) {
						if (hv_Sorted[i] >= m_iInvalidValue) {
							_iCurrentIndex = hv_SortedIndex[i];
							_dZValue = hv_Sorted[i].D();
							break;
						}
					}

					sDrawCross				_Cross;
					//_Cross.Center			= ptMeasureCenter;
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_iIndex = _iCurrentIndex + _iFirstIndex;
					_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex * _dCol);
					_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex * _dRow);
					_dPosY = _dZValue;
					_PointLst.push_back(_Cross);
				}	break;
				case 3: {	//中值
					int	_iCurrentIndex = hv_SortedIndex[ihvLenth / 2.0];
					_dZValue = hv_Sorted[ihvLenth / 2.0].D();
					sDrawCross _Cross;
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_iIndex = iLenth / 2.0 + _iFirstIndex;
					_Cross.Center = ptMeasureCenter;
					//_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex * _dCol);
					//_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex * _dRow);
					_PointLst.push_back(_Cross);

					_dPosY = m_hGrayval[abs(_iIndex)].D();
					{
						QLineF _line;
						_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
						_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = LineType::HorizonLine;
						if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
					}
					_bEnableShowPoint = false;

				}	break;
				case 4: {	//均值
					int	_iCurrentIndex = -1;
					HTuple	hv_Median;
					TupleMedian(m_hGrayval, &hv_Median);
					for (int i = 0; i < ihvLenth; i++) {
						if (hv_Sorted[i] > hv_Median.D()) {
							_iCurrentIndex = i;	break;
						}
					}
					_iIndex = _iCurrentIndex + _iFirstIndex;
					_dZValue = hv_Median.D();
					if (_iCurrentIndex > -1) {
						sDrawCross			_Cross;
						_Cross.Center = ptMeasureCenter;
						_Cross.fAngle = 0;
						if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
						_Cross.DrawColor = QColor(0, 255, 0);
						_PointLst.push_back(_Cross);

						{
							QLineF _line;
							_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
							_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
							ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
							ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = LineType::HorizonLine;
							if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
						}
						//_iIndex = abs(_iCurrentIndex) + abs(_iFirstIndex);
					}
					_bEnableShowPoint = false;
				}	break;
				case 5: {	//中线
					_dZValue = m_hGrayval[ihvLenth / 2.0].D();
					sDrawCross				_Cross;
					_Cross.Center = ptMeasureCenter;
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_PointLst.push_back(_Cross);
					_iIndex = ((ptMeasureCenter.y() - _lineF.p2().y()) / _dRow);
					_dPosY = m_hGrayval[abs(_iIndex)].D();
					_bEnableShowPoint = false;
					//_iIndex = abs(_iIndex) + abs(_iFirstIndex);
				}	break;
				default: {	}	break;
				}

				_dx.push_back(abs(_iIndex));
				_dy.push_back(_dPosY);

				if (_bEnableShowPoint) {

					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("%1(%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(ptMeasureCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(ptMeasureCenter.y(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision))
					);
				}

				ui.widgetPlot->DrawItemPoint(0, _dx, _dy, strVecName, strVecQPen);
			}
			else {
				if (hFirstNumber.I() > 0)
					if (m_ShowidToProperty.count(showResult) > 0)
						if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
							sDrawCross _Cross;
							_Cross.Center.setX(ColumnEdgeFirst[0].D());
							_Cross.Center.setY(RowEdgeFirst[0].D());
							_Cross.fAngle = 0;
							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(255, 0, 0);
							_PointLst.push_back(_Cross);
						}
				if (hSecondNumber.I() > 0)
					if (m_ShowidToProperty.count(showResult) > 0)
						if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
							sDrawCross _Cross;
							_Cross.Center.setX(ColumnEdgeSecond[0].D());
							_Cross.Center.setY(RowEdgeSecond[0].D());
							_Cross.fAngle = 0;
							if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
							_Cross.DrawColor = QColor(255, 0, 0);
							_PointLst.push_back(_Cross);
						}
			}
		}
		else
		{
			int _iIndex = 0;
			int _iFirstIndex = 0;
			int	_iEndIndex = iLenth - 1;
			double	_dPosX = 0;
			double	_dPosY = m_hGrayval[abs(_iFirstIndex)].D();
			double	_dZValue = 0;
			bool	_bShowPaint = true;
			HTuple hv_Sorted, hv_SortedIndex;
			TupleSort(m_hGrayval, &hv_Sorted);
			TupleSortIndex(m_hGrayval, &hv_SortedIndex);

			switch (ui.cbx_ALargrimCheckType->currentIndex()) {
			case 0: {	//百分比
				int	_iCurrentIndex = -1;
				double	_dValue = 0;
				int	_iLowerIndex = ui.SliderPercent->lowerValue() / 100.0 * iLenth;
				int	_iUpperIndex = ui.SliderPercent->upperValue() / 100.0 * iLenth;
				int	_iUseIndex = 0;
				int _iTotalIndex = 0;
				QVector<int>		vecIndex;
				QVector<int>		vecValue;
				for (int K = _iLowerIndex; K < _iUpperIndex; K++) {
					int _iTempIndex = hv_SortedIndex[K].I();
					_dValue = _dValue + hv_Sorted[K].D();
					vecValue.push_back(hv_Sorted[K].D());
					_iUseIndex++;
					vecIndex.push_back(_iTempIndex);
					_iTotalIndex = _iTotalIndex + _iTempIndex;
				}
				_dZValue		= _dValue / _iUseIndex;
				_iIndex			= _iTotalIndex / _iUseIndex;
				_dPosY			= m_hGrayval[_iIndex];
				_bShowPaint		= false;
				{
					QLineF _line;
					_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
					_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
					ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
					ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = HorizonLine;
					if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
				}	{
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("(%1,%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number((_LineItem.line.p2().y() + (_iIndex)*	_dRow)
							, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
				}	{
					sDrawCross _Cross;
					QPointF ptQPoint = _LineItem.line.p2();
					_Cross.Center.setX(_LineItem.line.p2().x() + (_iIndex)*	_dCol);
					_Cross.Center.setY(_LineItem.line.p2().y() + (_iIndex)*	_dRow);
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_PointLst.push_back(_Cross);
				}
			}	break;
			case 1: {	//最大值				
				int	_iCurrentIndex = hv_SortedIndex[iLenth - 1];
				_dZValue = hv_Sorted[iLenth - 1].D();
				_iIndex = _iCurrentIndex;
				_dPosY = _dZValue;

				sDrawCross _Cross;
				_Cross.Center = ptMeasureCenter;
				_Cross.Center.setX(_LineItem.line.p2().x() + (_iIndex)*	_dCol);
				_Cross.Center.setY(_LineItem.line.p2().y() + (_iIndex)*	_dRow);
				_Cross.fAngle = 0;
				if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_Cross.DrawColor = QColor(0, 255, 0);
				_PointLst.push_back(_Cross);
				{
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("(%1,%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number((_LineItem.line.p2().y() + (_iIndex) *	_dRow)
							,'f',databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
				}
			}	break;
			case 2: {	//最小值
				int	_iCurrentIndex = hv_SortedIndex[0];
				_dZValue = hv_Sorted[0].D();
				//需要过滤无效值
				for (int i = 0; i < hv_Sorted.Length(); i++) {
					if (hv_Sorted[i] >= m_iInvalidValue) {
						_iCurrentIndex = hv_SortedIndex[i];
						_dZValue = hv_Sorted[i].D();
						break;
					}
				}

				_iIndex = _iCurrentIndex;
				_dPosY	= _dZValue;

				sDrawCross _Cross;
				_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex *	_dCol);
				_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex *	_dRow);
				_Cross.fAngle = 0;
				if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_Cross.DrawColor = QColor(0, 255, 0);
				_PointLst.push_back(_Cross);
				{
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("(%1,%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number((_LineItem.line.p2().y() + (_iIndex)*	_dRow)
							, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
				}
			}	break;
			case 3: {	//中值
				int	_iCurrentIndex = hv_SortedIndex[iLenth / 2.0];

				_dZValue	= hv_Sorted[iLenth / 2.0].D();
				_iIndex		= _iCurrentIndex;
				_dPosY		= m_hGrayval[abs(_iIndex)].D();

				sDrawCross _Cross;
				_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex *	_dCol);
				_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex *	_dRow);
				_Cross.fAngle = 0;
				if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_Cross.DrawColor = QColor(0, 255, 0);
				_PointLst.push_back(_Cross);
				{
					QLineF _line;
					_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
					_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
					ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
					ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = HorizonLine;
					if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
				}
				_bShowPaint = false;
				{
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("(%1,%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number((_LineItem.line.p2().y() + (_iIndex)*	_dRow)
							, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
				}
			}	break;
			case 4: {	//均值
				int	_iCurrentIndex = -1;
				HTuple	hv_Median;
				TupleMedian(m_hGrayval, &hv_Median);
				for (int i = 0; i < iLenth; i++) {
					if (hv_SortedIndex[i] > hv_Median.D()) {
						_iCurrentIndex = i;	break;
					}
				}
				_dZValue = hv_Median.D();
				if (_iCurrentIndex > -1) {
					sDrawCross _Cross;
					_Cross.Center = ptMeasureCenter;
					_Cross.fAngle = 0;
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_PointLst.push_back(_Cross);
					{
						QLineF _line;
						_line.setP1(QPointF(ui.widgetPlot->xAxis->range().lower, _dZValue));
						_line.setP2(QPointF(ui.widgetPlot->xAxis->range().upper, _dZValue));
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->setPosLine(_line, true);
						ui.widgetPlot->m_XxwTraceItemLineLst[0]->m_type = HorizonLine;
						if (ui.widgetPlot->m_XxwTraceItemLineLst.size() > 0)	ui.widgetPlot->m_XxwTraceItemLineLst[0]->setVisible(true);
					}
					{
						strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
						strVecName.push_back(QString("(%1,%2,%3,%4)")
							.arg(tr("Point"))
							.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
							.arg(QString::number((_LineItem.line.p2().y() + (_iIndex)*	_dRow)
								, 'f', databaseVar::Instance().form_System_Precision))
							.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
					}
				}
				_bShowPaint = false;
			}	break;
			case 5: {	//中线
				_dZValue	= m_hGrayval[iLenth / 2.0].D();
				_iIndex		= iLenth / 2.0;
				_dPosY		= m_hGrayval[abs(_iIndex)].D();

				sDrawCross _Cross;
				_Cross.Center.setX(_LineItem.line.p2().x() + _iIndex *	_dCol);
				_Cross.Center.setY(_LineItem.line.p2().y() + _iIndex *	_dRow);
				_Cross.fAngle = 0;
				if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_Cross.DrawColor = QColor(0, 255, 0);
				_PointLst.push_back(_Cross);
				_bShowPaint = false;
				{
					strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
					strVecName.push_back(QString("(%1,%2,%3,%4)")
						.arg(tr("Point"))
						.arg(QString::number(_dPosY, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number((_LineItem.line.p2().y() + (_iIndex)*	_dRow)
							, 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(_dZValue, 'f', databaseVar::Instance().form_System_Precision)));
				}

			}	break;
			default: {		}	break;
			}
			_dx.push_back(abs(_iIndex));
			_dy.push_back(_dPosY);
			if (_bShowPaint)	{
				strVecQPen.push_back(QPen(Qt::green, 2, Qt::DashDotDotLine));
				strVecName.push_back(QString("(%1,%2,%3,%4)")
					.arg(tr("Point"))
					.arg(QString::number(ptMeasureCenter.x(),'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(ptMeasureCenter.y(),'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(_dZValue,'f',databaseVar::Instance().form_System_Precision)));
			}
			ui.widgetPlot->DrawItemPoint(0, _dx, _dy, strVecName, strVecQPen);
		}

		m_hRow.Clear();
		m_hCol.Clear();
		for (int i = 0; i < iLenth; i++)	{
			vecX.push_back(i);
			if (i == 0)		{
				//vecX.push_back(_LineItem.line.p1().x());
				vecY.push_back(_LineItem.line.p1().y());
				vecZ.push_back(m_hGrayval[i].D());
			}
			else			{
				//vecX.push_back(_LineItem.line.p1().x() + _dCol * i	);
				vecY.push_back(_LineItem.line.p1().y() + _dRow * i );
				vecZ.push_back(m_hGrayval[i].D());

			}
		}
	}
	catch (const HException& except) {
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_strText.strText = tr("Error In Glue Detection") + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...) {
		sDrawText			_strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("Error In Glue Detection");
		_VecText1D.push_back(_strText);
	}

	int		xMinLimit = -1, xMaxLimit = 1;
	double	yMinLimit = -1, yMaxLimit = 1;

	//将数据转化为绘图要的
	bool _bFirst = true;
	int iGrayLenth = vecZ.size();
	int _iInvalidValue = -9000;
	if (databaseVar::Instance().m_mapParam.contains("spinInvalidateValue"))
	{
		_iInvalidValue = databaseVar::Instance().m_mapParam["spinInvalidateValue"].toDouble();
	}
	for (int i = 0; i < vecZ.size(); i++)
	{
		//if (m_bRemovePoint)		//去除杂点
		//{
		//	if (CheckPoint(vecZ[i]))
		//	{
		//		x.push_back(i);
		//		y.push_back(vecZ[i]);
		//	}
		//}
		//else
		{
			x.push_back(vecX[i]);
			y.push_back(vecZ[i]);
		}
		if (_bFirst)	{
			_bFirst = false;
			if (i == 0)		{
				xMinLimit = i;
				xMaxLimit = iGrayLenth;
				yMinLimit = vecZ[i];
				yMaxLimit = yMinLimit;

				if(vecZ[i] <= _iInvalidValue)
					_bFirst = true;
			}
			else	{
				xMaxLimit = max(xMaxLimit, iGrayLenth);
				if (vecZ[i] >= _iInvalidValue)	{
					yMinLimit = min(yMinLimit, vecZ[i]);
				}
				yMaxLimit = max(yMaxLimit, vecZ[i]);
			}
		}
		else	{
			if (vecZ[i] >= _iInvalidValue) {
				yMinLimit = min(yMinLimit, vecZ[i]);
			}
			yMaxLimit = max(yMaxLimit, vecZ[i]);
		}
	}

	int i = 0;
	QPen pen;
	pen.setColor(QColor(qSin(i * 0.3) * 100 + 100, qSin(i * 0.6 + 0.7) * 100 + 100, qSin(i * 0.4 + 0.6) * 100 + 100));				// generate data:
	pen.setWidth(2);
	ui.widgetPlot->graph(0)->setData(x, y);
	ui.widgetPlot->graph(0)->rescaleAxes(true);
	ui.widgetPlot->graph(0)->setPen(pen);
	ui.widgetPlot->replot();

	ui.widgetPlot->xAxis->setRange(xMinLimit, xMaxLimit);
	float _fDurLimit = abs(yMaxLimit - yMinLimit) / 10.0;
	ui.widgetPlot->yAxis->setRange(yMinLimit - _fDurLimit, yMaxLimit + _fDurLimit);
	slot_customplot_FitAxis();
	ui.widgetPlot->replot();
	ui.widgetPlot->update();
	ui.widgetPlot->setFocus();

	m_view->DispPoint(_VecPt1D, QColor(0, 255, 0));
	m_view->DispTextList(_VecText1D);
	m_view->DispCrossPoint(_PointLst);
	m_view->update();

}

QList<QPointF> frmGlueMeasureTool::lineRectIntersection(const QLineF & line, const QRectF & rect)
{
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
		auto _Type = line.intersect(edge, &intersection);
		switch (_Type) {
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

void frmGlueMeasureTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
}

