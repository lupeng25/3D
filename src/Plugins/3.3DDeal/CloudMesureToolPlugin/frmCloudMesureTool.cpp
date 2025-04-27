#include "frmCloudMesureTool.h"
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
#include "databaseVar.h"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"

frmCloudMesureTool::frmCloudMesureTool(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmBaseTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	initTitleBar();
	initConnect();
	initShapeProperty();
	initResultProperty();
	initShowProperty();
	initData();
}

frmCloudMesureTool::~frmCloudMesureTool()
{
	if (btnGroupRadio != nullptr)				delete btnGroupRadio;					btnGroupRadio = nullptr;
	if (m_view != nullptr)						delete m_view;							m_view = nullptr;
	if (variantFactory != nullptr)				delete variantFactory;					variantFactory = nullptr;
	if (variantManager != nullptr)				delete variantManager;					variantManager = nullptr;
	if (propertyEditor != nullptr)				delete propertyEditor;					propertyEditor = nullptr;
	if (m_ResultpropertyEditor != nullptr)		delete m_ResultpropertyEditor;			m_ResultpropertyEditor = nullptr;
	if (m_ResultvariantManager != nullptr)		delete m_ResultvariantManager;			m_ResultvariantManager = nullptr;
	if (m_ResultvariantFactory != nullptr)		delete m_ResultvariantFactory;			m_ResultvariantFactory = nullptr;
	if (m_ResultEnumPropertyManager != nullptr)	delete m_ResultEnumPropertyManager;		m_ResultEnumPropertyManager = nullptr;
	if (m_ResultEnumEditorFactory != nullptr)	delete m_ResultEnumEditorFactory;		m_ResultEnumEditorFactory = nullptr;
	this->deleteLater();
}

void frmCloudMesureTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/3DMeasure.png");
}

void frmCloudMesureTool::initShapeProperty()
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

void frmCloudMesureTool::initResultProperty()
{
	m_ResultpropertyEditor					= new QtTreePropertyBrowser(ui.widget_Result);
	m_ResultvariantManager					= new QtVariantPropertyManager(this);			//相机属性
	m_ResultvariantFactory					= new QtVariantEditorFactory(this);				//相机属性
	m_ResultEnumPropertyManager				= new QtEnumPropertyManager(this);				//相机属性
	m_ResultEnumEditorFactory				= new QtEnumEditorFactory(this);

	connect(m_ResultvariantManager,			SIGNAL(valueChanged(QtProperty *, const QVariant &)),	this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	connect(m_ResultEnumPropertyManager,	&QtEnumPropertyManager::valueChanged,					this, [=](QtProperty *property, int val) {	
		slot_valueChanged(property, QVariant(val));
	});

	m_ResultpropertyEditor->setFactoryForManager(m_ResultvariantManager,		m_ResultvariantFactory);
	m_ResultpropertyEditor->setFactoryForManager(m_ResultEnumPropertyManager,	m_ResultEnumEditorFactory);
	ui.ResultLayout->addWidget(m_ResultpropertyEditor);
}

void frmCloudMesureTool::initShowProperty()
{
	initShowEditorProperty(ui.widget_Show, ui.ShowLayout);
	connect(m_ShowvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Result_valueChanged(QtProperty *, const QVariant &)));
	ui.gImage->setVisible(false);

	QtVariantProperty*	_ptrproperty = nullptr;
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr(showregion));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showregion, tr(showregion));
		_ptrproperty->setPropertyName(tr("showregion"));
	}
}

void frmCloudMesureTool::initConnect()
{
	connect(ui.cbx_Camerachannal,		SIGNAL(activated(int)),								this,			SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW,				SIGNAL(valueChanged(int)),							this,			SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut,			SIGNAL(activated(int)),								this,			SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view,						SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)),	this,			SLOT(slot_MouseClicked(QList<QGraphicsItem*>&)));
	connect(m_view,						SIGNAL(sig_MouseMouve(QList<QGraphicsItem*>&)),		this,			SLOT(slot_MouseMouve(QList<QGraphicsItem*>&)));
	connect(ui.tabWidgetROI,			SIGNAL(currentChanged(int)),						this,			SLOT(slot_ChangeROITabIndex(int)));
	connect(ui.cbx_RegionType,			SIGNAL(currentIndexChanged(int)),					this,			SLOT(slot_ChangeRegionTabIndex(int)));
	connect(ui.cbx_ALargrimType,		SIGNAL(activated(int)),								this,			SLOT(slot_ChangeALargrimType(int)));
	connect(ui.tabWidget,				SIGNAL(currentChanged(int)),						this,			SLOT(slot_tabWidget_ChangIndex(int)));
	connect(ui.SliderPercent,			SIGNAL(lowerValueChanged(int)),						ui.lbl_Low,		SLOT(setNum(int)));
	connect(ui.SliderPercent,			SIGNAL(upperValueChanged(int)),						ui.lbl_High,	SLOT(setNum(int)));
	connect(ui.SliderPercent,			SIGNAL(spanChanged(int,int)),						this,			SLOT(slot_ChangeSpanSlider(int,int)	));

	connect(ui.btnAddROI,				&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnAddROI_clicked);
	connect(ui.btnDeleteROISelect,		&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDeleteROISelect_clicked);
	connect(ui.btnDeleteROIAll,			&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDeleteROIAll_clicked);

	connect(ui.btnLinkContent,			&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,		&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDelLinkContent_clicked);
	
	connect(ui.btnAddROI1,				&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnAddROI1_clicked);
	connect(ui.btnDeleteROI1Select,		&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDeleteROI1Select_clicked);
	connect(ui.btnDeleteROI1All,		&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDeleteROI1All_clicked);

	connect(ui.btnLink1Content,			&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnLink1Content_clicked);
	connect(ui.btnDelLink1Content,		&QPushButton::pressed,								this,			&frmCloudMesureTool::on_btnDelLink1Content_clicked);
}

void frmCloudMesureTool::initData()
{
	ui.tabWidget->setCurrentIndex(0);
	ui.tabWidgetROI->setCurrentIndex(0);
	m_iChanal = 0;
	m_vecMesureRegion.resize(2);
	ui.stackedROI->setCurrentIndex(2);
}

void frmCloudMesureTool::ClearROIProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

//绘制ROI
void frmCloudMesureTool::on_btnAddROI_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth = (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;

	int iCount = m_VecBaseItem.size();
	float fData[5] = { 0 };
	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	switch (ui.comboROIShape->currentIndex()){
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = qMax<int>(ui.spinRoiW->value(), iMinRadius);
		_BaseItem = new PointItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 5.0, iHeight / 5.0);
		fData[2] = MAX(iMinRadius, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 2: {	//椭圆
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new EllipseItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 3: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 4: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 5: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 6: {	//多边形
		_BaseItem = new PolygonItem();
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;	}

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
	case 0:		{	_BaseItem->m_eRegionType = RegionType_Add;		}	break;
	case 1:		{	_BaseItem->m_eRegionType = RegionType_Differe;	}	break;
	default:	{													}	break;	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	_BaseItem->setSelected(true);
	_BaseItem->m_bSelected = true;
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2	
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		if (m_VecBaseItem[i] == nullptr)	continue;
		m_VecBaseItem[i]->setIndex(i);
	}
}

void frmCloudMesureTool::on_btnDeleteROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)	{
		if (iter != nullptr)
			if (iter->type() == 10)			{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecBaseItem.size(); i++)				{
					if (m_VecBaseItem[i] == iter)					{
						_bFinded = true;
					}
				}
				if (_bFinded)				{
					(m_view->scene())->removeItem(iter);
					m_VecBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}	
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		if (m_VecBaseItem[i] == nullptr)	continue;
		m_VecBaseItem[i]->setIndex(i);
	}
}

void frmCloudMesureTool::on_btnDeleteROIAll_clicked()
{
	ClearROIProperty();
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->items();
	for (auto iter : shapeLst)	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecBaseItem.size(); i++)			{
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
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		if (m_VecBaseItem[i] == nullptr)	continue;
		m_VecBaseItem[i]->setIndex(i);
	}
}

void frmCloudMesureTool::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();
	//frm->m_strModelIndex		= m_strModelIndex;
	//frm->m_strTypeVariable		= m_strTypeVariable;
	//frm->m_strValueName			= m_strValueName;
	frm->SetCurrentLinkContent(ui.txtLinkFollow->text());
	frm->SetLimitModel(QString::number(((CloudMesureTool*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
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

void frmCloudMesureTool::on_btnDelLinkContent_clicked()
{
	ui.txtLinkFollow->clear();
}

void frmCloudMesureTool::on_btnAddROI1_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth	= (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;
	//_ptrproperty	= variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");

	int iCount = m_VecROI2Item.size();
	float fData[5] = { 0 };
	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
	switch (ui.comboROI1Shape->currentIndex())	{
	case 0: {	//点
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = qMax<int>(ui.spinRoiW->value(), iMinRadius);
		_BaseItem = new PointItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = qMin<int>(iWidth / 5.0, iHeight / 5.0);
		fData[2] = qMax<int>(iMinRadius, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 2: {	//椭圆
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new EllipseItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);

	}   break;
	case 3: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);

	}   break;
	case 4: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);

	}   break;
	case 5: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);

	}   break;
	case 6: {	//多边形
		_BaseItem = new PolygonItem();
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;}


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
	switch (ui.comboxROI1Type->currentIndex())	{
	case 0:	{	_BaseItem->m_eRegionType = RegionType_Add;		}break;
	case 1:	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default:	break;	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	_BaseItem->setSelected(true);
	_BaseItem->m_bSelected = true;
	m_VecROI2Item.push_back(_BaseItem);//录入矩形2

	for (size_t i = 0; i < m_VecROI2Item.size(); i++) {
		if (m_VecROI2Item[i] == nullptr)	continue;
		m_VecROI2Item[i]->setIndex(i);
	}
}

void frmCloudMesureTool::on_btnDeleteROI1Select_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr)
			if (iter->type() == 10)
			{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecROI2Item.size(); i++)
				{
					if (m_VecROI2Item[i] == iter)
					{
						_bFinded = true;
					}
				}
				if (_bFinded)
				{
					(m_view->scene())->removeItem(iter);
					m_VecROI2Item.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}
	for (size_t i = 0; i < m_VecROI2Item.size(); i++) {
		if (m_VecROI2Item[i] == nullptr)	continue;
		m_VecROI2Item[i]->setIndex(i); 
	}
}

void frmCloudMesureTool::on_btnDeleteROI1All_clicked()
{
	ClearROIProperty();
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecROI2Item.size(); i++)
			{
				if (m_VecROI2Item[i] == iter)
				{
					_bFinded = true;
				}
			}
			if (_bFinded)
			{
				m_VecROI2Item.removeOne((BaseItem*)iter);
				(m_view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}
	for (size_t i = 0; i < m_VecROI2Item.size(); i++) {
		if (m_VecROI2Item[i] == nullptr)	continue;
		m_VecROI2Item[i]->setIndex(i);
	}
}

void frmCloudMesureTool::on_btnLink1Content_clicked()
{
	frmAllVariableTool * frm	= new frmAllVariableTool();

	frm->SetCurrentLinkContent(ui.txtLink1Follow->text());
	frm->SetLimitModel(QString::number(((CloudMesureTool*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();

	if (frm->m_iRetn == 2)	{
		//关联变量
		m_strModelIndex		= frm->m_strModelIndex;
		m_strTypeVariable	= frm->m_strTypeVariable;
		m_strValueName		= frm->m_strValueName;
		ui.txtLink1Follow->setText(QString("%1:%2:%3").arg(m_strModelIndex).arg(m_strTypeVariable).arg(m_strValueName));
	}
	delete frm;
}

void frmCloudMesureTool::on_btnDelLink1Content_clicked()
{
	ui.txtLink1Follow->clear();
}

void frmCloudMesureTool::onChannelClicked(int Index)
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

void frmCloudMesureTool::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto iter : m_VecROI2Item)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
}

void frmCloudMesureTool::slot_tabWidget_ChangIndex(int Index)
{
	if (Index > 1){
		for (auto pIter : m_VecBaseItem) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(false);
		}
		for (auto pIter : m_VecROI2Item) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(false);
		}
	}
	else {
		ChangeRoiIndex(ui.tabWidgetROI->currentIndex());
	}
}

void frmCloudMesureTool::on_UpDateCheckType(int Index)
{

}

int frmCloudMesureTool::PrExecute()
{
	for (auto iter : m_ResultidToProperty)	if (iter != nullptr)	delete iter;	m_ResultidToProperty.clear();
	m_ResultpropertyToId.clear();
	return 0;
}

EnumNodeResult frmCloudMesureTool::Execute(MiddleParam & param, QString & strError)
{
	std::vector<QPointF>	_VecPt1D;
	std::vector<sDrawText>	_VecText1D;
	//m_view->ClearAllDrawItem();

	HTuple Width, Height, hType;
	HalconCpp::GetImageType(m_srcImage, &hType);
	HalconCpp::GetImageSize(m_srcImage, &Width, &Height);

	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;
	_ptrproperty = m_ResultvariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");
	QString strParent = "CloudMesure";
	_ptrproperty->setPropertyName(strParent);
	_ptrproperty->setAttribute("Parent", strParent);

	for (int i = 0; i < m_vecMesureRegion.size(); i++) {
		if (i == 0) {
			ShapeToData(m_VecBaseItem, m_vecMesureRegion[i].vecRoi);
			m_vecMesureRegion[i].bEnableLink = ui.checkUseFollow->isChecked();
			m_vecMesureRegion[i].strLinkName = ui.txtLinkFollow->text();
		}
		else if (i == 1) {
			ShapeToData(m_VecROI2Item, m_vecMesureRegion[i].vecRoi);
			m_vecMesureRegion[i].bEnableLink = ui.check1UseFollow->isChecked();
			m_vecMesureRegion[i].strLinkName = ui.txtLink1Follow->text();
		}
	}
	int _iCheckMode = databaseVar::Instance().m_iCheckMode;
	switch (_iCheckMode) {
	case 0: {	//2.5D检测
		for (int i = 0; i < m_vecMesureRegion.size(); i++) {
			auto& _model = m_vecMesureRegion[i];
			HObject _ModelROIRegion;
			CloudMesureTool::ChangeParamToRegion(m_vecMesureRegion[i].vecRoi, _ModelROIRegion);
			HObject _MetrixRegion = _ModelROIRegion;
			//链接
			if (_model.bEnableLink) {
				HTuple _Metrix;
				HTuple	Rows, Columns, _Lenth;
				if (CloudMesureTool::CheckMetrixType(_model.strLinkName, _Metrix) == 0) {
					AffineTransRegion(_ModelROIRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
					if (m_ShowidToProperty.value(showregion)->value().toBool()) {
						AffineShapeTrans(m_vecMesureRegion[i].vecRoi, _Metrix, param,true);
					}
				}
				else {	//跟随
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty.value(FontSize)->value().toInt();
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = tr(" The Link Matrix Failed!");
					param.VecDrawText.push_back(_strText);

					if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
					else			return NodeResult_NG;
				}
			}
			else {
				if (m_ShowidToProperty.value(showregion)->value().toBool()) {
					AffineShapeTrans(m_vecMesureRegion[i].vecRoi, HTuple(), param);
				}
			}
			try {
				//算法
				switch (_model.eAlargrim) {
				case ALargrim_Medium: {
					HTuple Mean, Deviation;
					HalconCpp::Intensity(_MetrixRegion, m_srcImage, &Mean, &Deviation);
					_model.dMeasureValue = Mean.D();
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = QString("%1.%2:%3").arg(QString::number(i)).arg(tr("Medium"))
						.arg(QString::number(Mean.D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_strText);
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);					//显示结果
					_ptrSubproperty->setValue(QString::number(Mean.D(), 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					_ptrSubproperty->setPropertyName(QString::number(i) + "." + tr("Medium"));
					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				} break;
				case ALargrim_Max: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, m_srcImage, 0, &Min, &Max, &Range);
					_model.dMeasureValue = Max.D();
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = QString("%1.%2:%3").arg(QString::number(i)).arg(tr("Max")).arg(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_strText);
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);			//显示结果
					_ptrSubproperty->setValue(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					_ptrSubproperty->setPropertyName(QString::number(i) + "." + tr("Max"));
					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				} break;
				case ALargrim_Mean: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, m_srcImage, 50, &Min, &Max, &Range);
					_model.dMeasureValue = Max.D();
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = QString("%1.%2:%3").arg(QString::number(i)).arg(tr("Mean")).arg(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_strText);
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double);					//显示结果
					_ptrSubproperty->setValue(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					_ptrSubproperty->setPropertyName(QString::number(i) + "." + tr("Mean"));
					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				} break;
				case ALargrim_Min: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, m_srcImage, 0, &Min, &Max, &Range);
					_model.dMeasureValue = Min.D();
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = QString("%1.%2:%3").arg(QString::number(i)).arg(tr("Min")).arg(QString::number(Min.D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_strText);
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, QString::number(i) + "." + tr("Min") );				//显示结果
					_ptrSubproperty->setValue(QString::number(Min.D(), 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					_ptrSubproperty->setPropertyName(QString::number(i) + "." + tr("Min"));
					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				} break;
				case ALargrim_GrayHisto: {
					HTuple Percent, Min, Max, Range, AbsoluteHisto, RelativeHisto, Area, Row, Column;
					HalconCpp::GrayHisto(_MetrixRegion, m_srcImage, &AbsoluteHisto, &RelativeHisto);
					HalconCpp::MinMaxGray(_MetrixRegion, m_srcImage, 0, &Min, &Max, &Range);
					HalconCpp::AreaCenter(_MetrixRegion, &Area, &Row, &Column);
					//WriteTuple(AbsoluteHisto,	"d:\\AbsoluteHisto.tup");
					//WriteTuple(RelativeHisto,	"d:\\RelativeHisto.tup");
					double	_dMinValue = Min.D();
					double	_dMaxValue = Max.D();
					int		_iAllNumber = Area.L();
					long	_lLowNumber = Area.L() * m_vecMesureRegion[i].dLowValue;
					long	_HighNumber = Area.L() * m_vecMesureRegion[i].dHighValue;
					int		_iLenth = RelativeHisto.Length();
					long	_lCurrentValue = 0;
					long	_lFirstTrrigerLTime = 0;	//触发次数
					long	_lFirstTrrigerHTime = 0;	//触发次数
					double	_dCurrentAllValue = 0;
					double	_dCurrentValue = 0;
					_lCurrentValue = Area.L();
					for (int i = _iLenth - 1; i >= 0; i--) {
						int		_iNumber = AbsoluteHisto[i].I();
						double	_dHistoValue = _dMaxValue - abs(_dMaxValue - _dMinValue) / (_iLenth - 1);
						_lCurrentValue -= _iNumber;
						if (_lFirstTrrigerLTime > 0 && _lFirstTrrigerHTime <= 0) {
							if (_lCurrentValue > _lLowNumber) {
								_dCurrentAllValue += _iNumber * _dHistoValue;
							}
						}
						if (_lFirstTrrigerLTime <= 0) {
							if (_lCurrentValue <= _HighNumber) {
								_dCurrentAllValue += abs(_lCurrentValue - _HighNumber) * _dHistoValue;
								_lFirstTrrigerLTime++;
							}
						}
						if (_lFirstTrrigerHTime <= 0) {
							if (_lCurrentValue <= _lLowNumber) {
								_dCurrentAllValue += abs(_lCurrentValue + _iNumber - _lLowNumber) * _dHistoValue;
								_lFirstTrrigerHTime++;
							}
						}
					}
					_dCurrentValue = _dCurrentAllValue / (_iAllNumber		*	(m_vecMesureRegion[i].dHighValue - m_vecMesureRegion[i].dLowValue));
					_model.dMeasureValue = _dCurrentValue;

					sDrawText			_strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = QString("%1.%2:%3").arg(QString::number(i)).arg(tr("Histo")).arg(QString::number(_dCurrentValue, 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_strText);
					_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, QString::number(i) + "." + tr("Histo"));				//显示结果
					_ptrSubproperty->setValue(QString::number(_dCurrentValue, 'f', databaseVar::Instance().form_System_Precision));
					_ptrSubproperty->setAttribute("Parent", strParent);
					_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					_ptrSubproperty->setEnabled(false);
					_ptrSubproperty->setPropertyName(QString::number(i) + "." + tr("Histo"));

					m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
					m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
					_ptrproperty->addSubProperty(_ptrSubproperty);
				} break;
				default: {
				}break;
				}
			}
			catch (const HException& except) {
				QString strError = except.ErrorText();
				QStringList strErrorArray = strError.split(':');
				if (strErrorArray.size() > 1) {
					if (!strErrorArray[1].isEmpty()) {
						strError.clear();
						for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
						sDrawText			_strText;
						_strText.bControl = true;
						if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
						_strText.DrawColor = QColor(255, 0, 0);
						_strText.strText = strError;
						_VecText1D.push_back(_strText);
					}
				}
			}
			catch (...) {
				sDrawText			_strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = QObject::tr("Unknow Error");
				_VecText1D.push_back(_strText);
			}
		}
		double _dMeasureValue = 0;
		if (m_vecMesureRegion.size() > 1) {
			_dMeasureValue = m_vecMesureRegion[1].dMeasureValue - m_vecMesureRegion[0].dMeasureValue;
		}
		if (_dMeasureValue >= ui.spinMinValue->value()
			&& _dMeasureValue <= ui.spinMaxValue->value()) {
			sDrawText			_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(0, 255, 0);
			_strText.strText = QString("%1:%2")
				.arg(tr("Check OK"))
				.arg(QString::number(_dMeasureValue, 'f', databaseVar::Instance().form_System_Precision));
			_VecText1D.insert(_VecText1D.begin(), _strText);
		}
		else {
			sDrawText			_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = QString("%1:%2")
				.arg(tr("Check Fail"))
				.arg(QString::number(_dMeasureValue, 'f', databaseVar::Instance().form_System_Precision));
			_VecText1D.insert(_VecText1D.begin(), _strText);
		}

		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, tr("CloudMesure"));
		_ptrSubproperty->setValue(QString::number(_dMeasureValue, 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setEnabled(false);
		_ptrSubproperty->setPropertyName(tr("CloudMesure"));

		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);

		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, tr("Max"));
		_ptrSubproperty->setValue(QString::number(ui.spinMaxValue->value(), 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setEnabled(false);
		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);

		_ptrSubproperty = m_ResultvariantManager->addProperty(QVariant::Double, tr("Min"));
		_ptrSubproperty->setValue(QString::number(ui.spinMinValue->value(), 'f', databaseVar::Instance().form_System_Precision));
		_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
		_ptrSubproperty->setAttribute("Parent", strParent);
		_ptrSubproperty->setEnabled(false);
		m_ResultidToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
		m_ResultpropertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
		_ptrproperty->addSubProperty(_ptrSubproperty);

	}	break;
	case 1: { //3D检测

	}	break;
	default:	break;
	}
	param.VecShowQPointFs = (_VecPt1D);
	param.VecDrawText = (_VecText1D);

	return NodeResult_OK;
}

int frmCloudMesureTool::ExecuteComplete()
{
	for (auto iter = m_ResultidToProperty.begin(); iter != m_ResultidToProperty.end(); iter++) {
		QtBrowserItem *item = m_ResultpropertyEditor->addProperty(*iter);
		m_ResultpropertyEditor->setExpanded(item, true);
	}
	return 0;
}

void frmCloudMesureTool::slot_MouseClicked(QList<QGraphicsItem*>& items)
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

void frmCloudMesureTool::slot_MouseMouve(QList<QGraphicsItem*>& items)
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

int frmCloudMesureTool::Load()
{
	m_titleBar->setTitleContent(((CloudMesureTool*)m_pTool)->GetItemId());

	//设置图像输入
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((CloudMesureTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((CloudMesureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((CloudMesureTool*)m_pTool)->GetItemId()))	{	ui.cbx_ImgInPut->addItem(iter.first);	}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((CloudMesureTool*)m_pTool)->m_strInImg);

	if (ui.cbx_ImgInPut->currentIndex() >= 0) {
		slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	}
	if (MiddleParam::MapMiddle()[((CloudMesureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count(((CloudMesureTool*)m_pTool)->m_strInImg ) > 0)	{
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((CloudMesureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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
	on_btnDeleteROI1All_clicked();

	if (((CloudMesureTool*)m_pTool)->m_mapParam.contains("spinRoiW"))	ui.spinRoiW->setValue(((CloudMesureTool*)m_pTool)->m_mapParam["spinRoiW"].toInt());
	//设置默认参数
	if (ui.cbx_ImgInPut->currentIndex() >= 0)					slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());

	m_vecMesureRegion											= ((CloudMesureTool*)m_pTool)->m_vecMesureRegion;
	if (m_vecMesureRegion.size() <= 1)							m_vecMesureRegion.resize(2);

	for (int i = 0; i < m_vecMesureRegion.size(); i++)			{
		if (i == 0){
			ui.checkUseFollow->setChecked(m_vecMesureRegion[i].bEnableLink);
			ui.txtLinkFollow->setText(m_vecMesureRegion[i].strLinkName);
			DataToShape(m_vecMesureRegion[i].vecRoi, m_VecBaseItem);
			for(auto pItem : m_VecBaseItem)			m_view->AddItems(pItem);//录入矩形1
		}
		else if (i == 1){
			ui.check1UseFollow->setChecked(m_vecMesureRegion[i].bEnableLink);
			ui.txtLink1Follow->setText(m_vecMesureRegion[i].strLinkName);
			DataToShape(m_vecMesureRegion[i].vecRoi, m_VecROI2Item);
			for (auto pItem : m_VecROI2Item)		m_view->AddItems(pItem);//录入矩形1
		}
	}

	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		if (m_VecBaseItem[i] == nullptr)	continue;
		m_VecBaseItem[i]->setIndex(i);
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}

	for (size_t i = 0; i < m_VecROI2Item.size(); i++) {
		if (m_VecROI2Item[i] == nullptr)	continue;
		m_VecROI2Item[i]->setIndex(i);
		m_VecROI2Item[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
	//检测结果
	if (((CloudMesureTool*)m_pTool)->m_mapParam.contains("spinMinValue"))			ui.spinMinValue->setValue(((CloudMesureTool*)m_pTool)->m_mapParam["spinMinValue"].toDouble());
	if (((CloudMesureTool*)m_pTool)->m_mapParam.contains("spinMaxValue"))			ui.spinMaxValue->setValue(((CloudMesureTool*)m_pTool)->m_mapParam["spinMaxValue"].toDouble());
	
	//设置显示参数
	for (auto iter = ((CloudMesureTool*)m_pTool)->m_mapShowParam.begin();	
		iter != ((CloudMesureTool*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)		
			m_ShowidToProperty[iter.key()]->setValue(iter.value());

	ui.tabWidgetROI->setCurrentIndex(0);
	slot_ChangeROITabIndex(0);

	return 0;
}

int frmCloudMesureTool::Save()
{
	((CloudMesureTool*)m_pTool)->m_strInImg									= ui.cbx_ImgInPut->currentText();
	((CloudMesureTool*)m_pTool)->m_mapParam.insert("spinRoiW",				QString::number(ui.spinRoiW->value()));
	for (int i = 0; i < m_vecMesureRegion.size(); i++)	{
		if (i == 0) {
			ShapeToData(m_VecBaseItem,					m_vecMesureRegion[i].vecRoi);
			m_vecMesureRegion[i].bEnableLink			= ui.checkUseFollow->isChecked();
			m_vecMesureRegion[i].strLinkName			= ui.txtLinkFollow->text();
		}
		else if (i == 1) {
			ShapeToData(m_VecROI2Item,					m_vecMesureRegion[i].vecRoi);
			m_vecMesureRegion[i].bEnableLink			= ui.check1UseFollow->isChecked();
			m_vecMesureRegion[i].strLinkName			= ui.txtLink1Follow->text();
		}
	}
	((CloudMesureTool*)m_pTool)->m_vecMesureRegion							= m_vecMesureRegion;
	((CloudMesureTool*)m_pTool)->m_mapParam.insert("spinMinValue",			QString::number(ui.spinMinValue->value()));
	((CloudMesureTool*)m_pTool)->m_mapParam.insert("spinMaxValue",			QString::number(ui.spinMaxValue->value()));

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((CloudMesureTool*)m_pTool)->m_mapShowParam[iter.key()] = iter.value()->value().toString();

	return 0;
}

void frmCloudMesureTool::addResultProperty(QtVariantProperty * property, const QString & id)
{
	m_ResultpropertyToId[property] = id;
	m_ResultidToProperty[id] = property;
	QtBrowserItem *item = m_ResultpropertyEditor->addProperty(property);
	m_ResultpropertyEditor->setExpanded(item, true);
}

void frmCloudMesureTool::addProperty(QtVariantProperty * property, const QString & id)
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

void frmCloudMesureTool::slot_valueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)	return;
	if(!m_bValueChange)			return;

	QString strName = property->propertyName();
	if (propertyToId.count(property) > 0) {
		QString _strName = propertyToId[property];
		if (idToProperty.count(propertyToId[property]) > 0) {
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++) {
				bool bFind = false;
				switch (ui.tabWidgetROI->currentIndex()) {
				case 0: {	for (auto Iter : m_VecBaseItem) { if (Iter == items[i]) { bFind = true; } }	}	break;
				case 1: {	for (auto Iter : m_VecROI2Item) { if (Iter == items[i]) { bFind = true; } } }	break;
				default:			break;
				}
				if (!bFind) { continue; }
				float fData[5] = { 0 };
				switch (((BaseItem*)items[i])->GetItemType()) {
				case ItemPoint: {
					PointItem *pValueItem = (PointItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemCircle: {
					CircleItem *pValueItem = (CircleItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemEllipse: {
					EllipseItem *pValueItem = (EllipseItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemConcentric_Circle: {
					ConcentricCircleItem *pValueItem = (ConcentricCircleItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemRectangle: {
					RectangleItem *pValueItem = (RectangleItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemRectangleR: {
					RectangleRItem *pValueItem = (RectangleRItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemSquare: {
				}break;
				case ItemPolygon: {
					PolygonItem *pValueItem = (PolygonItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemLineObj: {
					LineItem *pValueItem = (LineItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ItemBezierLineObj: {
					BezierLineItem *pItemValue = (BezierLineItem*)items[i];
					pItemValue->SetParam(propertyToId[property], value.toString());
					pItemValue->UpdateLinePath();
					pItemValue->update();
				}break;
				default: {

				}break;
				}
			}
		}
	}

}

void frmCloudMesureTool::slot_Result_valueChanged(QtProperty * property, const QVariant & value)
{
	if (property == nullptr)		return;
}

void frmCloudMesureTool::slot_ChangeMatchImg_Index(int Index)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((CloudMesureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((CloudMesureTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

void frmCloudMesureTool::slot_ChangeROITabIndex(int iIndex)
{
	ChangeRoiIndex(iIndex);
	ui.cbx_RegionType->setCurrentIndex(iIndex);
	ChangeParamIndex(iIndex);	//切换参数
}

void frmCloudMesureTool::slot_ChangeRegionTabIndex(int iIndex)
{
	ChangeRoiIndex(iIndex);
	ui.tabWidgetROI->setCurrentIndex(iIndex);
}

void frmCloudMesureTool::slot_ChangeALargrimType(int Index)
{
	switch (ui.cbx_ALargrimType->currentIndex())
	{
	case 0: {//平均
		ui.stackedROI->setCurrentIndex(2);
	}break;
	case 1: {//最大值
		ui.stackedROI->setCurrentIndex(2);
	}break;
	case 2: {//中值
		ui.stackedROI->setCurrentIndex(2);
	}break;
	case 3: {//最小值
		ui.stackedROI->setCurrentIndex(2);
	}break;
	case 4: {//直方图比例
		ui.stackedROI->setCurrentIndex(1);
	}break;
	default:{}break;	}

	if (ui.tabWidgetROI->currentIndex() < m_vecMesureRegion.size()) 
		m_vecMesureRegion[ ui.tabWidgetROI->currentIndex() ].eAlargrim = (EnumALargrim)ui.cbx_ALargrimType->currentIndex();

}

void frmCloudMesureTool::slot_ChangeSpanSlider(int lower, int upper)
{
	if (ui.tabWidgetROI->currentIndex() < m_vecMesureRegion.size()) {
		m_vecMesureRegion[ui.tabWidgetROI->currentIndex()].eAlargrim	= (EnumALargrim)ui.cbx_ALargrimType->currentIndex();
		m_vecMesureRegion[ui.tabWidgetROI->currentIndex()].dLowValue	= lower * 0.01;
		m_vecMesureRegion[ui.tabWidgetROI->currentIndex()].dHighValue	= upper * 0.01;
	}
}

void frmCloudMesureTool::ChangeRoiIndex(int iIndex)
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
	switch (iIndex)
	{
	case 0: {
		for (auto pIter : m_VecBaseItem) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(true);
		}
		for (auto pIter : m_VecROI2Item) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(false);
		}
	}	break;
	case 1: {
		for (auto pIter : m_VecBaseItem) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(false);
		}
		for (auto pIter : m_VecROI2Item) {
			if (pIter == nullptr)	continue;
			pIter->setVisible(true);
		}
	}	break;
	default: {
	}	break;
	}
}

void frmCloudMesureTool::ChangeParamIndex(int iIndex)
{
	if (iIndex < m_vecMesureRegion.size()){
		ui.cbx_ALargrimType->setCurrentIndex(m_vecMesureRegion[iIndex].eAlargrim);
		switch (m_vecMesureRegion[iIndex].eAlargrim)	{
		case ALargrim_Medium: {
		} break;
		case ALargrim_Max: {
		} break;
		case ALargrim_Mean: {
		} break;
		case ALargrim_Min: {
		} break;
		case ALargrim_GrayHisto: {
			ui.SliderPercent->setLowerValue(m_vecMesureRegion[iIndex].dLowValue * 100);
			ui.SliderPercent->setUpperValue(m_vecMesureRegion[iIndex].dHighValue * 100);
		} break;
		default:
			break;
		}
	}

}

void frmCloudMesureTool::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
}

