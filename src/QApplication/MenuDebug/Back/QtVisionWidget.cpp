#include "QtVisionWidget.h"
#include <QSerialPortInfo>
#include <QStringList>
#include "Vision/PluginsManager.h"
#include "Vision/CameraManager.h"
#include "Vision/VisionClass.h"
#include "Motion\MotionSerialPort.h"
#include "Pages/QtWaitWidgetsClass.h"
//#include "Motion/MotionRobot.h"
#include "Motion\MotionSerialPort.h"
#include "Motion\MOtionIO.h"
#include "ImageShow/ImageItem.h"
#include "ImageShow/QGraphicsScenes.h"
#include <QtConcurrent>
#include <QElapsedTimer>

void VisionTask::run()
{
	if (m_ptrVision != nullptr) {
		if (m_ptrVision->m_bDealVisionMode)	{
			m_ptrVision->Excute(m_hImage);
		}
		QApplication::postEvent(m_ptrVision, new MsgEvent(0, 0, ""));
	}
}

int VisionTask::RecieveImgBuffer(QString strCamName, unsigned char * gray, int iwidth, int iheight, int ibit, QString imgtype, int icount)
{
	HTuple Type, Width, Height, PixelPointer;

	Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
	if (ibit == 8)
	{
		HalconCpp::GenImage1(&m_hImage, Type, iwidth, iheight, Hlong(gray));
	}
	else if (ibit == 24)
	{
		long imgSize = iwidth * iheight;
		//BYTE* BImg = new BYTE[imgSize];
		//BYTE* GImg = new BYTE[imgSize];
		//BYTE* RImg = new BYTE[imgSize];
		//size_t	Pixed = 0;
		//for (size_t i = 0; i < iheight; i++) {
		//	size_t _Lenth = i * iwidth;
		//	for (size_t j = 0; j < iwidth; j++) {
		//		Pixed = j * 3 + _Lenth * 3;
		//		BImg[j + _Lenth] = gray[Pixed + 0];
		//		GImg[j + _Lenth] = gray[Pixed + 1];
		//		RImg[j + _Lenth] = gray[Pixed + 2];
		//	}
		//}
		int nImgMatType = 3;
		unsigned char *dataRed = new unsigned char[iwidth * iheight];
		unsigned char *dataGreen = new unsigned char[iwidth * iheight];
		unsigned char *dataBlue = new unsigned char[iwidth * iheight];

		for (int i = 0; i<iheight; i++)
		{
			for (int j = 0; j<iwidth; j++)
			{
				dataBlue[iwidth*i + j]	= gray[(iwidth*nImgMatType)*i + j*nImgMatType + 0];
				dataGreen[iwidth*i + j] = gray[(iwidth*nImgMatType)*i + j*nImgMatType + 1];
				dataRed[iwidth*i + j] = gray[(iwidth*nImgMatType)*i + j*nImgMatType + 2];
			}
		}

		GenImage3(&m_hImage, Type, iwidth, iheight, (Hlong)dataRed, (Hlong)dataGreen, (Hlong)dataBlue);
		delete[]dataRed;
		delete[]dataGreen;
		delete[]dataBlue;
	}
	if (m_ptrVision != nullptr)	{
		m_ptrVision->m_dstImage = m_hImage.Clone();;
	}
	return 0;
}

QtVisionWidget::QtVisionWidget(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	initUI();
	InitConnect();
}

QtVisionWidget::~QtVisionWidget()
{
	CloseUI();
}

QImage QtVisionWidget::ByteToQImage(unsigned char* bytes, int width, int height, int bytesPerLine, QImage::Format imageFormat)
{
	return QImage(bytes, width, height, bytesPerLine, imageFormat);
}

int QtVisionWidget::RecieveBuffer(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	if (isVisible())
	{
		VisionTask* _pTask = new VisionTask();
		_pTask->m_ptrVision = this;
		_pTask->setAutoDelete(true);

		if (m_pMarkGraphViews != nullptr) {
			m_pMarkGraphViews->DispImage(ByteToQImage(gray, iwidth, iheight, iwidth * (ibit == 24 ? 3 : 1), ((ibit == 24) ? QImage::Format_RGB32 : QImage::Format_Indexed8)), false);
		}
		_pTask->RecieveImgBuffer(strCamName, gray, iwidth, iheight, ibit, imgtype, icount);

		VisionManager::ThreadPool().start(_pTask);

	}
	return 0;
}

int QtVisionWidget::RecieveRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	return 0;
}

void QtVisionWidget::CheckConConnected()
{
	ui.cbx_CameraName->setEnabled(true);
	if (m_pCamera == nullptr)
	{
		ui.btnConnect->setIcon(QIcon(":/images/skin/images/RedDot.png"));
		ui.btnConnect->setText(tr("Connect"));
	}
	else
	{
		if (m_pCamera->IsCamConnect())
		{
			ui.btnConnect->setIcon(QIcon(":/images/skin/images/GreenDot.png"));
			ui.btnConnect->setText(tr("DisConnect"));
			ui.cbx_CameraName->setEnabled(false);
		}
		else
		{
			ui.btnConnect->setIcon(QIcon(":/images/skin/images/RedDot.png"));
			ui.btnConnect->setText(tr("Connect"));
		}
	}
}

void QtVisionWidget::ConnectCamera()
{
	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera == nullptr) {
		m_pCamera = PluginsManager::Instance().InitCamera(strCam);
	}
	if(strCam.isEmpty())	return;
	if (m_pCamera == nullptr ) {
		dataVar::Instance().pManager->notify(tr("Open Camera Error!"), NotifyType_Error);
		return;
	}
	if (!m_pCamera->IsCamConnect()) {
		m_pCamera->Init();
		m_pCamera->Start();
		m_pCamera->SetStream(this);
		m_pCamera->StartVedioLive();
	}
	if (m_pCamera->IsCamConnect())
	{
		QString	strValue;
		m_pCamera->GetCameraParam(CameraMaxExposure, strValue);
		ui.spBx_Exposure->setMaximum(strValue.toDouble());
		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.spBx_Exposure->setMinimum(strValue.toDouble());
		m_pCamera->SetCameraParam(CameraExposure, QString::number(ui.spBx_Exposure->value() ));
		//ui.spBx_Exposure->setValue(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_Gain->setMaximum(strValue.toDouble());
		ui.Slider_Gain->setMaximum(strValue.toDouble() * 10);
		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_Gain->setMinimum(strValue.toDouble());
		ui.Slider_Gain->setMinimum(strValue.toDouble() * 10);

		m_pCamera->SetCameraParam(CameraGain, QString::number(ui.spBx_Gain->value()));
		//ui.spBx_Gain->setValue(strValue.toDouble());
		//ui.Slider_Gain->setValue(strValue.toDouble() * 10);
	}
	CheckConConnected();
}

void QtVisionWidget::initUI()
{	
	SetSystem("width", 5120);
	SetSystem("height", 5120);
	ResetObjDb(5120, 5120, 3);

	//两视觉
	m_pMarkGraphViews = new QGraphicsViews(this);
	m_pMarkGraphViews->setText(tr("MarkA"));
	connect(m_pMarkGraphViews, SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)), this, SLOT(slotMouseClicked(QList<QGraphicsItem*>&)));

	ui.MarkLayout->addWidget(m_pMarkGraphViews);

	m_VecBaseItem.clear();
	m_VecSearchBaseItem.clear();
	m_VecCodeBaseItem.clear();

	m_vecCommonData.clear();
	m_vecSearchData.clear();
	m_vecCodeData.clear();

	m_pBtnGroupRadio = new QButtonGroup(this);
	m_pBtnGroupRadio->addButton(ui.radioButton_ROI, 0);
	m_pBtnGroupRadio->addButton(ui.radioButton_Search, 1);
	ui.radioButton_ROI->click();
	ui.radioButton_ROI->setChecked(true);
	connect(m_pBtnGroupRadio, SIGNAL(buttonClicked(int)), this, SLOT(slot_btnGroupClicked(int)));

	slotUpDateCameras();

}

bool QtVisionWidget::CheckPageAvalible()
{
	return false;
}

int QtVisionWidget::CloseUI()
{
	if(m_pCamera != nullptr)
	{
		m_pCamera->StopVedioLive();
		m_pCamera->Stop();
		m_pCamera->Exit();
	}
	//MotionIO::getInstance().SetOutPortStatus(0, EnumOutPut_TrrigerLight1 + m_iIndex, false);
	m_pCamera = nullptr;
	if (m_pMarkGraphViews != nullptr)	delete m_pMarkGraphViews;	m_pMarkGraphViews = nullptr;
	return 0;
}

int QtVisionWidget::initData(QString& strError)
{
	m_vecCodeData.clear();
	m_vecCommonData.clear();
	m_vecSearchData.clear();

	CheckConConnected();
	slotUpDateCameras();
	for (size_t i = 0; i < ui.tableCodeWidget->rowCount(); i++)		ui.tableCodeWidget->removeRow(i);
	ui.tableCodeWidget->horizontalHeader()->setVisible(true);

	for (size_t i = 0; i < ui.tableZIFUWidget->rowCount(); i++)		ui.tableZIFUWidget->removeRow(i);
	ui.tableZIFUWidget->horizontalHeader()->setVisible(true);

	return 0;
}

int QtVisionWidget::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("spBx_Exposure",					QString::number(ui.spBx_Exposure->value()));
	CommonParam.insert("spBx_Gain",						QString::number(ui.spBx_Gain->value()));
	CommonParam.insert("spBx_Light",					QString::number(ui.spBx_Light->value()));
	CommonParam.insert("cBx_Channel",					QString::number(ui.cBx_Channel->currentIndex()));
	CommonParam.insert("cbx_CameraName",				QString(ui.cbx_CameraName->currentText()));
	dataVar::Instance().mapCameraName[m_iIndex]			= ui.cbx_CameraName->currentText();
	//Mark点
	CommonParam.insert("spinRoiW",						QString::number(ui.spinRoiW->value()));
	CommonParam.insert("m_MarkAResultPosfMatchX",		QString::number(m_MarkAResultPos.fMatchX));
	CommonParam.insert("m_MarkAResultPosfMatchY",		QString::number(m_MarkAResultPos.fMatchY));
	CommonParam.insert("m_MarkAResultPosfMatchAngle",	QString::number(m_MarkAResultPos.fMatchAngle));
	CommonParam.insert("spBx_JudgeScore",				QString::number(ui.spBx_JudgeScore->value()));

	m_vecCommonData.clear();
	for (auto iter : m_VecBaseItem){
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			CommonData _data;
			_data.type = Item->GetItemType();
			switch (Item->GetItemType())
			{
			case ItemCircle: {
				CircleItem *pitem = (CircleItem*)Item;		MCircle Cir;
				pitem->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
			}	break;
			case ItemEllipse: {
				EllipseItem *pitem = (EllipseItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *pitem = (ConcentricCircleItem*)Item;		CCircle Cir;
				pitem->GetConcentricCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.small_radius))
					.arg(QString::number(Cir.big_radius));
			}	break;
			case ItemRectangle: {
				RectangleItem *pitem = (RectangleItem*)Item;		MRectangle Cir;
				pitem->GetRect(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.height));
			}	break;
			case ItemRectangleR: {
				RectangleRItem *pitem = (RectangleRItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
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
				PolygonItem *pitem = (PolygonItem*)Item;		MPolygon Pointlst;
				pitem->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++)
				{
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("_");
				}
			}	break;
			case ItemLineObj: {
				LineItem *pitem = (LineItem*)Item;		LineData Cir;
				pitem->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			}
			_data.eRegionType = Item->m_eRegionType;
			m_vecCommonData.push_back(_data);
		}
	}

	QJsonArray ROIConFig;
	for (auto& iter : m_vecCommonData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		ROIConFig.append(_Object);
	}
	CommonParam.insert("ROIConFig", ROIConFig);

	m_vecSearchData.clear();
	for (auto iter : m_VecSearchBaseItem){
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			CommonData _data;
			_data.type = Item->GetItemType();
			switch (Item->GetItemType()){
			case ItemCircle: {
				CircleItem *pitem = (CircleItem*)Item;		MCircle Cir;
				pitem->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
			}	break;
			case ItemEllipse: {
				EllipseItem *pitem = (EllipseItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *pitem = (ConcentricCircleItem*)Item;		CCircle Cir;
				pitem->GetConcentricCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.small_radius))
					.arg(QString::number(Cir.big_radius));
			}	break;
			case ItemRectangle: {
				RectangleItem *pitem = (RectangleItem*)Item;		MRectangle Cir;
				pitem->GetRect(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.height));
			}	break;
			case ItemRectangleR: {
				RectangleRItem *pitem = (RectangleRItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
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
				PolygonItem *pitem = (PolygonItem*)Item;		MPolygon Pointlst;
				pitem->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++){
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("_");
				}
			}	break;
			case ItemLineObj: {
				LineItem *pitem = (LineItem*)Item;		LineData Cir;
				pitem->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			}
			_data.eRegionType = Item->m_eRegionType;
			m_vecSearchData.push_back(_data);
		}
	}

	QJsonArray SearchROIConFig;
	for (auto& iter : m_vecSearchData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		SearchROIConFig.append(_Object);
	}
	CommonParam.insert("SearchROIConFig", SearchROIConFig);

	m_vecCodeData.clear();
	for (auto iter : m_VecCodeBaseItem) {
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			CommonData _data;
			_data.type = Item->GetItemType();
			switch (Item->GetItemType()) {
			case ItemCircle: {
				CircleItem *pitem = (CircleItem*)Item;		MCircle Cir;
				pitem->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
			}	break;
			case ItemEllipse: {
				EllipseItem *pitem = (EllipseItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *pitem = (ConcentricCircleItem*)Item;		CCircle Cir;
				pitem->GetConcentricCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.small_radius))
					.arg(QString::number(Cir.big_radius));
			}	break;
			case ItemRectangle: {
				RectangleItem *pitem = (RectangleItem*)Item;		MRectangle Cir;
				pitem->GetRect(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.height));
			}	break;
			case ItemRectangleR: {
				RectangleRItem *pitem = (RectangleRItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
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
				PolygonItem *pitem = (PolygonItem*)Item;		MPolygon Pointlst;
				pitem->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++) {
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("_");
				}
			}	break;
			case ItemLineObj: {
				LineItem *pitem = (LineItem*)Item;		LineData Cir;
				pitem->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			}
			_data.eRegionType = Item->m_eRegionType;
			m_vecCodeData.push_back(_data);
		}
	}

	QJsonArray CodeDataConFig;
	for (auto& iter : m_vecCodeData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		CodeDataConFig.append(_Object);
	}
	CommonParam.insert("CodeDataConFig", CodeDataConFig);

	m_vecZIFUData.clear();
	for (auto iter : m_VecZIFUBaseItem) {
		BaseItem* Item = (BaseItem*)iter;
		if (iter != nullptr) if (iter->type() == 10) {
			CommonData _data;
			_data.type = Item->GetItemType();
			switch (Item->GetItemType()) {
			case ItemCircle: {
				CircleItem *pitem = (CircleItem*)Item;		MCircle Cir;
				pitem->GetCircle(Cir);
				_data.strData = QString("%1_%2_%3").arg(QString::number(Cir.row)).arg(QString::number(Cir.col)).arg(QString::number(Cir.radius));
			}	break;
			case ItemEllipse: {
				EllipseItem *pitem = (EllipseItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
				_data.strData = QString("%1_%2_%3_%4_%5")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.phi))
					.arg(QString::number(Cir.lenth1))
					.arg(QString::number(Cir.lenth2));
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *pitem = (ConcentricCircleItem*)Item;		CCircle Cir;
				pitem->GetConcentricCircle(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.small_radius))
					.arg(QString::number(Cir.big_radius));
			}	break;
			case ItemRectangle: {
				RectangleItem *pitem = (RectangleItem*)Item;		MRectangle Cir;
				pitem->GetRect(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.width))
					.arg(QString::number(Cir.height));
			}	break;
			case ItemRectangleR: {
				RectangleRItem *pitem = (RectangleRItem*)Item;		MRotatedRect Cir;
				pitem->GetRotatedRect(Cir);
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
				PolygonItem *pitem = (PolygonItem*)Item;		MPolygon Pointlst;
				pitem->GetPolygon(Pointlst);
				_data.strData.clear();
				//多边形
				for (int i = 0; i < Pointlst.points.size(); i++) {
					_data.strData.append(QString("%1,%2").arg(Pointlst.points[i].x()).arg(Pointlst.points[i].y()));
					_data.strData.append("_");
				}
			}	break;
			case ItemLineObj: {
				LineItem *pitem = (LineItem*)Item;		LineData Cir;
				pitem->GetLineData(Cir);
				_data.strData = QString("%1_%2_%3_%4")
					.arg(QString::number(Cir.row))
					.arg(QString::number(Cir.col))
					.arg(QString::number(Cir.row1))
					.arg(QString::number(Cir.col1));
			}	break;
			}
			_data.eRegionType = Item->m_eRegionType;
			m_vecZIFUData.push_back(_data);
		}
	}

	QJsonArray ZIFUDataConFig;
	for (auto& iter : m_vecZIFUData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		ZIFUDataConFig.append(_Object);
	}
	CommonParam.insert("ZIFUDataConFig", ZIFUDataConFig);

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableCodeWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableCodeWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableCodeWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableCodeWidget->cellWidget(iRow, iCol);
				if (currentItem != nullptr)
					ConFig.append(currentItem->text());
				else if (_pQWidget != nullptr) {
					QString	strClass = _pQWidget->metaObject()->className();
					if (strClass == "QDoubleSpinBox") {
						ConFig.append(QString::number(((QDoubleSpinBox*)_pQWidget)->value()));
					}
					else if (strClass == "QLabel") {
						ConFig.append(QString(((QLabel*)_pQWidget)->text()));
					}
					else if (strClass == "QComboBox") {
						ConFig.append(QString::number(((QComboBox*)_pQWidget)->currentIndex()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("CodeParam", PositionParam);
	}

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableZIFUWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableZIFUWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableZIFUWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableZIFUWidget->cellWidget(iRow, iCol);
				if (currentItem != nullptr)
					ConFig.append(currentItem->text());
				else if (_pQWidget != nullptr) {
					QString	strClass = _pQWidget->metaObject()->className();
					if (strClass == "QDoubleSpinBox") {
						ConFig.append(QString::number(((QDoubleSpinBox*)_pQWidget)->value()));
					}
					else if (strClass == "QLabel") {
						ConFig.append(QString(((QLabel*)_pQWidget)->text()));
					}
					else if (strClass == "QComboBox") {
						ConFig.append(QString::number(((QComboBox*)_pQWidget)->currentIndex()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("ZIFUParam", PositionParam);
	}
	strData.insert("QtVisionWidget" + QString::number(m_iIndex), CommonParam);
	UpDateParam();
	return 0;
}

int QtVisionWidget::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtVisionWidget" + QString::number(m_iIndex)).value().toObject();
	if (CommonParam.contains("spBx_Exposure"))					ui.spBx_Exposure->setValue(CommonParam["spBx_Exposure"].toString().toDouble());
	if (CommonParam.contains("spBx_Gain"))						ui.spBx_Gain->setValue(CommonParam["spBx_Gain"].toString().toDouble());
	if (CommonParam.contains("spBx_Light"))						ui.spBx_Light->setValue(CommonParam["spBx_Light"].toString().toDouble());
	if (CommonParam.contains("cBx_Channel"))					ui.cBx_Channel->setCurrentIndex(CommonParam["cBx_Channel"].toString().toDouble());
	if (CommonParam.contains("cbx_CameraName"))					ui.cbx_CameraName->setCurrentText(CommonParam["cbx_CameraName"].toString()	);
	if (CommonParam.contains("spinRoiW"))						ui.spinRoiW->setValue(CommonParam["spinRoiW"].toString().toInt());
	if (CommonParam.contains("spBx_JudgeScore"))				ui.spBx_JudgeScore->setValue(CommonParam["spBx_JudgeScore"].toString().toDouble());

	//将区域转化成ROI
	QJsonArray array_Roiobject = CommonParam.find("ROIConFig").value().toArray();
	m_vecCommonData.clear();
	if (!array_Roiobject.empty()) {
		for (size_t i = 0; i < array_Roiobject.count(); i++) {
			CommonData _CommonData;
			QJsonArray _Object = array_Roiobject.at(i).toArray();
			_CommonData.type = (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType = (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData = _Object.at(2).toString();
			m_vecCommonData.push_back(_CommonData);
		}
	}

	QJsonArray array_Searchobject = CommonParam.find("SearchROIConFig").value().toArray();
	m_vecSearchData.clear();
	if (!array_Searchobject.empty()) {
		for (size_t i = 0; i < array_Searchobject.count(); i++) {
			CommonData _CommonData;
			QJsonArray _Object = array_Searchobject.at(i).toArray();
			_CommonData.type = (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType = (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData = _Object.at(2).toString();
			m_vecSearchData.push_back(_CommonData);
		}
	}

	QJsonArray CodeDataConFig = CommonParam.find("CodeDataConFig").value().toArray();
	m_vecCodeData.clear();
	if (!CodeDataConFig.empty()) {
		for (size_t i = 0; i < CodeDataConFig.count(); i++) {
			CommonData _CommonData;
			QJsonArray _Object			= CodeDataConFig.at(i).toArray();
			_CommonData.type			= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType		= (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData			= _Object.at(2).toString();
			m_vecCodeData.push_back(_CommonData);
		}
	}

	QJsonArray ZIFUDataConFig = CommonParam.find("ZIFUDataConFig").value().toArray();
	m_vecZIFUData.clear();
	if (!ZIFUDataConFig.empty()) {
		for (size_t i = 0; i < ZIFUDataConFig.count(); i++) {
			CommonData _CommonData;
			QJsonArray _Object		= ZIFUDataConFig.at(i).toArray();
			_CommonData.type		= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType = (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData		= _Object.at(2).toString();
			m_vecZIFUData.push_back(_CommonData);
		}
	}

	float fData[5] = { 0 };
	//ROI区域
	on_btnDeleteROIAll_clicked();
	for (int i = 0; i < m_vecCommonData.size(); i++) {
		CommonData&Data = m_vecCommonData[i];
		QStringList dArray = Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();

		switch (Data.type) {
		case ItemCircle: {
			CircleItem *Circle = new CircleItem(fData[1], fData[0], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemEllipse: {
			EllipseItem *Circle = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *Circle = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemRectangle: {
			RectangleItem *Circle = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemRectangleR: {
			RectangleRItem *Circle = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			PolygonItem *Circle = new PolygonItem();
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		case ItemLineObj: {
			LineItem *Circle = new LineItem(fData[1], fData[0], fData[3], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecBaseItem.push_back(Circle);
		}; break;
		default:	break;
		}
	}
	//搜索区域
	on_btnDeleteSearchROIAll_clicked();
	for (int i = 0; i < m_vecSearchData.size(); i++) {
		CommonData&Data = m_vecSearchData[i];
		QStringList dArray = Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();

		switch (Data.type) {
		case ItemCircle: {
			CircleItem *Circle = new CircleItem(fData[1], fData[0], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemEllipse: {
			EllipseItem *Circle = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *Circle = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemRectangle: {
			RectangleItem *Circle = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemRectangleR: {
			RectangleRItem *Circle = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			PolygonItem *Circle = new PolygonItem();
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		case ItemLineObj: {
			LineItem *Circle = new LineItem(fData[1], fData[0], fData[3], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecSearchBaseItem.push_back(Circle);
		}; break;
		default:	break;
		}
	}

	//搜索区域
	on_btnDeleteCodeROIAll_clicked();
	memset(fData, 0, sizeof(float) * 5);
	for (int i = 0; i < m_vecCodeData.size(); i++)	{
		CommonData&Data			= m_vecCodeData[i];
		QStringList dArray		= Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();

		switch (Data.type) {
		case ItemCircle: {
			CircleItem *Circle = new CircleItem(fData[1], fData[0], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemEllipse: {
			EllipseItem *Circle = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *Circle = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemRectangle: {
			RectangleItem *Circle = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemRectangleR: {
			RectangleRItem *Circle = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			PolygonItem *Circle = new PolygonItem();
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		case ItemLineObj: {
			LineItem *Circle = new LineItem(fData[1], fData[0], fData[3], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecCodeBaseItem.push_back(Circle);
		}; break;
		default:	break;
		}
	}

	//字符识别区域
	//on_btnDeleteZIFUROIAll_clicked();
	memset(fData, 0, sizeof(float) * 5);
	for (int i = 0; i < m_vecZIFUData.size(); i++) {
		CommonData&Data = m_vecZIFUData[i];
		QStringList dArray = Data.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();

		switch (Data.type) {
		case ItemCircle: {
			CircleItem *Circle = new CircleItem(fData[1], fData[0], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemEllipse: {
			EllipseItem *Circle = new EllipseItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemConcentric_Circle: {
			ConcentricCircleItem *Circle = new ConcentricCircleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemRectangle: {
			RectangleItem *Circle = new RectangleItem(fData[1], fData[0], fData[2], fData[3]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemRectangleR: {
			RectangleRItem *Circle = new RectangleRItem(fData[1], fData[0], fData[3] * 2, fData[4] * 2, fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemSquare: {
		}; break;
		case ItemPolygon: {
			PolygonItem *Circle = new PolygonItem();
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		case ItemLineObj: {
			LineItem *Circle = new LineItem(fData[1], fData[0], fData[3], fData[2]);
			Circle->m_eRegionType = Data.eRegionType;
			m_pMarkGraphViews->AddItems(Circle);
			m_VecZIFUBaseItem.push_back(Circle);
		}; break;
		default:	break;
		}
	}


	ui.radioButton_ROI->setChecked(true);
	slot_btnGroupClicked(m_pBtnGroupRadio->checkedId());
	{
		int iRowCount = ui.tableCodeWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableCodeWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("CodeParam").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableCodeWidget->insertRow(iRow);
			int	_iCode = 0;
			for (int iCol = 0; iCol < CParam.count(); iCol++) {
				QString strValue = CParam.at(iCol).toString();
				QWidget* _pQWidget = nullptr;
				QTableWidgetItem* _pItem = nullptr;
				switch (iCol) {
				case 0: {
					_pItem = new QTableWidgetItem();
					_pItem->setTextAlignment(Qt::AlignCenter);
					_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
					_pItem->setText(QString::number(iRow));
					_pItem->setToolTip(QString::number(iRow));
				}	break;
				case 1: {
					_pItem = new QTableWidgetItem();
					_pItem->setTextAlignment(Qt::AlignCenter);
					_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
					_pItem->setText(strValue);
					_pItem->setToolTip(strValue);
				}	break;
				case 2: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("一维码"));
					((QComboBox*)_pQWidget)->addItem(tr("二维码"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
					_iCode = strValue.toDouble();
				}	break;
				case 3: {
					_pQWidget = new QComboBox();
					QStringList	_codeLst;
					switch (_iCode) {
					case 0: {
						_codeLst.append(tr("25"));
						_codeLst.append(tr("39"));
						_codeLst.append(tr("128"));
					}break;
					case 1: {
						_codeLst.append(tr("QR"));
						_codeLst.append(tr("DM"));
						_codeLst.append(tr("MQR"));
					}break;
					default:			break;
					}
					((QComboBox*)_pQWidget)->addItems(_codeLst);
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
					//((QComboBox*)_pQWidget)->setCurrentText(strValue);
				}	break;
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 7: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("Disable"));
					((QComboBox*)_pQWidget)->addItem(tr("Enable"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					ui.tableCodeWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableCodeWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}
	{
		int iRowCount = ui.tableZIFUWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableZIFUWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("ZIFUParam").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableZIFUWidget->insertRow(iRow);
			int	_iCode = 0;
			for (int iCol = 0; iCol < CParam.count(); iCol++) {
				QString strValue = CParam.at(iCol).toString();
				QWidget* _pQWidget = nullptr;
				QTableWidgetItem* _pItem = nullptr;
				switch (iCol) {
				case 0: {
					_pItem = new QTableWidgetItem();
					_pItem->setTextAlignment(Qt::AlignCenter);
					_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
					_pItem->setText(QString::number(iRow));
					_pItem->setToolTip(QString::number(iRow));
				}	break;
				case 1: {
					_pItem = new QTableWidgetItem();
					_pItem->setTextAlignment(Qt::AlignCenter);
					_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
					_pItem->setText(strValue);
					_pItem->setToolTip(strValue);
				}	break;
				case 2: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("一维码"));
					((QComboBox*)_pQWidget)->addItem(tr("二维码"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
					_iCode = strValue.toDouble();
				}	break;
				case 3: {
					_pQWidget = new QComboBox();
					QStringList	_codeLst;
					switch (_iCode) {
					case 0: {
						_codeLst.append(tr("25"));
						_codeLst.append(tr("39"));
						_codeLst.append(tr("128"));
					}break;
					case 1: {
						_codeLst.append(tr("QR"));
						_codeLst.append(tr("DM"));
						_codeLst.append(tr("MQR"));
					}break;
					default:			break;
					}
					((QComboBox*)_pQWidget)->addItems(_codeLst);
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
					//((QComboBox*)_pQWidget)->setCurrentText(strValue);
				}	break;
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 7: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("Disable"));
					((QComboBox*)_pQWidget)->addItem(tr("Enable"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					ui.tableZIFUWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableZIFUWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	UpDateParam();
	if (dataVar::Instance().bAutoConnectedWhenStart){
		if (dataVar::Instance().bUseVision) {
			ConnectCamera();
		}
	}
	return 0;
}

int QtVisionWidget::NewProject()
{
	return 0;
}

void QtVisionWidget::InitConnect()
{
	connect(ui.cbx_CameraName,	SIGNAL(sigPopup()),					this,	SLOT(slotUpDateCameras()));
	connect(ui.Slider_Exposure, SIGNAL(valueChanged(int)),			this,	SLOT(slotUpDateExposure(int)));
	connect(ui.Slider_Gain,		SIGNAL(valueChanged(int)),			this,	SLOT(slotUpDateGain(int)));
	connect(ui.Slider_Light,	SIGNAL(sliderReleased()),			this,	SLOT(slotUpDateLight()));

	connect(ui.Slider_Gain,		&QSlider::valueChanged,				this, [=](int value) {
		ui.spBx_Gain->setValue(value * 0.1);
	});
	connect(ui.spBx_Gain,		&QDoubleSpinBox::editingFinished,	this, [=]() {
		ui.Slider_Gain->setValue(ui.spBx_Gain->value() * 10);
	});
	connect(ui.Slider_JudgeScore, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_JudgeScore->setValue(value * 0.01);
	});
	connect(ui.spBx_JudgeScore, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_JudgeScore->setValue(ui.spBx_JudgeScore->value() * 100);
	});

	connect(ui.tabModelWidget,					SIGNAL(currentChanged(int)),		this, SLOT(slot_ChangeMarkModelTab(int)));
	connect(ui.cbx_MatchType,					SIGNAL(currentIndexChanged(int)),	this, SLOT(slot_ChangeMatchType(int)));

	connect(ui.spBx_FindGreedy,					SIGNAL(valueChanged(double)),		this, SLOT(slot_SetGreedyChanged(double)));
	connect(ui.spBx_FindMinOverLap,				SIGNAL(valueChanged(double)),		this, SLOT(slot_SetMinOverLapChanged(double)));
	connect(ui.spBx_FindMinScore,				SIGNAL(valueChanged(double)),		this, SLOT(slot_SetMinScoreChanged(double)));
	connect(ui.tabWidget,						SIGNAL(currentChanged(int)),		this, SLOT(slot_ChangeTab(int)));

	connect(ui.hSliderGreedy,					&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindGreedy->setValue(value * 0.01);	});
	connect(ui.hSliderMinOverLap,				&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindMinOverLap->setValue(value * 0.01);	});
	connect(ui.hSlider_MinScore,				&QSlider::valueChanged, this, [this](int value) {	ui.spBx_FindMinScore->setValue(value * 0.01);	});
	connect(ui.spinRoiW,						SIGNAL(valueChanged(int)), this, SLOT(onRoiWidthChanged(int)));

	QObject::connect(ui.spBx_FindMinNumber,		SIGNAL(valueChanged(int)), ui.horizontalSlider_9,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_9,		SIGNAL(valueChanged(int)), ui.spBx_FindMinNumber,	SLOT(setValue(int)));
	QObject::connect(ui.spBx_FindAngleStart,	SIGNAL(valueChanged(int)), ui.horizontalSlider_11,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_11,	SIGNAL(valueChanged(int)), ui.spBx_FindAngleStart,	SLOT(setValue(int)));
	QObject::connect(ui.spBx_FindAngleRangle,	SIGNAL(valueChanged(int)), ui.horizontalSlider_12,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_12,	SIGNAL(valueChanged(int)), ui.spBx_FindAngleRangle, SLOT(setValue(int)));
	QObject::connect(ui.spBx_PyLevel,			SIGNAL(valueChanged(int)), ui.horizontalSlider_4,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_4,		SIGNAL(valueChanged(int)), ui.spBx_PyLevel,			SLOT(setValue(int)));
	QObject::connect(ui.spBx_CAngleStart,		SIGNAL(valueChanged(int)), ui.horizontalSlider_5,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_5,		SIGNAL(valueChanged(int)), ui.spBx_CAngleStart,		SLOT(setValue(int)));
	QObject::connect(ui.spBx_CAngleRange,		SIGNAL(valueChanged(int)), ui.horizontalSlider_6,	SLOT(setValue(int)));
	QObject::connect(ui.horizontalSlider_6,		SIGNAL(valueChanged(int)), ui.spBx_CAngleRange,		SLOT(setValue(int)));

	connect(ui.cBx_CodeType,					SIGNAL(currentIndexChanged(int)),	this,			SLOT(slotChangeCodeType(int)));
	connect(ui.tableCodeWidget,					SIGNAL(cellClicked(int,int)),		this,			SLOT(slot_cellClicked(int, int)));

	connect(ui.Slider_CodeROI, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_CodeROI->setValue(value );
	});
	connect(ui.spBx_CodeROI, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_CodeROI->setValue(ui.spBx_CodeROI->value());
	});
	connect(ui.Slider_CodeLight, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_CodeLight->setValue(value);
	});
	connect(ui.spBx_CodeLight, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_CodeLight->setValue(ui.spBx_CodeLight->value());
	});

	connect(ui.Slider_CodeExposure, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_CodeExposure->setValue(value);
	});
	connect(ui.spBx_CodeExposure, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_CodeExposure->setValue(ui.spBx_CodeExposure->value());
	});
	connect(ui.Slider_CodeGain, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_CodeGain->setValue(value * 0.1);
	});
	connect(ui.spBx_CodeGain, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_CodeGain->setValue(ui.spBx_CodeGain->value() * 10);
	});
}

int QtVisionWidget::GetSystemData(QJsonObject & strData)
{
	//串口
	QJsonObject CommonParam;

	strData.insert("QtVisionWidget", CommonParam);

	return 0;
}

int QtVisionWidget::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtVisionWidget").value().toObject();

	CheckConConnected();
	return 0;
}

bool QtVisionWidget::CheckCylinderSafety()
{
	int _iCard = 0;
	//QString strError;
	//if (!MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin1)) {
	//	strError.append(MotionIO::getInstance().GetOutName(_iCard, EnumOutPut_Cylinder1));
	//}
	//if (!MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin2)) {
	//	if (!strError.isEmpty()) strError.append(",");
	//	strError.append(MotionIO::getInstance().GetOutName(_iCard, EnumOutPut_Cylinder2));
	//}
	//if (!MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin3)) {
	//	if (!strError.isEmpty()) strError.append(",");
	//	strError.append(MotionIO::getInstance().GetOutName(_iCard, EnumOutPut_Cylinder3));
	//}
	//if (!strError.isEmpty())
	//{
	//	auto res = MyMessageBox::question(this, strError + tr(" Cylinder Up Error! Are You Sure Continue!"), tr("Alart"));
	//	if (res == MyMessageBox::Yes)
	//	{
	//		StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
	//			MotionIO::getInstance().SetOutPortStatus(_iCard, EnumOutPut_Cylinder1, false);
	//			MotionIO::getInstance().SetOutPortStatus(_iCard, EnumOutPut_Cylinder2, false);
	//			MotionIO::getInstance().SetOutPortStatus(_iCard, EnumOutPut_Cylinder3, false);
	//			QElapsedTimer	_timer;
	//			_timer.start();
	//			bool _bTimeOut = false;
	//			while (true)
	//			{
	//				double	fExcuteTime = _timer.nsecsElapsed() / 1000000;
	//				if (fExcuteTime > dataVar::Instance().Param.CylinderWaitDelay)
	//				{
	//					_bTimeOut = true;
	//					break;	//超时
	//				}
	//				if (MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin1)
	//					&& MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin2)
	//					&& MotionIO::getInstance().GetInPortStatus(_iCard, EnumInPut_CylinderOrgin3))
	//				{
	//					break;	//超时
	//				}
	//			}
	//			if (_bTimeOut) {
	//				return  StandardMsgResult::MsgResult_TimeOut;
	//			}
	//			return  StandardMsgResult::MsgResult_OK;
	//		}, tr("Close Cylinder Now !"), QtWaitWidgetsClass::MsgType_Close);
	//		switch (result)	{
	//		case MsgResult_NG:
	//		case MsgResult_Close:
	//		case MsgResult_TimeOut:	return false;
	//		default:
	//		case MsgResult_OK:			break;
	//		}
	//	}
	//	else
	//	{
	//		return false;
	//	}

	//}
	return true;
}

int QtVisionWidget::ShapesToRegion(QVector<BaseItem*>& vecItems, HObject & Regions)
{
	Regions.Clear();
	Regions.GenEmptyObj();
	for (auto iter : vecItems)  {
		BaseItem* Item = (BaseItem*)iter;
		HObject	_Region;	//ROI形状区域
		if (iter != nullptr)
			if (iter->type() == 10)	{
			switch (Item->GetItemType()){
			case ItemCircle: {
				CircleItem *_Item = (CircleItem*)Item;						
				MCircle _data;
				_Item->GetCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.radius);
			}	break;
			case ItemEllipse: {
				EllipseItem *_Item = (EllipseItem*)Item;				
				MRotatedRect _data;
				_Item->GetRotatedRect(_data);
				GenEllipse(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_Item = (ConcentricCircleItem*)Item;	
				CCircle _data;
				_Item->GetConcentricCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.big_radius);	
				//WriteObject(_Region, "E:/Robot/_Region.hobj");

				HObject	_Region1;	//ROI形状区域
				GenCircle(&_Region1, _data.row, _data.col, _data.small_radius);
				//WriteObject(_Region1, "E:/Robot/_Region1.hobj");
				Difference(_Region, _Region1, &_Region);
				//WriteObject(_Region, "E:/Robot/_DifferenceRegion.hobj");

			}	break;
			case ItemRectangle: {
				RectangleItem *_Item = (RectangleItem*)Item;			
				MRectangle _data;
				_Item->GetRect(_data);
				GenRectangle1(&_Region, _data.row, _data.col, _data.row + _data.height, _data.col + _data.width);
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_Item = (RectangleRItem*)Item;				
				MRotatedRect _data;
				_Item->GetRotatedRect(_data);
				GenRectangle2(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemPolygon: {
				PolygonItem *_Item = (PolygonItem*)Item;				
				MPolygon Pointlst;
				_Item->GetPolygon(Pointlst);
				HTuple hRow, hColumns;
				for (int i = 0; i < Pointlst.points.size(); i++)	{
					hRow.Append(Pointlst.points[i].x());
					hColumns.Append(Pointlst.points[i].y());
				}
				GenRegionPolygonFilled(&_Region, hRow, hColumns);
			}	break;
			case ItemLineObj: {
				LineItem *_Item = (LineItem*)Item;						
				LineData _data;
				_Item->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case ItemArrowLineObj: {
				LineItem *_Item = (LineItem*)Item;		
				LineData _data;
				_Item->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			}
			switch (Item->m_eRegionType){
			case RegionType_Add: {		Union2(Regions, _Region, &Regions);	} break;
			case RegionType_Differe: {	Difference(Regions, _Region, &Regions);	} break;
			}
		}
	}
	return 0;
}

int QtVisionWidget::ShapeToRegion(BaseItem*& vecItems, HObject & Regions)
{
	Regions.Clear();
	Regions.GenEmptyObj();

	BaseItem* Item = (BaseItem*)vecItems;
	HObject	_Region;	//ROI形状区域
	if (vecItems != nullptr)
		if (vecItems->type() == 10) {
			switch (Item->GetItemType()) {
			case ItemCircle: {
				CircleItem *_Item = (CircleItem*)Item;
				MCircle _data;
				_Item->GetCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.radius);
			}	break;
			case ItemEllipse: {
				EllipseItem *_Item = (EllipseItem*)Item;
				MRotatedRect _data;
				_Item->GetRotatedRect(_data);
				GenEllipse(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemConcentric_Circle: {
				ConcentricCircleItem *_Item = (ConcentricCircleItem*)Item;
				CCircle _data;
				_Item->GetConcentricCircle(_data);
				GenCircle(&_Region, _data.row, _data.col, _data.big_radius);
				//WriteObject(_Region, "E:/Robot/_Region.hobj");

				HObject	_Region1;	//ROI形状区域
				GenCircle(&_Region1, _data.row, _data.col, _data.small_radius);
				//WriteObject(_Region1, "E:/Robot/_Region1.hobj");
				Difference(_Region, _Region1, &_Region);
				//WriteObject(_Region, "E:/Robot/_DifferenceRegion.hobj");

			}	break;
			case ItemRectangle: {
				RectangleItem *_Item = (RectangleItem*)Item;
				MRectangle _data;
				_Item->GetRect(_data);
				GenRectangle1(&_Region, _data.row, _data.col, _data.row + _data.height, _data.col + _data.width);
			}	break;
			case ItemRectangleR: {
				RectangleRItem *_Item = (RectangleRItem*)Item;
				MRotatedRect _data;
				_Item->GetRotatedRect(_data);
				GenRectangle2(&_Region, _data.row, _data.col, _data.phi, _data.lenth1, _data.lenth2);
			}	break;
			case ItemPolygon: {
				PolygonItem *_Item = (PolygonItem*)Item;
				MPolygon Pointlst;
				_Item->GetPolygon(Pointlst);
				HTuple hRow, hColumns;
				for (int i = 0; i < Pointlst.points.size(); i++) {
					hRow.Append(Pointlst.points[i].x());
					hColumns.Append(Pointlst.points[i].y());
				}
				GenRegionPolygonFilled(&_Region, hRow, hColumns);
			}	break;
			case ItemLineObj: {
				LineItem *_Item = (LineItem*)Item;
				LineData _data;
				_Item->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			case ItemArrowLineObj: {
				LineItem *_Item = (LineItem*)Item;
				LineData _data;
				_Item->GetLineData(_data);
				GenRegionLine(&_Region, _data.row, _data.col, _data.row1, _data.col1);
			}	break;
			}
			Regions = _Region;
			//switch (Item->m_eRegionType) {
			//case RegionType_Add: {		Union2(Regions, _Region, &Regions);	} break;
			//case RegionType_Differe: {	Difference(Regions, _Region, &Regions);	} break;
			//}
		}
	return 0;
}

void QtVisionWidget::TableToStruct(QTableWidget * table, QVector<VisionStruct>& vecVision)
{
	vecVision.clear();
	int iRowCount = table->rowCount();
	for (size_t iRow = 0; iRow < iRowCount; iRow++)
	{
		VisionStruct _data;
		_data.iCodeType		= ((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 2))->currentIndex();
		_data.iCode			= ((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 3))->currentIndex();
		_data.iExposure		= ((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 4))->value();
		_data.dGain			= ((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 5))->value();
		vecVision.push_back(_data);
	}
}

void QtVisionWidget::TableToZIFUStruct(QTableWidget * table, QVector<ZIFUVisionStruct>& vecVision)
{
	vecVision.clear();
	int iRowCount = table->rowCount();
	for (size_t iRow = 0; iRow < iRowCount; iRow++)
	{
		ZIFUVisionStruct _data;
		_data.dScore = ((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 2))->currentIndex();
		//_data.iCode = ((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 3))->currentIndex();
		//_data.iExposure = ((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 4))->value();
		//_data.dGain = ((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 5))->value();
		vecVision.push_back(_data);
	}
}

void QtVisionWidget::slotMouseClicked(QList<QGraphicsItem*>& items)
{
	if ( items.size() > 0)
	{
		QJsonObject	Json;
		QJsonDocument	root_document;
		for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++)		{
			if (m_VecCodeBaseItem[i] == ((BaseItem*)items[0]))		{
				((BaseItem*)items[0])->GetData(Json);
				root_document.setObject(Json);
				QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
				QString	strText = QString(root_string_compact);
				ui.tableCodeWidget->item(i, 1)->setText(strText);
				ui.tableCodeWidget->selectRow(i);
			}
		}
		for (size_t i = 0; i < m_VecZIFUBaseItem.size(); i++) {
			if (m_VecZIFUBaseItem[i] == ((BaseItem*)items[0])) {
				((BaseItem*)items[0])->GetData(Json);
				root_document.setObject(Json);
				QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
				QString	strText = QString(root_string_compact);
				ui.tableZIFUWidget->item(i, 1)->setText(strText);
				ui.tableZIFUWidget->selectRow(i);
			}
		}

	}

}

void QtVisionWidget::on_btnTest_clicked()
{
	QString strCam = ui.cbx_CameraName->currentText();
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getOpenFileName(this, tr("打开图片"), path_C, "*.bmp *png *jpg *.jpeg *.tif *.tiff");
	if (false == dirPath.isEmpty())
	{
		HalconCpp::HObject		hImage;
		ReadImage(&hImage, dirPath.toStdString().c_str());
		//ReadImage(&hImage, "D:\\123.bmp");
		HalconCpp::HImage		hImageObject(hImage);
		HString Type; 
		Hlong Width, Height;

		switch (hImageObject.CountChannels().I())	{
		case 1: {
			void* pPtr = hImageObject.GetImagePointer1(&Type, &Width, &Height);
			RecieveBuffer(strCam, (unsigned char*)pPtr, nullptr, Width, Height, 8 * hImageObject.CountChannels().I(), 1, "", 0);
		}	break;
		case 3: {
			void* PointerRed; 
			void* PointerGreen;
			void* PointerBlue;
			HString Type;
			Hlong Width, Height;
			hImageObject.GetImagePointer3(&PointerRed,&PointerGreen,&PointerBlue,&Type,&Width,&Height);
			hImageObject.InterleaveChannels("rgb", "match", 255);
			void* pPtr = hImageObject.GetImagePointer1(&Type, &Width, &Height);
			RecieveBuffer(strCam, (unsigned char*)pPtr, nullptr, Width, Height, 8 * hImageObject.CountChannels().I(), 1, "", 0);

			//const long lsize = Width * Height * 3;
			//char *_ptrImg = new char[lsize];
			//memset(_ptrImg,0, lsize);
			//size_t	Pixed = 0;
			//int iheight = Height;
			//int iwidth	= Width;
			//for (size_t i = 0; i < iheight; i++)
			//{
			//	size_t _Lenth = i * iwidth;
			//	for (size_t j = 0; j < iwidth; j++)
			//	{
			//		Pixed = j * 3 + _Lenth * 3;
			//		_ptrImg[Pixed] = PointerBlue[j + _Lenth];
			//	}
			//}

			//RecieveBuffer(strCam, (unsigned char*)_ptrImg, nullptr, Width, Height, 8 * hImageObject.CountChannels().I(), 1, "", 0);
			//delete[] _ptrImg;

		}	break;
		default:
			break;
		}
	}
}

void QtVisionWidget::on_btnConnect_clicked()
{
	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera == nullptr)	{
		m_pCamera = PluginsManager::Instance().InitCamera(strCam);
	}
	if (m_pCamera == nullptr)	{
		dataVar::Instance().pManager->notify(tr("Open Camera Error!"), NotifyType_Error);
		return ;
	}
	//Connect Camera
	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
		if (!m_pCamera->IsCamConnect()) {
			m_pCamera->Init();
			m_pCamera->Start();
			m_pCamera->SetStream(this);
			m_pCamera->StartVedioLive();
		}
		else {
			m_pCamera->StopVedioLive();
			m_pCamera->Stop();
			m_pCamera->Exit();
		}
		return  StandardMsgResult::MsgResult_OK;
	},tr("Connect Camera Now !"), QtWaitWidgetsClass::MsgType_Close);

	if (m_pCamera->IsCamConnect()){
		QString	strValue;
		m_pCamera->GetCameraParam(CameraMaxExposure, strValue);
		ui.spBx_Exposure->setMaximum(strValue.toDouble());
		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.spBx_Exposure->setMinimum(strValue.toDouble());
		m_pCamera->SetCameraParam(CameraExposure, QString::number(ui.spBx_Exposure->value()));

		//m_pCamera->GetCameraParam(CameraExposure, strValue);
		//ui.spBx_Exposure->setValue(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_Gain->setMaximum(strValue.toDouble());
		ui.Slider_Gain->setMaximum(strValue.toDouble() * 10);
		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_Gain->setMinimum(strValue.toDouble());
		ui.Slider_Gain->setMinimum(strValue.toDouble() * 10);

		m_pCamera->SetCameraParam(CameraGain, QString::number(ui.spBx_Gain->value()));
		//m_pCamera->GetCameraParam(CameraGain, strValue);
		//ui.spBx_Gain->setValue(strValue.toDouble());
		//ui.Slider_Gain->setValue(strValue.toDouble() * 10);
	}
	CheckConConnected();
}

void QtVisionWidget::on_btnTrrigerlight_clicked()
{
	//MotionIO::getInstance().SetOutPortStatus(0, EnumOutPut_TrrigerLight1 + m_iIndex,false);
	//Sleep(200);
	//MotionIO::getInstance().SetOutPortStatus(0, EnumOutPut_TrrigerLight1 + m_iIndex, true);
}

void QtVisionWidget::slotUpDateParam(int index)
{
	UpDateParam();
}

void QtVisionWidget::UpDateParam()
{

	QString strCameraName				= ui.cbx_CameraName->currentText();
	if (strCameraName != "None") {
	}

	slot_ChangeMarkModelTab(0);
}

void QtVisionWidget::ClearMatchShapeModel()
{
	try { ClearShapeModel(m_hShapeModel); }
	catch (...) {}
}

void QtVisionWidget::ClearMatchNCCModel()
{
	try { ClearNccModel(m_hNCCModel); }
	catch (...) {}
}

void QtVisionWidget::ClearTableWidget()
{

}

void QtVisionWidget::slotSetValue(int index, double dRobotX, double dRobotY, double dImgX, double dImgY)
{

}

//绘制ROI
void QtVisionWidget::on_btnAddROI_clicked()
{
	BaseItem* _BaseItem;
	int iWidth = (m_pMarkGraphViews->pImagItem)->w;
	int iHeight = (m_pMarkGraphViews->pImagItem)->h;

	int iCount = m_VecBaseItem.size();
	float fData[10] = { 0 };
	memset(fData, 0, 10 * sizeof(float));
	switch (ui.comboROIShape->currentIndex())	{
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 5.0, iHeight / 5.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		m_pMarkGraphViews->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 2: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 3: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 4: {	//多边形
		_BaseItem = new PolygonItem();
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	}
	switch (ui.comboxROIType->currentIndex()) {
	case 0:	//合并
	{	_BaseItem->m_eRegionType = RegionType_Add;	}break;
	case 1:	//差集
	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default:	break;
	}
	m_VecBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecBaseItem.size(); i++) { 
		m_VecBaseItem[i]->setIndex(i); 	
		m_VecBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
}

void QtVisionWidget::on_btnDeleteROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->selectedItems();
	for (auto iter : shapeLst) {
		if (iter != nullptr)
			if (iter->type() == 10) {
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecBaseItem.size(); i++) {
					if (m_VecBaseItem[i] == iter) {
						_bFinded = true;
					}
				}
				if (_bFinded) {
					(m_pMarkGraphViews->scene())->removeItem(iter);
					m_VecBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}

	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void QtVisionWidget::on_btnDeleteROIAll_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->items();
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
				(m_pMarkGraphViews->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
}

void QtVisionWidget::on_btnAddSearchROI_clicked()
{
	BaseItem* _BaseItem;
	int iWidth = (m_pMarkGraphViews->pImagItem)->w;
	int iHeight = (m_pMarkGraphViews->pImagItem)->h;

	int iCount = m_VecSearchBaseItem.size();
	float fData[10] = { 0 };
	memset(fData,0, 10 * sizeof(float));
	switch (ui.comboSearchROIShape->currentIndex()) {
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 2.0, iHeight / 2.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		m_pMarkGraphViews->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 2: {	//矩形
		fData[0] = 0;
		fData[1] = 0;
		fData[2] = iWidth;
		fData[3] = iHeight;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 3: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth;
		fData[3] = iHeight;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 4: {	//多边形
		_BaseItem = new PolygonItem();
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	}
	switch (ui.comboxSearchROIType->currentIndex()) {
	case 0:	//合并
	{	_BaseItem->m_eRegionType = RegionType_Add;	}break;
	case 1:	//差集
	{	_BaseItem->m_eRegionType = RegionType_Differe;	}break;
	default:	break;
	}
	m_VecSearchBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++) {
		m_VecSearchBaseItem[i]->setIndex(i);
		m_VecSearchBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
}

void QtVisionWidget::on_btnDeleteSearchROISelect_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->selectedItems();
	for (auto iter : shapeLst)	{
		if (iter != nullptr)
			if (iter->type() == 10)	{
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	{
					if (m_VecSearchBaseItem[i] == iter)		{
						_bFinded = true;
					}
				}
				if (_bFinded)	{
					(m_pMarkGraphViews->scene())->removeItem(iter);
					m_VecSearchBaseItem.removeOne((BaseItem*)iter);
					delete iter;
					iter = nullptr;
				}
			}
	}

	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
}

void QtVisionWidget::on_btnDeleteSearchROIAll_clicked()
{
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->items();
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
				(m_pMarkGraphViews->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
}

void QtVisionWidget::on_btnCreateModel_clicked()
{
	QString strCam = ui.cbx_CameraName->currentText();

	QVector<QPointF>	_VecPt1D;
	QVector<sDrawText>	_VecText1D;
	QVector<sDrawCross> _PointLst;

	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
	else
	{
		dataVar::Instance().pManager->notify(tr("Didn't Connected Camera ") + strCam, NotifyType_Error);
		return;
	}
	m_pMarkGraphViews->ClearAllDrawItem();

	QString _strMsg = tr(" CreateModel");;
	//QFuture<StandardMsgResult> value = QtConcurrent::run([=] {
	Sleep(500);
	if (m_pCamera != nullptr) {
		m_pCamera->Trigger();
		if (m_pCamera->WaitTrigger(5000) < 0) {
			dataVar::Instance().pManager->notify(tr("Trigger Camera TimeOut") + strCam, NotifyType_Error);
			return /*StandardMsgResult::MsgResult_TimeOut*/;
		}
	}

	if (!m_dstImage.IsInitialized()) { return /*StandardMsgResult::MsgResult_NG;*/; }

	try {
		HObject	_ReduceImg, _ModelImages, _ModelRegions, _SelectObj, _ModelContours, ho_TransContours;
		HTuple _hv_ModelId, _NumLevels, _Contrast, _Number, Rows, Columns, _Lenth;

		ShapesToRegion(m_VecBaseItem, m_ModelROIRegion);
		//WriteObject(m_ModelROIRegion, "E:/Robot/m_ModelROIRegion.hobj");

		HalconCpp::ReduceDomain(m_dstImage, m_ModelROIRegion, &_ReduceImg);
		_NumLevels = ui.spBx_PyLevel->value();
		//_Contrast = ui.spBx_LowContrast->value();
		//_Contrast.TupleConcat(ui.spBx_HighContrast->value());
		//_Contrast.TupleConcat(ui.spBx_MinSize->value());

		switch ((EnumModeType)ui.cbx_MatchType->currentIndex()) {
		case EnumModeType_Shape: {
			//Matching 01: create the shape model
			HalconCpp::CreateShapeModel(_ReduceImg, /*"auto"*/0, HTuple(ui.spBx_CAngleStart->value()).TupleRad(), HTuple(ui.spBx_CAngleRange->value()).TupleRad(),
				"auto", "auto", "use_polarity", "auto", "auto", &_hv_ModelId);
			HTuple hv_AngleStart, hv_AngleExtent, hv_AngleStep, hv_ScaleMin, hv_ScaleMax, hv_ScaleStep, hv_Metric, hv_MinContrast;
			HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
			HalconCpp::GetShapeModelParams(_hv_ModelId, &_NumLevels, &hv_AngleStart, &hv_AngleExtent,
				&hv_AngleStep, &hv_ScaleMin, &hv_ScaleMax, &hv_ScaleStep, &hv_Metric, &hv_MinContrast);
			ui.spBx_PyLevel->setValue(_NumLevels.I());

			HalconCpp::GetShapeModelContours(&_ModelContours, _hv_ModelId, 1);
			HalconCpp::FindShapeModel(m_dstImage, _hv_ModelId, hv_AngleStart, hv_AngleExtent, 0.5, 1, 0.5, "least_squares", 0, 0.5, &Row, &Column, &Angle, &Score);

			int	_iScoreIndex = Score.TupleLength();
			m_MarkAResultPos.Rst();
			if (_iScoreIndex <= 0) {
				sDrawText _strText;
				_VecText1D.push_back(_strText);
				//_VecText1D[_VecText1D.size() - 1].strText.append(_strMsg + tr("NG"));
				_VecText1D[_VecText1D.size() - 1].strText = _strMsg + tr("NG");
				_VecText1D[_VecText1D.size() - 1].bControl = true;
				_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(255, 0, 0);
			}
			else {
				m_MarkAResultPos.fMatchX = Column[0].D();
				m_MarkAResultPos.fMatchY = Row[0].D();
				m_MarkAResultPos.fMatchAngle = Angle[0].D();

				sDrawText _strText;
				_VecText1D.push_back(_strText);
				_VecText1D[_VecText1D.size() - 1].strText = _strMsg + tr("OK");
				_VecText1D[_VecText1D.size() - 1].bControl = true;
				_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(0, 255, 0);
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
					_VecText1D.push_back(_item);

					_VecText1D[_VecText1D.size() - 1].bControl = true;
					_VecText1D[_VecText1D.size() - 1].Center.setX(20);
					_VecText1D[_VecText1D.size() - 1].Center.setY((hv_MatchingObjIdx + 1) * 20);
					_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(0, 255, 0);
					_VecText1D[_VecText1D.size() - 1].strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision));
					;

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
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
			HalconCpp::CreateNccModel(_ReduceImg, /*"auto"*/0, HTuple(ui.spBx_CAngleStart->value()).TupleRad(), HTuple(ui.spBx_CAngleRange->value()).TupleRad(), "auto", "use_polarity", &_hv_ModelId);
			HTuple hv_AngleStart, hv_AngleExtent, hv_AngleStep, hv_ScaleMin, hv_ScaleMax, hv_ScaleStep, hv_Metric, hv_MinContrast;
			HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
			HalconCpp::GetNccModelParams(_hv_ModelId, &_NumLevels, &hv_AngleStart, &hv_AngleExtent, &hv_AngleStep, &hv_Metric);
			HalconCpp::FindNccModel(m_dstImage, _hv_ModelId, hv_AngleStart, hv_AngleExtent, 0.5, 1, 0.5, "true", 0, &Row, &Column, &Angle, &Score);

			m_MarkAResultPos.Rst();
			int	_iScoreIndex = Score.TupleLength();
			if (_iScoreIndex <= 0) {
				sDrawText _strText;
				_VecText1D.push_back(_strText);
				_VecText1D[_VecText1D.size() - 1].bControl = true;
				_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(255, 0, 0);
				_VecText1D[_VecText1D.size() - 1].strText = _strMsg + tr("NG");
			}
			else {

				m_MarkAResultPos.fMatchX = Column[0].D();
				m_MarkAResultPos.fMatchY = Row[0].D();
				m_MarkAResultPos.fMatchAngle = Angle[0].D();

				sDrawText _strText;
				_VecText1D.push_back(_strText);
				_VecText1D[_VecText1D.size() - 1].bControl = true;
				_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(0, 255, 0);
				_VecText1D[_VecText1D.size() - 1].strText = _strMsg + tr("OK");
			} {
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;
				m_VecCreateCross.clear();
				for (HTuple hv_MatchingObjIdx = 0; hv_MatchingObjIdx <= end_val19; hv_MatchingObjIdx += step_val19) {
					sDrawText _item;
					_VecText1D.push_back(_item);
					_VecText1D[_VecText1D.size() - 1].bControl = true;
					_VecText1D[_VecText1D.size() - 1].Center.setX(20);
					_VecText1D[_VecText1D.size() - 1].Center.setY((hv_MatchingObjIdx + 1) * 20);
					_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(0, 255, 0);
					_VecText1D[_VecText1D.size() - 1].strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision));

					sDrawCross _Cross;
					_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
					_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
					_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
					_Cross.DrawColor = QColor(0, 0, 255);
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
		_VecText1D.push_back(_strText);
		_VecText1D[_VecText1D.size() - 1].bControl = true;
		_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_VecText1D[_VecText1D.size() - 1].strText = strError;// QString("创建模板错误:%1").arg(strError);
			}
		}
	}
	catch (...) {
		sDrawText _strText;
		_VecText1D.push_back(_strText);
		_VecText1D[_VecText1D.size() - 1].bControl = true;
		_VecText1D[_VecText1D.size() - 1].DrawColor = QColor(255, 0, 0);
		_VecText1D[_VecText1D.size() - 1].strText = _strMsg + tr("Error");
	}

	m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
	m_pMarkGraphViews->DispTextList(_VecText1D);
	m_pMarkGraphViews->DispCrossPoint(_PointLst);
	m_pMarkGraphViews->update();

	//return StandardMsgResult::MsgResult_OK;
	//});
	//while (!value.isFinished()) {
	//	QApplication::processEvents();
	//}
	//switch (value.result()) {
	//case MsgResult_TimeOut:
	//	dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
	//	break;
	//case MsgResult_Close:
	//	dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
	//	break;
	//case MsgResult_NG:
	//case MsgResult_OK:
	//default: { }	break;
	//}
	Sleep(300);
	if (m_pCamera != nullptr) m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_Continnue));


}

void QtVisionWidget::on_btnFindModel_clicked()
{
	HTuple hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_Greedy, hv_SubPixel;
	HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
	QVector<QPointF>	_VecPt1D;
	QVector<sDrawText>	_VecText1D;
	QVector<sDrawCross> _PointLst;
	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
	else
	{
		dataVar::Instance().pManager->notify(tr("Didn't Connected Camera ") + strCam, NotifyType_Error);
		return;
	}
	QString _strMsg = tr("FindModel");;
	Sleep(500);
	if (m_pCamera != nullptr) {
		m_pCamera->Trigger();
		if (m_pCamera->WaitTrigger(5000) < 0) {
			dataVar::Instance().pManager->notify(tr("Trigger Camera TimeOut") + strCam, NotifyType_Error);
			return;
		}
	}

	if (!m_dstImage.IsInitialized()) { return /*StandardMsgResult::MsgResult_NG;*/; }
	try
	{
		hv_AngleStart = HTuple(ui.spBx_CAngleStart->value()).TupleRad();
		hv_AngleExtent = HTuple(ui.spBx_CAngleRange->value()).TupleRad();
		hv_MinScore = ui.spBx_FindMinScore->value();
		hv_NumMatches = ui.spBx_FindMinNumber->value();
		hv_Greedy = ui.spBx_FindGreedy->value();
		hv_MaxOverlap = ui.spBx_FindMinOverLap->value();

		m_pMarkGraphViews->ClearAllDrawItem();
		HObject	_ReduceImg;
		ShapesToRegion(m_VecSearchBaseItem, m_SearchROIRegion);
		HalconCpp::ReduceDomain(m_dstImage, m_SearchROIRegion, &_ReduceImg);

		QElapsedTimer	_timer;
		switch ((EnumModeType)ui.cbx_MatchType->currentIndex()) {
		case EnumModeType_Shape: {
			switch (ui.cbx_SubPixType->currentIndex()) {
			case 0: {	hv_SubPixel = "none";						}break;
			case 1: {	hv_SubPixel = "interpolation";				}break;
			case 2: {	hv_SubPixel = "least_squares";				}break;
			case 3: {	hv_SubPixel = "least_squares_high";			}break;
			case 4: {	hv_SubPixel = "least_squares_very_high";	}break;
			default:	break;
			}
			_timer.start();
			HalconCpp::FindShapeModel(_ReduceImg, m_hShapeModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, hv_Greedy, &Row, &Column, &Angle, &Score);
			double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;

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
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = _strMsg + tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
					_VecText1D.push_back(_strText);
				}
				else {
					sDrawText _strText;
					_strText.bControl = true;
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = _strMsg + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
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
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision));
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
		case EnumModeType_NCC: {
			switch (ui.cbx_SubPixType->currentIndex()) {
			case 0: {	hv_SubPixel = "false";			}break;
			case 1: {	hv_SubPixel = "true";			}break;
			default:	break;
			}

			_timer.start();
			HalconCpp::FindNccModel(_ReduceImg, m_hNCCModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, &Row, &Column, &Angle, &Score);
			double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;
			{
				HObject _ModelContours, ho_TransContours, _SelectObj, _ModelRegions;
				HTuple	Rows, Columns, _Lenth;
				HTuple end_val19 = (Score.TupleLength()) - 1;
				HTuple step_val19 = 1;

				int	_iScoreIndex = Score.TupleLength();
				if (_iScoreIndex <= 0) {
					sDrawText _strText;
					_strText.bControl = true;
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = _strMsg + tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
					_VecText1D.push_back(_strText);
				}
				else {
					sDrawText _strText;
					_strText.bControl = true;
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = _strMsg + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
					_VecText1D.push_back(_strText);
				}
				for (int hv_MatchingObjIdx = 0; hv_MatchingObjIdx < _iScoreIndex; hv_MatchingObjIdx++) {
					sDrawText _item;
					_item.bControl = true;
					_item.Center.setX(20);
					_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
					_item.DrawColor = QColor(0, 255, 0);
					_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
						.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', dataVar::Instance().form_System_Precision))
						.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision));
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
		_strText.DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_strText.strText = "查找模板错误" + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...) {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = "查找模板错误";
		_VecText1D.push_back(_strText);
	}
	m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
	m_pMarkGraphViews->DispTextList(_VecText1D);
	m_pMarkGraphViews->DispCrossPoint(_PointLst);
	m_pMarkGraphViews->update();
	Sleep(300);
	if (m_pCamera != nullptr) m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_Continnue));


}

void QtVisionWidget::slot_ChangeMatchType(int Index)
{
	switch ((EnumModeType)ui.cbx_MatchType->currentIndex())
	{
	case EnumModeType_Shape: {
		//ui.Layout_LowContrast->setVisible(true);
		//ui.Layout_HighContrast->setVisible(true);
		//ui.Layout_MinSize->setVisible(true);
		ui.cbx_SubPixType->clear();
		QStringList _strLst;
		_strLst << "none" << "interpolation" << "least_squares" << "least_squares_high" << "least_squares_very_high";
		ui.cbx_SubPixType->addItems(_strLst);
	}	break;
	case EnumModeType_NCC: {
		//ui.Layout_LowContrast->setVisible(false);
		//ui.Layout_HighContrast->setVisible(false);
		//ui.Layout_MinSize->setVisible(false);
		ui.cbx_SubPixType->clear();
		QStringList _strLst;
		_strLst << "false" << "true";
		ui.cbx_SubPixType->addItems(_strLst);
	}	break;
	default:
		break;
	}
}

void QtVisionWidget::slot_btnGroupClicked(int iID)
{
	if (m_pBtnGroupRadio->checkedId() == 0)
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

void QtVisionWidget::onRoiWidthChanged(int Index)
{
	for (auto iter : m_VecBaseItem)			if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto iter : m_VecSearchBaseItem)	if (iter != nullptr) iter->SetDrawPenWidth(ui.spinRoiW->value());
	m_pMarkGraphViews->update();

}

void QtVisionWidget::slot_SetGreedyChanged(double value)
{
	ui.hSliderGreedy->setValue(value * 100);
}

void QtVisionWidget::slot_SetMinOverLapChanged(double value)
{
	ui.hSliderMinOverLap->setValue(value * 100);
}

void QtVisionWidget::slot_SetMinScoreChanged(double value)
{
	ui.hSlider_MinScore->setValue(value * 100);
}

void QtVisionWidget::slot_ChangeTab(int iTab)
{
	m_pMarkGraphViews->ClearAllDrawItem();

	for (auto &iter : m_VecBaseItem)			if (iter != nullptr) { iter->setVisible(false); iter->SetItemSelect(false); }
	for (auto &iter : m_VecSearchBaseItem)		if (iter != nullptr) { iter->setVisible(false); iter->SetItemSelect(false); }
	for (auto &iter : m_VecCodeBaseItem)		if (iter != nullptr) { iter->setVisible(false); iter->SetItemSelect(false); }
	for (auto &iter : m_VecZIFUBaseItem)		if (iter != nullptr) { iter->setVisible(false); iter->SetItemSelect(false); }

	switch (iTab) {
	case 0: {
		if (m_pBtnGroupRadio->checkedId() == 0) {
			for (auto &iter : m_VecBaseItem)		if (iter != nullptr) { iter->setVisible(true); }
		}
		else {
			for (auto &iter : m_VecSearchBaseItem)	if (iter != nullptr)iter->setVisible(true);
		}
	}	break;
	case 1: {
		for (auto &iter : m_VecCodeBaseItem)		if (iter != nullptr) { iter->setVisible(true); }
	}	break;
	case 2: {
		for (auto &iter : m_VecZIFUBaseItem)		if (iter != nullptr) { iter->setVisible(true); }
	}	break;
	case 3: {
	}	break;
	default: {
	}	break;
	}
	ResizeUI();
	for (auto &iter : m_VecBaseItem)			if (iter != nullptr)	iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto &iter : m_VecSearchBaseItem)		if (iter != nullptr)	iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto &iter : m_VecCodeBaseItem)		if (iter != nullptr)	iter->SetDrawPenWidth(ui.spinRoiW->value());
	for (auto &iter : m_VecZIFUBaseItem)		if (iter != nullptr)	iter->SetDrawPenWidth(ui.spinRoiW->value());
	m_pMarkGraphViews->update();
}

void QtVisionWidget::slot_cellClicked(int row, int column)
{
	int	_iCurrentRow = ui.tableCodeWidget->currentIndex().row();
	for (size_t iRow = 0; iRow < m_VecCodeBaseItem.size(); iRow++) {
		if (_iCurrentRow == iRow)		{
			m_VecCodeBaseItem[iRow]->SetItemSelect(true);
			m_VecCodeBaseItem[iRow]->m_bSelected = (true);
			m_VecCodeBaseItem[iRow]->m_bShowMoveSelected = (true);
			m_VecCodeBaseItem[iRow]->setSelected(true);

			ui.cBx_CodeType->setCurrentIndex(	((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 2))->currentIndex()	);
			QStringList	_codeLst;
			switch (ui.cBx_CodeType->currentIndex()) {
			case 0: {
				_codeLst.append(tr("25"));
				_codeLst.append(tr("39"));
				_codeLst.append(tr("128"));
			}break;
			case 1: {
				_codeLst.append(tr("QR"));
				_codeLst.append(tr("DM"));
				_codeLst.append(tr("MQR"));
			}break;
			default:			break;
			}
			ui.cBx_QRCodeType->addItems(_codeLst);

			ui.cBx_QRCodeType->setCurrentIndex(	((QComboBox*)ui.tableCodeWidget->cellWidget(iRow, 3))->currentIndex()	);

			ui.spBx_CodeExposure->setValue(		((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 4))->value()	);
			
			ui.spBx_CodeGain->setValue(			((QDoubleSpinBox*)ui.tableCodeWidget->cellWidget(iRow, 5))->value()	);
		}
		else m_VecCodeBaseItem[iRow]->SetItemSelect(false);
	}
	m_pMarkGraphViews->update();

}

void QtVisionWidget::slot_ChangeMarkModelTab(int iTab)
{
	switch (ui.tabWidget->currentIndex()) {
	case 0: {
		if (m_pBtnGroupRadio->checkedId() == 0) {
			for (auto &iter : m_VecBaseItem)		iter->setVisible(true);
			for (auto &iter : m_VecSearchBaseItem) { iter->setVisible(false); iter->SetItemSelect(false); }
		}
		else	{
			for (auto &iter : m_VecBaseItem) { iter->setVisible(false); iter->SetItemSelect(false); }
			for (auto &iter : m_VecSearchBaseItem)	iter->setVisible(true);
		}
	} break;
	case 1: {
		for (auto &iter : m_VecBaseItem) { iter->setVisible(false); iter->SetItemSelect(false); }
		for (auto &iter : m_VecSearchBaseItem) { iter->setVisible(false); iter->SetItemSelect(false); }
	} break;
	default:
		break;
	}
}

void QtVisionWidget::slotUpDateCameras()
{
	QString	_strCamName = ui.cbx_CameraName->currentText();
	ui.cbx_CameraName->clear();
	auto _CameraNames = PluginsManager::Instance().getCameraNames();
	ui.cbx_CameraName->addItem("None");
	ui.cbx_CameraName->addItems(_CameraNames);
	ui.cbx_CameraName->setCurrentText(_strCamName);
}

void QtVisionWidget::slotUpDateExposure(int value)
{
	if (m_pCamera != nullptr)	{
		m_pCamera->SetCameraParam(CameraExposure,QString::number(value));
	}
}

void QtVisionWidget::slotUpDateGain(int value)
{
	if (m_pCamera != nullptr) {
		m_pCamera->SetCameraParam(CameraGain, QString::number(value * 0.1));
	}
}

void QtVisionWidget::slotUpDateLight()
{
	int _iChannel	= ui.cBx_Channel->currentIndex();
	int _iValue		= ui.Slider_Light->value();
	MotionSerialPort::getInstance().WriteLightValue(_iChannel, _iValue);
}

void QtVisionWidget::on_btnAddCodeROI_clicked()
{
	BaseItem* _BaseItem;
	int iWidth = (m_pMarkGraphViews->pImagItem)->w;
	int iHeight = (m_pMarkGraphViews->pImagItem)->h;

	int iCount = m_VecCodeBaseItem.size();
	float fData[10] = { 0 };
	memset(fData, 0, 10 * sizeof(float));
	switch (ui.comboCodeROIShape->currentIndex()) {
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 5.0, iHeight / 5.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		m_pMarkGraphViews->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 2: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 3: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 4: {	//多边形
		_BaseItem = new PolygonItem();
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	}

	int iRowCount = ui.tableCodeWidget->rowCount();
	ui.tableCodeWidget->setRowCount(iRowCount + 1);

	QWidget* _pQWidget = nullptr;
	{
		QTableWidgetItem* _pItem = new QTableWidgetItem();
		_pItem->setTextAlignment(Qt::AlignCenter);
		_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
		_pItem->setText(QString::number(iRowCount));
		ui.tableCodeWidget->setItem(iRowCount, 0, _pItem);
	} 
	{
		QTableWidgetItem* _pItem = new QTableWidgetItem();
		_pItem->setTextAlignment(Qt::AlignCenter);
		_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
	
		QJsonObject	Json;
		((BaseItem*)_BaseItem)->GetData(Json);
		QJsonDocument	root_document;
		root_document.setObject(Json);
		QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
		QString	strText = QString(root_string_compact);
		_pItem->setText(strText);
		//ui.tableCodeWidget->item(_iCurrentIndex, 1)->setText(strText);
		ui.tableCodeWidget->setItem(iRowCount, 1, _pItem);
	}	
	{
		_pQWidget = new QComboBox();
		((QComboBox*)_pQWidget)->setEnabled(false);
		((QComboBox*)_pQWidget)->addItem(tr("一维码"));
		((QComboBox*)_pQWidget)->addItem(tr("二维码"));
		((QComboBox*)_pQWidget)->setCurrentIndex(ui.cBx_CodeType->currentIndex());
		ui.tableCodeWidget->setCellWidget(iRowCount, 2, _pQWidget);
	}
	{
		_pQWidget = new QComboBox();
		((QComboBox*)_pQWidget)->setEnabled(false);
		QStringList	_codeLst;
		switch (ui.cBx_CodeType->currentIndex()) {
		case 0: {
			_codeLst.append(tr("25"));
			_codeLst.append(tr("39"));
			_codeLst.append(tr("128"));
		}break;
		case 1: {
			_codeLst.append(tr("QR"));
			_codeLst.append(tr("DM"));
			_codeLst.append(tr("MQR"));
		}break;
		default:			break;
		}
		((QComboBox*)_pQWidget)->addItems(_codeLst);
		((QComboBox*)_pQWidget)->setCurrentIndex(ui.cBx_QRCodeType->currentIndex());
		ui.tableCodeWidget->setCellWidget(iRowCount, 3, _pQWidget);
	}	
	{
		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setMinimum(ui.spBx_CodeExposure->minimum());
		((QDoubleSpinBox*)_pQWidget)->setMaximum(ui.spBx_CodeExposure->maximum());
		((QDoubleSpinBox*)_pQWidget)->setValue(ui.spBx_CodeExposure->value());
		ui.tableCodeWidget->setCellWidget(iRowCount, 4, _pQWidget);
	}	{
		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setMinimum(ui.spBx_CodeGain->minimum());
		((QDoubleSpinBox*)_pQWidget)->setMaximum(ui.spBx_CodeGain->maximum());
		((QDoubleSpinBox*)_pQWidget)->setValue(ui.spBx_CodeGain->value()) ;
		ui.tableCodeWidget->setCellWidget(iRowCount, 5, _pQWidget);
	}

	_BaseItem->m_eRegionType = RegionType_Add;
	m_VecCodeBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++) {
		m_VecCodeBaseItem[i]->setIndex(i);
		m_VecCodeBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
	UpdateIndex();
}

void QtVisionWidget::on_btnDeleteCodeROISelect_clicked()
{
	int iRowCount = ui.tableCodeWidget->rowCount();
	int iCurrentIndex = ui.tableCodeWidget->currentIndex().row();
	ui.tableCodeWidget->removeRow(iCurrentIndex);

	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->selectedItems();
	for (auto iter : shapeLst) {
		if (iter != nullptr)
			if (iter->type() == 10) {
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++) {
					if (m_VecCodeBaseItem[i] == iter) {
						_bFinded = true;
					}
				}
				if (_bFinded) {
					(m_pMarkGraphViews->scene())->removeItem(iter);
					m_VecCodeBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}

	for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++)	m_VecCodeBaseItem[i]->setIndex(i);

	UpdateIndex();
}

void QtVisionWidget::on_btnDeleteCodeROIAll_clicked()
{
	int iRowCount = ui.tableCodeWidget->rowCount();
	for (size_t i = 0; i < iRowCount; i++)		ui.tableCodeWidget->removeRow(0);
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++)
			{
				if (m_VecCodeBaseItem[i] == iter)
				{
					_bFinded = true;
				}
			}
			//ui.tableROIWidget->removeRow(((BaseItem*)iter)->getIndex());
			if (_bFinded)
			{
				m_VecCodeBaseItem.removeOne((BaseItem*)iter);
				(m_pMarkGraphViews->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++)	m_VecCodeBaseItem[i]->setIndex(i);
	UpdateIndex();
}

void QtVisionWidget::on_btnCodeIdentify_clicked()
{
	m_pMarkGraphViews->ClearAllDrawItem();
	QElapsedTimer _timer;
	_timer.start();

	QVector<VisionStruct> vecVision;
	TableToStruct(ui.tableCodeWidget, vecVision);

	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
	else
	{
		dataVar::Instance().pManager->notify(tr("Didn't Connected Camera ") + strCam, NotifyType_Error);
		return;
	}
	QtConcurrent::run([=] {
		QVector<QPointF>	_VecPt1D;
		QVector<sDrawText>	_VecText1D;
		QVector<sDrawCross> _PointLst;
		try	{
			for (size_t iRow = 0; iRow < vecVision.size(); iRow++) {
				Sleep(500);
				m_pCamera->SetCameraParam(CameraExposure, QString::number(vecVision[iRow].iExposure));
				m_pCamera->SetCameraParam(CameraGain, QString::number(vecVision[iRow].dGain));
				m_pCamera->Trigger();
				if (m_pCamera->WaitTrigger(5000) < 0) {
					dataVar::Instance().pManager->notify(tr("Trigger Camera TimeOut") + strCam, NotifyType_Error);
					return;
				}
				HObject	_ReduceImg;
				ShapeToRegion(m_VecCodeBaseItem[iRow], m_SearchROIRegion);
				HalconCpp::ReduceDomain(m_dstImage, m_SearchROIRegion, &_ReduceImg);

				QElapsedTimer	_timer;
				_timer.start();
				switch (vecVision[iRow].iCodeType) {
				case 0: {
					HObject  ho_SymbolRegions;
					// Local control variables
					HTuple  hv_BarCodeHandle, hv_DecodedDataStrings;
					HTuple  hv_BarCodeResults;
					CreateBarCodeModel(HTuple(), HTuple(), &hv_BarCodeHandle);
					SetBarCodeParam(hv_BarCodeHandle, "stop_after_result_num", 0);
					FindBarCode(_ReduceImg, &ho_SymbolRegions, hv_BarCodeHandle, "auto", &hv_DecodedDataStrings);
					GetBarCodeResult(hv_BarCodeHandle, "all", "decoded_types", &hv_BarCodeResults);
					ClearBarCodeModel(hv_BarCodeHandle);
					double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;
					if (hv_BarCodeResults.TupleLength() > 0)
					{
						for (size_t i = 0; i < hv_BarCodeResults.TupleLength().I(); i++)	{
							sDrawText			_strText;
							_strText.bControl	= true;
							_strText.DrawColor	= QColor(255, 0, 0);
							_strText.strText	= hv_BarCodeResults[i].S() + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
							_VecText1D.push_back(_strText);
						}
					}
					else
					{
						sDrawText			_strText;
						_strText.bControl	= true;
						_strText.DrawColor	= QColor(255, 0, 0);
						_strText.strText	= tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
						_VecText1D.push_back(_strText);
					}
				}	break;
				case 1: {
					HObject  ho_SymbolXLDs;
					HTuple  hv_DataCodeHandle, hv_ResultHandles, hv_DecodedDataStrings1;
					switch (vecVision[iRow].iCode)	{
					case 0: {
						CreateDataCode2dModel("QR Code", HTuple(), HTuple(), &hv_DataCodeHandle);
					}	break;
					case 1: {
						CreateDataCode2dModel("Data Matrix ECC 200", HTuple(), HTuple(), &hv_DataCodeHandle);
					}	break;
					case 2: {
						CreateDataCode2dModel("Micro QR Code", HTuple(), HTuple(), &hv_DataCodeHandle);
					}	break;
					default:
						break;
					}
					//*识别模式 最强
					SetDataCode2dParam(hv_DataCodeHandle, "default_parameters", "maximum_recognition");
					FindDataCode2d(_ReduceImg, &ho_SymbolXLDs, hv_DataCodeHandle, HTuple(), HTuple(),
						&hv_ResultHandles, &hv_DecodedDataStrings1);
					ClearDataCode2dModel(hv_DataCodeHandle);
					double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;
					if (hv_DecodedDataStrings1.TupleLength() > 0)
					{
						for (size_t i = 0; i < hv_DecodedDataStrings1.TupleLength().I(); i++) {
							sDrawText			_strText;
							_strText.bControl = true;
							_strText.DrawColor = QColor(255, 0, 0);
							_strText.strText = hv_DecodedDataStrings1[i].S() + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
							_VecText1D.push_back(_strText);
						}
					}
					else
					{
						sDrawText			_strText;
						_strText.bControl = true;
						_strText.DrawColor = QColor(255, 0, 0);
						_strText.strText = tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
						_VecText1D.push_back(_strText);
					}
				}	break;
				default:
					break;
				}
			}
		}
		catch (const HException& except) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			QString strError = except.ErrorText();
			QStringList strErrorArray = strError.split(':');
			if (strErrorArray.size() > 1) {
				if (!strErrorArray[1].isEmpty()) {
					strError.clear();
					for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
					_strText.strText = "查找模板错误" + strError;
				}
			}
			_VecText1D.push_back(_strText);
		}
		catch (...) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = "查找模板错误";
			_VecText1D.push_back(_strText);
		}
		m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
		m_pMarkGraphViews->DispTextList(_VecText1D);
		m_pMarkGraphViews->DispCrossPoint(_PointLst);
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->UpdateImg();
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->update();
	});


}

void QtVisionWidget::slotChangeCodeType(int value)
{
	ui.cBx_QRCodeType->clear();
	QStringList	_codeLst;
	switch (value)	{
	case 0: {
		_codeLst.append(tr("25"));
		_codeLst.append(tr("39"));
		_codeLst.append(tr("128"));
	}break;
	case 1: {
		_codeLst.append(tr("QR"));
		_codeLst.append(tr("DM"));
		_codeLst.append(tr("MQR"));
	}break;
	default:	break;
	}
	ui.cBx_QRCodeType->addItems(_codeLst);
}

void QtVisionWidget::on_btnAddZIFUROI_clicked()
{
	BaseItem* _BaseItem;
	int iWidth = (m_pMarkGraphViews->pImagItem)->w;
	int iHeight = (m_pMarkGraphViews->pImagItem)->h;

	int iCount = m_VecZIFUBaseItem.size();
	float fData[10] = { 0 };
	memset(fData, 0, 10 * sizeof(float));
	switch (ui.comboZIFUROIShape->currentIndex()) {
	case 0: {	//园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = MIN(iWidth / 5.0, iHeight / 5.0);
		fData[2] = MAX(50, fData[2]);
		_BaseItem = new CircleItem(fData[0], fData[1], fData[2]);
		m_pMarkGraphViews->AddItems(_BaseItem);//录入矩形1
	}	break;
	case 1: {	//同心园
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = 100;
		fData[3] = 200;
		_BaseItem = new ConcentricCircleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 2: {	//矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleItem(fData[0], fData[1], fData[2], fData[3]);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 3: {	//旋转矩形
		fData[0] = iWidth / 2.0;
		fData[1] = iHeight / 2.0;
		fData[2] = iWidth / 5.0;
		fData[3] = iHeight / 5.0;
		_BaseItem = new RectangleRItem(fData[0], fData[1], fData[2], fData[3], 0);
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	case 4: {	//多边形
		_BaseItem = new PolygonItem();
		m_pMarkGraphViews->AddItems(_BaseItem);
	}   break;
	}

	int iRowCount = ui.tableZIFUWidget->rowCount();
	ui.tableZIFUWidget->setRowCount(iRowCount + 1);

	QWidget* _pQWidget = nullptr;
	{
		QTableWidgetItem* _pItem = new QTableWidgetItem();
		_pItem->setTextAlignment(Qt::AlignCenter);
		_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
		_pItem->setText(QString::number(iRowCount));
		ui.tableZIFUWidget->setItem(iRowCount, 0, _pItem);
	}
	{
		QTableWidgetItem* _pItem = new QTableWidgetItem();
		_pItem->setTextAlignment(Qt::AlignCenter);
		_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
		QJsonObject	Json;
		((BaseItem*)_BaseItem)->GetData(Json);
		QJsonDocument	root_document;
		root_document.setObject(Json);
		QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
		QString	strText = QString(root_string_compact);
		_pItem->setText(strText);
		ui.tableZIFUWidget->setItem(iRowCount, 1, _pItem);
	}
	{
		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setMinimum(0);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(1);
		((QDoubleSpinBox*)_pQWidget)->setValue(0.9);
		((QDoubleSpinBox*)_pQWidget)->setSingleStep(0.1);
		ui.tableZIFUWidget->setCellWidget(iRowCount, 2, _pQWidget);
	}
	_BaseItem->m_eRegionType = RegionType_Add;
	m_VecZIFUBaseItem.push_back(_BaseItem);//录入矩形2
	for (size_t i = 0; i < m_VecZIFUBaseItem.size(); i++) {
		m_VecZIFUBaseItem[i]->setIndex(i);
		m_VecZIFUBaseItem[i]->SetDrawPenWidth(ui.spinRoiW->value());
	}
	UpdateIndex();
}

void QtVisionWidget::on_btnDeleteZIFUROISelect_clicked()
{
	int iRowCount = ui.tableZIFUWidget->rowCount();
	int iCurrentIndex = ui.tableZIFUWidget->currentIndex().row();
	ui.tableZIFUWidget->removeRow(iCurrentIndex);

	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->selectedItems();
	for (auto iter : shapeLst) {
		if (iter != nullptr)
			if (iter->type() == 10) {
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecZIFUBaseItem.size(); i++) {
					if (m_VecZIFUBaseItem[i] == iter) {
						_bFinded = true;
					}
				}
				if (_bFinded) {
					(m_pMarkGraphViews->scene())->removeItem(iter);
					m_VecZIFUBaseItem.removeOne((BaseItem*)iter);
					delete iter;	iter = nullptr;
				}
			}
	}

	for (size_t i = 0; i < m_VecZIFUBaseItem.size(); i++)	m_VecZIFUBaseItem[i]->setIndex(i);

	UpdateIndex();
}

void QtVisionWidget::on_btnDeleteZIFUROIAll_clicked()
{
	int iRowCount = ui.tableZIFUWidget->rowCount();
	for (size_t i = 0; i < iRowCount; i++)		ui.tableZIFUWidget->removeRow(0);
	QList<QGraphicsItem *> shapeLst = (m_pMarkGraphViews->scene())->items();
	for (auto iter : shapeLst)
	{
		if (iter != nullptr) if (iter->type() == 10) {
			bool	_bFinded = false;
			for (size_t i = 0; i < m_VecZIFUBaseItem.size(); i++)		{
				if (m_VecZIFUBaseItem[i] == iter)						{
					_bFinded = true;
				}
			}
			//ui.tableROIWidget->removeRow(((BaseItem*)iter)->getIndex());
			if (_bFinded)		{
				m_VecZIFUBaseItem.removeOne((BaseItem*)iter);
				(m_pMarkGraphViews->scene())->removeItem(iter);	delete iter;	iter = nullptr;
			}
		}
	}

	for (size_t i = 0; i < m_VecCodeBaseItem.size(); i++)	m_VecCodeBaseItem[i]->setIndex(i);
	UpdateIndex();
}

void QtVisionWidget::on_btnCreateZIFUIdentify_clicked()
{
	m_pMarkGraphViews->ClearAllDrawItem();
	QElapsedTimer _timer;
	_timer.start();

	QVector<ZIFUVisionStruct> vecVision;
	TableToZIFUStruct(ui.tableCodeWidget, vecVision);

	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
	else
	{
		dataVar::Instance().pManager->notify(tr("Didn't Connected Camera ") + strCam, NotifyType_Error);
		return;
	}
	QtConcurrent::run([=] {
		QVector<QPointF>	_VecPt1D;
		QVector<sDrawText>	_VecText1D;
		QVector<sDrawCross> _PointLst;
		try {
			for (size_t iRow = 0; iRow < vecVision.size(); iRow++) {
				Sleep(500);
				m_pCamera->Trigger();
				if (m_pCamera->WaitTrigger(5000) < 0) {
					dataVar::Instance().pManager->notify(tr("Trigger Camera TimeOut") + strCam, NotifyType_Error);
					return;
				}
				HObject	_ReduceImg;
				ShapeToRegion(m_VecCodeBaseItem[iRow], m_SearchROIRegion);
				HalconCpp::ReduceDomain(m_dstImage, m_SearchROIRegion, &_ReduceImg);

				QElapsedTimer	_timer;
				_timer.start();

				HTuple  hv_OCVHandle, hv_I, hv_Quality, hv_Color;
				CreateOcvProj(QString::number(iRow).toStdString().c_str(), &hv_OCVHandle);
				TraindOcvProj(_ReduceImg, hv_OCVHandle, QString::number(iRow).toStdString().c_str(), "single");

				//识别
				DoOcvSimple(_ReduceImg, hv_OCVHandle, "A", "true", "true", "true", "true",5, &hv_Quality);
				sDrawText _strText;
				_strText.bControl = false;
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = QString::number(hv_Quality.D());
				_VecText1D.push_back(_strText);
			}
		}
		catch (const HException& except) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			QString strError = except.ErrorText();
			QStringList strErrorArray = strError.split(':');
			if (strErrorArray.size() > 1) {
				if (!strErrorArray[1].isEmpty()) {
					strError.clear();
					for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
					_strText.strText = "查找模板错误" + strError;
				}
			}
			_VecText1D.push_back(_strText);
		}
		catch (...) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = "查找模板错误";
			_VecText1D.push_back(_strText);
		}
		m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
		m_pMarkGraphViews->DispTextList(_VecText1D);
		m_pMarkGraphViews->DispCrossPoint(_PointLst);
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->UpdateImg();
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->update();
	});

}

void QtVisionWidget::on_btnZIFUIdentify_clicked()
{
	m_pMarkGraphViews->ClearAllDrawItem();
	QElapsedTimer _timer;
	_timer.start();

	QVector<ZIFUVisionStruct> vecVision;
	TableToZIFUStruct(ui.tableCodeWidget, vecVision);

	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
	else
	{
		dataVar::Instance().pManager->notify(tr("Didn't Connected Camera ") + strCam, NotifyType_Error);
		return;
	}
	QtConcurrent::run([=] {
		QVector<QPointF>	_VecPt1D;
		QVector<sDrawText>	_VecText1D;
		QVector<sDrawCross> _PointLst;
		try {
			for (size_t iRow = 0; iRow < vecVision.size(); iRow++) {
				Sleep(500);
				m_pCamera->Trigger();
				if (m_pCamera->WaitTrigger(5000) < 0) {
					dataVar::Instance().pManager->notify(tr("Trigger Camera TimeOut") + strCam, NotifyType_Error);
					return;
				}
				HObject	_ReduceImg;
				ShapeToRegion(m_VecCodeBaseItem[iRow], m_SearchROIRegion);
				HalconCpp::ReduceDomain(m_dstImage, m_SearchROIRegion, &_ReduceImg);

				QElapsedTimer	_timer;
				_timer.start();


				HTuple  hv_OCVHandle, hv_I, hv_Quality, hv_Color;
				CreateOcvProj(QString::number(iRow).toStdString().c_str(), &hv_OCVHandle);
				TraindOcvProj(_ReduceImg, hv_OCVHandle, QString::number(iRow).toStdString().c_str(), "single");

				//识别
				DoOcvSimple(_ReduceImg, hv_OCVHandle, "A", "true", "true", "true", "true",
					5, &hv_Quality);
				//switch (vecVision[iRow].iCodeType) {
				//case 0: {
				//	HObject  ho_SymbolRegions;
				//	// Local control variables
				//	HTuple  hv_BarCodeHandle, hv_DecodedDataStrings;
				//	HTuple  hv_BarCodeResults;
				//	CreateBarCodeModel(HTuple(), HTuple(), &hv_BarCodeHandle);
				//	SetBarCodeParam(hv_BarCodeHandle, "stop_after_result_num", 0);
				//	FindBarCode(_ReduceImg, &ho_SymbolRegions, hv_BarCodeHandle, "auto", &hv_DecodedDataStrings);
				//	GetBarCodeResult(hv_BarCodeHandle, "all", "decoded_types", &hv_BarCodeResults);
				//	ClearBarCodeModel(hv_BarCodeHandle);
				//	double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;
				//	if (hv_BarCodeResults.TupleLength() > 0)
				//	{
				//		for (size_t i = 0; i < hv_BarCodeResults.TupleLength().I(); i++) {
				//			sDrawText			_strText;
				//			_strText.bControl = true;
				//			_strText.DrawColor = QColor(255, 0, 0);
				//			_strText.strText = hv_BarCodeResults[i].S() + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
				//			_VecText1D.push_back(_strText);
				//		}
				//	}
				//	else
				//	{
				//		sDrawText			_strText;
				//		_strText.bControl = true;
				//		_strText.DrawColor = QColor(255, 0, 0);
				//		_strText.strText = tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
				//		_VecText1D.push_back(_strText);
				//	}
				//}	break;
				//case 1: {
				//	HObject  ho_SymbolXLDs;
				//	HTuple  hv_DataCodeHandle, hv_ResultHandles, hv_DecodedDataStrings1;
				//	switch (vecVision[iRow].iCode) {
				//	case 0: {
				//		CreateDataCode2dModel("QR Code", HTuple(), HTuple(), &hv_DataCodeHandle);
				//	}	break;
				//	case 1: {
				//		CreateDataCode2dModel("Data Matrix ECC 200", HTuple(), HTuple(), &hv_DataCodeHandle);
				//	}	break;
				//	case 2: {
				//		CreateDataCode2dModel("Micro QR Code", HTuple(), HTuple(), &hv_DataCodeHandle);
				//	}	break;
				//	default:
				//		break;
				//	}
				//	//*识别模式 最强
				//	SetDataCode2dParam(hv_DataCodeHandle, "default_parameters", "maximum_recognition");
				//	FindDataCode2d(_ReduceImg, &ho_SymbolXLDs, hv_DataCodeHandle, HTuple(), HTuple(),
				//		&hv_ResultHandles, &hv_DecodedDataStrings1);
				//	ClearDataCode2dModel(hv_DataCodeHandle);
				//	double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;
				//	if (hv_DecodedDataStrings1.TupleLength() > 0)
				//	{
				//		for (size_t i = 0; i < hv_DecodedDataStrings1.TupleLength().I(); i++) {
				//			sDrawText			_strText;
				//			_strText.bControl = true;
				//			_strText.DrawColor = QColor(255, 0, 0);
				//			_strText.strText = hv_DecodedDataStrings1[i].S() + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
				//			_VecText1D.push_back(_strText);
				//		}
				//	}
				//	else
				//	{
				//		sDrawText			_strText;
				//		_strText.bControl = true;
				//		_strText.DrawColor = QColor(255, 0, 0);
				//		_strText.strText = tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
				//		_VecText1D.push_back(_strText);
				//	}
				//}	break;
				//default:
				//	break;
				//}
			}
		}
		catch (const HException& except) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			QString strError = except.ErrorText();
			QStringList strErrorArray = strError.split(':');
			if (strErrorArray.size() > 1) {
				if (!strErrorArray[1].isEmpty()) {
					strError.clear();
					for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
					_strText.strText = "查找模板错误" + strError;
				}
			}
			_VecText1D.push_back(_strText);
		}
		catch (...) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = "查找模板错误";
			_VecText1D.push_back(_strText);
		}
		m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
		m_pMarkGraphViews->DispTextList(_VecText1D);
		m_pMarkGraphViews->DispCrossPoint(_PointLst);
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->UpdateImg();
		if (m_pMarkGraphViews != nullptr)	m_pMarkGraphViews->update();
	});

}

bool QtVisionWidget::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType) {
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent != nullptr) {
			m_pMarkGraphViews->UpdateImg();
		}
	}
	return QtWidgetsBase::event(ev);
}

void QtVisionWidget::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
	QString strCam = ui.cbx_CameraName->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraExposure, QString::number(ui.spBx_Exposure->value()));
		m_pCamera->SetCameraParam(CameraGain, QString::number(ui.spBx_Gain->value()));
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_Continnue));
	}
	ResizeUI();
}

void QtVisionWidget::hideEvent(QHideEvent * ev)
{
	QtWidgetsBase::hideEvent(ev);
	//if (m_pCamera != nullptr)
	//	m_pCamera->SetStream();
}

void QtVisionWidget::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev);
	ResizeUI();
}

void QtVisionWidget::ResizeUI()
{
	{
		int _iWidth = ui.tableCodeWidget->width() / ui.tableCodeWidget->columnCount() - 1;
		for (int i = 0; i < ui.tableCodeWidget->columnCount(); i++) {
			ui.tableCodeWidget->setColumnWidth(i, _iWidth);
		}
	}	{
		int _iWidth = ui.tableZIFUWidget->width() / ui.tableZIFUWidget->columnCount() - 1;
		for (int i = 0; i < ui.tableZIFUWidget->columnCount(); i++) {
			ui.tableZIFUWidget->setColumnWidth(i, _iWidth);
		}
	}
}

void QtVisionWidget::UpdateIndex()
{
	for (int iRow = 0; iRow < ui.tableCodeWidget->rowCount(); iRow++) {
		if (ui.tableCodeWidget->item(iRow, 0) != nullptr) {
			ui.tableCodeWidget->item(iRow, 0)->setText(QString::number(iRow));
			ui.tableCodeWidget->item(iRow, 0)->setToolTip(QString::number(iRow));
		}
	}
	for (int iRow = 0; iRow < ui.tableZIFUWidget->rowCount(); iRow++) {
		if (ui.tableZIFUWidget->item(iRow, 0) != nullptr) {
			ui.tableZIFUWidget->item(iRow, 0)->setText(QString::number(iRow));
			ui.tableZIFUWidget->item(iRow, 0)->setToolTip(QString::number(iRow));
		}
	}
}

int QtVisionWidget::Excute(HalconCpp::HObject& image)
{
	QVector<QPointF>	_VecPt1D;
	QVector<sDrawText>	_VecText1D;
	QVector<sDrawCross> _PointLst;
	m_pMarkGraphViews->ClearAllDrawItem();

	HTuple hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_Greedy, hv_SubPixel;
	HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
	hv_AngleStart	= HTuple(m_param.m_fStartAngle).TupleRad();
	hv_AngleExtent	= HTuple(m_param.m_fEndAngle).TupleRad();
	hv_MinScore		= m_param.m_fMinScore ;
	hv_NumMatches	= m_param.m_iMatchNumber;
	hv_Greedy		= m_param.m_fMatchGreedy;
	hv_MaxOverlap	= m_param.m_fMatchOverLap;
	QString _strMsg = tr("FindModel");;

	QElapsedTimer	_timer;
	_timer.start();
	try {
		HObject	_ReduceImg, _ROIRegion;
		ReduceDomain(image, m_param.m_ROISearchRegion, &_ReduceImg);
		switch (m_param.m_eMatchtype) {
		case EnumModeType_Shape: {
			switch (m_param.m_iSubPixType) {
			case 0: {	hv_SubPixel = "none";						}break;
			case 1: {	hv_SubPixel = "interpolation";				}break;
			case 2: {	hv_SubPixel = "least_squares";				}break;
			case 3: {	hv_SubPixel = "least_squares_high";			}break;
			case 4: {	hv_SubPixel = "least_squares_very_high";	}break;
			default:	break;
			}
			FindShapeModel(_ReduceImg, m_hShapeModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, hv_Greedy, &Row, &Column, &Angle, &Score);
		}	break;
		case EnumModeType_NCC: {
			switch (m_param.m_iSubPixType) {
			case 0: {	hv_SubPixel = "false";			}break;
			case 1: {	hv_SubPixel = "true";			}break;
			default:	break;
			}
			FindNccModel(_ReduceImg, m_hNCCModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, &Row, &Column, &Angle, &Score);
		}	break;
		default:	break;
		}
		double	_fExcuteTime = _timer.nsecsElapsed() / 1000000;

		int	_iScoreIndex = Score.TupleLength();
		if (_iScoreIndex <= 0) {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = _strMsg + tr("NG!耗时") + QString::number(_fExcuteTime) + tr("ms");
			_VecText1D.push_back(_strText);
		}
		else {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.DrawColor = QColor(0, 255, 0);
			_strText.strText = _strMsg + tr("OK!耗时") + QString::number(_fExcuteTime) + tr("ms");
			_VecText1D.push_back(_strText);
		}			
		m_MatchResult.Rst();
		//m_MatchResult.iMatchNumber = _iScoreIndex;
		for (int hv_MatchingObjIdx = 0; hv_MatchingObjIdx < _iScoreIndex; hv_MatchingObjIdx++) {
			sDrawText _item;
			_item.bControl = true;
			_item.Center.setX(20);
			_item.Center.setY((hv_MatchingObjIdx + 1) * 20);
			_item.DrawColor = QColor(0, 255, 0);
			_item.strText = QString("r:%1,c:%2,a:%3,s:%4")
				.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
				.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision))
				.arg(QString::number(Angle[hv_MatchingObjIdx].ToTuple().TupleDeg().D(), 'f', dataVar::Instance().form_System_Precision))
				.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', dataVar::Instance().form_System_Precision));
			_VecText1D.push_back(_item);

			sDrawCross _Cross;
			_Cross.Center.setX(Column[hv_MatchingObjIdx].D());
			_Cross.Center.setY(Row[hv_MatchingObjIdx].D());
			_Cross.fAngle = Angle[hv_MatchingObjIdx].D();
			_Cross.DrawColor = QColor(0, 0, 255);
			_PointLst.push_back(_Cross);

			ResultPos _ptPos;
			_ptPos.fMatchX		= Column[hv_MatchingObjIdx].D();
			_ptPos.fMatchY		= Row[hv_MatchingObjIdx].D();
			_ptPos.fMatchAngle	= Angle[hv_MatchingObjIdx].D();
			m_MatchResult.vecPos.push_back(_ptPos);
		}
	}
	catch (const HException& except) {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				_strText.strText = tr("查找模板错误") + strError;
			}
		}
		_VecText1D.push_back(_strText);
	}
	catch (...) {
		sDrawText _strText;
		_strText.bControl = true;
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = tr("查找模板错误");
		_VecText1D.push_back(_strText);
	}

	m_pMarkGraphViews->DispPoint(_VecPt1D, QColor(0, 255, 0));
	m_pMarkGraphViews->DispTextList(_VecText1D);
	m_pMarkGraphViews->DispCrossPoint(_PointLst);
	//m_pMarkGraphViews->update();
	m_QCreateEvent.SetEvent();
	if (m_param.bSaveImage){
		QString strPath = QApplication::applicationDirPath() + "/SaveImage/" 
			+ QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + ".bmp";
		WriteImage(image,"bmp",0, strPath.toStdString().c_str());
	}

	return 0;
}
