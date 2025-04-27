#include "frmBaseTool.h"
#include "ui_frmBaseTool.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QColorDialog>
#include "ImageShow/ImageItem.h"
#include "ImageShow/QGraphicsScenes.h"
#include <fstream>
#include "frmAllVariableTool.h"

#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"
#include "MyEvent.h"
#include "databaseVar.h"

frmBaseTool::frmBaseTool(QWidget* parent)
	: BaseWindow(/*parent*/)
	,ui(new Ui::frmBaseToolClass())
	, m_pTool(nullptr)
{
	ui->setupUi(this);
	//FramelessWindowHint属性设置窗口去除边框
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);

	initTitleBar();
}

frmBaseTool::~frmBaseTool()
{
	if (m_titleBar != nullptr)	delete m_titleBar;		m_titleBar = nullptr;
	this->deleteLater();
	if (ui != nullptr)			delete ui;				ui = nullptr;
}

QWidget * frmBaseTool::GetCenterWidget()
{
	return ui->widgetTool;
}

void frmBaseTool::initTitleBar()
{
	m_titleBar->move(0, 0);
	connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
	m_titleBar->setTitleIcon(":/image/icon/images/QApplication.png");
	m_titleBar->setButtonType(MIN_MAX_BUTTON);
	m_titleBar->setTitleWidth(this->width());
}

bool frmBaseTool::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType) {
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent->iIndex == 0xFFFF && MyEvent->isubIndex == 0xFFFF)	{
			if (m_view != nullptr)	{
				m_view->UpdateImg();
			}
		}
	}
	return BaseWindow::event(ev);
}

void frmBaseTool::resizeEvent(QResizeEvent * ev)
{
	BaseWindow::resizeEvent(ev);
	if (m_view != nullptr) {
		m_view->UpdateImg();
		m_view->update();
		QTimer::singleShot(100, this, [this]() {
			m_view->UpdateImg();
			m_view->update();
		});
	}
}

void frmBaseTool::onButtonCloseClicked()
{
	this->close();
}

void frmBaseTool::scale_image_range(HObject ho_Image, HObject * ho_ImageScaled)
{
	HObject ho_Rectangle, ho_ImageScale;
	HTuple  hv_Width, hv_Height, hv_Min, hv_Max, _Range;
	GetImageSize(ho_Image, &hv_Width, &hv_Height);
	GenRectangle1(&ho_Rectangle, 0, 0, hv_Height, hv_Width);
	MinMaxGray(ho_Rectangle, ho_Image, 0, &hv_Min, &hv_Max, &_Range);

	// Local iconic variables
	HObject  ho_ImageSelected, ho_SelectedChannel;
	HObject  ho_LowerRegion, ho_UpperRegion, ho_ImageSelectedScaled;

	// Local control variables
	HTuple  hv_LowerLimit, hv_UpperLimit, hv_Mult;
	HTuple  hv_Add, hv_NumImages, hv_ImageIndex, hv_Channels;
	HTuple  hv_ChannelIndex, hv_MinGray, hv_MaxGray, hv_Range;

	if (0 != ((hv_Min.TupleLength()) == 2))
	{
		hv_LowerLimit = ((const HTuple&)hv_Min)[1];
		hv_Min = ((const HTuple&)hv_Min)[0];
	}
	else
	{
		hv_LowerLimit = 0.0;
	}
	if (0 != ((hv_Max.TupleLength()) == 2))
	{
		hv_UpperLimit = ((const HTuple&)hv_Max)[1];
		hv_Max = ((const HTuple&)hv_Max)[0];
	}
	else
	{
		hv_UpperLimit = 255.0;
	}
	//
	//Calculate scaling parameters.
	hv_Mult = ((hv_UpperLimit - hv_LowerLimit).TupleReal()) / (hv_Max - hv_Min);
	hv_Add = ((-hv_Mult)*hv_Min) + hv_LowerLimit;
	//
	//Scale image.
	ScaleImage(ho_Image, &ho_Image, hv_Mult, hv_Add);
	//
	//Clip gray values if necessary.
	//This must be done for each image and channel separately.
	GenEmptyObj(&ho_ImageScale);
	CountObj(ho_Image, &hv_NumImages);
	{
		HTuple end_val49 = hv_NumImages;
		HTuple step_val49 = 1;
		for (hv_ImageIndex = 1; hv_ImageIndex.Continue(end_val49, step_val49); hv_ImageIndex += step_val49)
		{
			SelectObj(ho_Image, &ho_ImageSelected, hv_ImageIndex);
			CountChannels(ho_ImageSelected, &hv_Channels);
			{
				HTuple end_val52 = hv_Channels;
				HTuple step_val52 = 1;
				for (hv_ChannelIndex = 1; hv_ChannelIndex.Continue(end_val52, step_val52); hv_ChannelIndex += step_val52)
				{
					AccessChannel(ho_ImageSelected, &ho_SelectedChannel, hv_ChannelIndex);
					MinMaxGray(ho_SelectedChannel, ho_SelectedChannel, 0, &hv_MinGray, &hv_MaxGray,
						&hv_Range);
					Threshold(ho_SelectedChannel, &ho_LowerRegion, (hv_MinGray.TupleConcat(hv_LowerLimit)).TupleMin(),
						hv_LowerLimit);
					Threshold(ho_SelectedChannel, &ho_UpperRegion, hv_UpperLimit, (hv_UpperLimit.TupleConcat(hv_MaxGray)).TupleMax());
					PaintRegion(ho_LowerRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_LowerLimit,
						"fill");
					PaintRegion(ho_UpperRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_UpperLimit,
						"fill");
					if (0 != (hv_ChannelIndex == 1))
					{
						CopyObj(ho_SelectedChannel, &ho_ImageSelectedScaled, 1, 1);
					}
					else
					{
						AppendChannel(ho_ImageSelectedScaled, ho_SelectedChannel, &ho_ImageSelectedScaled
						);
					}
				}
			}
			ConcatObj(ho_ImageScale, ho_ImageSelectedScaled, &ho_ImageScale);
		}
	}
	HalconCpp::ConvertImageType(ho_ImageScale, &(*ho_ImageScaled), "byte");

}

int frmBaseTool::ChangeTableToData(QTableWidget * pTable, QJsonArray & strData)
{
	for (int iRow = 0; iRow < pTable->rowCount(); iRow++) {
		QJsonArray ConFig;
		for (int iCol = 0; iCol < pTable->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = pTable->item(iRow, iCol);
			QWidget* _pQWidget = pTable->cellWidget(iRow, iCol);
			QJsonObject _data;
			if (currentItem != nullptr) {
				_data.insert("QTableWidgetItem", currentItem->text());
			}
			else if (_pQWidget != nullptr) {
				QString	strClass = _pQWidget->metaObject()->className();
				if (strClass == "QDoubleSpinBox") {
					_data.insert(strClass, QString::number(((QDoubleSpinBox*)_pQWidget)->value()));
					_data.insert("minimum", QString::number(((QDoubleSpinBox*)_pQWidget)->minimum()));
					_data.insert("maximum", QString::number(((QDoubleSpinBox*)_pQWidget)->maximum()));
				}
				else if (strClass == "QSpinBox") {
					_data.insert(strClass, QString::number(((QSpinBox*)_pQWidget)->value()));
					_data.insert("minimum", QString::number(((QSpinBox*)_pQWidget)->minimum()));
					_data.insert("maximum", QString::number(((QSpinBox*)_pQWidget)->maximum()));
				}
				else if (strClass == "QLabel") {
					_data.insert(strClass, QString(((QLabel*)_pQWidget)->text()));
				}
				else if (strClass == "QCheckBox") {
					_data.insert(strClass, QString::number(((QCheckBox*)_pQWidget)->isChecked()));
				}
				else if (strClass == "QComboBox") {
					QJsonArray ConFigLst;
					for (int i = 0; i < ((QComboBox*)_pQWidget)->count(); ++i) {
						auto itemText = ((QComboBox*)_pQWidget)->itemText(i);
						ConFigLst.append(itemText);
					}
					_data.insert("Lst", ConFigLst);
					_data.insert(strClass, QString::number(((QComboBox*)_pQWidget)->currentIndex()));
				}
			}
			ConFig.append(_data);
		}
		strData.append(ConFig);
	}
	return 0;
}

int frmBaseTool::ChangeDataToTable(QJsonArray & strData, QTableWidget * pTable)
{
	int iRowCount = pTable->rowCount();
	int iColCount = pTable->columnCount();
	for (int i = 0; i < iRowCount; i++)		pTable->removeRow(0);
	for (int iRow = 0; iRow < strData.count(); iRow++) {
		iRowCount = pTable->rowCount();
		pTable->setRowCount(iRowCount + 1);
		QJsonArray CParam = strData.at(iRow).toArray();
		for (int iCol = 0; iCol < pTable->columnCount(); iCol++) {
			QJsonObject _Param = CParam.at(iCol).toObject();
			QWidget* _pQWidget = nullptr;
			QTableWidgetItem* _pItem = nullptr;
			if (_Param.contains("QTableWidgetItem")) {
				_pItem = new QTableWidgetItem();
				_pItem->setTextAlignment(Qt::AlignCenter);
				_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
				_pItem->setText(_Param["QTableWidgetItem"].toString());
				_pItem->setToolTip(_Param["QTableWidgetItem"].toString());
			}
			else if (_Param.contains("QDoubleSpinBox")) {
				_pQWidget = new QDoubleSpinBox();
				((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				if (_Param.contains("minimum")) {
					((QDoubleSpinBox*)_pQWidget)->setMinimum(_Param["minimum"].toString().toDouble());
				}
				if (_Param.contains("maximum")) {
					((QDoubleSpinBox*)_pQWidget)->setMaximum(_Param["maximum"].toString().toDouble());
				}
				((QDoubleSpinBox*)_pQWidget)->setValue(_Param["QDoubleSpinBox"].toString().toDouble());
				Sleep(1);
			}
			else if (_Param.contains("QSpinBox")) {
				_pQWidget = new QSpinBox();
				((QSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
				if (_Param.contains("minimum")) {
					((QSpinBox*)_pQWidget)->setMinimum(_Param["minimum"].toString().toDouble());
				}
				if (_Param.contains("maximum")) {
					((QSpinBox*)_pQWidget)->setMaximum(_Param["maximum"].toString().toDouble());
				}
				((QSpinBox*)_pQWidget)->setValue(_Param["QSpinBox"].toString().toDouble());
			}
			else if (_Param.contains("QCheckBox")) {
				_pQWidget = new QCheckBox();
				((QCheckBox*)_pQWidget)->setCheckable(true);
				((QCheckBox*)_pQWidget)->setChecked(_Param["QCheckBox"].toString().toDouble());
				((QCheckBox*)_pQWidget)->setFocusPolicy(Qt::NoFocus);
			}
			else if (_Param.contains("QComboBox")) {
				_pQWidget = new QComboBox();
				((QComboBox*)_pQWidget)->clear();
				QStringList items;
				if (_Param.contains("Lst")) {
					QJsonArray _lst = _Param["Lst"].toArray();
					for (int _iI = 0; _iI < _lst.size(); _iI++) {
						items.append(_lst.at(_iI).toString());
					}
				}
				((QComboBox*)_pQWidget)->addItems(items);
				((QComboBox*)_pQWidget)->setCurrentIndex(_Param["QComboBox"].toString().toDouble());
			}
			if (_pItem != nullptr) {
				_pItem->setTextAlignment(Qt::AlignCenter);
				pTable->setItem(iRow, iCol, _pItem);
			}
			if (_pQWidget != nullptr) {
				//emit sigSetQTableWidget(iRow, iCol, _pQWidget, pTable);
				pTable->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	return 0;
}

int frmBaseTool::ShapeToRegion(BaseItem* iter, HObject & Regions)
{
	if (iter == nullptr)	return -1;
	BaseItem* Item = (BaseItem*)iter;
	HObject	_Region;	//ROI形状区域
	if (iter != nullptr)
		if (iter->type() == 10) {
			switch (Item->GetItemType()) {
			case ItemPoint: {
				PointItem *_pItem = (PointItem*)Item;
				MCircle _data;
				_pItem->GetCircle(_data);
				switch (_pItem->m_showShape) {
				default:
				case ItemPoint: {	GenCircle(&_Region, _data.row, _data.col, _data.radius);	}	break;
				case ItemRectangle:
				case ItemRectangleR:
				case ItemSquare: {
					GenRectangle1(&_Region, _data.row - _data.radius, _data.col - _data.radius, _data.row + _data.radius, _data.col + _data.radius);
				}break;
				}
			}	break;
			case ItemCircle: {
				CircleItem *_pItem = (CircleItem*)Item;
				MCircle _data;
				_pItem->GetCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.radius);
			}	break;
			case ItemEllipse: {
				EllipseItem *_pItem = (EllipseItem*)Item;
				MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				GenEllipse(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_pItem = (ConcentricCircleItem*)Item;
				CCircle _data;
				_pItem->GetConcentricCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.big_radius);
				HObject	_Region1;	//ROI形状区域
				GenCircle(&_Region1, _data.row, _data.col, _data.small_radius);
				Difference(_Region, _Region1, &_Region);
			}	break;
			case ItemRectangle: {
				RectangleItem *_pItem = (RectangleItem*)Item;
				MRectangle _data;
				_pItem->GetRect(_data);
				GenRectangle1(&_Region, _data.row - _data.height / 2.0, _data.col - _data.width / 2.0, _data.row + _data.height / 2.0, _data.col + _data.width / 2.0);
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_pItem = (RectangleRItem*)Item;
				MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				GenRectangle2(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemPolygon: {
				PolygonItem *_pItem = (PolygonItem*)Item;
				MPolygon Pointlst;
				_pItem->GetPolygon(Pointlst);
				HTuple hRow, hColumns;
				for (int i = 0; i < Pointlst.points.size(); i++) {
					hRow.Append(Pointlst.points[i].y());
					hColumns.Append(Pointlst.points[i].x());
				}
				GenRegionPolygonFilled(&_Region, hRow, hColumns);
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *_pItem = (BezierLineItem*)Item;
				//MPolygon Pointlst;
				QList<QPointF> points;
				QList<QLineF> lines = _pItem->GetDrawLines(points);
				//_pItem->GetPolygon(Pointlst);
				_Region.GenEmptyObj();
				HTuple hRow, hColumns;
				for (int i = 0; i < lines.size(); i++) {
					HObject	_RegionLine;					//ROI形状区域
					GenRegionLine(&_RegionLine, lines[i].p1().y(), lines[i].p1().x(), lines[i].p2().y(), lines[i].p2().x());
					ConcatObj(_Region, _RegionLine, &_Region);
				}
			}	break;
			case ItemLineObj: {
				LineItem *_pItem = (LineItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *_pItem = (LineArrowItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case LineCaliperObj: {
				LineCaliperItem *_pItem = (LineCaliperItem*)Item;
				LineCaliperP _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.y1, _data.x1, _data.y2, _data.x2);
			}	break;
			}
			Regions = _Region.Clone();
		}
	return 0;
}

int frmBaseTool::ShapeToRegion(QVector<BaseItem*>& vecItems, HObject & Regions)
{
	Regions.Clear();
	Regions.GenEmptyObj();
	for (auto iter : vecItems) {
		BaseItem* Item = (BaseItem*)iter;
		HObject	_Region;	//ROI形状区域
		if (iter != nullptr) if (iter->type() == 10) {
			switch (Item->GetItemType()) {
			case ItemPoint: {
				PointItem *_pItem = (PointItem*)Item;
				MCircle _data;
				_pItem->GetCircle(_data);
				switch (_pItem->m_showShape) {
				default:
				case ItemPoint: {	GenCircle(&_Region, _data.row, _data.col, _data.radius);	}	break;
				case ItemRectangle:
				case ItemRectangleR:
				case ItemSquare: { GenRectangle1(&_Region, _data.row - _data.radius, _data.col - _data.radius, _data.row + _data.radius, _data.col + _data.radius); }
								 break;
				}
			}	break;
			case ItemCircle: {
				CircleItem *_pItem = (CircleItem*)Item;							MCircle _data;
				_pItem->GetCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.radius);
			}	break;
			case ItemEllipse: {
				EllipseItem *_pItem = (EllipseItem*)Item;						MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				GenEllipse(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_pItem = (ConcentricCircleItem*)Item;		CCircle _data;
				_pItem->GetConcentricCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.big_radius);
				HObject	_Region1;	//ROI形状区域
				GenCircle(&_Region1, _data.row, _data.col, _data.small_radius);
				Difference(_Region, _Region1, &_Region);
			}	break;
			case ItemRectangle: {
				RectangleItem *_pItem = (RectangleItem*)Item;					MRectangle _data;
				_pItem->GetRect(_data);
				GenRectangle1(&_Region, _data.row - _data.height / 2.0, _data.col - _data.width / 2.0, _data.row + _data.height / 2.0, _data.col + _data.width / 2.0);
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_pItem = (RectangleRItem*)Item;					MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				GenRectangle2(&_Region, _data.row, _data.col, HTuple(_data.phi).TupleRad(), _data.lenth1, _data.lenth2);
			}	break;
			case ItemPolygon: {
				PolygonItem *_pItem = (PolygonItem*)Item;						MPolygon Pointlst;
				_pItem->GetPolygon(Pointlst);
				HTuple hRow, hColumns;
				for (int i = 0; i < Pointlst.points.size(); i++) {
					hRow.Append(Pointlst.points[i].y());
					hColumns.Append(Pointlst.points[i].x());
				}
				GenRegionPolygonFilled(&_Region, hRow, hColumns);
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *_pItem = (BezierLineItem*)Item;
				QList<QPointF> points;
				QList<QLineF> lines = _pItem->GetDrawLines(points);
				_Region.GenEmptyObj();
				HTuple hRow, hColumns;
				for (int i = 0; i < lines.size(); i++) {
					HObject	_RegionLine;					//ROI形状区域
					GenRegionLine(&_RegionLine, lines[i].p1().y(), lines[i].p1().x(), lines[i].p2().y(), lines[i].p2().x());
					ConcatObj(_Region, _RegionLine, &_Region);
				}
			}	break;
			case ItemLineObj: {
				LineItem *_pItem = (LineItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *_pItem = (LineArrowItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case LineCaliperObj: {
				LineCaliperItem *_pItem = (LineCaliperItem*)Item;
				LineCaliperP _data;
				_pItem->GetLineData(_data);
				GenRegionLine(&_Region, _data.y1, _data.x1, _data.y2, _data.x2);
			}	break;
			case ArcCaliperObj: {
				ArcCaliperItem *_pItem = (ArcCaliperItem*)Item;
				ArcCaliperP _data;
				_pItem->GetArcCaliper(_data);
			}	break;
			}
			switch (Item->m_eRegionType) {
			case RegionType_Add: {
				HTuple _hNumber;
				CountObj(Regions, &_hNumber);
				bool _bEmpty = true;
				if (_hNumber.I() > 0) {
					HTuple _hArea, _hRow, _hColumn;
					AreaCenter(Regions, &_hArea, &_hRow, &_hColumn);
					if (_hArea.I() > 0) {
						_bEmpty = false;
					}
				}
				if (_bEmpty) {
					Regions = _Region;
				}
				else {
					//ConcatObj(Regions, _Region, &Regions);
					Union2(Regions, _Region, &Regions);
				}
			} break;
			case RegionType_Differe: {
				Difference(Regions, _Region, &Regions);
			} break;
			}
		}
	}
	return 0;
}

int frmBaseTool::ShapeToData(QVector<BaseItem*>& vecItems, QVector<CommonData> &RegionData)
{
	RegionData.clear();
	for (auto iter : vecItems) {
		BaseItem* Item = (BaseItem*)iter;
		HObject	_Region;	//ROI形状区域
		if (iter != nullptr) if (iter->type() == 10) {
			CommonData _data;
			_data.type = Item->GetItemType();
			switch (Item->GetItemType()) {
			case ItemPoint: {
				PointItem *Circle = (PointItem*)Item;
				MCircle Cir;
				Circle->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius)).arg(QString::number(Circle->m_showShape));
			}	break;
			case ItemCircle: {
				CircleItem *Circle = (CircleItem*)Item;
				MCircle Cir;
				Circle->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
			}	break;
			case ItemEllipse: {
				EllipseItem *Circle = (EllipseItem*)Item;
				MRotatedRect Cir;
				Circle->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *Circle = (ConcentricCircleItem*)Item;
				CCircle Cir;
				Circle->GetConcentricCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.small_radius))
					.arg(QString::number(Cir.big_radius));
			}	break;
			case ItemRectangle: {
				RectangleItem *Circle = (RectangleItem*)Item;
				MRectangle Cir;
				Circle->GetRect(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.height));
			}	break;
			case ItemRectangleR: {
				RectangleRItem *Circle = (RectangleRItem*)Item;
				MRotatedRect Cir;
				Circle->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemSquare: {
			}	break;
			case ItemPolygon: {
				PolygonItem *Circle = (PolygonItem*)Item;
				MPolygon Pointlst;
				Circle->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++) {
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("|");
				}
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *Circle = (BezierLineItem*)Item;
				MPolygon Pointlst;
				Circle->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++) {
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("|");
				}
				_data.strData.append("@");
				_data.strData.append(QString::number(Circle->fTwoLineLenth));
				_data.strData.append("@");
				_data.strData.append(QString::number(Circle->fNormalLineHeight));
				_data.strData.append("@");
				_data.strData.append(QString::number(Circle->bDrawNormalLine));
				_data.strData.append("@");
				_data.strData.append(QString::number(Circle->fNormalLineWidth));
			}	break;
			case ItemLineObj: {
				LineItem *Circle = (LineItem*)Item;
				LineData Cir;
				Circle->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *Circle = (LineArrowItem*)Item;
				LineData Cir;
				Circle->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			case LineCaliperObj: {
				LineCaliperItem *Circle = (LineCaliperItem*)Item;
				LineCaliperP Cir;
				Circle->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9")
					.arg(QString::number(Cir.y1))
					.arg(QString::number(Cir.x1))
					.arg(QString::number(Cir.y2))
					.arg(QString::number(Cir.x2))
					.arg(QString::number(Cir.sSigma))
					.arg(QString::number(Cir.sThreshold))
					.arg(QString::number(Cir.sActiveNum))
					.arg(QString(Cir.sTransition))
					.arg(QString(Cir.sSelect))
					;
			}	break;
			case ArcCaliperObj: {
				ArcCaliperItem *Circle = (ArcCaliperItem*)Item;
				ArcCaliperP Cir;
				Circle->GetArcCaliper(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9_%10")
					.arg(QString::number(Cir.y))
					.arg(QString::number(Cir.x))
					.arg(QString::number(Cir.Radius))
					.arg(QString::number(Cir.height))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.sSigma))
					.arg(QString::number(Cir.sThreshold))
					.arg(QString::number(Cir.sActiveNum))
					.arg(QString(Cir.sTransition))
					.arg(QString(Cir.sSelect))
					;
			}	break;
			}
			RegionData.push_back(_data);
		}
	}
	return 0;
}

int frmBaseTool::ShapeToQJson(QVector<BaseItem*>& vecItems, QJsonArray & RegionData)
{
	for (auto iter : vecItems) {
		BaseItem* Item = (BaseItem*)iter;
		HObject	_Region;	//ROI形状区域
		QJsonObject _JsonArray;
		if (iter != nullptr) if (iter->type() == 10) {
			_JsonArray.insert("ShapeItemType", QString::number(Item->GetItemType()));

			QJsonObject _JsonData;
			switch (Item->GetItemType()) {
			case ItemPoint: {
				PointItem *_pItem = (PointItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemCircle: {
				CircleItem *_pItem = (CircleItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemEllipse: {
				EllipseItem *_pItem = (EllipseItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_pItem = (ConcentricCircleItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemRectangle: {
				RectangleItem *_pItem = (RectangleItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_pItem = (RectangleRItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemSquare: {
			}	break;
			case ItemPolygon: {
				PolygonItem *_pItem = (PolygonItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *_pItem = (BezierLineItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemLineObj: {
				LineItem *_pItem = (LineItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *_pItem = (LineArrowItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case LineCaliperObj: {
				LineCaliperItem *_pItem = (LineCaliperItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			case ArcCaliperObj: {
				ArcCaliperItem *_pItem = (ArcCaliperItem*)Item;
				_pItem->GetData(_JsonData);
			}	break;
			}
			_JsonArray.insert("ROIConFig", _JsonData);
		}
		RegionData.append(_JsonArray);
	}
	return 0;
}

int frmBaseTool::QJsonToShape(QJsonArray& array_Roiobject, QVector<BaseItem*>& items)
{
	items.clear();
	if (!array_Roiobject.empty()) {
		for (size_t i = 0; i < array_Roiobject.count(); i++) {
			//CommonData _CommonData;
			QJsonObject _ArrayObject = array_Roiobject.at(i).toObject();
			ShapeItemType _type = ItemBezierLineObj;
			if (_ArrayObject.contains("ShapeItemType"))		_type = (ShapeItemType)_ArrayObject["ShapeItemType"].toString().toInt();
			QJsonObject _DataObject = _ArrayObject.find("ROIConFig").value().toObject();

			BaseItem*_ptrShape = nullptr;
			float fData[5] = { 0 };
			switch (_type) {
			case ItemPoint: {
				_ptrShape = new PointItem(fData[1], fData[0], fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemCircle: {
				_ptrShape = new CircleItem(fData[1], fData[0], fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemEllipse: {
				_ptrShape = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemConcentric_Circle: {
				_ptrShape = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemRectangle: {
				_ptrShape = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemRectangleR: {
				_ptrShape = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemSquare: {
			}	break;
			case ItemPolygon: {
				_ptrShape = new PolygonItem();
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemLineObj: {
				_ptrShape = new LineItem(fData[1], fData[0], fData[3], fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemArrowLineObj: {
				_ptrShape = new LineArrowItem(fData[1], fData[0], fData[3], fData[2]);
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemBezierLineObj: {
				_ptrShape = new BezierLineItem();
				_ptrShape->SetData(_DataObject);
			}	break;
			case LineCaliperObj: {
				_ptrShape = new LineCaliperItem();
				_ptrShape->SetData(_DataObject);
			}	break;
			case ArcCaliperObj: {
				_ptrShape = new ArcCaliperItem();
				_ptrShape->SetData(_DataObject);
			}	break;
			case ItemAutoItem:
			default: {				}	break;
			}
			items.push_back(_ptrShape);

		}
	}
	return 0;
}

int frmBaseTool::DataToShape(QVector<CommonData> &vecData, QVector<BaseItem*>& vecItems)
{
	vecItems.clear();
	for (auto Data : vecData) {
		float fData[10] = { 0 };
		QStringList dArray = Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();
		if (dArray.size() > 5)	fData[5] = dArray[5].toDouble();
		if (dArray.size() > 6)	fData[6] = dArray[6].toDouble();
		if (dArray.size() > 7)	fData[7] = dArray[7].toDouble();
		if (dArray.size() > 8)	fData[8] = dArray[8].toDouble();
		if (dArray.size() > 9)	fData[9] = dArray[9].toDouble();
		BaseItem*_ptrShape = nullptr;
		switch (Data.type) {
		case ItemPoint: {
			_ptrShape = new PointItem(fData[1], fData[0], fData[2]);
			((PointItem*)_ptrShape)->m_showShape = (ShapeItemType)(int)fData[3];
		}; break;
		case ItemCircle: {
			_ptrShape = new CircleItem(fData[1], fData[0], fData[2]);
			MCircle				_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.radius		= fData[2];
			((CircleItem*)_ptrShape)->SetCircle(_data);
		}; break;
		case ItemEllipse: {
			_ptrShape = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			MRotatedRect		_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.phi			= fData[2];
			_data.lenth1		= fData[3];
			_data.lenth2		= fData[4];
			((EllipseItem*)_ptrShape)->SetRotatedRect(_data);
		}; break;
		case ItemConcentric_Circle: {
			_ptrShape = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
			CCircle				_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.small_radius	= fData[2];
			_data.big_radius	= fData[3];
			((ConcentricCircleItem*)_ptrShape)->SetConcentricCircle(_data);
		}; break;
		case ItemRectangle: {
			_ptrShape = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
			MRectangle		_data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.width		= fData[2];
			_data.height	= fData[3];
			((RectangleItem*)_ptrShape)->SetRect(_data);
		}; break;
		case ItemRectangleR: {
			_ptrShape = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			MRotatedRect		_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.phi			= fData[2];
			_data.lenth1		= fData[3] ;
			_data.lenth2		= fData[4] ;
			((RectangleRItem*)_ptrShape)->SetRotatedRect(_data);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			_ptrShape = new PolygonItem();
			auto _vecData = Data.strData.split("|");
			MPolygon _data;
			for (int i = 0; i < (_vecData.size() - 1); i++) {
				auto _vecPos = _vecData[i].split(",");
				QPointF _PtPos;
				if (_vecPos.size() > 0)	_PtPos.setX(_vecPos[0].toDouble());
				if (_vecPos.size() > 1)	_PtPos.setY(_vecPos[1].toDouble());
				_data.points.push_back(_PtPos);
				_data.list_p.push_back(_PtPos);
			}
			((PolygonItem*)_ptrShape)->SetPolygon(_data);
			((PolygonItem*)_ptrShape)->SetDrawEnd();

		}; break;
		case ItemBezierLineObj: {
			_ptrShape = new BezierLineItem();
			auto _vecQData = Data.strData.split("@");
			if (_vecQData.size() > 0)	{
				auto _vecData = _vecQData[0].split("|");
				MPolygon _data;
				for (int i = 0; i < (_vecData.size() - 1); i++) {
					auto _vecPos = _vecData[i].split(",");
					QPointF _PtPos;
					if (_vecPos.size() > 0)	_PtPos.setX(_vecPos[0].toDouble());
					if (_vecPos.size() > 1)	_PtPos.setY(_vecPos[1].toDouble());
					_data.points.push_back(_PtPos);
					_data.list_p.push_back(_PtPos);
				}
				((BezierLineItem*)_ptrShape)->SetPolygon(_data);
				((BezierLineItem*)_ptrShape)->SetDrawEnd();
				if (_vecQData.size() > 1) ((BezierLineItem*)_ptrShape)->fTwoLineLenth		= _vecQData[1].toDouble();
				if (_vecQData.size() > 2) ((BezierLineItem*)_ptrShape)->fNormalLineHeight	= _vecQData[2].toDouble();
				if (_vecQData.size() > 3) ((BezierLineItem*)_ptrShape)->bDrawNormalLine		= _vecQData[3].toDouble();
				if (_vecQData.size() > 4)  ((BezierLineItem*)_ptrShape)->fNormalLineWidth	= _vecQData[4].toDouble();
			}
		}; break;
		case ItemLineObj: {
			_ptrShape = new LineItem(fData[1], fData[0], fData[3], fData[2]);
			LineData			_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.row1			= fData[2];
			_data.col1			= fData[3];
			((LineItem*)_ptrShape)->SetLineData(_data);
		}; break;
		case ItemArrowLineObj: {
			_ptrShape = new LineArrowItem(fData[1], fData[0], fData[3], fData[2]);
			LineData			_data;
			_data.row			= fData[0];
			_data.col			= fData[1];
			_data.row1			= fData[2];
			_data.col1			= fData[3];
			((LineArrowItem*)_ptrShape)->SetLineData(_data);
		}; break;
		case LineCaliperObj: {
			_ptrShape = new LineCaliperItem(fData[1], fData[0], fData[3], fData[2], fData[4]);
			LineCaliperP			_data;
			_data.y1	= fData[0];
			_data.x1	= fData[1];
			_data.y2	= fData[2];
			_data.x2	= fData[3];
			((LineCaliperItem*)_ptrShape)->SetLineData(_data);
		}; break;
		case ArcCaliperObj: {
			_ptrShape = new ArcCaliperItem();
			ArcCaliperP			_data;
			_data.x = fData[0];
			_data.y = fData[1];
			_data.Radius = fData[2];
			_data.height = fData[3];
			((ArcCaliperItem*)_ptrShape)->SetArcCaliper(_data);
		}; break;
		default: continue;	break;
		}
		_ptrShape->m_eRegionType = Data.eRegionType;
		vecItems.push_back(_ptrShape);
	}
	return 0;
}

int frmBaseTool::AffineShapeTrans(QVector<BaseItem*>&vecItems,const HTuple& HomMat2D,MiddleParam & param,bool bUse)
{
	for (auto iter : vecItems) {
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			HTuple	_Rows, _Columns;
			switch (Item->GetItemType()) {
			case ItemPoint: {
				PointItem *_pItem = (PointItem*)Item;
				MCircle _data;
				_pItem->GetCircle(_data);
				switch (_pItem->m_showShape) {
				default:
				case ItemPoint: {
					if (bUse) {		AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns);		}
					else {	_Rows = _data.row;	_Columns = _data.col;			}
					sColorCircle _shape;
					_shape.CenterPoints.setX(_Columns.D());
					_shape.CenterPoints.setY(_Rows.D());
					_shape.qRadius			= _data.radius;
					_shape.DrawQColor		= QColor(0, 0, 255);;
					_shape.LineWidth		= -1;
					param.VecShowColorCircle.push_back(_shape);
				}	break;
				case ItemRectangle:
				case ItemRectangleR:
				case ItemSquare: {
					sColorLineRect _shape;
					if (bUse) {	AffineTransPixel(HomMat2D, _data.row - _data.radius, _data.col, &_Rows, &_Columns);			}
					else {	_Rows = _data.row - _data.radius;	_Columns = _data.col;		}
					_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
					if (bUse)	{	AffineTransPixel(HomMat2D, _data.row + _data.radius, _data.col, &_Rows, &_Columns);		}
					else		{	_Rows		= _data.row + _data.radius;	_Columns	= _data.col;				}
					_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
					_shape.DrawQColor	= QColor(0, 0, 255);;
					_shape.LineWidth	= -1;
					_shape.DWidth		= _data.radius;
					param.VecShowColorLineRect.push_back(_shape);
				}	break;
				}
			}	break;
			case ItemCircle: {
				CircleItem *_pItem = (CircleItem*)Item;				
				MCircle _data;
				_pItem->GetCircle(_data);
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				sColorCircle _shape;
				_shape.CenterPoints.setX(_Columns.D());
				_shape.CenterPoints.setY(_Rows.D());
				_shape.qRadius = _data.radius;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				param.VecShowColorCircle.push_back(_shape);
			}	break;
			case ItemEllipse: {		//暂时不对
				EllipseItem *_pItem = (EllipseItem*)Item;	
				MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				sColorRRectangle _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				_shape.Center.setX(_Columns.D());
				_shape.Center.setY(_Rows.D());
				_shape.DHeight	= _data.lenth1;
				_shape.DWidth	= _data.lenth2;
				_shape.Angle	= _data.phi;
				_shape.DrawQColor	= QColor(0, 0, 255);;
				_shape.LineWidth	= -1;
				param.VecShowColorEllipse.push_back(_shape);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_pItem = (ConcentricCircleItem*)Item;		CCircle _data;
				_pItem->GetConcentricCircle(_data);
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				{
					sColorCircle _shape;
					_shape.CenterPoints.setX(_Columns.D());
					_shape.CenterPoints.setY(_Rows.D());
					_shape.qRadius = _data.big_radius;
					_shape.DrawQColor = QColor(0, 0, 255);;
					_shape.LineWidth = -1;
					param.VecShowColorCircle.push_back(_shape);
				}		{
					sColorCircle _shape;
					_shape.CenterPoints.setX(_Columns.D());
					_shape.CenterPoints.setY(_Rows.D());
					_shape.qRadius		= _data.small_radius;
					_shape.DrawQColor	= QColor(0, 0, 255);;
					_shape.LineWidth	= -1;
					param.VecShowColorCircle.push_back(_shape);
				}
			}	break;
			case ItemRectangle: {
				RectangleItem *_pItem = (RectangleItem*)Item;					MRectangle _data;
				_pItem->GetRect(_data);
				sColorLineRect _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.row - _data.height / 2.0, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row - _data.height / 2.0;	_Columns = _data.col; }

				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				if (bUse) { AffineTransPixel(HomMat2D, _data.row + _data.height / 2.0, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row + _data.height / 2.0;	_Columns = _data.col; }

				_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
				_shape.DrawQColor	= QColor(0, 0, 255);;
				_shape.LineWidth	= -1;
				_shape.DWidth		= _data.width;
				param.VecShowColorLineRect.push_back(_shape);

			}	break;
			case ItemRectangleR: {		//暂时不对
				RectangleRItem *_pItem = (RectangleRItem*)Item;					MRotatedRect _data;
				_pItem->GetRotatedRect(_data);
				sColorRRectangle _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				_shape.Center.setX(_Columns.D());
				_shape.Center.setY(_Rows.D());
				_shape.DHeight = _data.lenth1;
				_shape.DWidth = _data.lenth2;
				_shape.Angle = _data.phi;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				_shape.DWidth = _data.lenth2;
				param.VecShowColorRRect.push_back(_shape);
			}	break;
			case ItemPolygon: {
				PolygonItem *_pItem = (PolygonItem*)Item;						MPolygon Pointlst;
				_pItem->GetPolygon(Pointlst);
				sColorPoint _shape;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				for (size_t i = 0; i < Pointlst.points.size(); i++)	{
					QPointF ptCenter = Pointlst.points[i];

					if (bUse) {
						AffineTransPixel(HomMat2D, ptCenter.y(), ptCenter.x(), &_Rows, &_Columns);
						_shape.vecPoints.push_back(QPointF(_Columns.D(), _Rows.D()));
					}
					else		{
						_shape.vecPoints.push_back(ptCenter);
					}
				}
				param.VecShowColorPoints.push_back(_shape);
			}	break;
			case ItemBezierLineObj: {
				BezierLineItem *_pItem = (BezierLineItem*)Item;
				QVector<MeasureRect2> _Rlines;
				QVector<MeasureRect2> _lines = _pItem->GetDrawLines();
				for (auto& _oneLine : _lines) {
					MeasureRect2 _line = _oneLine;
					if (bUse) {
						AffineTransPixel(HomMat2D, _oneLine.line.p1().y(), _oneLine.line.p1().x(), &_Rows, &_Columns);
						_line.line.setP1(QPointF(_Columns.D(), _Rows.D()));
						AffineTransPixel(HomMat2D, _oneLine.line.p2().y(), _oneLine.line.p2().x(), &_Rows, &_Columns);
						_line.line.setP2(QPointF(_Columns.D(), _Rows.D()));
						_Rlines.push_back(_line);
					}
					else	{
						_line.line.setP1(QPointF(_oneLine.line.p1().x(), _oneLine.line.p1().y()));
						_line.line.setP2(QPointF(_oneLine.line.p2().x(), _oneLine.line.p2().y()));
					}

					sColorLineRect _LineRect;
					_LineRect.Dline			= _line.line;
					_LineRect.DWidth		= _line.dNormalLineWidth;
					_LineRect.DrawQColor	= QColor(0, 0, 255);;
					_LineRect.LineWidth		= -1;
					param.VecShowColorLineRect.push_back(_LineRect);
				}
			}	break;
			case ItemLineObj: {
				LineItem *_pItem = (LineItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);

				sColorLine	_shape;
				if (bUse)	{ AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				if (bUse) { AffineTransPixel(HomMat2D, _data.row1, _data.col1, &_Rows, &_Columns); }
				else { _Rows = _data.row1;	_Columns = _data.col1; }
				_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
				param.VecShowColorLine.push_back(_shape);
			}	break;
			case ItemArrowLineObj: {
				LineArrowItem *_pItem = (LineArrowItem*)Item;
				LineData _data;
				_pItem->GetLineData(_data);

				sColorLine _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				if (bUse) { AffineTransPixel(HomMat2D, _data.row1, _data.col1, &_Rows, &_Columns); }
				else { _Rows = _data.row1;	_Columns = _data.col1; }
				_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
				param.VecShowColorLine.push_back(_shape);
			}	break;
			case LineCaliperObj: {
				LineCaliperItem *_pItem = (LineCaliperItem*)Item;
				LineCaliperP _data;
				_pItem->GetLineData(_data);

				sColorLine _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.y1, _data.x1, &_Rows, &_Columns); }
				else { _Rows = _data.y1;	_Columns = _data.x1; }
				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				if (bUse) { AffineTransPixel(HomMat2D, _data.y2, _data.x2, &_Rows, &_Columns); }
				else { _Rows = _data.y2;	_Columns = _data.x2; }
				_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
				param.VecShowColorLine.push_back(_shape);
			}	break;
			case ArcCaliperObj: {
				ArcCaliperItem *_pItem = (ArcCaliperItem*)Item;
				ArcCaliperP _data;
				_pItem->SetArcCaliper(_data);

				sColorLine _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.y, _data.x, &_Rows, &_Columns); }
				else { _Rows = _data.y;	_Columns = _data.x; }
				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				param.VecShowColorLine.push_back(_shape);
			}	break;
			}
		}
	}
	return	0;
}

int frmBaseTool::AffineShapeTrans(QVector<CommonData>&vecRegions,const HTuple& HomMat2D,MiddleParam & param,bool bUse)
{
	for (auto Data : vecRegions) {
		float fData[10] = { 0 };
		QStringList dArray = Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();
		if (dArray.size() > 5)	fData[5] = dArray[5].toDouble();
		if (dArray.size() > 6)	fData[6] = dArray[6].toDouble();
		if (dArray.size() > 7)	fData[7] = dArray[7].toDouble();
		BaseItem*_ptrShape = nullptr;
		HTuple	_Rows, _Columns;
		switch (Data.type) {
		case ItemPoint: {
			MCircle _data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.radius	= fData[2];
			switch ((ShapeItemType)(int)fData[3])	{
			default:
			case ItemPoint: {
				if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row;	_Columns = _data.col; }
				sColorCircle _shape;
				_shape.CenterPoints.setX(_Columns.D());
				_shape.CenterPoints.setY(_Rows.D());
				_shape.qRadius = _data.radius;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				param.VecShowColorCircle.push_back(_shape);
			}	break;
			case ItemRectangle:
			case ItemRectangleR:
			case ItemSquare: {
				sColorLineRect _shape;
				if (bUse) { AffineTransPixel(HomMat2D, _data.row - _data.radius, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row - _data.radius;	_Columns = _data.col; }
				_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
				if (bUse) { AffineTransPixel(HomMat2D, _data.row + _data.radius, _data.col, &_Rows, &_Columns); }
				else { _Rows = _data.row + _data.radius;	_Columns = _data.col; }				
				_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				_shape.DWidth = _data.radius;
				param.VecShowColorLineRect.push_back(_shape);
			}	break;
			}
		}; break;
		case ItemCircle: {
			MCircle _data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.radius	= fData[2];
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			sColorCircle _shape;
			_shape.CenterPoints.setX(_Columns.D());
			_shape.CenterPoints.setY(_Rows.D());
			_shape.qRadius = _data.radius;
			_shape.DrawQColor = QColor(0, 0, 255);;
			_shape.LineWidth = -1;
			param.VecShowColorCircle.push_back(_shape);
		}; break;
		case ItemEllipse: {		//暂时不对
			//_ptrShape = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			MRotatedRect _data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.lenth1	= fData[3];
			_data.lenth2	= fData[4];
			_data.phi		= fData[2];
			sColorRRectangle _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Center.setX(_Columns.D());
			_shape.Center.setY(_Rows.D());
			_shape.DHeight		= _data.lenth1;
			_shape.DWidth		= _data.lenth2;
			_shape.Angle		= _data.phi;
			_shape.DrawQColor	= QColor(0, 0, 255);;
			_shape.LineWidth	= -1;
			param.VecShowColorEllipse.push_back(_shape);
		}; break;
		case ItemConcentric_Circle: {
			CCircle _data;
			_data.row = fData[0];
			_data.col = fData[1];
			_data.small_radius	= fData[2];
			_data.big_radius	= fData[3];
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			{
				sColorCircle _shape;
				_shape.CenterPoints.setX(_Columns.D());
				_shape.CenterPoints.setY(_Rows.D());
				_shape.qRadius = _data.big_radius;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				param.VecShowColorCircle.push_back(_shape);
			} {
				sColorCircle _shape;
				_shape.CenterPoints.setX(_Columns.D());
				_shape.CenterPoints.setY(_Rows.D());
				_shape.qRadius = _data.small_radius;
				_shape.DrawQColor = QColor(0, 0, 255);;
				_shape.LineWidth = -1;
				param.VecShowColorCircle.push_back(_shape);
			}
		}; break;
		case ItemRectangle: {
			MRectangle		_data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.width		= fData[2];
			_data.height	= fData[3];
			sColorLineRect _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.row - _data.height / 2.0, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row - _data.height / 2.0;	_Columns = _data.col; }

			_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
			if (bUse) { AffineTransPixel(HomMat2D, _data.row + _data.height / 2.0, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row + _data.height / 2.0;	_Columns = _data.col; }

			_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
			_shape.DrawQColor	= QColor(0, 0, 255);;
			_shape.LineWidth	= -1;
			_shape.DWidth		= _data.width;
			param.VecShowColorLineRect.push_back(_shape);
		}; break;
		case ItemRectangleR: {
			MRotatedRect	_data;
			_data.row		= fData[0];
			_data.col		= fData[1];
			_data.lenth1	= fData[3];
			_data.lenth2	= fData[4];
			_data.phi		= fData[2];
			sColorRRectangle _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Center.setX(_Columns.D());
			_shape.Center.setY(_Rows.D());
			_shape.DHeight		= _data.lenth1;
			_shape.DWidth		= _data.lenth2;
			_shape.Angle		= _data.phi;
			_shape.DrawQColor	= QColor(0, 0, 255);;
			_shape.LineWidth	= -1;
			param.VecShowColorRRect.push_back(_shape);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			_ptrShape = new PolygonItem();
			auto _vecData = Data.strData.split("|");
			MPolygon Pointlst;
			for (int i = 0; i < (_vecData.size() - 1); i++) {
				auto _vecPos = _vecData[i].split(",");
				QPointF _PtPos;
				if (_vecPos.size() > 0)	_PtPos.setX(_vecPos[0].toDouble());
				if (_vecPos.size() > 1)	_PtPos.setY(_vecPos[1].toDouble());
				Pointlst.points.push_back(_PtPos);
				Pointlst.list_p.push_back(_PtPos);
			}
			sColorPoint _shape;
			_shape.DrawQColor = QColor(0, 0, 255);;
			_shape.LineWidth = -1;
			for (size_t i = 0; i < Pointlst.points.size(); i++) {
				QPointF ptCenter = Pointlst.points[i];
				if (bUse) { AffineTransPixel(HomMat2D, ptCenter.y(), ptCenter.x(), &_Rows, &_Columns); }
				else { _Rows = ptCenter.y();	_Columns = ptCenter.x(); }

				_shape.vecPoints.push_back(QPointF(_Columns.D(), _Rows.D()));
			}
			param.VecShowColorPoints.push_back(_shape);
		}; break;
		case ItemBezierLineObj: {		//暂时不对
			_ptrShape = new BezierLineItem();
			auto _vecQData = Data.strData.split("@");
			if (_vecQData.size() > 0)		{
				auto _vecData = _vecQData[0].split("|");
				MPolygon _data;
				for (int i = 0; i < (_vecData.size() - 1); i++) {
					auto _vecPos = _vecData[i].split(",");
					QPointF _PtPos;
					if (_vecPos.size() > 0)	_PtPos.setX(_vecPos[0].toDouble());
					if (_vecPos.size() > 1)	_PtPos.setY(_vecPos[1].toDouble());
					_data.points.push_back(_PtPos);
					_data.list_p.push_back(_PtPos);
				}
				((BezierLineItem*)_ptrShape)->SetPolygon(_data);
				((BezierLineItem*)_ptrShape)->SetDrawEnd();
				if (_vecQData.size() > 1) ((BezierLineItem*)_ptrShape)->fTwoLineLenth = _vecQData[1].toDouble();
				if (_vecQData.size() > 2) ((BezierLineItem*)_ptrShape)->fNormalLineHeight = _vecQData[2].toDouble();
				if (_vecQData.size() > 3) ((BezierLineItem*)_ptrShape)->bDrawNormalLine = _vecQData[3].toDouble();
				if (_vecQData.size() > 4)  ((BezierLineItem*)_ptrShape)->fNormalLineWidth = _vecQData[4].toDouble();
			}
		}; break;
		case ItemLineObj: {
			LineData _data;
			_data.row = fData[0];
			_data.col = fData[1];
			_data.row1 = fData[2];
			_data.col1 = fData[3];
			sColorLine _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
			param.VecShowColorLine.push_back(_shape);
		}; break;
		case ItemArrowLineObj: {
			LineData _data;
			_data.row	= fData[0];
			_data.col	= fData[1];
			_data.row1	= fData[2];
			_data.col1	= fData[3];
			sColorLine _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
			if (bUse) { AffineTransPixel(HomMat2D, _data.row, _data.col, &_Rows, &_Columns); }
			else { _Rows = _data.row;	_Columns = _data.col; }
			_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
			param.VecShowColorLine.push_back(_shape);
		}; break;
		case LineCaliperObj: {
			LineCaliperP _data;
			_data.y1 = fData[0];
			_data.x1 = fData[1];
			_data.y2 = fData[2];
			_data.x2 = fData[3];
			sColorLine _shape;
			if (bUse) { AffineTransPixel(HomMat2D, _data.y1, _data.x1, &_Rows, &_Columns); }
			else { _Rows = _data.y1;	_Columns = _data.x1; }
			_shape.Dline.setP1(QPointF(_Columns.D(), _Rows.D()));
			if (bUse) { AffineTransPixel(HomMat2D, _data.y2, _data.x2, &_Rows, &_Columns); }
			else { _Rows = _data.y2;	_Columns = _data.x2; }
			_shape.Dline.setP2(QPointF(_Columns.D(), _Rows.D()));
			param.VecShowColorLine.push_back(_shape);
		}	break;
		default: continue;	break;
		}
	}

	return	0;
}

QImage frmBaseTool::HObject2QImage(const HObject & hObject)
{
	HTuple Type, Channels;
	GetImageType(hObject, &Type);
	CountChannels(hObject, &Channels);

	Hlong  width = 0, height = 0;
	HString	htype;
	if (Type.S() == HString(HString("byte")))
	{
		switch (Channels.I())	{
		case 1:		{
			HImage from(hObject);
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			QImage img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
			return img;
		}	break;
		case 3:		{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)		{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		case 4:	{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)		{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		default:	{
		}	break;
		}
	}
	else if (Type.S() == HString(HString("real")) || Type.S() == HString(HString("uint2")) || Type.S() == HString(HString("int4")))	{
		int	iChannel = Channels.I();
		switch (Channels.I())	{
		case 1:	{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			QImage img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
			return img;
		}	break;
		case 2: {
			HObject	_Object;
			AccessChannel(hObject,&_Object,1);
			scale_image_range(_Object, &_Object);
			HImage	from(_Object);
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			QImage img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
			return img;
		}	break;
		case 3:		{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		case 4:		{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		default:	{
		}	break;
		}
	}
	return QImage();
}

void  frmBaseTool::HObjectToQImage(const HalconCpp::HObject &hobj, QImage& image) //Halcon中的HObject类型转QImage类型
{
	if (!hobj.IsInitialized())
	{
		image = QImage();
		return;
	}
	HTuple channel, Type;
	HalconCpp::CountChannels(hobj, &channel);
	HalconCpp::GetImageType(hobj, &Type);
	int m_iChanal = 0;

	if (Type.S() == HString(HString("byte")))
	{
		switch (channel.I())	//将Halcon转换为QImage
		{
		case 1:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);
			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)
				image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++)
			{
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}
		}	break;
		case 3:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				hSelectChannal = hobj;
			else
			{
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
				return;
			}
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
			return;
		}	break;
		case 4:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				hSelectChannal = hobj;
			else
			{
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
				return;
			}
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_ARGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
			return;
		}	break;
		default:
		{
		}	break;
		}
	}
	else if (Type.S() == HString(HString("real")) || Type.S() == HString(HString("uint2")) || Type.S() == HString(HString("int4")))		//用来显示相关的参数
	{
		switch (channel.I())
		{
		case 1:
		{
			HObject	_Object;
			scale_image_range(hobj, &_Object);
			HImage	himg(_Object);

			//HImage	himg(hobj);
			Hlong	width;
			Hlong	height;
			HString type;

			//himg	= himg.ScaleImageMax();
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);

			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)	image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++)
			{
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}
			//return image;
		}	break;
		case 2:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
			HObject	_Object;
			scale_image_range(hSelectChannal, &_Object);
			HImage	himg(_Object);

			//HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			//himg = himg.ScaleImageMax();
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);

			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)
				image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++)
			{
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}

		}	break;
		default:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);

			HObject	_Object;
			scale_image_range(hSelectChannal, &_Object);
			HImage	himg(_Object);

			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			//himg = himg.ScaleImageMax();
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
		}	break;
		}
	}
}

QImage frmBaseTool::Hal2QImage(const HObject & hobj)
{
	if (!hobj.IsInitialized())	{
		return QImage();
	}
	try	{
		HTuple channel, Type;
		HalconCpp::CountChannels(hobj, &channel);
		HalconCpp::GetImageType(hobj, &Type);
		if (Type.TupleIsString() == 0)
		{
			return QImage();
		}
		if (Type.S() == HString("byte"))
		{
			switch (channel.I())	//将Halcon转换为QImage
			{
			case 1:
			{
				HObject	hSelectChannal;
				if (m_iChanal == 0)
					AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
				else
					AccessChannel(hobj, &hSelectChannal, m_iChanal);
				HImage	himg(hSelectChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				QImage image(width, height, QImage::Format_Indexed8);
				image.setColorCount(256);
				for (int i = 0; i < 256; i++)
					image.setColor(i, qRgb(i, i, i));
				uchar* pSrc = static_cast<UCHAR*>(ptr);
				for (int row = 0; row < height; row++)
				{
					uchar* pDest = image.scanLine(row);
					memcpy(pDest, pSrc, width);
					pSrc += width;
				}
				return image;

			}	break;
			case 3:
			{
				HObject	hSelectChannal;
				if (m_iChanal == 0)
					hSelectChannal = hobj;
				else
				{
					AccessChannel(hobj, &hSelectChannal, m_iChanal);
					return Hal2QImage(hSelectChannal);
				}
				HImage	himg(hSelectChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				uchar*	PointerRed;
				uchar*	PointerGreen;
				uchar*	PointerBlue;

				himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
				QImage image(width, height, QImage::Format_RGB32);
				for (int row = 0; row < height; row++)
				{
					QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
					for (int col = 0; col < width; col++)
						line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
				}
				return image;
			}	break;
			case 4:
			{
				HObject	hSelectChannal;
				if (m_iChanal == 0)
					hSelectChannal = hobj;
				else
				{
					AccessChannel(hobj, &hSelectChannal, m_iChanal);
					return Hal2QImage(hSelectChannal);
				}
				HImage	himg(hSelectChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				uchar*	PointerRed;
				uchar*	PointerGreen;
				uchar*	PointerBlue;
				himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
				QImage image(width, height, QImage::Format_ARGB32);
				for (int row = 0; row < height; row++)
				{
					QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
					for (int col = 0; col < width; col++)
						line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
				}
				return image;
			}	break;
			default:
			{
			}	break;
			}
		}
		else if (Type.S() == HString("real") || Type.S() == HString("uint2") || Type.S() == HString("int4"))		//用来显示相关的参数
		{
			switch (channel.I())
			{
			case 1:
			{
				HObject	_Object;
				scale_image_range(hobj, &_Object);
				HImage	himg(_Object);

				//HImage	himg(hobj);
				Hlong	width;
				Hlong	height;
				HString type;
				//himg = himg.ScaleImageMax();
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				QImage image(width, height, QImage::Format_Indexed8);
				image.setColorCount(256);
				for (int i = 0; i < 256; i++)	image.setColor(i, qRgb(i, i, i));
				uchar* pSrc = static_cast<UCHAR*>(ptr);
				for (int row = 0; row < height; row++)
				{
					uchar* pDest = image.scanLine(row);
					memcpy(pDest, pSrc, width);
					pSrc += width;
				}
				return image;
			}	break;
			case 2:
			{
				HObject	hSelectChannal;
				if (m_iChanal == 0)
					AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
				else
					AccessChannel(hobj, &hSelectChannal, m_iChanal);

				HObject	_Object;
				scale_image_range(hSelectChannal, &_Object);
				HImage	himg(_Object);

				//HImage	himg(hSelectChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				//himg = himg.ScaleImageMax();
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				QImage	image(width, height, QImage::Format_Indexed8);
				image.setColorCount(256);
				for (int i = 0; i < 256; i++)
					image.setColor(i, qRgb(i, i, i));
				uchar* pSrc = static_cast<UCHAR*>(ptr);
				for (int row = 0; row < height; row++)
				{
					uchar* pDest = image.scanLine(row);
					memcpy(pDest, pSrc, width);
					pSrc += width;
				}
				return image;

			}	break;
			default:
			{
				HObject	hSelectChannal;
				if (m_iChanal == 0)
					AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
				else
					AccessChannel(hobj, &hSelectChannal, m_iChanal);
				HObject	_Object;
				scale_image_range(hSelectChannal, &_Object);
				HImage	himg(_Object);
				//HImage	himg(hSelectChannal);

				Hlong	width;
				Hlong	height;
				HString type;
				uchar*	PointerRed;
				uchar*	PointerGreen;
				uchar*	PointerBlue;

				himg = himg.ScaleImageMax();
				himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
				QImage image(width, height, QImage::Format_RGB32);
				for (int row = 0; row < height; row++)
				{
					QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
					for (int col = 0; col < width; col++)
						line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
				}
				return image;

			}	break;
			}
		}
	}
	catch (const std::exception&)
	{

	}
	catch (...)
	{
	}
	return QImage();
}

void frmBaseTool::ChangeLanguage()
{
	BaseWindow::ChangeLanguage();
}

void frmBaseTool::UpdateQGraphicsViews()
{
	if (m_dstImage.IsInitialized()) {
		m_view->DispImage(HObject2QImage(m_dstImage), false);				//这东西这么耗时不能用界面刷啊要不然很卡啊,4相机就顶不住了
		QApplication::postEvent(this, new MsgEvent(0xFFFF, 0xFFFF, ""));
	}
	if (m_view != nullptr) {
		m_view->UpdateImg();
		m_view->update(); 
	}

}

QString frmBaseTool::GetShapeInfo(BaseItem * Item)
{
	switch (Item->GetItemType()) {
	case ItemPoint: {
		PointItem *item = (PointItem*)Item;
		MCircle Cir;
		item->GetCircle(Cir);
		return QString("(x:%1,y:%2,r:%3)").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));// .arg(QString::number(item->m_showShape));
	}	break;
	case ItemCircle: {
		CircleItem *item = (CircleItem*)Item;
		MCircle Cir;
		item->GetCircle(Cir);
		return QString("(x:%1,y:%2,r:%3)").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
	}	break;
	case ItemEllipse: {
		EllipseItem *item = (EllipseItem*)Item;
		MRotatedRect Cir;
		item->GetRotatedRect(Cir);
		return QString("(x:%1,y:%2,phi:%3,L1:%4,L2:%5)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.phi))
			.arg(QString::number(Cir.lenth1))
			.arg(QString::number(Cir.lenth2));
	}	break;
	case ItemConcentric_Circle: {
		ConcentricCircleItem *item = (ConcentricCircleItem*)Item;
		CCircle Cir;
		item->GetConcentricCircle(Cir);
		return QString("(x:%1,y:%2,sr:%4,lr:%5)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.small_radius))
			.arg(QString::number(Cir.big_radius));
	}	break;
	case ItemRectangle: {
		RectangleItem *item = (RectangleItem*)Item;
		MRectangle Cir;
		item->GetRect(Cir);
		return QString("(x:%1,y:%2,w:%4,h:%5)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.width))
			.arg(QString::number(Cir.height));
	}	break;
	case ItemRectangleR: {
		RectangleRItem *item = (RectangleRItem*)Item;
		MRotatedRect Cir;
		item->GetRotatedRect(Cir);
		return QString("(x:%1,y:%2,phi:%3,L1:%4,L2:%5)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.phi))
			.arg(QString::number(Cir.lenth1))
			.arg(QString::number(Cir.lenth2));
	}	break;
	case ItemSquare: {
		//RectangleRItem *item = (RectangleRItem*)Item;		MRotatedRect Cir;
		//item->GetRotatedRect(Cir);
		//ui.tableROIWidget->item(_iRow, 1)->setText(QString("(x:%1,y:%2,phi:%3,L1:%4,L2:%5)")
		//	.arg(QString::number(Cir.row))
		//	.arg(QString::number(Cir.col))
		//	.arg(QString::number(Cir.phi))
		//	.arg(QString::number(Cir.lenth1))
		//	.arg(QString::number(Cir.lenth2)));
	}	break;
	case ItemPolygon: {
		EllipseItem *item = (EllipseItem*)Item;
		MRotatedRect Cir;
		item->GetRotatedRect(Cir);
		return QString("(x:%1,y:%2,phi:%3,L1:%4,L2:%5)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.phi))
			.arg(QString::number(Cir.lenth1))
			.arg(QString::number(Cir.lenth2));
	}	break;
	case ItemLineObj: {
		LineItem *item = (LineItem*)Item;
		LineData Cir;
		item->GetLineData(Cir);
		return QString("(y1:%1,x1:%2,y2:%3,x2:%4)")
			.arg(QString::number(Cir.row))
			.arg(QString::number(Cir.col))
			.arg(QString::number(Cir.row1))
			.arg(QString::number(Cir.col1));
	}	break;
	case ItemBezierLineObj: {
		BezierLineItem *item = (BezierLineItem*)Item;
		MPolygon pylogon;
		item->GetPolygon(pylogon);
		QString _str;
		for (int i = 0; i < pylogon.points.size(); i++) {
			QString	strName = "Point" + QString::number(i);
			_str.append("x" + QString::number(i) + ":").append(QString::number(pylogon.points[i].x())).append(",");
			_str.append("y" + QString::number(i) + ":").append(QString::number(pylogon.points[i].y())).append(",");
		}
		return _str;
	}	break;
	case LineCaliperObj: {
		LineCaliperItem *item = (LineCaliperItem*)Item;
		LineCaliperP Cir;
		item->GetLineData(Cir);
		return QString("(y1:%1,x1:%2,y2:%3,x2:%4)")
			.arg(QString::number(Cir.y1))
			.arg(QString::number(Cir.x1))
			.arg(QString::number(Cir.y2))
			.arg(QString::number(Cir.x2))
			;
	}	break;
	}
	return QString();
}

void frmBaseTool::moveRow(QTableWidget* pTable, int nFrom, int nTo)
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
		pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
	}
	if (nFrom < nTo)
	{
		nTo--;
	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}

void frmBaseTool::on_btnExecute_clicked()
{
	ui->btnExecute->setEnabled(false);
	QApplication::processEvents();
	m_iChanal = 0;
	QElapsedTimer _timer;
	_timer.start();

	m_Middle_Param.ClearAll();
	m_strError.clear();
	PrExecute();
	QFuture<int> value = QtConcurrent::run([=] {
		int _iRetn = 0;
		try {	_iRetn = Execute(m_Middle_Param, m_strError);		}
		catch (const std::exception& ex) {
			sDrawText					_strText;
			_strText.bControl			= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor			= QColor(255, 0, 0);
			_strText.strText			= ex.what();
			m_Middle_Param.VecDrawText.push_back(_strText);
		}
		catch (const HException& except) {
			QString strError = except.ErrorText();
			QStringList strErrorArray = strError.split(':');
			if (strErrorArray.size() > 1) {
				if (!strErrorArray[1].isEmpty()) {
					strError.clear();
					for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
					sDrawText					_strText;
					_strText.bControl			= true;
					if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
					_strText.DrawColor			= QColor(255, 0, 0);
					_strText.strText			= QObject::tr("SoftWare Error") + strError;
					m_Middle_Param.VecDrawText.push_back(_strText);
					//qCritical() << QObject::tr("软件发生异常") << strError;
				}
			}
		}
		catch (...) {
			//qCritical() << QObject::tr("软件异常");
			sDrawText					_strText;
			_strText.bControl			= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor			= QColor(255, 0, 0);
			_strText.strText			= QObject::tr("SoftWare Error");
			m_Middle_Param.VecDrawText.push_back(_strText);
		}
		return _iRetn;
	});
	while (!value.isFinished()) {
		QApplication::processEvents();
	}
	double	_dDuringTime = _timer.nsecsElapsed() / 1000000.0;
	int	_iPrecision = databaseVar::Instance().form_System_Precision;
	QString	_strExcute = QString::number(_dDuringTime, 'f', databaseVar::Instance().form_System_Precision);
	ui->lbl_Excute->setText(_strExcute + tr(" ms"));
	QFuture<int> values = QtConcurrent::run([=] {
		try {
			if (m_view != nullptr) {
				m_view->ClearAllDrawItem();
				QVector<sDrawCross> _PointLst;
				for (auto& iter : m_Middle_Param.MapDrawCross) { _PointLst.push_back(iter.second); }
				m_view->DispCrossPoint(_PointLst);
				{
					QVector<QPointF> _showPointLst;
					for (auto& iter : m_Middle_Param.VecShowQPointFs) { _showPointLst.push_back(iter); }
					m_view->DispPoint(_showPointLst);
				}
				{	QVector<sColorPoint> _showPointLst;
					for (auto& iter : m_Middle_Param.VecShowColorPoints) { _showPointLst.push_back(iter); }
					m_view->DispColorPoints(_showPointLst);
				}
				QVector<sDrawText>	_VecText1D;
				int iIndex = 0;
				for (auto& iter : m_Middle_Param.VecDrawText) { _VecText1D.push_back(iter); }
				m_view->DispTextList(_VecText1D);
				{	QVector<sColorLineRect> _list;
					for (auto& iter : m_Middle_Param.VecShowColorLineRect) { _list.push_back(iter); }
					m_view->DispLineRectPoint(_list);
				}
				{
					QVector<sColorRRectangle> _list;
					for (auto& iter : m_Middle_Param.VecShowColorRRect) { _list.push_back(iter); }
					m_view->DispRRectanglePoint(_list);
				} {
					QVector<sColorRRectangle> _list;
					for (auto& iter : m_Middle_Param.VecShowColorEllipse) { _list.push_back(iter); }
					m_view->DispREllipsePoint(_list);
				}	{
					QVector<sColorLine> _list;
					for (auto& iter : m_Middle_Param.VecShowColorLine) { _list.push_back(iter); }
					m_view->DispLinePoint(_list);
				}
				{
					QVector<sColorCircle> _list;
					for (auto& iter : m_Middle_Param.VecShowColorCircle) { _list.push_back(iter); }
					m_view->DispColorCircle(_list);
				}
				m_view->ClearImgShow();
				for (auto& iter : m_Middle_Param.MapPImgVoid) {
					HObject* _ptrImg = ((HObject*)iter.second);
					if (_ptrImg == nullptr)			continue;
					if (!_ptrImg->IsInitialized())	continue;
					m_view->DispImage(HObject2QImage(*_ptrImg), false);
					QApplication::postEvent(this, new MsgEvent(0xFFFF, 0xFFFF, ""));
				}
				if (m_dstImage.IsInitialized())	{
					m_view->DispImage(HObject2QImage(m_dstImage), false);
					QApplication::postEvent(this, new MsgEvent(0xFFFF, 0xFFFF, ""));
				}
			}
		}
		catch (...) {	}
		return 0;
	});
	while (!values.isFinished()) {
		QApplication::processEvents();
	}
	ExecuteComplete();
	if (m_view != nullptr)	m_view->UpdateImg();
	ui->btnExecute->setEnabled(true);
	if (m_view != nullptr)	m_view->update();
}

void frmBaseTool::on_btnSave_clicked()
{
	Save();
}

void frmBaseTool::slot_MouseMoveInImg(QPoint pos, QString & imgtype, QString & strInfo)
{
	try {
		if (m_dstImage.IsInitialized()) {
			HImage from(m_dstImage);
			HTuple gray;
			if (pos.y() < from.Height() && pos.x() < from.Width()) {
				GetGrayval(m_dstImage, pos.y(), pos.x(), &gray);
			}
			int iChannal = from.CountChannels().I();
			if (iChannal == 1 && gray.Length() > 0) {
				strInfo = QString("Gray:%1")
					.arg(QString::number(gray.D(), 'f', databaseVar::Instance().form_System_Precision));
			}
			else if (iChannal == 2 && gray.Length() > 1) {
				strInfo = QString("Gray:%1,%2")
					.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision));
			}
			else if (iChannal == 3 && gray.Length() > 2) {
				strInfo = QString("R:%1,G:%2,B:%3")
					.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(gray[2].D(), 'f', databaseVar::Instance().form_System_Precision));
			}
			else if (iChannal == 4 && gray.Length() > 3) {
				strInfo = QString("R:%1,G:%2,B:%3")
					.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(gray[2].D(), 'f', databaseVar::Instance().form_System_Precision));
			}
		}
	}
	catch (const std::exception& ex) {
		qCritical() << ex.what();
	}
	catch (const HException& except) {
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				qCritical() << QObject::tr("软件发生异常") << strError;
			}
		}
	}
	catch (...) {
		qCritical() << QObject::tr("软件异常");
	}
}

int frmBaseTool::Load()
{
	return 0;
}

int frmBaseTool::Save()
{
	return 0;
}

int frmBaseTool::NewProject()
{
	return 0;
}

int frmBaseTool::PrExecute()
{
	return 0;
}

int frmBaseTool::ExecuteComplete()
{
	return 0;
}

void frmBaseTool::addShowProperty(QtVariantProperty * property, const QString & id, const QString & strLanguage)
{
	property->setPropertyName(strLanguage);
	m_ShowidToProperty[id] = property;
	m_ShowpropertyToId[property] = id;
	m_ShowpropertyEditor->addProperty(property);
}

void frmBaseTool::initShowEditorProperty(QWidget* parent, QLayout* layout)
{
#pragma region ROI属性
	m_ShowvariantManager = new QtVariantPropertyManager(this);			//相机属性
	m_ShowvariantFactory = new QtVariantEditorFactory(this);
	m_ShowpropertyEditor = new QtTreePropertyBrowser(parent);
	m_ShowpropertyEditor->setFactoryForManager(m_ShowvariantManager, m_ShowvariantFactory);
	layout->addWidget(m_ShowpropertyEditor);
	m_ShowidToProperty.clear();
	m_ShowpropertyToId.clear();
#pragma endregion

	QtVariantProperty* _ptrSubproperty = m_ShowvariantManager->addProperty(QVariant::Int, tr("FontSize"));
	_ptrSubproperty->setValue(QString::number(25));
	_ptrSubproperty->setAttribute(QLatin1String("minimum"), 1);
	_ptrSubproperty->setAttribute(QLatin1String("maximum"), 99999999999999);
	_ptrSubproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
	addShowProperty(_ptrSubproperty, FontSize, tr("FontSize"));
}
