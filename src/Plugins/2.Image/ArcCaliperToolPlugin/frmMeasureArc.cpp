#include "frmMeasureArc.h"
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
#include "frmAllVariableTool.h"

frmMeasureArc::frmMeasureArc(QWidget* parent)
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

	//图像显示
	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmBaseTool::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);

	//初始化标题栏
	initTitleBar();
	initConnect();
	initShapeProperty();
	initResultProperty();
	initShowProperty();
	initData();

}

frmMeasureArc::~frmMeasureArc()
{
	if (m_view != nullptr)				delete m_view;						m_view = nullptr;

	if (variantFactory != nullptr)			delete variantFactory;				variantFactory = nullptr;
	if (variantManager != nullptr)			delete variantManager;				variantManager = nullptr;
	if (propertyEditor != nullptr)			delete propertyEditor;				propertyEditor = nullptr;

	if (m_ResultvariantFactory != nullptr)	delete m_ResultvariantFactory;		m_ResultvariantFactory = nullptr;
	if (m_ResultvariantManager != nullptr)	delete m_ResultvariantManager;		m_ResultvariantManager = nullptr;
	if (m_ResultpropertyEditor != nullptr)	delete m_ResultpropertyEditor;		m_ResultpropertyEditor = nullptr;
	this->deleteLater();
	this->deleteLater();
}

void frmMeasureArc::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/Contour.png");
}

void frmMeasureArc::initShapeProperty()
{
	variantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	variantFactory = new QtVariantEditorFactory(this);
	propertyEditor = new QtTreePropertyBrowser(ui.ROIwidget);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.ROILayout->addWidget(propertyEditor);

	idToProperty.clear();
	propertyToId.clear();
	m_VecSearchItem.clear();
}

void frmMeasureArc::initResultProperty()
{
	m_ResultvariantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(m_ResultvariantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(slot_valueChanged(QtProperty *, const QVariant &)));
	m_ResultvariantFactory = new QtVariantEditorFactory(this);
	m_ResultpropertyEditor = new QtTreePropertyBrowser(ui.widget_Result);
	m_ResultpropertyEditor->setFactoryForManager(m_ResultvariantManager, m_ResultvariantFactory);
	ui.ResultLayout->addWidget(m_ResultpropertyEditor);
}

void frmMeasureArc::initShowProperty()
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
		_ptrproperty->setPropertyName(tr("showregion"));
	} {
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showResult, tr("showResult"));
		_ptrproperty->setPropertyName(tr("showResult"));
	} {
		_ptrproperty = m_ShowvariantManager->addProperty(QtVariantPropertyManager::enumTypeId());
		QStringList enumNames;
		enumNames << tr("No") << tr("Yes");
		_ptrproperty->setAttribute(QLatin1String("enumNames"), enumNames);
		_ptrproperty->setValue(0);
		addShowProperty(_ptrproperty, showDrawResult, tr("showDrawResult"));
		_ptrproperty->setPropertyName(tr("showDrawResult"));
	}
}

void frmMeasureArc::initConnect()
{
	connect(ui.cbx_Camerachannal,	SIGNAL(activated(int)),								this, SLOT(onChannelClicked(int)));
	connect(ui.spinRoiW,			SIGNAL(valueChanged(int)),							this, SLOT(onRoiWidthChanged(int)));
	connect(ui.cbx_ImgInPut,		SIGNAL(activated(int)),								this, SLOT(slot_ChangeMatchImg_Index(int)));
	connect(m_view,					&QGraphicsViews::sig_MouseClicked,					this, &frmMeasureArc::slot_MouseClicked);
	connect(m_view,					&QGraphicsViews::sig_MouseMouve,					this, &frmMeasureArc::slot_MouseMouve);

	connect(ui.btnLinkContent,		&QPushButton::pressed,								this, &frmMeasureArc::on_btnLinkContent_clicked);
	connect(ui.btnDelLinkContent,	&QPushButton::pressed,								this, &frmMeasureArc::on_btnDelLinkContent_clicked);

	connect(ui.btnSetLine,			&QPushButton::pressed,								this, &frmMeasureArc::on_btnSetLine_clicked);
	connect(ui.btnDeleteSelect,		&QPushButton::pressed,								this, &frmMeasureArc::on_btnDeleteSelect_clicked);
	connect(ui.btnClearLine,		&QPushButton::pressed,								this, &frmMeasureArc::on_btnClearLine_clicked);
}

void frmMeasureArc::initData()
{
	ui.tabWidget->setCurrentIndex(0);
	m_iChanal = 0;
}

void frmMeasureArc::ClearShape()
{
	try { ClearShapeModel(m_Measurehandle); }	catch (...) {}
}

void frmMeasureArc::ClearROIProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

void frmMeasureArc::on_btnSetLine_clicked()
{
	try
	{
		HTuple width, height;
		GetImageSize(m_dstImage, &width, &height);
		float fWidth	= width.D();
		float fHeight	= height.D();
		float fData[10] = { 0 };
		fData[0] = fWidth / 2.0;
		fData[1] = fHeight / 2.0;
		fData[2] = MIN(fWidth, fHeight) / 5.0;
		fData[3] = MIN(fWidth, fHeight) / 5.0;
		fData[4] = 20;
		fData[5] = 20;
		//fData[4] = ui.sBx_CreateDetectHeight->value();
		//if (m_pCaliper == nullptr)
		//{
		ArcCaliperItem* m_pCaliper = new ArcCaliperItem(fData[0], fData[1], fData[2]);
		m_pCaliper->m_eRegionType = RegionType_Add;
		m_VecSearchItem.push_back(m_pCaliper);
		m_view->AddItems(m_pCaliper);
		//}
		ArcCaliperP			_Line;
		_Line.x				= fData[0];
		_Line.y				= fData[1];
		_Line.Radius		= fData[2];
		_Line.height		= fData[3];
		_Line.width			= 20;
		_Line.num			= 10;
		m_pCaliper->SetArcCaliper(_Line);

		for (auto iter : m_VecSearchItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	}
	catch (...)
	{	}
}

QVector<ArcCaliperP> frmMeasureArc::QJsonToShapeData(QJsonArray & RegionData)
{
	QVector<ArcCaliperP> _VecData;
	QVector<BaseItem*>	_VecSearchItem;
	QJsonToShape(RegionData, _VecSearchItem);
	for (auto iter : _VecSearchItem)
		if (iter != nullptr) {
			ArcCaliperP _data;
			((ArcCaliperItem*)iter)->GetArcCaliper(_data);
			_VecData.push_back(_data);
			delete iter;
		};
	_VecSearchItem.clear();

	return _VecData;
}

void frmMeasureArc::on_btnDeleteSelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->selectedItems();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr)
			if (iter->type() == 10){
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecSearchItem.size(); i++)	{
					if (m_VecSearchItem[i] == iter)	{
						_bFinded = true;
					}
				}
				if (_bFinded)	{
					(m_view->scene())->removeItem(iter);
					m_VecSearchItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}
	for (size_t i = 0; i < m_VecSearchItem.size(); i++)	m_VecSearchItem[i]->setIndex(i);
}

void frmMeasureArc::on_btnClearLine_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_view->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecSearchItem.size(); i++)
			{
				if (m_VecSearchItem[i] == iter)
				{
					_bFinded = true;
				}
			}
			if (_bFinded)
			{
				m_VecSearchItem.removeOne((BaseItem*)iter);
				(m_view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}
	for (size_t i = 0; i < m_VecSearchItem.size(); i++)	m_VecSearchItem[i]->setIndex(i);
}

void frmMeasureArc::on_btnLinkContent_clicked()
{
	frmAllVariableTool * frm = new frmAllVariableTool();
	frm->m_strModelIndex	= m_strModelIndex;
	frm->m_strTypeVariable	= m_strTypeVariable;
	frm->m_strValueName		= m_strValueName;
	frm->SetLimitModel(QString::number(((MeasureArc*)m_pTool)->m_iFlowIndex), EnumLimitType_Metrix);
	frm->Load();
	databaseVar::Instance().CenterMainWindow(this);
	frm->exec();
	frm->Save();

	if (frm->m_iRetn == 2)	{	//关联变量
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

void frmMeasureArc::on_btnDelLinkContent_clicked()
{
	ui.txtLinkFollow->clear();
	m_strModelIndex.clear();
	m_strTypeVariable.clear();
	m_strValueName.clear();
}

void frmMeasureArc::onChannelClicked(int Index)
{
	if (Index < 0)	return;
	try	{
		m_iChanal = Index;
		QImage img(Hal2QImage(m_dstImage));
		m_view->DispImage(img);
		m_view->update();
	}
	catch (...) {}
}

void frmMeasureArc::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecSearchItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	if (m_view != nullptr)	m_view->update();
}

EnumNodeResult frmMeasureArc::Execute(MiddleParam& param, QString& strError)
{
	std::vector<QPointF>	_VecPt1D;
	std::vector<sDrawText>	_VecText1D;
	std::vector<sDrawCross> _PointLst;
	std::vector<sColorLine> _LineLst;

	QVector<ArcCaliperP>	_vecLines ;
	for (int i = 0; i < m_VecSearchItem.size(); i++){
		ArcCaliperP _data;
		((ArcCaliperItem*)m_VecSearchItem[i])->GetArcCaliper(_data);
		_vecLines.push_back(_data);
	}
	QVector<ArcCaliperP>	_vecResultLines = _vecLines;

	if (ui.checkUseFollow->isChecked()) {
		HTuple	_Metrix;
		HTuple	Rows, Columns, _Lenth;
		HObject _MetrixContour;
		if (MeasureArc::CheckMetrixType(ui.txtLinkFollow->text(), _Metrix) == 0) {
			_vecResultLines.clear();
			for (auto& _Lines : _vecLines) {
				ArcCaliperP _line = _Lines;
				HTuple	_Rows, _Columns;
				AffineTransPixel(_Metrix, _Lines.y, _Lines.x, &_Rows, &_Columns);
				_line.y = _Rows.D();
				_line.x = _Columns.D();
				_vecResultLines.push_back(_line);
			}
		}
		else {
			sDrawText	_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr(" The Link Matrix Failed!");
			_VecText1D.push_back(_strText);

			param.VecShowQPointFs = (_VecPt1D);
			param.VecDrawText = (_VecText1D);
			for (int i = 0; i < _PointLst.size(); i++) {
				param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
			}
			return NodeResult_ParamError;
		}
	}
	int _iCheckMode = databaseVar::Instance().m_iCheckMode;
	HObject ho_Regions, ho_Circle;
	HTuple hv_ResultRow, hv_ResultColumn;
	HTuple  hv_ArcType, hv_RowCenter, hv_ColCenter, hv_Radius;
	HTuple  hv_StartPhi, hv_EndPhi, hv_PointOrder, hv_ArcAngle;
	for (auto& _Lines : _vecResultLines) {
		MeasureArc::Spoke(m_srcImage,&ho_Regions, _Lines.num, _Lines.height, _Lines.width,
			_Lines.sSigma, _Lines.sThreshold, _Lines.sTransition.toStdString().c_str(),
			_Lines.sSelect.toStdString().c_str(), _Lines.y, _Lines.x, _Lines.Radius, 
			_Lines.dStartAngle, _Lines.dEndAngle,&hv_ResultRow, &hv_ResultColumn);
		if (m_ShowidToProperty.count(showDrawResult) > 0) {
			if (m_ShowidToProperty[showDrawResult]->value().toInt() > 0) {
				for (int i = 0; i < hv_ResultRow.TupleLength().I(); i++) {
					sDrawCross _Cross;
					_Cross.Center.setX(hv_ResultColumn[i].D());
					_Cross.Center.setY(hv_ResultRow[i].D());
					_Cross.DrawColor = QColor(0, 0, 255);
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_PointLst.push_back(_Cross);
				}
			}
		}
		MeasureArc::pts_to_best_circle(&ho_Circle, hv_ResultRow, hv_ResultColumn, _Lines.sActiveNum, "circle",
			&hv_RowCenter, &hv_ColCenter, &hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder,
			&hv_ArcAngle);
		if (hv_RowCenter.Length() > 0)		{

			sDrawText _strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(0, 255, 0);
			_strText.strText = tr("Measure OK ");
			_VecText1D.push_back(_strText);

			if (m_ShowidToProperty.count(showregion) > 0) {
				if (m_ShowidToProperty[showregion]->value().toInt() > 0) {
					QPointF dStartPos;
					QPointF dEndPos;
					QPointF Center;
					QPointF MeasureCenter;
					double angle = _Lines.dStartAngle;
					double	dAngleRang = (_Lines.dEndAngle - _Lines.dStartAngle);
					if (dAngleRang > 360) {
						dAngleRang = (_Lines.dEndAngle - _Lines.dStartAngle) - ((int)(_Lines.dEndAngle - _Lines.dStartAngle) / 360) * 360;
					}
					Center.setX(_Lines.x);
					Center.setY(_Lines.y);
					if (dAngleRang >= 360) {
						double dAngleStep = dAngleRang / (double)_Lines.num;
						for (int i = 0; i < _Lines.num; i++) {
							angle = _Lines.dStartAngle + (dAngleStep * i);
							dStartPos	= MeasureArc::GetAnglePos(Center, _Lines.Radius - _Lines.height, angle);
							dEndPos		= MeasureArc::GetAnglePos(Center, _Lines.Radius + _Lines.height, angle);
							sColorLineRect _LineRect;
							_LineRect.Dline.setP1(dStartPos);
							_LineRect.Dline.setP2(dEndPos);
							_LineRect.DWidth = _Lines.width * 2;
							_LineRect.DrawQColor = QColor(0, 0, 255);;
							_LineRect.LineWidth = -1;
							param.VecShowColorLineRect.push_back(_LineRect);
						}
					}
					else	{
						double dAngleStep = dAngleRang / (_Lines.num - 1);
						for (int i = 0; i < _Lines.num; i++) {
							angle = _Lines.dStartAngle + (dAngleStep * i);
							dStartPos = MeasureArc::GetAnglePos(Center, _Lines.Radius - _Lines.height, angle);
							dEndPos = MeasureArc::GetAnglePos(Center, _Lines.Radius + _Lines.height, angle);
							sColorLineRect _LineRect;
							_LineRect.Dline.setP1(dStartPos);
							_LineRect.Dline.setP2(dEndPos);
							_LineRect.DWidth = _Lines.width * 2;
							_LineRect.DrawQColor = QColor(0, 0, 255);;
							_LineRect.LineWidth = -1;
							param.VecShowColorLineRect.push_back(_LineRect);
						}
					}

					sDrawCross _Cross;
					_Cross.Center.setX(hv_ColCenter.D());
					_Cross.Center.setY(hv_RowCenter.D());
					_Cross.DrawColor = QColor(0, 255, 0);
					if (m_ShowidToProperty.count(FontSize) > 0)	_Cross.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_PointLst.push_back(_Cross);

					sColorCircle _circle;
					_circle.CenterPoints.setX(hv_ColCenter.D());
					_circle.CenterPoints.setY(hv_RowCenter.D());
					_circle.qRadius = hv_Radius.D();
					_circle.DrawQColor = QColor(255, 0, 0);
					_circle.LineWidth = -1;
					param.VecShowColorCircle.push_back(_circle);
				}
			}

			if (m_ShowidToProperty.count(showResult) > 0) {
				if (m_ShowidToProperty[showResult]->value().toInt() > 0) {
					sDrawText _strText;
					_strText.bControl = true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = tr("Measure Result ") +
						QString("X:%1,Y:%2,R:%3")
						.arg(QString::number(hv_ColCenter.D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(hv_RowCenter.D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(hv_Radius.D(), 'f', databaseVar::Instance().form_System_Precision))
						;
					_VecText1D.push_back(_strText);
				}
			}

		}
		else  {
			sDrawText _strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("Measure Arc NG");
			_VecText1D.push_back(_strText);
		}
	}
	param.VecShowQPointFs	= _VecPt1D;
	param.VecDrawText		= _VecText1D;
	param.VecShowColorLine	= _LineLst;
	for (int i = 0; i < _PointLst.size(); i++) {
		param.MapDrawCross.insert(std::pair<QString, sDrawCross>(QString::number(i), _PointLst[i]));
	}

	return NodeResult_OK;
}

int frmMeasureArc::Load()
{
	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((MeasureArc*)m_pTool)->m_iFlowIndex)	{
		for (auto iter : MiddleParam::MapMiddle()[((MeasureArc*)m_pTool)->m_iFlowIndex].MapPImgVoid)	//图像
			ui.cbx_ImgInPut->addItem(iter.first);
	}
	ui.cbx_ImgInPut->setCurrentText(((MeasureArc*)m_pTool)->m_strImg);
	if (((MeasureArc*)m_pTool)->m_mapParam.count("spinRoiW") > 0) ui.spinRoiW->setValue(((MeasureArc*)m_pTool)->m_mapParam["spinRoiW"].toInt());
	ui.txtLinkFollow->setText(((MeasureArc*)m_pTool)->m_strSendContent);

	if (((MeasureArc*)m_pTool)->m_mapParam.contains("checkUseFollow"))			ui.checkUseFollow->setChecked(((MeasureArc*)m_pTool)->m_mapParam["checkUseFollow"].toInt());
	if (((MeasureArc*)m_pTool)->m_mapParam.contains("txtLinkFollow"))			ui.txtLinkFollow->setText(((MeasureArc*)m_pTool)->m_mapParam["txtLinkFollow"]);

	slot_ChangeMatchImg_Index(0);

	on_btnClearLine_clicked();
	QJsonToShape(((MeasureArc*)m_pTool)->m_vecROIData, m_VecSearchItem);
	for (auto pItem : m_VecSearchItem)	m_view->AddItems(pItem);
	for (auto iter : m_VecSearchItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());

	//设置显示参数
	for (auto iter = ((MeasureArc*)m_pTool)->m_mapShowParam.begin(); iter != ((MeasureArc*)m_pTool)->m_mapShowParam.end(); iter++)
		if (m_ShowidToProperty.count(iter.key()) > 0)	m_ShowidToProperty[iter.key()]->setValue(iter.value());

	return 0;
}

int frmMeasureArc::Save()
{
	((MeasureArc*)m_pTool)->m_strImg								= ui.cbx_ImgInPut->currentText();
	((MeasureArc*)m_pTool)->m_strSendContent						= ui.txtLinkFollow->text();
	((MeasureArc*)m_pTool)->m_mapParam.insert("spinRoiW",			QString::number(ui.spinRoiW->value()));
	((MeasureArc*)m_pTool)->m_mapParam.insert("checkUseFollow",	QString::number(ui.checkUseFollow->isChecked()));
	((MeasureArc*)m_pTool)->m_mapParam.insert("txtLinkFollow",		QString(ui.txtLinkFollow->text()));

	((MeasureArc*)m_pTool)->m_vecROIData			= QJsonArray();
	ShapeToQJson(m_VecSearchItem, ((MeasureArc*)m_pTool)->m_vecROIData);

	//显示属性
	for (auto iter = m_ShowidToProperty.begin(); iter != m_ShowidToProperty.end(); iter++)
		((MeasureArc*)m_pTool)->m_mapShowParam[iter.key()] = iter.value()->value().toString();

	return 0;
}

void frmMeasureArc::slot_valueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)	return;
	if (!m_bValueChange)			return;

	QString strName = property->propertyName();
	if (propertyToId.count(property) > 0) {
		QString _strName = propertyToId[property];
		if (idToProperty.count(propertyToId[property]) > 0) {
			QList<QGraphicsItem *> items = m_view->scene()->selectedItems();
			for (int i = 0; i < items.size(); i++) {
				bool bFind = false;
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
				case LineCaliperObj: {
					LineCaliperItem *pValueItem = (LineCaliperItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				case ArcCaliperObj: {
					ArcCaliperItem *pValueItem = (ArcCaliperItem*)items[i];
					pValueItem->SetParam(propertyToId[property], value.toString());
					pValueItem->update();
				}break;
				default: {

				}break;
				}
			}
		}
	}

}

void frmMeasureArc::slot_ChangeMatchImg_Index(int)
{
	m_dstImage.Clear();
	if (MiddleParam::MapMiddle()[((MeasureArc*)m_pTool)->m_iFlowIndex].MapPImgVoid.count(ui.cbx_ImgInPut->currentText()) > 0) {
		HObject& pImg = *((HObject*)MiddleParam::MapMiddle()[((MeasureArc*)m_pTool)->m_iFlowIndex].MapPImgVoid[ui.cbx_ImgInPut->currentText()]);
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

void frmMeasureArc::slot_MouseMouve(QList<QGraphicsItem *>& items)
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
					if (ValueToParam(	(*pIter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
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

void frmMeasureArc::slot_MouseClicked(QList<QGraphicsItem *>& items)
{
	ClearROIProperty();
	m_bValueChange = false;

	QtVariantProperty *_ptrproperty = nullptr;
	QtVariantProperty *_ptrSubproperty = nullptr;
	QtVariantProperty *_ptrSubSubproperty = nullptr;

	for (int i = 0; i < items.size(); i++) {
		bool bFind = false;
		QVector<QPair<QString, QVector<QPair<QString, QString>>>>	_mapValue;
		_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "");
		float fData[5] = { 0 };
		switch (((BaseItem*)items[i])->GetItemType()) {
		case ItemPoint: {
			PointItem *pValueItem = (PointItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemCircle: {
			CircleItem *pValueItem = (CircleItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemEllipse: {
			EllipseItem *pValueItem = (EllipseItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *pValueItem = (ConcentricCircleItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemRectangle: {
			RectangleItem *pValueItem = (RectangleItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemRectangleR: {
			RectangleRItem *pValueItem = (RectangleRItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemSquare: {
			QString strParent = QString::number(i) + ".方形";
			_ptrproperty->setPropertyName(strParent);
		}break;
		case ItemPolygon: {
			PolygonItem *pValueItem = (PolygonItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemLineObj: {
			LineItem *pValueItem = (LineItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemArrowLineObj: {
			LineArrowItem *pValueItem = (LineArrowItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ItemBezierLineObj: {
			BezierLineItem *pValueItem = (BezierLineItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case LineCaliperObj: {
			LineCaliperItem *pValueItem = (LineCaliperItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		case ArcCaliperObj: {
			ArcCaliperItem *pValueItem = (ArcCaliperItem*)items[i];
			if (pValueItem != nullptr)	pValueItem->GetAllParam(_mapValue);
		}break;
		default: {

		}break;
		}
		items[i]->setSelected(true);
		for (auto mapIter = _mapValue.begin(); mapIter != _mapValue.end(); mapIter++) {
			if (	((*mapIter).first).count(cstItem) > 0) {
				_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), (*mapIter).first);
				for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
					int				iEnable;
					EnumParamType	iType;
					QString			Param;
					QStringList		strEnumLst;
					QString			LowParam;
					QString			HighParam;
					QString			strLang;
					if (ValueToParam((*pIter).second,strLang,iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
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

				addProperty(_ptrproperty, ((*mapIter).first)	);
				_ptrproperty->setPropertyName(BaseItem::tr(cstItem.toStdString().c_str()));
			}
			else
			{
				_ptrSubproperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), ((*mapIter).first)	);
				for (auto pIter = (*mapIter).second.begin(); pIter != (*mapIter).second.end(); pIter++) {
					int				iEnable;
					EnumParamType	iType;
					QString			Param;
					QStringList		strEnumLst;
					QString			LowParam;
					QString			HighParam;
					QString			strLang;
					if (ValueToParam((*pIter).second, strLang,iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
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
				addProperty(_ptrSubproperty, (*mapIter).first );
			}
		}
	}

	m_bValueChange = true;
}

void frmMeasureArc::addProperty(QtVariantProperty * property, const QString & id)
{
	propertyToId[property] = id;
	idToProperty[id] = property;
	QtBrowserItem *item = propertyEditor->addProperty(property);
}

void frmMeasureArc::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
}
