#include "frmPlaneFitTool.h"
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

#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "databaseVar.h"

frmPlaneFitTool::frmPlaneFitTool(QWidget* parent)
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
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/Plane.png"));

	initTitleBar();
	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmPlaneFitTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	initConnect();
	initShapeProperty();
	initResultProperty();
	initShowProperty();
	initData();
}

frmPlaneFitTool::~frmPlaneFitTool()
{
	if (btnGroupRadio != nullptr)			delete btnGroupRadio;				btnGroupRadio = nullptr;
	if (m_titleBar != nullptr)				delete m_titleBar;					m_titleBar = nullptr;
	if (m_view != nullptr)					delete m_view;						m_view = nullptr;

	if (variantFactory != nullptr)			delete variantFactory;				variantFactory = nullptr;
	if (variantManager != nullptr)			delete variantManager;				variantManager = nullptr;
	if (propertyEditor != nullptr)			delete propertyEditor;				propertyEditor = nullptr;

	if (m_ResultvariantFactory != nullptr)	delete m_ResultvariantFactory;		m_ResultvariantFactory = nullptr;
	if (m_ResultvariantManager != nullptr)	delete m_ResultvariantManager;		m_ResultvariantManager = nullptr;
	if (m_ResultpropertyEditor != nullptr)	delete m_ResultpropertyEditor;		m_ResultpropertyEditor = nullptr;
	this->deleteLater();
}

void frmPlaneFitTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Plane.png");
}

void frmPlaneFitTool::initShapeProperty()
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

void frmPlaneFitTool::initResultProperty()
{
	m_ResultvariantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_ResultvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	m_ResultvariantFactory = new QtVariantEditorFactory(this);
	m_ResultpropertyEditor = new QtTreePropertyBrowser(ui.widget_Result);
	m_ResultpropertyEditor->setFactoryForManager(m_ResultvariantManager, m_ResultvariantFactory);
	ui.ResultLayout->addWidget(m_ResultpropertyEditor);
}

void frmPlaneFitTool::initShowProperty()
{
	initShowEditorProperty(ui.widget_Show, ui.ShowLayout);
	//connect(m_ShowvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Result_valueChanged(QtProperty *, const QVariant &)));
	ui.gImage->setVisible(false);

	QtVariantProperty*	_ptrproperty = nullptr;
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showregion, tr("showregion"));
	}
}

void frmPlaneFitTool::initConnect()
{
	connect(ui.cbx_Camerachannal, SIGNAL(activated(int)), this, SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW, SIGNAL(valueChanged(int)), this, SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut, SIGNAL(activated(int)), this, SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view, SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)), this, SLOT(slot_MouseClicked(QList<QGraphicsItem*>&)));
	connect(m_view, SIGNAL(sig_MouseMouve(QList<QGraphicsItem*>&)), this, SLOT(slot_MouseMouve(QList<QGraphicsItem*>&)));

	connect(ui.btnAddROI,			&QPushButton::pressed,	this,	&frmPlaneFitTool::on_btnAddROI_clicked);
	connect(ui.btnDeleteROISelect,	&QPushButton::pressed,	this,	&frmPlaneFitTool::on_btnDeleteROISelect_clicked);
	connect(ui.btnDeleteROIAll,		&QPushButton::pressed,	this,	&frmPlaneFitTool::on_btnDeleteROIAll_clicked);
	connect(ui.btnLinkContent,		&QPushButton::pressed,	this,	&frmPlaneFitTool::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,	&QPushButton::pressed,	this,	&frmPlaneFitTool::on_btnDelLinkContent_clicked);

}

void frmPlaneFitTool::initData()
{
	ui.tabWidget->setCurrentIndex(0);
	m_iChanal = 0;
}

void frmPlaneFitTool::ClearROIProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

//绘制ROI
void frmPlaneFitTool::on_btnAddROI_clicked()
{
	BaseItem* _BaseItem;
	int iWidth = (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;

	float fData[5] = { 0 };
	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	switch (ui.comboROIShape->currentIndex())
	{
	case 0: {	//园
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
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
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
	switch (ui.comboxROIType->currentIndex()) {
	case 0:	{	_BaseItem->m_eRegionType = RegionType_Add;		}break;
	case 1:	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default: {	}	break;	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmPlaneFitTool::on_btnDeleteROISelect_clicked()
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

void frmPlaneFitTool::on_btnDeleteROIAll_clicked()
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

void frmPlaneFitTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	frm->m_strModelIndex		= m_strModelIndex;
	frm->m_strTypeVariable		= m_strTypeVariable;
	frm->m_strValueName			= m_strValueName;
	frm->SetLimitModel(QString::number(((PlaneFitTool*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
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

void frmPlaneFitTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkFollow->clear();
}

void frmPlaneFitTool::onChannelClicked(int Index)
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

void frmPlaneFitTool::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
}

void frmPlaneFitTool::on_UpDateCheckType(int Index)
{

}

int frmPlaneFitTool::PrExecute()
{
	for (auto iter : m_ResultidToProperty)	if (iter != nullptr)	delete iter;	m_ResultidToProperty.clear();
	m_ResultpropertyToId.clear();
	return 0;
}

EnumNodeResult frmPlaneFitTool::Execute(MiddleParam & param, QString & strError)
{
	//QVector<QPointF>	_VecPt1D;
	std::vector<QPointF>		_VecPt1D;	//
	std::vector<sDrawText>		_VecText1D;

	for (auto iter : m_ResultidToProperty)	if (iter != nullptr)	delete iter;	m_ResultidToProperty.clear();
	m_ResultpropertyToId.clear();

	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;
	_ptrproperty = m_ResultvariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");
	QString strParent = "Flatness";
	_ptrproperty->setPropertyName(tr("Flatness"));
	_ptrproperty->setAttribute("Parent", strParent);

	ShapeToRegion(m_VecBaseItem, ModelROIRegion);

	HObject _MetrixRegion = ModelROIRegion;

	if (!_MetrixRegion.IsInitialized() || m_VecBaseItem.size() <= 0) {
		sDrawText			_strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("Region Not Inited");
		_VecText1D.push_back(_strText);
		param.VecDrawText.push_back(_strText);
		return NodeResult_ParamError;
	}
	if (ui.checkUseFollow->isChecked()) {
		HTuple _Metrix;
		HTuple	Rows, Columns, _Lenth;
		HObject _MetrixContour;
		if (PlaneFitTool::CheckMetrixType(ui.txtLinkFollow->text(), _Metrix) == 0) {
			AffineTransRegion(ModelROIRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
			_MetrixContour.GenEmptyObj();
			GetRegionContour(_MetrixRegion, &Rows, &Columns);
			HalconCpp::TupleLength(Rows, &_Lenth);
			for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
				_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
			}
		}
		else {
			sDrawText				_strText;
			_strText.bControl		= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	
				_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText = tr(" The Link Matrix Failed!");
			_VecText1D.push_back(_strText);

			param.VecShowQPointFs	= (_VecPt1D);
			param.VecDrawText.push_back(_strText);
			return NodeResult_ParamError;
		}
	}

	int _iCheckMode = databaseVar::Instance().m_iCheckMode;
	switch (_iCheckMode)
	{
	case 0:	//2.5D检测
	{
		HObject _ImageSurface, _ImgSub;
		HTuple Alpha, Beta, Gamma, hType;
		HTuple Width, Height;
		HalconCpp::GetImageType(m_srcImage, &hType);
		HalconCpp::GetImageSize(m_srcImage, &Width, &Height);

		//平面度矫正
		int	_iIterations = ui.spinIterationsTimes->value();
		float _fClippingFactor = ui.spinClippingFactor->value();
		HalconCpp::FitSurfaceFirstOrder(_MetrixRegion, m_srcImage, HTuple(ui.cbx_ALargrimType->currentText().toStdString().c_str()), HTuple(_iIterations), HTuple(_fClippingFactor), &Alpha, &Beta, &Gamma);
		HalconCpp::GenImageSurfaceFirstOrder(&_ImageSurface, hType, Alpha, Beta, Gamma, 0, 0, Width, Height);
		HalconCpp::SubImage(m_srcImage, _ImageSurface, &_ImgSub, 1, 0);
		HTuple Min, Max, Range;

		//求平面度的方法有以下几种
		HalconCpp::MinMaxGray(_MetrixRegion, _ImgSub, 10, &Min, &Max, &Range);
		float fPlaneFit = Range.D();
		//输出结果
		if (fPlaneFit >= ui.spinMinValue->value() && fPlaneFit <= ui.spinMaxValue->value())	{
			sDrawText			_strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(0, 255, 0);
			_strText.strText = tr("FlatNess Check OK");
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_VecText1D.push_back(_strText);

			_strText.bControl = true;
			_strText.DrawColor = QColor(0, 255, 0);
			_strText.strText = QString("%1 %2 %3 %4 -- %5")
				.arg(tr("FlatNess"))
				.arg(QString::number(fPlaneFit, 'f', databaseVar::Instance().form_System_Precision))
				.arg(tr(" In Range "))
				.arg(QString::number(ui.spinMinValue->value(), 'f', databaseVar::Instance().form_System_Precision))
				.arg(QString::number(ui.spinMaxValue->value(), 'f', databaseVar::Instance().form_System_Precision));
			_VecText1D.push_back(_strText);
		}
		else	{
			sDrawText			_strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("FlatNess Check NG");
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_VecText1D.push_back(_strText);
			_strText.bControl = true;
			_strText.strText = QString("%1 %2 %3 %4 -- %5")
				.arg(tr("FlatNess"))
				.arg(QString::number(fPlaneFit, 'f', databaseVar::Instance().form_System_Precision))
				.arg(tr("Out Of Range "))
				.arg(QString::number(ui.spinMinValue->value(), 'f', databaseVar::Instance().form_System_Precision))
				.arg(QString::number(ui.spinMaxValue->value(), 'f', databaseVar::Instance().form_System_Precision));
			_VecText1D.push_back(_strText);
		}
		//显示结果
		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);
		_ptrSubproperty->setValue(QString::number(fPlaneFit, 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setEnabled(false);
		_ptrSubproperty->setPropertyName( tr("FlatNess"));
		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);

		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);
		_ptrSubproperty->setValue(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setEnabled(false);
		_ptrSubproperty->setPropertyName(tr("Max"));
		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);

		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);
		_ptrSubproperty->setValue(QString::number(Min.D(), 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setEnabled(false);
		_ptrSubproperty->setPropertyName(tr("Min"));
		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);
	}	break;
	case 1: { //3D检测

	}	break;
	default: {		}	break;
	}
	param.VecShowQPointFs	= _VecPt1D;
	param.VecDrawText		= (_VecText1D);

	return NodeResult_OK;
}

int frmPlaneFitTool::ExecuteComplete()
{
	for (auto iter = m_ResultidToProperty.begin(); iter != m_ResultidToProperty.end(); iter++) {
		QtBrowserItem *item = m_ResultpropertyEditor->addProperty(*iter);
		m_ResultpropertyEditor->setExpanded(item, true);
	}
	return 0;
}

void frmPlaneFitTool::slot_MouseClicked(QList<QGraphicsItem*>& items)
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

void frmPlaneFitTool::slot_MouseMouve(QList<QGraphicsItem*>& items)
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

int frmPlaneFitTool::Load()
{
	m_titleBar->setTitleContent(((PlaneFitTool*)m_pTool)->GetItemId());

	//设置图像输入
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((PlaneFitTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((PlaneFitTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((PlaneFitTool*)m_pTool)->GetItemId()))	{
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((PlaneFitTool*)m_pTool)->m_strInImg);
	if (ui.cbx_ImgInPut->currentIndex() >= 0) {
		slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	}

	if (MiddleParam::MapMiddle()[((PlaneFitTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ((PlaneFitTool*)m_pTool)->m_strInImg) > 0)	{
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((PlaneFitTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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
	DataToShape(((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_vecCommonData, m_VecBaseItem);
	for (auto pItem : m_VecBaseItem)	m_view->AddItems(pItem);

	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("spinRoiW"))	ui.spinRoiW->setValue(((PlaneFitTool*)m_pTool)->m_mapParam["spinRoiW"].toInt());
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		m_VecBaseItem[i]->setIndex(i);
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}

	//设置默认参数
	if (ui.cbx_ImgInPut->currentIndex() >= 0)					slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("checkUseFollow"))			ui.checkUseFollow->setChecked(((PlaneFitTool*)m_pTool)->m_mapParam["checkUseFollow"].toInt());
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("txtLinkFollow"))			ui.txtLinkFollow->setText(((PlaneFitTool*)m_pTool)->m_mapParam["txtLinkFollow"]);

	//检测条件
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("cbx_ALargrimType"))		ui.cbx_ALargrimType->setCurrentText(((PlaneFitTool*)m_pTool)->m_mapParam["cbx_ALargrimType"]);
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("spinIterationsTimes"))	ui.spinIterationsTimes->setValue(((PlaneFitTool*)m_pTool)->m_mapParam["spinIterationsTimes"].toDouble());
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("spinClippingFactor"))		ui.spinClippingFactor->setValue(((PlaneFitTool*)m_pTool)->m_mapParam["spinClippingFactor"].toDouble());

	//检测结果
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("spinMinValue"))			ui.spinMinValue->setValue(((PlaneFitTool*)m_pTool)->m_mapParam["spinMinValue"].toDouble());
	if (((PlaneFitTool*)m_pTool)->m_mapParam.contains("spinMaxValue"))			ui.spinMaxValue->setValue(((PlaneFitTool*)m_pTool)->m_mapParam["spinMaxValue"].toDouble());
	
	//设置显示参数
	for (auto iter = ((PlaneFitTool*)m_pTool)->m_mapShowParam.begin(); iter != ((PlaneFitTool*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)	m_ShowidToProperty[iter.key()]->setValue(iter.value());

	return 0;
}

int frmPlaneFitTool::Save()
{
	((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_strInImg = ui.cbx_ImgInPut->currentText();

	((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_vecCommonData.clear();
	ShapeToData(m_VecBaseItem, ((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_vecCommonData);

	((PlaneFitTool*)m_pTool)->m_mapParam.insert("spinRoiW",				QString::number(ui.spinRoiW->value()));
	((PlaneFitTool*)m_pTool)->m_mapParam.insert("checkUseFollow",		QString::number(ui.checkUseFollow->isChecked()));
	((PlaneFitTool*)m_pTool)->m_mapParam.insert("txtLinkFollow",			ui.txtLinkFollow->text());

	((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_strLinkName								= ui.txtLinkFollow->text();
	((PlaneFitTool*)((PlaneFitTool*)m_pTool))->m_bEnableCom								= ui.checkUseFollow->isChecked();

	((PlaneFitTool*)m_pTool)->m_mapParam.insert("cbx_ALargrimType",		(ui.cbx_ALargrimType->currentText()));
	((PlaneFitTool*)m_pTool)->m_mapParam.insert("spinIterationsTimes",	QString::number(ui.spinIterationsTimes->value()));
	((PlaneFitTool*)m_pTool)->m_mapParam.insert("spinClippingFactor",	QString::number(ui.spinClippingFactor->value()));

	((PlaneFitTool*)m_pTool)->m_mapParam.insert("spinMinValue",			QString::number(ui.spinMinValue->value()));
	((PlaneFitTool*)m_pTool)->m_mapParam.insert("spinMaxValue",			QString::number(ui.spinMaxValue->value()));

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((PlaneFitTool*)m_pTool)->m_mapShowParam[iter.key()] = iter.value()->value().toString();

	return 0;
}

void frmPlaneFitTool::addResultProperty(QtVariantProperty * property, const QString & id)
{
	m_ResultpropertyToId[property] = id;
	m_ResultidToProperty[id] = property;
	QtBrowserItem *item = m_ResultpropertyEditor->addProperty(property);
	if (cstItem == id) {
		m_ResultpropertyEditor->setExpanded(item, true);
	}
	else {
		m_ResultpropertyEditor->setExpanded(item, false);
	}
}

void frmPlaneFitTool::addProperty(QtVariantProperty * property, const QString & id)
{
	propertyToId[property] = id;
	idToProperty[id] = property;
	QtBrowserItem *item = propertyEditor->addProperty(property);
	if (cstItem == id) {
		propertyEditor->setExpanded(item, true);
	}
	else {
		propertyEditor->setExpanded(item, false);
	}
}

void frmPlaneFitTool::slot_valueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)		return;
	QString strName = property->propertyName();
	if (propertyToId.count(property) > 0) {
		QString _strName = propertyToId[property];
		if (idToProperty.count(propertyToId[property]) > 0) {
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++) {
				((BaseItem*)items[i])->SetParam(propertyToId[property], value.toString());

			}
		}
	}

}

void frmPlaneFitTool::slot_Result_valueChanged(QtProperty * property, const QVariant & value)
{
	if (property == nullptr)		return;


}

void frmPlaneFitTool::slot_ChangeMatchImg_Index(int Index)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((PlaneFitTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((PlaneFitTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

void frmPlaneFitTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
}
