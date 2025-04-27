#include "frmMatchTool.h"
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
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmMatchTool::frmMatchTool(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

	m_VecCreateCross.clear();
	//设置窗口背景透明
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/match.png"));

	//图像显示
	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmBaseTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	initTitleBar();
	initConnect();
	initShapeProperty();
	initResultProperty();
	initData();
}

frmMatchTool::~frmMatchTool()
{
	if (m_view != nullptr)					delete m_view;						m_view		= nullptr;
	if (m_titleBar != nullptr)				delete m_titleBar;					m_titleBar	= nullptr;
	for (auto iter : m_VecBaseItem)			if (iter != nullptr)				{ delete iter; iter = nullptr; }
	for (auto iter : m_VecSearchBaseItem)	if (iter != nullptr)				{ delete iter; iter = nullptr; }
	for (auto iter : idToProperty)			if (iter != nullptr)				delete iter;	idToProperty.clear();
	propertyToId.clear();
	if (propertyEditor != nullptr)			delete propertyEditor;				propertyEditor = nullptr;
	if (variantManager != nullptr)			delete variantManager;				variantManager = nullptr;
	if (variantFactory != nullptr)			delete variantFactory;				variantFactory = nullptr;
	m_VecBaseItem.clear();
	m_VecSearchBaseItem.clear();
	ClearMatchShapeModel();
	ClearMatchNCCModel();
	this->deleteLater();
}

void frmMatchTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/match.png");

}

void frmMatchTool::initShapeProperty()
{
	variantManager		= new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	variantFactory		= new QtVariantEditorFactory(this);
	propertyEditor		= new QtTreePropertyBrowser(ui.ROIwidget);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.ROILayout->addWidget(propertyEditor);

	idToProperty.clear();
	propertyToId.clear();
	m_VecBaseItem.clear();
	m_VecSearchBaseItem.clear();

	btnGroupRadio = new QButtonGroup(this);
	btnGroupRadio->addButton(ui.radioButton_ROI, 0);
	btnGroupRadio->addButton(ui.radioButton_Search, 1);
	ui.radioButton_ROI->click();
	ui.radioButton_ROI->setChecked(true);
	connect(btnGroupRadio, SIGNAL(buttonClicked(int)), this, SLOT(slot_btnGroupClicked(int)));
}

void frmMatchTool::initResultProperty()
{
	initShowEditorProperty(ui.widget_Result, ui.ResultLayout);
	//connect(m_ShowvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_Result_valueChanged(QtProperty *, const QVariant &)));
	ui.gImage->setVisible(false);
	QtVariantProperty*	_ptrproperty = nullptr;
	QtVariantProperty*	_ptrSubproperty = nullptr;
	{
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("showregion"));
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes") ;
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showregion, tr("showregion"));
	}
}

void frmMatchTool::initConnect()
{
	connect(ui.cbx_Camerachannal,			SIGNAL(activated(int)),								this, SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW,					SIGNAL(valueChanged(int)),							this, SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut,				SIGNAL(activated(int)),								this, SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view,							SIGNAL(sig_MouseClicked(QList<QGraphicsItem *>&)),	this, SLOT(slot_MouseClicked(QList<QGraphicsItem *>&)));
	connect(m_view,							SIGNAL(sig_MouseMouve(QList<QGraphicsItem *>&)),	this, SLOT(slot_MouseMouve(QList<QGraphicsItem *>&)));
	connect(ui.cbx_MatchType,				SIGNAL(currentIndexChanged(int)),					this, SLOT(slot_ChangeMatchType(int)));
	connect(ui.spBx_LowContrast,			SIGNAL(valueChanged(int)),							this, SLOT(slot_SetLowContrastChanged(int)));
	connect(ui.spBx_HighContrast,			SIGNAL(valueChanged(int)),							this, SLOT(slot_SetHighContrastChanged(int)));
	connect(ui.spBx_FindGreedy,				SIGNAL(valueChanged(double)),						this, SLOT(slot_SetGreedyChanged(double)));
	connect(ui.spBx_FindMinOverLap,			SIGNAL(valueChanged(double)),						this, SLOT(slot_SetMinOverLapChanged(double)));
	connect(ui.spBx_FindMinScore,			SIGNAL(valueChanged(double)),						this, SLOT(slot_SetMinScoreChanged(double)));
	connect(ui.tabWidget,					SIGNAL(currentChanged(int)),						this, SLOT(slot_ChangeTab(int)));

	connect(ui.hSliderGreedy,				&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindGreedy->setValue(value * 0.01);	});
	connect(ui.hSliderMinOverLap,			&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindMinOverLap->setValue(value * 0.01);	});
	connect(ui.hSlider_MinScore,			&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindMinScore->setValue(value * 0.01);	});
	//connect(ui.spBx_FindMinOverLap,		SIGNAL(valueChanged(int)), this, SLOT(slot_SetHighContrastChanged(int)));
	
	connect(ui.btnAddROI,					&QPushButton::pressed,								this,&frmMatchTool::on_btnAddROI_clicked);
	connect(ui.btnDeleteROISelect,			&QPushButton::pressed,								this,&frmMatchTool::on_btnDeleteROISelect_clicked);
	connect(ui.btnDeleteROIAll,				&QPushButton::pressed,								this,&frmMatchTool::on_btnDeleteROIAll_clicked);
	
	connect(ui.btnAddSearchROI,				&QPushButton::pressed,								this,&frmMatchTool::on_btnAddSearchROI_clicked);
	connect(ui.btnDeleteSearchROISelect,	&QPushButton::pressed,								this,&frmMatchTool::on_btnDeleteSearchROISelect_clicked);
	connect(ui.btnDeleteSearchROIAll,		&QPushButton::pressed,								this,&frmMatchTool::on_btnDeleteSearchROIAll_clicked);
	//connect(ui.btnLinkContent,			&QPushButton::pressed,								this,			&frmMatchTool::on_btnLinkContent_clicked);
	//connect(ui.btnDelLinkContent,		&QPushButton::pressed,								this,			&frmMatchTool::on_btnDelLinkContent_clicked);

}

void frmMatchTool::initData()
{
	ui.tabWidget->setCurrentIndex(0);
}

void frmMatchTool::ClearROIProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

void frmMatchTool::ClearMatchShapeModel()
{
	try {	ClearShapeModel(m_hShapeModel);	}	catch (...) {}
}

void frmMatchTool::ClearMatchNCCModel()
{
	try {	ClearNccModel(m_hNCCModel);	}	catch (...) {}
}

//绘制ROI
void frmMatchTool::on_btnAddROI_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth	= (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;

	float fData[5]		= { 0 };
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
	switch (ui.comboxROIType->currentIndex())	{
	case 0:	//合并
	{	_BaseItem->m_eRegionType = RegionType_Add;	}break;
	case 1:	//差集
	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default: {	}	break;
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
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmMatchTool::on_btnDeleteROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)	{
		if (iter != nullptr)
			if (iter->type() == 10)		{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecBaseItem.size(); i++)	{
					if (m_VecBaseItem[i] == iter)		{
						_bFinded = true;
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

void frmMatchTool::on_btnDeleteROIAll_clicked()
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
			//ui.tableROIWidget->removeRow(((BaseItem*)iter)->getIndex());
			if (_bFinded)
			{
				m_VecBaseItem.removeOne((BaseItem*)iter);
				(m_view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void frmMatchTool::on_btnAddSearchROI_clicked()
{
	ClearROIProperty();
	BaseItem* _BaseItem;
	int iWidth = (m_view->pImagItem)->w;
	int iHeight = (m_view->pImagItem)->h;

	QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;

	QtVariantProperty *	_ptrproperty = nullptr;
	QtVariantProperty *	_ptrSubproperty = nullptr;
	QtVariantProperty *	_ptrSubSubproperty = nullptr;
	//_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");

	float fData[5] = { 0 };
	switch (ui.comboSearchROIShape->currentIndex())
	{
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 2.0, iHeight / 2.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}	break;
	case 1: {	//椭圆
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth;
		fData[3] = iHeight;
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

		fData[0] = 0;
		fData[1] = 0;
		fData[2] = iWidth ;
		fData[3] = iHeight;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		_BaseItem->GetAllParam(_mapValue);
		m_view->AddItems(_BaseItem);
	}   break;
	case 4: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth;
		fData[3] = iHeight;
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
	switch (ui.comboxSearchROIType->currentIndex())	{
	case 0:{ _BaseItem->m_eRegionType = RegionType_Add;		}break;
	case 1:{ _BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default:	break;
	}
	_BaseItem->SetDrawPenWidth(ui.spinRoiW->value());
	m_VecSearchBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
}

void frmMatchTool::on_btnDeleteSearchROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr)
			if (iter->type() == 10)
			{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)
				{
					if (m_VecSearchBaseItem[i] == iter)
					{
						_bFinded = true;
					}
				}
				if (_bFinded)
				{
					(m_view->scene())->removeItem(iter);
					m_VecSearchBaseItem.removeOne((BaseItem*)iter);
					delete iter;
					iter = nullptr;
				}
			}
	}

	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
}

void frmMatchTool::on_btnDeleteSearchROIAll_clicked()
{
	ClearROIProperty();
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			//ui.tableROIWidget->removeRow(((BaseItem*)iter)->getIndex());
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)
			{
				if (m_VecSearchBaseItem[i] == iter)
				{
					_bFinded = true;
				}
			}
			if (_bFinded)
			{
				m_VecSearchBaseItem.removeOne((BaseItem*)iter);
				(m_view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
}

void frmMatchTool::onChannelClicked(int Index)
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

void frmMatchTool::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto iter : m_VecSearchBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());

}

//设置对比度变化
void frmMatchTool::slot_SetLowContrastChanged(int Index)
{
	if (ui.spBx_LowContrast->value() > ui.spBx_HighContrast->value())
	{
		ui.spBx_HighContrast->setValue(ui.spBx_LowContrast->value());
	}

}

//设置对比度变化
void frmMatchTool::slot_SetHighContrastChanged(int Index)
{
	if (ui.spBx_HighContrast->value() < ui.spBx_LowContrast->value())
	{
		ui.spBx_LowContrast->setValue(ui.spBx_HighContrast->value());
	}
}

void frmMatchTool::slot_SetGreedyChanged(double value)
{
	ui.hSliderGreedy->setValue(value * 100);
}

void frmMatchTool::slot_SetMinOverLapChanged(double value)
{
	ui.hSliderMinOverLap->setValue(value * 100);
}

void frmMatchTool::slot_SetMinScoreChanged(double value)
{
	ui.hSlider_MinScore->setValue(value * 100);
}

void frmMatchTool::slot_ChangeTab(int iTab)
{
	switch (iTab)
	{
	case 0:
	case 1:
	{	
		m_view->ClearAllDrawItem();
		if (btnGroupRadio->checkedId() == 0)
		{
			for (auto &iter : m_VecBaseItem)		iter->setVisible(true);
			for (auto &iter : m_VecSearchBaseItem)	iter->setVisible(false);
		}
		else
		{
			for (auto &iter : m_VecBaseItem)		iter->setVisible(false);
			for (auto &iter : m_VecSearchBaseItem)	iter->setVisible(true);
		}
	}	break;
	default:
	{
		m_view->ClearAllDrawItem();
		for (auto& iter : m_VecBaseItem)	iter->setVisible(false);
		for (auto& iter : m_VecSearchBaseItem)	iter->setVisible(false);
	}	break;
	}
}

void frmMatchTool::slot_btnGroupClicked(int iID)
{
	if (btnGroupRadio->checkedId() == 0)
	{
		for (auto &iter : m_VecBaseItem)		iter->setVisible(true);
		for (auto &iter : m_VecSearchBaseItem)	iter->setVisible(false);
	}
	else
	{
		for (auto &iter : m_VecBaseItem)		iter->setVisible(false);
		for (auto &iter : m_VecSearchBaseItem)	iter->setVisible(true);
	}
}

void frmMatchTool::slot_ChangeMatchType(int Index)
{
	switch ((EnumModeType)ui.cbx_MatchType->currentIndex())
	{
	case EnumModeType_Shape: {
		ui.Layout_LowContrast->setVisible(true);
		ui.Layout_HighContrast->setVisible(true);
		ui.Layout_MinSize->setVisible(true);
		ui.cbx_SubPixType->clear();
		QStringList _strLst;
		_strLst << "none" << "interpolation" << "least_squares" << "least_squares_high" << "least_squares_very_high" ;
		ui.cbx_SubPixType->addItems(_strLst);
	}	break;
	case EnumModeType_NCC: {
		ui.Layout_LowContrast->setVisible(false);
		ui.Layout_HighContrast->setVisible(false);
		ui.Layout_MinSize->setVisible(false);
		ui.cbx_SubPixType->clear();
		QStringList _strLst;
		_strLst << "false" << "true" ;
		ui.cbx_SubPixType->addItems(_strLst);
	}	break;
	default:
		break;
	}
}

void frmMatchTool::slot_ChangeMatchImg_Index(int Index)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((MatchTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count( ui.cbx_ImgInPut->currentText() ) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((MatchTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

EnumNodeResult frmMatchTool::Execute(MiddleParam& param, QString& strError)
{
	if (!m_dstImage.IsInitialized()) { return NodeResult_Error; }
	try {
		switch (ui.tabWidget->currentIndex())
		{
		case 1: {	slot_CreateShapeModel(param, strError);	}break;
		case 0:
		case 2: {	slot_FindShapeModel(param,strError);		}break;
		default: {								}break;
		}
	}
	catch (...) {}

	return NodeResult_OK;
}

void frmMatchTool::slot_MouseClicked(QList<QGraphicsItem*>& items)
{
	if (items.size() < 0)	return;

	ClearROIProperty();

	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;
	QtVariantProperty *_ptrSubSubproperty = nullptr;

	for (int i = 0; i < items.size(); i++)	{
		QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
		((BaseItem*)items[i])->GetAllParam(_mapValue);

		for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
			if (	((*mapIter).first).count(cstItem) > 0) {
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

void frmMatchTool::slot_MouseMouve(QList<QGraphicsItem*>& items)
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

int frmMatchTool::slot_CreateShapeModel(MiddleParam& param, QString& strError)
{
	if (!m_dstImage.IsInitialized()) { return -1; }
	HObject	_ReduceImg, _ModelImages, _ModelRegions, _SelectObj, _ModelContours, ho_TransContours;
	HTuple _hv_ModelId, _NumLevels, _Contrast, _Number, Rows, Columns, _Lenth;
	//m_view->ClearAllDrawItem();
	std::vector<QPointF>	_VecPt1D;
	std::vector<sDrawText>	_VecText1D;
	std::vector<sDrawCross> _PointLst;
	try {
		ShapeToRegion(m_VecBaseItem, ModelROIRegion);
		HalconCpp::ReduceDomain(m_dstImage, ModelROIRegion, &_ReduceImg);
		if (ui.cBx_Auto->isChecked())
			_NumLevels = 0;
		else 
			_NumLevels	= ui.spBx_PyLevel->value();
		_Contrast	= ui.spBx_LowContrast->value();
		_Contrast.TupleConcat(ui.spBx_HighContrast->value());
		_Contrast.TupleConcat(ui.spBx_MinSize->value());

		switch ((EnumModeType)ui.cbx_MatchType->currentIndex()){
		case EnumModeType_Shape: {
			//Matching 01: create the shape model
			HalconCpp::CreateShapeModel(_ReduceImg, /*"auto"*/0, HTuple(ui.spBx_CAngleStart->value()).TupleRad(), HTuple(ui.spBx_CAngleRange->value()).TupleRad(),
				"auto", "auto", "use_polarity", _Contrast, "auto", &_hv_ModelId);
			HTuple hv_AngleStart, hv_AngleExtent, hv_AngleStep, hv_ScaleMin, hv_ScaleMax, hv_ScaleStep, hv_Metric, hv_MinContrast;
			HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
			HalconCpp::GetShapeModelParams(_hv_ModelId, &_NumLevels, &hv_AngleStart, &hv_AngleExtent,
				&hv_AngleStep, &hv_ScaleMin, &hv_ScaleMax, &hv_ScaleStep, &hv_Metric, &hv_MinContrast);
			ui.spBx_PyLevel->setValue(_NumLevels.I());

			HalconCpp::GetShapeModelContours(&_ModelContours, _hv_ModelId, 1);
			HalconCpp::FindShapeModel(m_dstImage, _hv_ModelId, hv_AngleStart, hv_AngleExtent, 0.5, 1, 0.5, "least_squares", 0, 0.5, &Row, &Column, &Angle, &Score);

			int	_iScoreIndex = Score.TupleLength();
			if (_iScoreIndex <= 0) {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = tr("CreateModel NG");
				_VecText1D.push_back(_strText);
			}
			else {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(0, 255, 0);
				_strText.strText = tr("CreateModel OK");
				_VecText1D.push_back(_strText);
			} {
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;
				m_VecCreateCross.clear();
				for (HTuple hv_MatchingObjIdx = 0; hv_MatchingObjIdx <= end_val19; hv_MatchingObjIdx += step_val19) {
					HalconCpp::HomMat2dIdentity(&hv_HomMat);
					HalconCpp::HomMat2dRotate(hv_HomMat, HTuple(Angle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
					HalconCpp::HomMat2dTranslate(hv_HomMat, HTuple(Row[hv_MatchingObjIdx]), HTuple(Column[hv_MatchingObjIdx]),
						&hv_HomMat);
					HalconCpp::AffineTransContourXld(_ModelContours, &ho_TransContours, hv_HomMat);
					HalconCpp::GenRegionContourXld(ho_TransContours, &_SelectObj, "margin");
					HalconCpp::Union1(_SelectObj, &_ModelRegions);
					HalconCpp::GetRegionPoints(_ModelRegions, &Rows, &Columns);
					HalconCpp::TupleLength(Rows, &_Lenth);

					for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
						_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
					}
					sDrawText _item;
					_item.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_item.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_item.Center.setX(20);
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_item);

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 0, 255);
					_PointLst.push_back(_Cross);
					//创建模板中心
					m_VecCreateCross.push_back(_Cross);
				}
			}

			ClearMatchShapeModel();
			m_hShapeModel = _hv_ModelId;
		}	break;
		case EnumModeType_NCC: {
			HalconCpp::CreateNccModel(_ReduceImg, /*"auto"*/0, HTuple(ui.spBx_CAngleStart->value()).TupleRad(), HTuple(ui.spBx_CAngleRange->value()).TupleRad(), "auto", "use_polarity", &_hv_ModelId );
			HTuple hv_AngleStart, hv_AngleExtent, hv_AngleStep, hv_ScaleMin, hv_ScaleMax, hv_ScaleStep, hv_Metric, hv_MinContrast;
			HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
			HalconCpp::GetNccModelParams(_hv_ModelId, &_NumLevels, &hv_AngleStart, &hv_AngleExtent,&hv_AngleStep, &hv_Metric);
			HalconCpp::FindNccModel(m_dstImage, _hv_ModelId, hv_AngleStart, hv_AngleExtent, 0.5, 1, 0.5, "true", 0, &Row, &Column, &Angle, &Score);

			int	_iScoreIndex = Score.TupleLength();
			if (_iScoreIndex <= 0) {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = tr("CreateModel NG");
				_VecText1D.push_back(_strText);
			}
			else {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(0, 255, 0);
				_strText.strText = tr("CreateModel OK");
				_VecText1D.push_back(_strText);
			} {
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;
				m_VecCreateCross.clear();
				for (HTuple hv_MatchingObjIdx = 0; hv_MatchingObjIdx <= end_val19; hv_MatchingObjIdx += step_val19) {
					//HalconCpp::HomMat2dIdentity(&hv_HomMat);
					//HalconCpp::HomMat2dRotate(hv_HomMat, HTuple(Angle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
					//HalconCpp::HomMat2dTranslate(hv_HomMat, HTuple(Row[hv_MatchingObjIdx]), HTuple(Column[hv_MatchingObjIdx]),&hv_HomMat);
					//HalconCpp::AffineTransContourXld(_ModelContours, &ho_TransContours, hv_HomMat);
					//HalconCpp::GenRegionContourXld(ho_TransContours, &_SelectObj, "margin");
					//HalconCpp::Union1(_SelectObj, &_ModelRegions);
					//HalconCpp::GetRegionPoints(_ModelRegions, &Rows, &Columns);
					//HalconCpp::TupleLength(Rows, &_Lenth);
					//for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
					//	_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
					//}
					sDrawText _item;
					_item.bControl = true;
					_item.Center.setX(20);
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_item);

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
					_Cross.DrawColor = QColor(0, 255, 0);
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_PointLst.push_back(_Cross);
					//创建模板中心
					m_VecCreateCross.push_back(_Cross);
				}
			}
			ClearMatchNCCModel();
			m_hNCCModel = _hv_ModelId;
		}	break;
		default:
			break;
		}

	}
	catch (const HException& except) {
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray =	strError.split(':');
		if (strErrorArray.size() > 1) {
			if(!strErrorArray[1].isEmpty())
			{
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_strText.strText = tr("CreateModel Error") + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...)	{
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("CreateModel Error");
		_VecText1D.push_back(_strText);
	}

	param.VecShowQPointFs	= (_VecPt1D);
	param.VecDrawText		= (_VecText1D);
	for (int i = 0; i < _PointLst.size(); i++	) {
		param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
	}

	//param.VecShowColorPoints.push_back(_ptColors);
	////m_view->DispPoint(_VecPt1D, QColor(0, 255, 0));
	//m_view->DispTextList(_VecText1D);
	//m_view->DispCrossPoint(_PointLst);
	return 0;
}

int frmMatchTool::slot_FindShapeModel(MiddleParam& param, QString& strError)
{
	HTuple hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_Greedy, hv_SubPixel;
	HTuple Row, Column, Angle, Scale, Score, hv_HomMat;		
	std::vector<QPointF>	_VecPt1D;
	std::vector<sDrawText>	_VecText1D;
	std::vector<sDrawCross> _PointLst;
	try
	{
		hv_AngleStart	= HTuple(ui.spBx_CAngleStart->value()).TupleRad();
		hv_AngleExtent	= HTuple(ui.spBx_CAngleRange->value()).TupleRad();
		hv_MinScore		= ui.spBx_FindMinScore->value() ;
		hv_NumMatches	= ui.spBx_FindMinNumber->value();
		hv_Greedy		= ui.spBx_FindGreedy->value();
		hv_MaxOverlap	= ui.spBx_FindMinOverLap->value();

		HObject	_ReduceImg;
		ShapeToRegion(m_VecSearchBaseItem, SearchROIRegion);
		HalconCpp::ReduceDomain(m_dstImage, SearchROIRegion, &_ReduceImg);

		switch ((EnumModeType)ui.cbx_MatchType->currentIndex())
		{
		case EnumModeType_Shape: {
			switch (ui.cbx_SubPixType->currentIndex()) {
			case 0: {	hv_SubPixel = "none";						}break;
			case 1: {	hv_SubPixel = "interpolation";				}break;
			case 2: {	hv_SubPixel = "least_squares";				}break;
			case 3: {	hv_SubPixel = "least_squares_high";			}break;
			case 4: {	hv_SubPixel = "least_squares_very_high";	}break;
			default:	break;			}
			HalconCpp::FindShapeModel(_ReduceImg, m_hShapeModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, hv_Greedy, &Row, &Column, &Angle, &Score);

			{
				HObject _ModelContours, ho_TransContours, _SelectObj, _ModelRegions;
				HTuple	Rows, Columns, _Lenth;
				HalconCpp::GetShapeModelContours(&_ModelContours, m_hShapeModel, 1);
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;

				int	_iScoreIndex = Score.TupleLength();
				if (_iScoreIndex <= 0) {
					sDrawText _strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = tr("Find Model NG");
					_VecText1D.push_back(_strText);
				}
				else {
					sDrawText _strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = tr("Find Model OK");
					_VecText1D.push_back(_strText);
				}

				for (int hv_MatchingObjIdx = 0; hv_MatchingObjIdx < _iScoreIndex; hv_MatchingObjIdx++) {
					HalconCpp::HomMat2dIdentity(&hv_HomMat);
					HalconCpp::HomMat2dRotate(hv_HomMat, HTuple(Angle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
					HalconCpp::HomMat2dTranslate(hv_HomMat, HTuple(Row[hv_MatchingObjIdx]), HTuple(Column[hv_MatchingObjIdx]), &hv_HomMat);
					HalconCpp::AffineTransContourXld(_ModelContours, &ho_TransContours, hv_HomMat);
					HalconCpp::GenRegionContourXld(ho_TransContours, &_SelectObj, "margin");
					HalconCpp::Union1(_SelectObj, &_ModelRegions);
					HalconCpp::GetRegionPoints(_ModelRegions, &Rows, &Columns);
					HalconCpp::TupleLength(Rows, &_Lenth);
					for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
						_VecPt1D.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
					}
					sDrawText _item;
					_item.bControl = true;
					_item.Center.setX(20);
					if (m_ShowidToProperty.count(FontSize) > 0)	_item.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_item);

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_Cross.DrawColor = QColor(0, 255, 0);
					_PointLst.push_back(_Cross);
				}
			}
		}	break;
		case EnumModeType_NCC: {
			switch (ui.cbx_SubPixType->currentIndex()) {
			case 0: {	hv_SubPixel = "false";			}break;
			case 1: {	hv_SubPixel = "true";			}break;
			default:	break;			}

			HalconCpp::FindNccModel(_ReduceImg, m_hNCCModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, &Row, &Column, &Angle, &Score);
			{
				HObject _ModelContours, ho_TransContours, _SelectObj, _ModelRegions;
				HTuple	Rows, Columns, _Lenth;
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;

				int	_iScoreIndex = Score.TupleLength();
				if (_iScoreIndex <= 0) {
					sDrawText _strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = tr("Find Model NG");
					_VecText1D.push_back(_strText);
				}
				else {
					sDrawText _strText;
					_strText.bControl = true;
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = tr("Find Model OK");
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_VecText1D.push_back(_strText);
				}
				for (int hv_MatchingObjIdx = 0; hv_MatchingObjIdx < _iScoreIndex; hv_MatchingObjIdx++) {
					sDrawText _item;
					_item.bControl = true;
					_item.Center.setX(20);
					if (m_ShowidToProperty.count(FontSize) > 0)	_item.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision));
					_VecText1D.push_back(_item);

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
					_Cross.DrawColor = QColor(0, 0, 255);
					_PointLst.push_back(_Cross);
				}
			}
		}	break;
		default:
			break;
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
				_strText.strText =	tr("Find Model Error") + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...) {
		sDrawText _strText;
		_strText.bControl = true;
		if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("Find Model Error");
		_VecText1D.push_back(_strText);
	}

	//m_view->DispPoint(_VecPt1D, QColor(0, 255, 0));
	//m_view->DispTextList(_VecText1D);
	//m_view->DispCrossPoint(_PointLst);

	param.VecShowQPointFs = (_VecPt1D);
	param.VecDrawText = (_VecText1D);
	for (int i = 0; i < _PointLst.size(); i++) {
		param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
	}

	if (m_ShowidToProperty.count(showregion) > 0)
		if (m_ShowidToProperty.value(showregion)->value().toBool()) {
			AffineShapeTrans(m_VecSearchBaseItem, HTuple(), param);
		}
	return 0;
}

int frmMatchTool::Load()
{
	m_titleBar->setTitleContent(((MatchTool*)m_pTool)->GetItemId());

	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((MatchTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((MatchTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((MatchTool*)m_pTool)->GetItemId())) {
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((MatchTool*)m_pTool)->m_strInImg);

	if (MiddleParam::MapMiddle()[((MatchTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count(((MatchTool*)m_pTool)->m_strInImg) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((MatchTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
		if (pImg.IsInitialized()) {
			m_srcImage = pImg.Clone();
			m_dstImage = m_srcImage.Clone();
			QImage img(Hal2QImage(m_dstImage));
			m_view->DispImage(img);

			ui.cbx_Camerachannal->clear();
			HTuple	hType, hChannel;
			GetImageType(m_dstImage,	&hType);
			CountChannels(m_dstImage,	&hChannel);
			ui.cbx_Camerachannal->addItem("All");
			for (int i = 0; i < hChannel.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));
		}
	}

	on_btnDeleteROIAll_clicked();
	DataToShape(((MatchTool*)((MatchTool*)m_pTool))->m_vecCommonData, m_VecBaseItem);
	for (auto pItem : m_VecBaseItem)	m_view->AddItems(pItem);

	on_btnDeleteSearchROIAll_clicked();
	DataToShape(((MatchTool*)((MatchTool*)m_pTool))->m_vecSearchData, m_VecSearchBaseItem);
	for (auto pItem : m_VecSearchBaseItem)	m_view->AddItems(pItem);

	if (ui.cbx_ImgInPut->currentIndex() >= 0)					slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spinRoiW") > 0)				ui.spinRoiW->setValue(((MatchTool*)m_pTool)->m_mapParam["spinRoiW"].toInt());
	//创建
	if (((MatchTool*)m_pTool)->m_mapParam.count("cbx_MatchType") > 0) {
		ui.cbx_MatchType->setCurrentIndex(((MatchTool*)m_pTool)->m_mapParam["cbx_MatchType"].toInt());
		((MatchTool*)((MatchTool*)m_pTool))->m_EMatchtype									= (EnumModeType)ui.cbx_MatchType->currentIndex();
	}
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_LowContrast") > 0)		ui.spBx_LowContrast->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_LowContrast"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_HighContrast") > 0)		ui.spBx_HighContrast->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_HighContrast"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_MinSize") > 0)			ui.spBx_MinSize->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_MinSize"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_PyLevel") > 0)			ui.spBx_PyLevel->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_PyLevel"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_CAngleStart") > 0)		ui.spBx_CAngleStart->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_CAngleStart"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_CAngleRange") > 0)		ui.spBx_CAngleRange->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_CAngleRange"].toInt());
	//创建
	if (((MatchTool*)m_pTool)->m_mapParam.count("cbx_SubPixType") > 0)			ui.cbx_SubPixType->setCurrentIndex(((MatchTool*)m_pTool)->m_mapParam["cbx_SubPixType"].toInt());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindMinScore") > 0)		ui.spBx_FindMinScore->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindMinScore"].toDouble());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindMinNumber") > 0)		ui.spBx_FindMinNumber->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindMinNumber"].toDouble());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindGreedy") > 0)			ui.spBx_FindGreedy->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindGreedy"].toDouble());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindMinOverLap") > 0)		ui.spBx_FindMinOverLap->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindMinOverLap"].toDouble());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindAngleStart") > 0)		ui.spBx_FindAngleStart->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindAngleStart"].toDouble());
	if (((MatchTool*)m_pTool)->m_mapParam.count("spBx_FindAngleRangle") > 0)	ui.spBx_FindAngleRangle->setValue(((MatchTool*)m_pTool)->m_mapParam["spBx_FindAngleRangle"].toDouble());
	ui.radioButton_ROI->setChecked(true);
	slot_btnGroupClicked(btnGroupRadio->checkedId());

	ClearMatchShapeModel();
	try
	{
		HTuple SerializedItemHandle;
		SerializeShapeModel(((MatchTool*)((MatchTool*)m_pTool))->m_hShapeModel, &SerializedItemHandle);
		HSerializedItem item(SerializedItemHandle);
		Hlong hSize;
		void *_pStart = item.GetSerializedItemPtr(&hSize);
		BYTE*_pData = new BYTE[hSize];
		memset(_pData, 0, hSize);
		memcpy_s(_pData, hSize, _pStart, hSize);

		HSerializedItem _item1(_pData, hSize, "true");
		DeserializeShapeModel(_item1, &m_hShapeModel);
		delete[]_pData;
	}
	catch (...)	{	}

	ClearMatchNCCModel();
	try	{
		HTuple SerializedItemHandle;
		SerializeNccModel(((MatchTool*)((MatchTool*)m_pTool))->m_hNCCModel, &SerializedItemHandle);
		HSerializedItem item(SerializedItemHandle);
		Hlong hSize;
		void *_pStart = item.GetSerializedItemPtr(&hSize);
		BYTE*_pData = new BYTE[hSize];
		memset(_pData, 0, hSize);
		memcpy_s(_pData, hSize, _pStart, hSize);

		HSerializedItem _item1(_pData, hSize, "true");
		DeserializeNccModel(_item1, &m_hNCCModel);
		delete[]_pData;
	}
	catch (...)	{	}

	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++) {
		m_VecSearchBaseItem[i]->setIndex(i);
		m_VecSearchBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
		m_VecBaseItem[i]->setIndex(i);
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}

	m_VecCreateCross = ((MatchTool*)((MatchTool*)m_pTool))->m_VecCreateCross;;

	//设置显示参数
	for (auto iter = ((MatchTool*)m_pTool)->m_mapShowParam.begin(); iter != ((MatchTool*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)			m_ShowidToProperty[iter.key()]->setValue(iter.value());


	return 0;
}

int frmMatchTool::Save()
{
	((MatchTool*)((MatchTool*)m_pTool))->m_vecCommonData.clear();
	ShapeToData(m_VecBaseItem, ((MatchTool*)((MatchTool*)m_pTool))->m_vecCommonData);

	((MatchTool*)((MatchTool*)m_pTool))->m_vecSearchData.clear();
	ShapeToData(m_VecSearchBaseItem, ((MatchTool*)((MatchTool*)m_pTool))->m_vecSearchData);

	((MatchTool*)m_pTool)->m_strInImg									= ui.cbx_ImgInPut->currentText();
	((MatchTool*)m_pTool)->m_mapParam.insert("spinRoiW",				QString::number(ui.spinRoiW->value()));
	//创建
	((MatchTool*)m_pTool)->m_mapParam.insert("cbx_MatchType",			QString::number(ui.cbx_MatchType->currentIndex()));
	((MatchTool*)((MatchTool*)m_pTool))->m_EMatchtype					= (EnumModeType)ui.cbx_MatchType->currentIndex();
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_LowContrast",		QString::number(ui.spBx_LowContrast->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_HighContrast",		QString::number(ui.spBx_HighContrast->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_MinSize",			QString::number(ui.spBx_MinSize->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_PyLevel",			QString::number(ui.spBx_PyLevel->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_CAngleStart",		QString::number(ui.spBx_CAngleStart->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_CAngleRange",		QString::number(ui.spBx_CAngleRange->value()));
	//查找
	((MatchTool*)m_pTool)->m_mapParam.insert("cbx_SubPixType",			QString::number(ui.cbx_SubPixType->currentIndex()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindMinScore",		QString::number(ui.spBx_FindMinScore->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindMinNumber",		QString::number(ui.spBx_FindMinNumber->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindGreedy",			QString::number(ui.spBx_FindGreedy->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindMinOverLap",		QString::number(ui.spBx_FindMinOverLap->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindAngleStart",		QString::number(ui.spBx_FindAngleStart->value()));
	((MatchTool*)m_pTool)->m_mapParam.insert("spBx_FindAngleRangle",	QString::number(ui.spBx_FindAngleRangle->value()));

	try { ClearShapeModel(((MatchTool*)((MatchTool*)m_pTool))->m_hShapeModel); } catch (...) { }
	try
	{
		HTuple SerializedItemHandle;
		SerializeShapeModel(m_hShapeModel, &SerializedItemHandle);
		HSerializedItem item(SerializedItemHandle);
		Hlong hSize;
		void *_pStart = item.GetSerializedItemPtr(&hSize);
		BYTE*_pData = new BYTE[hSize];
		memset(_pData, 0, hSize);
		memcpy_s(_pData, hSize, _pStart, hSize);

		HSerializedItem _item1(_pData, hSize, "true");
		DeserializeShapeModel(_item1, &((MatchTool*)m_pTool)->m_hShapeModel);
		delete[]_pData;
		//WriteShapeModel(((MatchTool*)m_pTool)->m_hShapeModel,"d:\\ShapeModel.shm");
	}
	catch (...) {	}

	try { ClearNccModel(((MatchTool*)((MatchTool*)m_pTool))->m_hNCCModel); }	catch (...) {}
	try	{
		HTuple SerializedItemHandle;
		SerializeNccModel(m_hNCCModel, &SerializedItemHandle);
		HSerializedItem item(SerializedItemHandle);
		Hlong hSize;
		void *_pStart = item.GetSerializedItemPtr(&hSize);
		BYTE*_pData = new BYTE[hSize];
		memset(_pData, 0, hSize);
		memcpy_s(_pData, hSize, _pStart, hSize);

		HSerializedItem _item1(_pData, hSize, "true");
		DeserializeNccModel(_item1, &((MatchTool*)m_pTool)->m_hNCCModel);
		delete[]_pData;
	}
	catch (...) {	}

	((MatchTool*)((MatchTool*)m_pTool))->m_VecCreateCross							= m_VecCreateCross;

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((MatchTool*)m_pTool)->m_mapShowParam[iter.key()] = iter.value()->value().toString();

	return 0;
}

void frmMatchTool::addProperty(QtVariantProperty * property, const QString & id)
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

void frmMatchTool::slot_valueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)		return;
	QString strName = property->propertyName();
	if (propertyToId.count(property) > 0) {
		QString _strName = propertyToId[property];
		if (idToProperty.count(propertyToId[property]) > 0) {
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++) {
				float fData[5] = { 0 };
				((BaseItem*)items[i])->SetParam(propertyToId[property], value.toString());
				((BaseItem*)items[i])->update();
			}
		}
	}
}

