#include "frmDeComposeTool.h"
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
#include <QDoubleSpinBox>
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmDeComposeTool::frmDeComposeTool(QWidget* parent)
	: frmBaseTool(parent){
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	//this->setWindowIcon(QIcon(":/QtApplication/Bitmaps/match.png"));
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/3DMeasure.png");
	connect(ui.cbx_ImgInPut, SIGNAL(activated(int)), this, SLOT(slot_ChangeMatchImg_Index(int)));
	//图像显示
	m_pView = new QGraphicsViews;
	connect(m_pView, &QGraphicsViews::sig_MouseMoveInImg, this, &frmDeComposeTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_pView);

	initShapeProperty();
}

frmDeComposeTool::~frmDeComposeTool()
{
	if (m_titleBar != nullptr)				delete m_titleBar;					m_titleBar	= nullptr;
	this->deleteLater();
}

void frmDeComposeTool::initShapeProperty()
{
	variantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	variantFactory = new QtVariantEditorFactory(this);
	propertyEditor = new QtTreePropertyBrowser(ui.widget);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.ROILayout->addWidget(propertyEditor);
}

void frmDeComposeTool::slot_valueChanged(QtProperty *property, const QVariant &value)
{

}

void frmDeComposeTool::slot_ChangeMatchImg_Index(int Index)
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;

	if (MiddleParam::MapMiddle().size() > ((DeComposeTool*)m_pTool)->m_iFlowIndex)	{
		if (MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count(ui.cbx_ImgInPut->currentText()) > 0)	{
			if( !( (HObject*)MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()] )->IsInitialized()  )	return;
			m_srcImage = ((HObject*)MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()])->Clone();
			if (!m_srcImage.IsInitialized())	return;
			m_dstImage = m_srcImage.Clone();
			QImage img(Hal2QImage(m_dstImage));
			m_pView->DispImage(img);

			ui.cbx_Camerachannal->clear();
			HTuple Width, Height, hType, hChannal;
			GetImageType(m_dstImage, &hType);
			CountChannels(m_dstImage, &hChannal);
			ui.cbx_Camerachannal->addItem("All");
			for (int i = 0; i < hChannal.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));

			QString strParent = ui.cbx_ImgInPut->currentText();
			_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");
			_ptrproperty->setPropertyName(strParent);
			_ptrproperty->setAttribute("Parent", strParent);

			GetImageSize(m_srcImage,&Width, &Height);
			_ptrSubproperty = variantManager->addProperty(QVariant::Int, "Width");
			_ptrSubproperty->setValue(QString::number(Width.D()));
			_ptrSubproperty->setAttribute("Parent", strParent);
			_ptrSubproperty->setEnabled(false);
			_ptrSubproperty->setPropertyName(	tr("Width")	);
			idToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
			propertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
			_ptrproperty->addSubProperty(_ptrSubproperty);

			_ptrSubproperty = variantManager->addProperty(QVariant::Int, "Height");
			_ptrSubproperty->setValue(QString::number(Height.D()));
			_ptrSubproperty->setAttribute("Parent", strParent);
			_ptrSubproperty->setEnabled(false);
			_ptrSubproperty->setPropertyName(tr("Height"));
			idToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
			propertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
			_ptrproperty->addSubProperty(_ptrSubproperty);

			GetImageType(m_srcImage,&hType);
			_ptrSubproperty = variantManager->addProperty(QVariant::String, "Type");
			_ptrSubproperty->setValue(QString(hType.S()));
			_ptrSubproperty->setAttribute("Parent", strParent);
			_ptrSubproperty->setEnabled(false);
			_ptrSubproperty->setPropertyName(tr("Type"));
			idToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
			propertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
			_ptrproperty->addSubProperty(_ptrSubproperty);

			CountChannels(m_srcImage, &hChannal);
			_ptrSubproperty = variantManager->addProperty(QVariant::Int, "Channel");
			_ptrSubproperty->setValue(QString::number(hChannal.D()));
			_ptrSubproperty->setAttribute("Parent", strParent);
			_ptrSubproperty->setEnabled(false);
			_ptrSubproperty->setPropertyName(tr("Channel"));
			idToProperty[strParent + _ptrSubproperty->propertyName()] = _ptrSubproperty;
			propertyToId[_ptrSubproperty] = strParent + _ptrSubproperty->propertyName();
			_ptrproperty->addSubProperty(_ptrSubproperty);

			addProperty(_ptrproperty, strParent);
		}
	}

}

EnumNodeResult frmDeComposeTool::Execute(MiddleParam& param, QString& strError) {
	if (!m_srcImage.IsInitialized()) { return NodeResult_ParamError; }

	return NodeResult_OK;
}

int frmDeComposeTool::ExecuteComplete()
{
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((DeComposeTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((DeComposeTool*)m_pTool)->GetItemId())) {
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((DeComposeTool*)m_pTool)->m_strInImg);
	if (MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid.count(((DeComposeTool*)m_pTool)->m_strInImg) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
		if (pImg.IsInitialized()) {
			m_srcImage = pImg.Clone();
			m_dstImage = m_srcImage.Clone();
			QImage img(Hal2QImage(m_dstImage));
			m_pView->DispImage(img);

			ui.cbx_Camerachannal->clear();
			HTuple	hType, hChannel;
			GetImageType(m_dstImage, &hType);
			CountChannels(m_dstImage, &hChannel);
			ui.cbx_Camerachannal->addItem("All");
			for (int i = 0; i < hChannel.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));
		}
	}
	return NodeResult_OK;
}

int frmDeComposeTool::Load()
{
	m_titleBar->setTitleContent(((DeComposeTool*)m_pTool)->GetItemId());

	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((DeComposeTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((DeComposeTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((DeComposeTool*)m_pTool)->GetItemId())){
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((DeComposeTool*)m_pTool)->m_strInImg);

	if (ui.cbx_ImgInPut->currentIndex() >= 0) {
		slot_ChangeMatchImg_Index(ui.cbx_ImgInPut->currentIndex());
	}
	return 0;
}

int frmDeComposeTool::Save()
{
	((DeComposeTool*)m_pTool)->m_strInImg = ui.cbx_ImgInPut->currentText();

	return 0;
}

void frmDeComposeTool::addProperty(QtVariantProperty * property, const QString & id)
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
