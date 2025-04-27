#include "QtProductWidgets.h"
#include <QMenu>
#include "Pages/QtWaitWidgetsClass.h"
//#include "Motion/MotionRobot.h"
#include "Motion\MotionSerialPort.h"
#include "Motion\MOtionIO.h"

#include "Log/MyEvent.h"
#include "Vision/PluginsManager.h"
#include "Vision/CameraManager.h"
#include "Vision/VisionClass.h"
#include "Data/dataVar.h"
#include "Common/MyMessageBox.h"

void VisionProductTask::run()
{
	if (m_ptrVision != nullptr) {
		m_ptrVision->Excute(m_hImage);
		QApplication::postEvent(m_ptrVision, new MsgEvent(0, 0, ""));
	}
}

int VisionProductTask::RecieveImgBuffer(QString strCamName, unsigned char * gray, int iwidth, int iheight, int ibit, QString imgtype, int icount)
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
		BYTE* BImg = new BYTE[imgSize];
		BYTE* GImg = new BYTE[imgSize];
		BYTE* RImg = new BYTE[imgSize];
		size_t	Pixed = 0;
		for (size_t i = 0; i < iheight; i++) {
			size_t _Lenth = i * iwidth;
			for (size_t j = 0; j < iwidth; j++) {
				Pixed = j * 3 + _Lenth * 3;
				BImg[j + _Lenth] = gray[Pixed + 0];
				GImg[j + _Lenth] = gray[Pixed + 1];
				RImg[j + _Lenth] = gray[Pixed + 2];
			}
		}
		GenImage3(&m_hImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		delete[]RImg;
		delete[]GImg;
		delete[]BImg;
	}
	return 0;
}

QtProductWidgets::QtProductWidgets(QWidget *parent)
	: QtWidgetsBase(parent)
{
	ui.setupUi(this);
	//两视觉
	m_pMarkGraphViews = new QGraphicsViews(this);
	m_pMarkGraphViews->setText(tr("MarkA"));
	ui.CameraLayout->addWidget(m_pMarkGraphViews);
	connect(ui.Slider_Exposure, SIGNAL(sliderReleased()),			this, SLOT(slotUpDateExposure()));
	connect(ui.Slider_Gain,		SIGNAL(sliderReleased()),			this, SLOT(slotUpDateGain()));
	connect(ui.Slider_Light,	SIGNAL(sliderReleased()),			this, SLOT(slotUpDateLight()));
	connect(ui.cbx_Camera,		SIGNAL(currentIndexChanged(int)),	this, SLOT(slotChangeCamera(int)));
	connect(ui.cbx_Camera,		SIGNAL(sigPopup()),					this, SLOT(slotUpDateCameras()));
	connect(ui.tabCameraWidget,	SIGNAL(currentChanged(int)),	this, SLOT(slotChangeCameraParamIndex(int)));

	connect(ui.Slider_Gain, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_Gain->setValue(value * 0.1);
	});
	connect(ui.spBx_Gain, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_Gain->setValue(ui.spBx_Gain->value() * 10);
	});

	//connect(ui.Slider_CodeGain, &QSlider::valueChanged, this, [=](int value) {
	//	ui.spBx_CodeGain->setValue(value * 0.1);
	//});
	//connect(ui.spBx_CodeGain, &QDoubleSpinBox::editingFinished, this, [=]() {
	//	ui.Slider_CodeGain->setValue(ui.spBx_CodeGain->value() * 10);
	//});

	//m_pMarkScanCodeGraphViews = new QGraphicsViews(this);
	//m_pMarkScanCodeGraphViews->setText(tr("ScanCode"));
	//ui.CameraCodeLayout->addWidget(m_pMarkScanCodeGraphViews);
	//connect(ui.Slider_CodeExposure, SIGNAL(sliderReleased()), this, SLOT(slotUpDateExposure()));
	//connect(ui.Slider_CodeGain,		SIGNAL(sliderReleased()), this, SLOT(slotUpDateGain()));
	//connect(ui.Slider_CodeLight,	SIGNAL(sliderReleased()), this, SLOT(slotUpDateLight()));

	connect(ui.Slider_ROI, &QSlider::valueChanged, this, [=](int value) {	UpDateParam();	});

	m_icomId = 0;

	m_vecIOOutPut.push_back(ui.checkBoxSucker);
	connect(ui.checkBoxSucker, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//
	m_vecIOOutPut.push_back(ui.checkBoxBoardBlock);
	connect(ui.checkBoxBoardBlock, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//
	m_vecIOOutPut.push_back(ui.checkBoxGuiDaoBlock);
	connect(ui.checkBoxGuiDaoBlock, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//
	m_vecIOOutPut.push_back(ui.checkBoxLongMenBoardBlock);
	connect(ui.checkBoxLongMenBoardBlock, SIGNAL(pressed()), this, SLOT(slotPressIOOutPut()));//

	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotChangeTab(int)));

	connect(ui.btnMoveAxisPX, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisPX_Pressed()));//
	connect(ui.btnMoveAxisPY, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisPY_Pressed()));//
	connect(ui.btnMoveAxisPZ, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisPZ_Pressed()));//
	connect(ui.btnMoveAxisNX, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisNX_Pressed()));//
	connect(ui.btnMoveAxisNY, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisNY_Pressed()));//
	connect(ui.btnMoveAxisNZ, SIGNAL(pressed()), this, SLOT(slotbtnMoveAxisNZ_Pressed()));//

	connect(ui.btnMoveAxisPX, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//
	connect(ui.btnMoveAxisPY, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//
	connect(ui.btnMoveAxisPZ, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//
	connect(ui.btnMoveAxisNX, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//
	connect(ui.btnMoveAxisNY, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//
	connect(ui.btnMoveAxisNZ, SIGNAL(released()), this, SLOT(slotbtnStopAxis()));//

	m_pBtnGroupRadio = new QButtonGroup(this);
	m_pBtnGroupRadio->addButton(ui.radioButtonRelative, 0);
	m_pBtnGroupRadio->addButton(ui.radioButtonAbsolute, 1);
	m_pBtnGroupRadio->addButton(ui.radioButtonJog, 2);
	ui.radioButtonRelative->click();
	ui.radioButtonRelative->setChecked(true);
	connect(ui.cBx_SelectedDistance, SIGNAL(activated(int)), this, SLOT(slotChangeDistance(int)));

}

QtProductWidgets::~QtProductWidgets()
{
	CloseUI();
	if (m_pBtnGroupRadio != nullptr) delete m_pBtnGroupRadio; m_pBtnGroupRadio = nullptr;
}

QImage QtProductWidgets::ByteToQImage(unsigned char* bytes, int width, int height, int bytesPerLine, QImage::Format imageFormat)
{
	return QImage(bytes, width, height, bytesPerLine, imageFormat);
}

int QtProductWidgets::RecieveBuffer(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	if (isVisible())
	{
		VisionProductTask* _pTask = new VisionProductTask();
		_pTask->m_ptrVision = this;
		_pTask->setAutoDelete(true);

		if (m_pMarkGraphViews != nullptr) {
			m_pMarkGraphViews->DispImage(ByteToQImage(gray, iwidth, iheight, iwidth * (ibit == 24 ? 3 : 1), ((ibit == 24) ? QImage::Format_RGB32 : QImage::Format_Indexed8)), false);
		}
		//switch (ui.tabCameraWidget->currentIndex()) {
		//case 1: {	
		//}	break;
		//case 2: {		
		//	if (m_pMarkGraphViews != nullptr) {
		//		m_pMarkGraphViews->DispImage(ByteToQImage(gray, iwidth, iheight, iwidth * (ibit == 24 ? 3 : 1), ((ibit == 24) ? QImage::Format_RGB32 : QImage::Format_Indexed8)), false);
		//	}
		//}	break;
		//default:	break;
		//}

		_pTask->RecieveImgBuffer(strCamName,gray, iwidth, iheight, ibit, imgtype, icount);
		VisionManager::ThreadPool().start(_pTask);

	}
	return 0;
}

int QtProductWidgets::RecieveRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	return 0;
}

void QtProductWidgets::Connected()
{
	
}

bool QtProductWidgets::CheckPageAvalible()
{
	ResizeUI();
	return false;
}

int QtProductWidgets::CloseUI()
{
	return 0;
}

int QtProductWidgets::initData(QString& strError)
{
	for (size_t i = 0; i < ui.tableWidget->rowCount(); i++)		ui.tableWidget->removeRow(i);
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidget, &QTableWidget::customContextMenuRequested, this, &QtProductWidgets::slotContextMenuRequested);
	ui.tableWidget->horizontalHeader()->setVisible(true);

	for (size_t i = 0; i < ui.tableWasteWidget->rowCount(); i++)		ui.tableWasteWidget->removeRow(i);
	ui.tableWasteWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWasteWidget, &QTableWidget::customContextMenuRequested, this, &QtProductWidgets::slotWasteContextMenuRequested);
	ui.tableWasteWidget->horizontalHeader()->setVisible(true);

	for (size_t i = 0; i < ui.tableLowSpeedWidget->rowCount(); i++)		ui.tableLowSpeedWidget->removeRow(i);
	ui.tableLowSpeedWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	//connect(ui.tableLowSpeedWidget, &QTableWidget::customContextMenuRequested, this, &QtProductWidgets::slotContextMenuRequested);
	ui.tableLowSpeedWidget->horizontalHeader()->setVisible(true);

	for (size_t i = 0; i < ui.tableHighSpeedWidget->rowCount(); i++)		ui.tableHighSpeedWidget->removeRow(i);
	ui.tableHighSpeedWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	//connect(ui.tableLowSpeedWidget, &QTableWidget::customContextMenuRequested, this, &QtProductWidgets::slotContextMenuRequested);
	ui.tableHighSpeedWidget->horizontalHeader()->setVisible(true);

	ui.tabWidget->setCurrentIndex(0);
	ui.tabCameraWidget->setCurrentIndex(0);


	return 0;
}

int QtProductWidgets::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	//延时	
	CommonParam.insert("dSpBx_ClampPickDelay",			QString::number(ui.dSpBx_ClampPickDelay->value()));

	CommonParam.insert("dSpBx_PhotoDelay",				QString::number(ui.dSpBx_PhotoDelay->value()));
	CommonParam.insert("dSpBx_CylinderDelay",			QString::number(ui.dSpBx_CylinderDelay->value()));
	CommonParam.insert("dSpBx_PickDelay",				QString::number(ui.dSpBx_PickDelay->value()));
	CommonParam.insert("dSpBx_PlaceDelay",				QString::number(ui.dSpBx_PlaceDelay->value()));
	CommonParam.insert("dSpBx_ThreeLightTrriger",		QString::number(ui.dSpBx_ThreeLightTrriger->value()));
	CommonParam.insert("dSpBx_ThreeLightFlash",			QString::number(ui.dSpBx_ThreeLightFlash->value()));

	CommonParam.insert("dSpBx_RePhotoTime",				QString::number(ui.dSpBx_RePhotoTime->value()));
	CommonParam.insert("dSpBx_CylinderTimeOut",			QString::number(ui.dSpBx_CylinderTimeOut->value()));
	CommonParam.insert("dSpBx_PailiaoDelay",			QString::number(ui.dSpBx_PailiaoDelay->value()));
	CommonParam.insert("dSpBx_PailiaoTimeOut",			QString::number(ui.dSpBx_PailiaoTimeOut->value()));
	CommonParam.insert("cBxClampType",					QString::number(ui.cBxClampType->currentIndex()));
	CommonParam.insert("dSpBx_PiDaiDelay",				QString::number(ui.dSpBx_PiDaiDelay->value()));

	CommonParam.insert("dSpBx_DenShenCylinderDelay",	QString::number(ui.dSpBx_DenShenCylinderDelay->value()));
	CommonParam.insert("dSpBx_ClampCylinderDelay",		QString::number(ui.dSpBx_ClampCylinderDelay->value()));
	CommonParam.insert("dSpBx_UpDownCylinderDelay",		QString::number(ui.dSpBx_UpDownCylinderDelay->value()));

	CommonParam.insert("dSpBx_DenShenCylinderDelay_2",	QString::number(ui.dSpBx_DenShenCylinderDelay_2->value()));
	CommonParam.insert("dSpBx_ClampCylinderDelay_2",	QString::number(ui.dSpBx_ClampCylinderDelay_2->value()));
	CommonParam.insert("dSpBx_UpDownCylinderDelay_2",	QString::number(ui.dSpBx_UpDownCylinderDelay_2->value()));

	CommonParam.insert("dSpBx_BlockCylinderDelay",		QString::number(ui.dSpBx_BlockCylinderDelay->value()));
	CommonParam.insert("dSpBx_PlatCylinderDelay",		QString::number(ui.dSpBx_PlatCylinderDelay->value()));
	CommonParam.insert("dSpBx_BeltStopDelay",			QString::number(ui.dSpBx_BeltStopDelay->value()));

	CommonParam.insert("cBx_CheckModeType",				QString::number(ui.cBx_CheckModeType->currentIndex()));

	CommonParam.insert("spBx_ROI",						QString::number(ui.spBx_ROI->value()));
	CommonParam.insert("spBx_Light",					QString::number(ui.spBx_Light->value()));
	CommonParam.insert("cBx_Channel",					QString::number(ui.cBx_Channel->currentIndex()));
	CommonParam.insert("Slider_Exposure",				QString::number(ui.Slider_Exposure->value()));
	CommonParam.insert("Slider_Gain",					QString::number(ui.spBx_Gain->value()));
	//位置参数
	CommonParam.insert("dSpBx_AxisSafePos",				QString::number(ui.dSpBx_AxisSafePos->value()));

	CommonParam.insert("dSpBx_AxisPRPosX",				QString::number(ui.dSpBx_AxisPRPosX->value()));
	CommonParam.insert("dSpBx_AxisPRPosY",				QString::number(ui.dSpBx_AxisPRPosY->value()));
	CommonParam.insert("dSpBx_AxisPRPosZ",				QString::number(ui.dSpBx_AxisPRPosZ->value()));
	//扫码
	CommonParam.insert("cBx_TargetColor",				QString::number(ui.cBx_TargetColor->currentIndex()));
	CommonParam.insert("cBx_CodeType",					QString::number(ui.cBx_CodeType->currentIndex()));
	CommonParam.insert("cBx_QRCodeType",				QString::number(ui.cBx_QRCodeType->currentIndex()));
	CommonParam.insert("Slider_CodeROI",				QString::number(ui.Slider_CodeROI->value()));
	CommonParam.insert("Slider_CodeLight",				QString::number(ui.Slider_CodeLight->value()));
	CommonParam.insert("cBx_CodeChannel",				QString::number(ui.cBx_CodeChannel->currentIndex()));
	CommonParam.insert("Slider_CodeExposure",			QString::number(ui.Slider_CodeExposure->value()));
	CommonParam.insert("Slider_CodeGain",				QString::number(ui.spBx_CodeGain->value()));

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableWidget->cellWidget(iRow, iCol);
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
						ConFig.append(QString(((QComboBox*)_pQWidget)->currentText()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("PositionParam", PositionParam);
	}

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableWasteWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableWasteWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableWasteWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableWasteWidget->cellWidget(iRow, iCol);
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
						ConFig.append(QString(((QComboBox*)_pQWidget)->currentText()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("tableWasteWidget", PositionParam);
	}

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableLowSpeedWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableLowSpeedWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableLowSpeedWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableLowSpeedWidget->cellWidget(iRow, iCol);
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
						ConFig.append(QString(((QComboBox*)_pQWidget)->currentText()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("tableLowSpeedWidget", PositionParam);
	}

	{
		QJsonArray PositionParam;
		for (int iRow = 0; iRow < ui.tableHighSpeedWidget->rowCount(); iRow++) {
			QJsonArray ConFig;
			for (int iCol = 0; iCol < ui.tableHighSpeedWidget->columnCount(); iCol++) {
				QTableWidgetItem * currentItem = ui.tableHighSpeedWidget->item(iRow, iCol);
				QWidget* _pQWidget = ui.tableHighSpeedWidget->cellWidget(iRow, iCol);
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
						ConFig.append(QString(((QComboBox*)_pQWidget)->currentText()));
					}
				}
			}
			PositionParam.append(ConFig);
		}
		CommonParam.insert("tableHighSpeedWidget", PositionParam);
	}

	strData.insert("QtProductWidgets", CommonParam);
	UpDateUI();
	return 0;
}

int QtProductWidgets::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtProductWidgets").value().toObject();
	if (CommonParam.contains("dSpBx_PhotoDelay"))				ui.dSpBx_PhotoDelay->setValue(CommonParam["dSpBx_PhotoDelay"].toString().toDouble());
	if (CommonParam.contains("dSpBx_CylinderDelay"))			ui.dSpBx_CylinderDelay->setValue(CommonParam["dSpBx_CylinderDelay"].toString().toDouble());
	if (CommonParam.contains("dSpBx_PickDelay"))				ui.dSpBx_PickDelay->setValue(CommonParam["dSpBx_PickDelay"].toString().toDouble());
	if (CommonParam.contains("dSpBx_PlaceDelay"))				ui.dSpBx_PlaceDelay->setValue(CommonParam["dSpBx_PlaceDelay"].toString().toDouble());
	if (CommonParam.contains("dSpBx_ThreeLightTrriger"))		ui.dSpBx_ThreeLightTrriger->setValue(CommonParam["dSpBx_ThreeLightTrriger"].toString().toDouble());
	if (CommonParam.contains("dSpBx_ThreeLightFlash"))			ui.dSpBx_ThreeLightFlash->setValue(CommonParam["dSpBx_ThreeLightFlash"].toString().toDouble());

	if (CommonParam.contains("dSpBx_RePhotoTime"))				ui.dSpBx_RePhotoTime->setValue(CommonParam["dSpBx_RePhotoTime"].toString().toInt());
	if (CommonParam.contains("dSpBx_CylinderTimeOut"))			ui.dSpBx_CylinderTimeOut->setValue(CommonParam["dSpBx_CylinderTimeOut"].toString().toInt());
	if (CommonParam.contains("dSpBx_PailiaoDelay"))				ui.dSpBx_PailiaoDelay->setValue(CommonParam["dSpBx_PailiaoDelay"].toString().toInt());
	if (CommonParam.contains("dSpBx_PailiaoTimeOut"))			ui.dSpBx_PailiaoTimeOut->setValue(CommonParam["dSpBx_PailiaoTimeOut"].toString().toInt());
	if (CommonParam.contains("cBxClampType"))					ui.cBxClampType->setCurrentIndex(CommonParam["cBxClampType"].toString().toInt());
	if (CommonParam.contains("dSpBx_PiDaiDelay"))				ui.dSpBx_PiDaiDelay->setValue(CommonParam["dSpBx_PiDaiDelay"].toString().toInt());
	ui.cBxClampType->setCurrentIndex(1);
	if (CommonParam.contains("dSpBx_DenShenCylinderDelay"))		ui.dSpBx_DenShenCylinderDelay->setValue(CommonParam["dSpBx_DenShenCylinderDelay"].toString().toInt());
	if (CommonParam.contains("dSpBx_ClampCylinderDelay"))		ui.dSpBx_ClampCylinderDelay->setValue(CommonParam["dSpBx_ClampCylinderDelay"].toString().toInt());
	if (CommonParam.contains("dSpBx_UpDownCylinderDelay"))		ui.dSpBx_UpDownCylinderDelay->setValue(CommonParam["dSpBx_UpDownCylinderDelay"].toString().toInt());

	if (CommonParam.contains("dSpBx_UpDownCylinderDelay_2"))	ui.dSpBx_UpDownCylinderDelay_2->setValue(CommonParam["dSpBx_UpDownCylinderDelay_2"].toString().toInt());
	if (CommonParam.contains("dSpBx_ClampCylinderDelay_2"))		ui.dSpBx_ClampCylinderDelay_2->setValue(CommonParam["dSpBx_ClampCylinderDelay_2"].toString().toInt());
	if (CommonParam.contains("dSpBx_DenShenCylinderDelay_2"))	ui.dSpBx_DenShenCylinderDelay_2->setValue(CommonParam["dSpBx_DenShenCylinderDelay_2"].toString().toInt());

	if (CommonParam.contains("dSpBx_BlockCylinderDelay"))		ui.dSpBx_BlockCylinderDelay->setValue(CommonParam["dSpBx_BlockCylinderDelay"].toString().toInt());
	if (CommonParam.contains("dSpBx_PlatCylinderDelay"))		ui.dSpBx_PlatCylinderDelay->setValue(CommonParam["dSpBx_PlatCylinderDelay"].toString().toInt());
	if (CommonParam.contains("dSpBx_BeltStopDelay"))			ui.dSpBx_BeltStopDelay->setValue(CommonParam["dSpBx_BeltStopDelay"].toString().toInt());

	if (CommonParam.contains("cBx_CheckModeType"))				ui.cBx_CheckModeType->setCurrentIndex(CommonParam["cBx_CheckModeType"].toString().toInt());

	if (CommonParam.contains("dSpBx_ClampPickDelay"))			ui.dSpBx_ClampPickDelay->setValue(CommonParam["dSpBx_ClampPickDelay"].toString().toInt());

	if (CommonParam.contains("spBx_ROI"))						ui.spBx_ROI->setValue(CommonParam["spBx_ROI"].toString().toInt());
	if (CommonParam.contains("spBx_Light"))						ui.spBx_Light->setValue(CommonParam["spBx_Light"].toString().toInt());
	if (CommonParam.contains("cBx_Channel"))					ui.cBx_Channel->setCurrentIndex(CommonParam["cBx_Channel"].toString().toInt());
	if (CommonParam.contains("Slider_Exposure"))				ui.Slider_Exposure->setValue(CommonParam["Slider_Exposure"].toString().toInt());
	if (CommonParam.contains("Slider_Gain"))					ui.spBx_Gain->setValue(CommonParam["Slider_Gain"].toString().toInt());

	//位置参数
	if (CommonParam.contains("dSpBx_AxisSafePos"))				ui.dSpBx_AxisSafePos->setValue(CommonParam["dSpBx_AxisSafePos"].toString().toDouble());
	
	if (CommonParam.contains("dSpBx_AxisPRPosX"))				ui.dSpBx_AxisPRPosX->setValue(CommonParam["dSpBx_AxisPRPosX"].toString().toDouble());
	if (CommonParam.contains("dSpBx_AxisPRPosY"))				ui.dSpBx_AxisPRPosY->setValue(CommonParam["dSpBx_AxisPRPosY"].toString().toDouble());
	if (CommonParam.contains("dSpBx_AxisPRPosZ"))				ui.dSpBx_AxisPRPosZ->setValue(CommonParam["dSpBx_AxisPRPosZ"].toString().toDouble());

	//扫码
	if (CommonParam.contains("cBx_TargetColor"))				ui.cBx_TargetColor->setCurrentIndex(CommonParam["cBx_TargetColor"].toString().toInt());
	if (CommonParam.contains("cBx_CodeType"))					ui.cBx_CodeType->setCurrentIndex(CommonParam["cBx_CodeType"].toString().toInt());
	if (CommonParam.contains("cBx_QRCodeType"))					ui.cBx_QRCodeType->setCurrentIndex(CommonParam["cBx_QRCodeType"].toString().toInt());
	if (CommonParam.contains("Slider_CodeROI"))					ui.spBx_CodeROI->setValue(CommonParam["spBx_CodeROI"].toString().toInt());
	if (CommonParam.contains("spBx_CodeLight"))					ui.spBx_CodeLight->setValue(CommonParam["spBx_CodeLight"].toString().toInt());
	if (CommonParam.contains("cBx_CodeChannel"))				ui.cBx_CodeChannel->setCurrentIndex(CommonParam["cBx_CodeChannel"].toString().toInt());
	if (CommonParam.contains("Slider_CodeExposure"))			ui.Slider_CodeExposure->setValue(CommonParam["Slider_CodeExposure"].toString().toInt());
	if (CommonParam.contains("Slider_CodeGain"))				ui.spBx_CodeGain->setValue(CommonParam["Slider_CodeGain"].toString().toInt());

	{
		int iRowCount = ui.tableWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("PositionParam").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableWidget->insertRow(iRow);
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
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
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
					ui.tableWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	{
		int iRowCount = ui.tableWasteWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableWasteWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("tableWasteWidget").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableWasteWidget->insertRow(iRow);
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
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 7: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem(tr("Enable"));
					((QComboBox*)_pQWidget)->addItem(tr("Disable"));
					((QComboBox*)_pQWidget)->setCurrentIndex(strValue.toDouble());
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					ui.tableWasteWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableWasteWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	{
		int iRowCount = ui.tableLowSpeedWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableLowSpeedWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("tableLowSpeedWidget").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableLowSpeedWidget->insertRow(iRow);
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
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 7: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem("1");
					((QComboBox*)_pQWidget)->addItem("2");
					((QComboBox*)_pQWidget)->addItem("3");
					((QComboBox*)_pQWidget)->setCurrentText(strValue);
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					ui.tableLowSpeedWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableLowSpeedWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	{
		int iRowCount = ui.tableHighSpeedWidget->rowCount();
		for (int i = 0; i < iRowCount; i++)		ui.tableHighSpeedWidget->removeRow(0);
		QJsonArray PositionParam = CommonParam.find("tableHighSpeedWidget").value().toArray();
		for (int iRow = 0; iRow < PositionParam.count(); iRow++) {
			QJsonArray CParam = PositionParam.at(iRow).toArray();
			ui.tableHighSpeedWidget->insertRow(iRow);
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
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6: {
					_pQWidget = new QDoubleSpinBox();
					((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
					((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
					((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
					((QDoubleSpinBox*)_pQWidget)->setValue(strValue.toDouble());
				}	break;
				case 7: {
					_pQWidget = new QComboBox();
					((QComboBox*)_pQWidget)->addItem("1");
					((QComboBox*)_pQWidget)->addItem("2");
					((QComboBox*)_pQWidget)->addItem("3");
					((QComboBox*)_pQWidget)->setCurrentText(strValue);
				}	break;
				default: {	}	break;
				}
				if (_pItem != nullptr) {
					_pItem->setTextAlignment(Qt::AlignCenter);
					ui.tableHighSpeedWidget->setItem(iRow, iCol, _pItem);
				}
				if (_pQWidget != nullptr) ui.tableHighSpeedWidget->setCellWidget(iRow, iCol, _pQWidget);
			}
		}
	}

	ResizeUI();
	////使用当前机器人的配方号
	//if (MotionRobot::getInstance().IsConnected()) {
	//	MotionRobot::getInstance().SetRoBotData(2, 1);
	//}
	slotChangeAxisSpeedParam();
	UpDateUI();

	return 0;
}

int QtProductWidgets::NewProject()
{
	return 0;
}

int QtProductWidgets::GetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam;


	strData.insert("QtProductWidgets", CommonParam);

	return 0;
}

int QtProductWidgets::SetSystemData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtProductWidgets").value().toObject();

	if (dataVar::Instance().bAutoConnectedWhenStart)	{
		Connected();
	}
	UpDateUI();
	return 0;
}

int QtProductWidgets::SCanTimerUI()
{
	for (int _iPort = 0; _iPort < m_vecIOOutPut.size(); _iPort++) {
		if (m_vecIOOutPut[_iPort] != nullptr) {
			int iCard = 0;
			int iPort = 0;
			bool bStatus = false;

			if (m_vecIOOutPut[_iPort] == ui.checkBoxSucker) {
				iPort = EnumOutPut_LongmenClamp;
			}
			else if (m_vecIOOutPut[_iPort] == ui.checkBoxBoardBlock) {
				iPort = EnumOutPut_BoardInBlock;
			}
			else if (m_vecIOOutPut[_iPort] == ui.checkBoxGuiDaoBlock) {
				iPort = EnumOutPut_GuidaoBlock;
			}
			else if (m_vecIOOutPut[_iPort] == ui.checkBoxLongMenBoardBlock) {
				iPort = EnumOutPut_LongmenBlock;
			}
			//else if (m_vecIOOutPut[_iPort] == ui.checkBoxBlock) {
			//	iPort = EnumOutPut_Zudang;
			//}
			MotionIO::getInstance().GetOutPortStatus(iCard, iPort, bStatus);
			(m_vecIOOutPut[_iPort])->setChecked(bStatus);
		}
	}

	return 0;
}

void QtProductWidgets::UpDateUI()
{
	dataVar::Instance().Param.Rst();

	dataVar::Instance().Param.iClampDelay				= ui.dSpBx_ClampPickDelay->value();

	dataVar::Instance().Param.iPhotoWaitDelay			= ui.dSpBx_PhotoDelay->value();
	dataVar::Instance().Param.CylinderWaitDelay			= ui.dSpBx_CylinderDelay->value();
	dataVar::Instance().Param.PickWaitDelay				= ui.dSpBx_PickDelay->value();
	dataVar::Instance().Param.PlaceWaitDelay			= ui.dSpBx_PlaceDelay->value();

	dataVar::Instance().Param.iRePhotoTime				= ui.dSpBx_RePhotoTime->value();
	dataVar::Instance().Param.iCylinderTimeOut			= ui.dSpBx_CylinderTimeOut->value();
	dataVar::Instance().Param.ThreeLightFlashSecond		= ui.dSpBx_ThreeLightFlash->value();
	dataVar::Instance().Param.iPiDaiMoveDelay			= ui.dSpBx_PailiaoDelay->value();
	dataVar::Instance().Param.iBeltMoveTimeOut			= ui.dSpBx_PailiaoTimeOut->value();
	dataVar::Instance().Param.iClampType				= ui.cBxClampType->currentIndex();
	dataVar::Instance().Param.iBoardOutPiDaiDelay		= ui.dSpBx_PiDaiDelay->value();

	dataVar::Instance().Param.iInDenShenCylinderDelay	= ui.dSpBx_DenShenCylinderDelay->value();
	dataVar::Instance().Param.iInClampCylinderDelay		= ui.dSpBx_ClampCylinderDelay->value();
	dataVar::Instance().Param.iInUpDownCylinderDelay	= ui.dSpBx_UpDownCylinderDelay->value();

	dataVar::Instance().Param.iOutDenShenCylinderDelay	= ui.dSpBx_DenShenCylinderDelay_2->value();
	dataVar::Instance().Param.iOutClampCylinderDelay	= ui.dSpBx_ClampCylinderDelay_2->value();
	dataVar::Instance().Param.iOutUpDownCylinderDelay	= ui.dSpBx_UpDownCylinderDelay_2->value();

	dataVar::Instance().Param.iBlockCylinderDelay		= ui.dSpBx_BlockCylinderDelay->value();
	dataVar::Instance().Param.iLonMenBlockCylinderDelay = ui.dSpBx_PlatCylinderDelay->value();
	dataVar::Instance().Param.iBeltPlatStopDelay		= ui.dSpBx_BeltStopDelay->value();

	dataVar::Instance().Param.icBx_CheckModeType		= ui.cBx_CheckModeType->currentIndex();
	//位置参数
	dataVar::Instance().Param.dAxisZSafePos				= ui.dSpBx_AxisSafePos->value();
	dataVar::Instance().Param.dPhotoPos.X				= ui.dSpBx_AxisPRPosX->value();
	dataVar::Instance().Param.dPhotoPos.Y				= ui.dSpBx_AxisPRPosY->value();
	dataVar::Instance().Param.dPhotoPos.Z				= ui.dSpBx_AxisPRPosZ->value();

	dataVar::Instance().Param.ThreeLightTrrigerSecond	= ui.dSpBx_ThreeLightTrriger->value();

	for (int iRow = 0; iRow < ui.tableWidget->rowCount(); iRow++) {
		ScrewPoint _ptCenter;
		for (int iCol = 0; iCol < ui.tableWidget->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = ui.tableWidget->item(iRow, iCol);
			QWidget* _pQWidget = ui.tableWidget->cellWidget(iRow, iCol);
			if (_pQWidget != nullptr) {
				QString	strClass = _pQWidget->metaObject()->className();
				if (strClass == "QDoubleSpinBox") {
					if (iCol == 1)	{
						_ptCenter.CameraPos.X = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if(iCol == 2) {
						_ptCenter.CameraPos.Y = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 3) {
						_ptCenter.CameraPos.Z = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 4) {
						_ptCenter.ScrewPos.X = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 5) {
						_ptCenter.ScrewPos.Y = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 6) {
						_ptCenter.ScrewPos.Z = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 7) {
						_ptCenter.iEnable = ((QComboBox*)_pQWidget)->currentIndex();
					}
				}
				else if (strClass == "QLabel") {

				}
				else if (strClass == "QComboBox") {

				}
			}
		}
		dataVar::Instance().Param.vecPos.push_back(_ptCenter);

	}

	for (int iRow = 0; iRow < ui.tableWasteWidget->rowCount(); iRow++) {
		ScrewPoint _ptCenter;
		for (int iCol = 0; iCol < ui.tableWasteWidget->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = ui.tableWasteWidget->item(iRow, iCol);
			QWidget* _pQWidget = ui.tableWasteWidget->cellWidget(iRow, iCol);
			if (_pQWidget != nullptr) {
				QString	strClass = _pQWidget->metaObject()->className();
				if (strClass == "QDoubleSpinBox") {
					if (iCol == 1) {
						_ptCenter.CameraPos.X = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 2) {
						_ptCenter.CameraPos.Y = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 3) {
						_ptCenter.CameraPos.Z = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 4) {
						_ptCenter.ScrewPos.X = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 5) {
						_ptCenter.ScrewPos.Y = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 6) {
						_ptCenter.ScrewPos.Z = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 7) {
						_ptCenter.iEnable = ((QComboBox*)_pQWidget)->currentIndex();
					}
				}
				else if (strClass == "QLabel") {

				}
				else if (strClass == "QComboBox") {

				}
			}
		}
		dataVar::Instance().Param.vecWastePos.push_back(_ptCenter);
	}

	for (int iRow = 0; iRow < ui.tableHighSpeedWidget->rowCount(); iRow++) {
		AxisSpeed _AxisSpeed;
		for (int iCol = 0; iCol < ui.tableHighSpeedWidget->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = ui.tableHighSpeedWidget->item(iRow, iCol);
			QWidget* _pQWidget = ui.tableHighSpeedWidget->cellWidget(iRow, iCol);
			if (_pQWidget != nullptr) {
				QString	strClass = _pQWidget->metaObject()->className();
				if (strClass == "QDoubleSpinBox") {
					if (iCol == 1) {
						_AxisSpeed.dStartSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 2) {
						_AxisSpeed.dEndSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 3) {
						_AxisSpeed.dAccSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 4) {
						_AxisSpeed.dDccSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 5) {
						_AxisSpeed.dSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
				}
				else if (strClass == "QLabel") {

				}
				else if (strClass == "QComboBox") {

				}
			}
		}
		dataVar::Instance().axisSpeedArray[iRow * 10 + 1].dAccSpeed		= _AxisSpeed.dAccSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 1].dDccSpeed		= _AxisSpeed.dDccSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 1].dStartSpeed	= _AxisSpeed.dStartSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 1].dEndSpeed		= _AxisSpeed.dEndSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 1].dSpeed		= _AxisSpeed.dSpeed;
		MotionIO::getInstance().SetAxisSpeedParam(iRow, 1, _AxisSpeed);
		//MotionIO::getInstance().SetAxisSpeedParam(iRow, 1, dataVar::Instance().axisSpeedArray[iRow * 10 + 1]);
	}

	for (int iRow = 0; iRow < ui.tableLowSpeedWidget->rowCount(); iRow++) {
		AxisSpeed _AxisSpeed;
		for (int iCol = 0; iCol < ui.tableLowSpeedWidget->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = ui.tableLowSpeedWidget->item(iRow, iCol);
			QWidget* _pQWidget = ui.tableLowSpeedWidget->cellWidget(iRow, iCol);
			if (_pQWidget != nullptr) {
				QString	strClass = _pQWidget->metaObject()->className();
				if (strClass == "QDoubleSpinBox") {
					if (iCol == 1) {
						_AxisSpeed.dStartSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 2) {
						_AxisSpeed.dEndSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 3) {
						_AxisSpeed.dAccSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 4) {
						_AxisSpeed.dDccSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
					else if (iCol == 5) {
						_AxisSpeed.dSpeed = ((QDoubleSpinBox*)_pQWidget)->value();
					}
				}
				else if (strClass == "QLabel") {

				}
				else if (strClass == "QComboBox") {

				}
			}
		}
		dataVar::Instance().axisSpeedArray[iRow * 10 + 0].dAccSpeed		= _AxisSpeed.dAccSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 0].dDccSpeed		= _AxisSpeed.dDccSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 0].dStartSpeed	= _AxisSpeed.dStartSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 0].dEndSpeed		= _AxisSpeed.dEndSpeed;
		dataVar::Instance().axisSpeedArray[iRow * 10 + 0].dSpeed		= _AxisSpeed.dSpeed;
		MotionIO::getInstance().SetAxisSpeedParam(iRow,0, _AxisSpeed);
		//MotionIO::getInstance().SetAxisSpeedParam(iRow, 0, dataVar::Instance().axisSpeedArray[iRow * 10 + 0]);
	}

	m_pMarkGraphViews->SetCircleSize(ui.Slider_ROI->value());
}

bool QtProductWidgets::CheckCylinderSafety()
{
	//int _iCard = 0;
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
	//		switch (result) {
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

void QtProductWidgets::on_btnTrriger_clicked()
{
	QString strMsg;
	switch (ui.cbxCameraType->currentIndex()){
	case 0: {
		dataVar::Instance().m_CTcpClient.WaitMsg("1", strMsg);
	}	break;
	case 1: {
		dataVar::Instance().m_CTcpClient.WaitMsg("2", strMsg);
	}	break;
	default:
		break;
	}
}

void QtProductWidgets::on_btnWorkPosMove_clicked()
{
	int _iIndex = ui.tableWidget->currentIndex().row();
	if (_iIndex < 0 ) {
		return;
	}
	if (!CheckCylinderSafety())	return;

	int _iCurrentRow = ui.tableWidget->currentIndex().row();
	if (_iCurrentRow < 0) {
		dataVar::Instance().pManager->notify(tr("Error"), tr(" Select Time Out!"), NotifyType_Error);
		return;
	}
	double _PosX = MotionIO::getInstance().GetAxisPos(0);
	double _PosY = MotionIO::getInstance().GetAxisPos(1);
	double _PosZ = MotionIO::getInstance().GetAxisPos(2);

	QWidget* _pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 1);
	if (_pQWidget != nullptr) {
		_PosX = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 2);
	if (_pQWidget != nullptr) {
		_PosY = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 3);
	if (_pQWidget != nullptr) {
		_PosZ = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;

	QString _strMsg = tr(" Move To Pos");

	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
		int _iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, dataVar::Instance().Param.dAxisZSafePos,
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		_iRetn = MotionIO::getInstance().WaitMutiAxisMovePosDone(0, _PosX,1, _PosY, 
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}
		_iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, _PosZ, 
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		Sleep(1000);
		return  StandardMsgResult::MsgResult_OK;
	}, _strMsg + tr(" Now !"), QtWaitWidgetsClass::MsgType_Close);
	switch (result) {
	case MsgResult_TimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
		break;
	case MsgResult_Close:
		dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
		break;
	case MsgResult_NG: {
		dataVar::Instance().pManager->notify(_strMsg + tr(" NG!"), NotifyType_Info);
	}	break;
	case MsgResult_OK:
	default: { }	break;
	}
}

void QtProductWidgets::on_btnWorkPhotoPosMove_clicked()
{
	int _iIndex = ui.tableWidget->currentIndex().row();
	if (_iIndex < 0) {
		return;
	}
	if (!CheckCylinderSafety())	return;

	int _iCurrentRow = ui.tableWidget->currentIndex().row();
	if (_iCurrentRow < 0) {
		dataVar::Instance().pManager->notify(tr("Error"), tr(" Select Time Out!"), NotifyType_Error);
		return;
	}
	double _PosX = MotionIO::getInstance().GetAxisPos(0);
	double _PosY = MotionIO::getInstance().GetAxisPos(1);
	double _PosZ = MotionIO::getInstance().GetAxisPos(2);

	QWidget* _pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 4);
	if (_pQWidget != nullptr) {
		_PosX = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 5);
	if (_pQWidget != nullptr) {
		_PosY = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(_iCurrentRow, 6);
	if (_pQWidget != nullptr) {
		_PosZ = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;

	QString _strMsg = tr(" Move To Pos");

	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
		int _iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, dataVar::Instance().Param.dAxisZSafePos, 
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		_iRetn = MotionIO::getInstance().WaitMutiAxisMovePosDone(0, _PosX, 1, _PosY,
			0,QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}
		_iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, _PosZ,
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		Sleep(1000);
		return  StandardMsgResult::MsgResult_OK;
	}, _strMsg + tr(" Now !"), QtWaitWidgetsClass::MsgType_Close);
	switch (result) {
	case MsgResult_TimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
		break;
	case MsgResult_Close:
		dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
		break;
	case MsgResult_NG: {
		dataVar::Instance().pManager->notify(_strMsg + tr(" NG!"), NotifyType_Info);
	}	break;
	case MsgResult_OK:
	default: { }	break;
	}
}

void QtProductWidgets::on_btnWorkClampPosLearn_clicked()
{
	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		double _PosX = MotionIO::getInstance().GetAxisPos(0);
		double _PosY = MotionIO::getInstance().GetAxisPos(1);
		double _PosZ = MotionIO::getInstance().GetAxisPos(2);

		int iCurrentIndex = ui.tableWidget->currentIndex().row();
		if (iCurrentIndex < 0)	{
			return;
		}

		QDoubleSpinBox* _pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 4);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosX);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosX);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 4, _pQWidget);
		}


		_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 5);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosY);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosY);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 5, _pQWidget);
		}

		_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 6);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosZ);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosZ);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 6, _pQWidget);
		}

	}
}

void QtProductWidgets::on_btnWorkPosLearn_clicked()
{
	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		double _PosX = MotionIO::getInstance().GetAxisPos(0);
		double _PosY = MotionIO::getInstance().GetAxisPos(1);
		double _PosZ = MotionIO::getInstance().GetAxisPos(2);

		int iCurrentIndex = ui.tableWidget->currentIndex().row();
		if (iCurrentIndex < 0)
		{
			return;
		}

		QDoubleSpinBox* _pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 1);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosX);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosX);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 1, _pQWidget);
		}


		_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 2);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosY);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosY);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 2, _pQWidget);
		}

		_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 3);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosZ);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosZ);
			if (_pQWidget != nullptr) ui.tableWidget->setCellWidget(iCurrentIndex, 3, _pQWidget);
		}

	}
}

void QtProductWidgets::on_btnWastePosMove_clicked()
{
	int _iIndex = ui.tableWasteWidget->currentIndex().row();
	if (_iIndex < 0) {
		return;
	}

	int _iCurrentRow = ui.tableWasteWidget->currentIndex().row();
	if (_iCurrentRow < 0) {
		dataVar::Instance().pManager->notify(tr("Error"), tr(" Select Time Out!"), NotifyType_Error);
		return;
	}
	double _PosX = MotionIO::getInstance().GetAxisPos(0);
	double _PosY = MotionIO::getInstance().GetAxisPos(1);
	double _PosZ = MotionIO::getInstance().GetAxisPos(2);

	QWidget* _pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(_iCurrentRow, 1);
	if (_pQWidget != nullptr) {
		_PosX = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(_iCurrentRow, 2);
	if (_pQWidget != nullptr) {
		_PosY = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;
	_pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(_iCurrentRow, 3);
	if (_pQWidget != nullptr) {
		_PosZ = ((QDoubleSpinBox*)_pQWidget)->value();
	}
	else return;

	QString _strMsg = tr(" MoveTo Waste Pannel");
	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
		int _iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, dataVar::Instance().Param.dAxisZSafePos,
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		_iRetn = MotionIO::getInstance().WaitMutiAxisMovePosDone(0, _PosX, 1, _PosY,
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}
		_iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, _PosZ,
			0, QString(), dataVar::Instance().iMotionTimeOut);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		Sleep(1000);
		return  StandardMsgResult::MsgResult_OK;
	}, _strMsg + tr(" Now !"), QtWaitWidgetsClass::MsgType_Close);
	switch (result) {
	case MsgResult_TimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
		break;
	case MsgResult_Close:
		dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
		break;
	case MsgResult_NG:
	case MsgResult_OK:
	default: { }	break;
	}
}

void QtProductWidgets::on_btnWastePosLearn_clicked()
{
	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		double _PosX = MotionIO::getInstance().GetAxisPos(0);
		double _PosY = MotionIO::getInstance().GetAxisPos(1);
		double _PosZ = MotionIO::getInstance().GetAxisPos(2);

		int iCurrentIndex = ui.tableWasteWidget->currentIndex().row();
		if (iCurrentIndex < 0)	{
			return;
		}

		QDoubleSpinBox* _pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(iCurrentIndex, 1);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosX);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(99999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosX);
			if (_pQWidget != nullptr) ui.tableWasteWidget->setCellWidget(iCurrentIndex, 1, _pQWidget);
		}


		_pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(iCurrentIndex, 2);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosY);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(99999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosY);
			if (_pQWidget != nullptr) ui.tableWasteWidget->setCellWidget(iCurrentIndex, 2, _pQWidget);
		}

		_pQWidget = (QDoubleSpinBox*)ui.tableWasteWidget->cellWidget(iCurrentIndex, 3);
		if (_pQWidget != nullptr)	_pQWidget->setValue(_PosZ);
		else {
			_pQWidget = new QDoubleSpinBox();
			((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
			((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999);
			((QDoubleSpinBox*)_pQWidget)->setMaximum(99999);
			((QDoubleSpinBox*)_pQWidget)->setValue(_PosZ);
			if (_pQWidget != nullptr) ui.tableWasteWidget->setCellWidget(iCurrentIndex, 3, _pQWidget);
		}
	}
}

void QtProductWidgets::on_btnPhotoSafePosLearn_clicked()
{
	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		double _PosX = MotionIO::getInstance().GetAxisPos(0);
		double _PosY = MotionIO::getInstance().GetAxisPos(1);
		double _PosZ = MotionIO::getInstance().GetAxisPos(2);
		ui.dSpBx_AxisSafePos->setValue(_PosZ);
	}
}

void QtProductWidgets::on_btnStopAllAxis_clicked()
{
	MotionIO::getInstance().StopAllAxis();
}

void QtProductWidgets::slotbtnMoveAxisPX_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {
		MotionIO::getInstance().AbsoluteMovePos(0, ui.doubleSpinBox->value());
	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(0, -ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(0, 0);
	}	break;
	}
}

void QtProductWidgets::slotbtnMoveAxisNX_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {	
		MotionIO::getInstance().AbsoluteMovePos(0, ui.doubleSpinBox->value());

	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(0, ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(0, 1);
	}	break;
	}
}

void QtProductWidgets::slotbtnMoveAxisPY_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {
		MotionIO::getInstance().AbsoluteMovePos(1,ui.doubleSpinBox->value());
	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(1, ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(1, 1);
	}	break;
	}
}

void QtProductWidgets::slotbtnMoveAxisNY_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {
		MotionIO::getInstance().AbsoluteMovePos(1, ui.doubleSpinBox->value());
	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(1, -ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(1, 0);
	}	break;
	}
}

void QtProductWidgets::slotbtnMoveAxisPZ_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {
		MotionIO::getInstance().AbsoluteMovePos(2, ui.doubleSpinBox->value());
	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(2,-ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(2, 0);
	}	break;
	}
}

void QtProductWidgets::slotbtnMoveAxisNZ_Pressed()
{
	switch (m_pBtnGroupRadio->checkedId()) {
	case 0: {
		MotionIO::getInstance().AbsoluteMovePos(2, ui.doubleSpinBox->value());
	}	break;
	case 1: {
		MotionIO::getInstance().MovePos(2, ui.doubleSpinBox->value());
	}	break;
	case 2: {
		MotionIO::getInstance().JogMovePos(2, 1);
	}	break;
	}
}

void QtProductWidgets::slotbtnStopAxis()
{
	auto pushbtn = (QPushButton*)sender();
	if (pushbtn == nullptr)	return;
	switch (m_pBtnGroupRadio->checkedId()) {
	case 2: {
		if (pushbtn == ui.btnMoveAxisNX || pushbtn == ui.btnMoveAxisPX)
		{
			MotionIO::getInstance().StopAxisMove(0);
		}
		else if (pushbtn == ui.btnMoveAxisNY || pushbtn == ui.btnMoveAxisPY)
		{
			MotionIO::getInstance().StopAxisMove(1);
		}
		else if (pushbtn == ui.btnMoveAxisNZ || pushbtn == ui.btnMoveAxisPZ)
		{
			MotionIO::getInstance().StopAxisMove(2);
		}
		else
		{
			MotionIO::getInstance().StopAllAxis();
		}
	}	break;
	}
}

void QtProductWidgets::on_btnPhotoLearn_clicked()
{
	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		double _PosX = MotionIO::getInstance().GetAxisPos(0);
		double _PosY = MotionIO::getInstance().GetAxisPos(1);
		double _PosZ = MotionIO::getInstance().GetAxisPos(2);

		ui.dSpBx_AxisPRPosX->setValue(_PosX);
		ui.dSpBx_AxisPRPosY->setValue(_PosY);
		ui.dSpBx_AxisPRPosZ->setValue(_PosZ);
	}
}

void QtProductWidgets::on_btnPhotoMove_clicked()
{
	double _PosX = MotionIO::getInstance().GetAxisPos(0);
	double _PosY = MotionIO::getInstance().GetAxisPos(1);
	double _PosZ = MotionIO::getInstance().GetAxisPos(2);

	_PosX = ui.dSpBx_AxisPRPosX->value();
	_PosY = ui.dSpBx_AxisPRPosY->value();
	_PosZ = ui.dSpBx_AxisPRPosZ->value();

	QString _strMsg = tr(" MoveTo Waste Pannel");
	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
		int _iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, dataVar::Instance().Param.dAxisZSafePos);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		_iRetn = MotionIO::getInstance().WaitMutiAxisMovePosDone(0, _PosX, 1, _PosY);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}
		_iRetn = MotionIO::getInstance().WaitAxisMovePosDone(2, _PosZ);
		if (_iRetn < 0) {
			return  StandardMsgResult::MsgResult_NG;
		}

		Sleep(1000);
		return  StandardMsgResult::MsgResult_OK;
	}, _strMsg + tr(" Now !"), QtWaitWidgetsClass::MsgType_Close);
	switch (result) {
	case MsgResult_TimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
		break;
	case MsgResult_Close:
		dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
		break;
	case MsgResult_NG:
	case MsgResult_OK:
	default: { }	break;
	}
}

void QtProductWidgets::slotUpDateExposure()
{
	switch (ui.tabCameraWidget->currentIndex())
	{
	case 1: {
		int _iValue = ui.Slider_Exposure->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraExposure, QString::number(_iValue));
		}
	}break;
	case 2: {
		int _iValue = ui.Slider_CodeExposure->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraExposure, QString::number(_iValue));
		}
	}break;
	default:
		break;
	}
}

void QtProductWidgets::slotUpDateGain()
{
	switch (ui.tabCameraWidget->currentIndex())
	{
	case 1: {
		int _iValue = ui.spBx_Gain->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraGain, QString::number(_iValue));
		}
	}break;
	case 2: {
		int _iValue = ui.spBx_CodeGain->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraGain, QString::number(_iValue));
		}
	}break;
	default:
		break;
	}
}

void QtProductWidgets::slotUpDateLight()
{
	switch (ui.tabCameraWidget->currentIndex()){
	case 1: {
		int _iChannel = ui.cBx_Channel->currentIndex();
		int _iValue = ui.Slider_Light->value();
		MotionSerialPort::getInstance().WriteLightValue(_iChannel, _iValue);
	}break;
	case 2: {	
		int _iChannel	= ui.cBx_CodeChannel->currentIndex();
		int _iValue		= ui.Slider_CodeLight->value();
		MotionSerialPort::getInstance().WriteLightValue(_iChannel, _iValue);
	}break;
	default:
		break;
	}
}

void QtProductWidgets::slotSetSpeed()
{
	//if (MotionRobot::getInstance().IsConnected()) {
	//	MotionRobot::getInstance().Set_Vel(ui.Slider_RobotSpeed->value());
	//}

}

void QtProductWidgets::slotChangeCamera(int)
{
	if (m_pCamera != nullptr) {
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
		m_pCamera->SetStream(nullptr);
	}
	QString strCam = ui.cbx_Camera->currentText();
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_Continnue));
	}
	QString	strValue;
	if (m_pCamera != nullptr)
	{
		m_pCamera->GetCameraParam(CameraMaxExposure, strValue);
		ui.Slider_CodeExposure->setMaximum(strValue.toInt());
		ui.spBx_CodeExposure->setMaximum(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.Slider_CodeExposure->setMinimum(strValue.toInt());
		ui.spBx_CodeExposure->setMinimum(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_CodeGain->setMaximum(strValue.toDouble());
		ui.Slider_CodeGain->setMaximum(strValue.toDouble() * 10);

		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_CodeGain->setMinimum(strValue.toDouble());
		ui.Slider_CodeGain->setMinimum(strValue.toDouble() * 10);
	}
}

void QtProductWidgets::slotPressIOOutPut()
{
	auto pushbtn = (QCheckBox*)sender();
	if (pushbtn == nullptr)	return;
	int iCard = 0;
	int iPort = 0;
	bool bStatus = false;

	if (pushbtn == ui.checkBoxSucker) {
		iPort = EnumOutPut_LongmenClamp;
	}
	else if (pushbtn == ui.checkBoxBoardBlock) {
		iPort = EnumOutPut_BoardInBlock;
	}
	else if (pushbtn == ui.checkBoxGuiDaoBlock) {
		iPort = EnumOutPut_GuidaoBlock;
	}
	else if (pushbtn == ui.checkBoxLongMenBoardBlock) {
		iPort = EnumOutPut_LongmenBlock;
	}
	MotionIO::getInstance().GetOutPortStatus(iCard, iPort, bStatus);
	MotionIO::getInstance().SetOutPortStatus(iCard, iPort, !bStatus);

}

void QtProductWidgets::slotUpDateParam(int index)
{
	UpDateParam();
}

void QtProductWidgets::slotChangeDistance(int index)
{
	ui.doubleSpinBox->setValue(ui.cBx_SelectedDistance->currentText().toDouble());
}

void QtProductWidgets::UpDateParam()
{
	m_pMarkGraphViews->SetCircleSize(ui.Slider_ROI->value());

	//m_param.iROISize		= ui.Slider_ROI->value();
	//m_param.Color			= (EnumTargetColor)ui.cBx_RecognizeColor->currentIndex();
	//m_param.Target			= (EnumTargetFeature)ui.cBx_RecognizeType->currentIndex();
	//m_param.AreaUpLimit		= ui.Slider_UpArea->value();
	//m_param.AreaDownLimit	= ui.Slider_DownArea->value();
	//m_param.RadiusUpLimit	= ui.Slider_UpRadius->value();
	//m_param.RadiusDownLimit = ui.Slider_DownRadius->value();
}

void QtProductWidgets::slotChangeAxisSpeedParam()
{
	int _iAxisNum = MotionIO::getInstance().GetAllAxisParam().size();
	{
		int iRowCount = ui.tableHighSpeedWidget->rowCount();
		if (iRowCount != _iAxisNum) {
			if (iRowCount > _iAxisNum) {
				for (int i = iRowCount; i > 0; i--) {
					ui.tableHighSpeedWidget->removeRow(i - 1);
				}
			}
			else if (iRowCount < _iAxisNum) {
				for (int i = 0; i < (_iAxisNum - iRowCount); i++) {
					AddtabWidgetRow(ui.tableHighSpeedWidget, iRowCount + i, AxisSpeed());
				}
			}
		}
	}

	{
		int iRowCount = ui.tableLowSpeedWidget->rowCount();
		if (iRowCount != _iAxisNum) {
			if (iRowCount > _iAxisNum) {
				for (int i = iRowCount; i > 0; i--) {
					ui.tableLowSpeedWidget->removeRow(i - 1);
				}
			}
			else if (iRowCount < _iAxisNum) {
				for (int i = 0; i < (_iAxisNum - iRowCount); i++) {
					AddtabWidgetRow(ui.tableLowSpeedWidget, iRowCount + i, AxisSpeed());
				}
			}
		}
	}

}

void QtProductWidgets::AddtabWidgetRow(QTableWidget*tabWidget, int row, AxisSpeed speed)
{
	int iRowCount = tabWidget->rowCount();
	tabWidget->setRowCount(iRowCount + 1);

	QTableWidgetItem* _pItem = new QTableWidgetItem();
	_pItem->setTextAlignment(Qt::AlignCenter);
	_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
	_pItem->setText(QString::number(row));
	_pItem->setToolTip(QString::number(row));
	tabWidget->setItem(iRowCount, 0, _pItem);

	QWidget* _pQWidget = nullptr;
	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
	((QDoubleSpinBox*)_pQWidget)->setValue(speed.dStartSpeed);
	tabWidget->setCellWidget(iRowCount, 1, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setValue(speed.dEndSpeed);
	tabWidget->setCellWidget(iRowCount, 2, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setValue(speed.dAccSpeed);
	tabWidget->setCellWidget(iRowCount, 3, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
	((QDoubleSpinBox*)_pQWidget)->setValue(speed.dDccSpeed);
	tabWidget->setCellWidget(iRowCount, 4, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-9999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(9999999);
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setValue(speed.dSpeed);
	tabWidget->setCellWidget(iRowCount, 5, _pQWidget);
}

int QtProductWidgets::GettabWidgetSpeed(QTableWidget*tabWidget, QMap<int, AxisSpeed>& speed)
{
	speed.clear();
	for (int iRow = 0; iRow < tabWidget->rowCount(); iRow++) {
		AxisSpeed	_Axis;
		for (int iCol = 0; iCol < tabWidget->columnCount(); iCol++) {
			QTableWidgetItem * currentItem = tabWidget->item(iRow, iCol);
			QWidget* _pQWidget = tabWidget->cellWidget(iRow, iCol);
			if (currentItem != nullptr) {}
			else if (_pQWidget != nullptr) {
				switch (iCol) {
				case 1: {
					_Axis.dStartSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
				}	break;
				case 2: {
					_Axis.dEndSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
				}	break;
				case 3: {
					_Axis.dAccSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
				}	break;
				case 4: {
					_Axis.dDccSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
				}	break;
				case 5: {
					_Axis.dSpeed = (double)((QDoubleSpinBox*)_pQWidget)->value();
				}	break;
				default:
					break;
				}
			}
		}
		speed.insert(iRow, _Axis);
	}


	return 0;
}

void QtProductWidgets::slotContextMenuRequested(const QPoint &pos)
{
	QMenu* menu = new QMenu(this);
	QAction *AddPosAction_buf = new QAction(tr("Add Pos"));
	QObject::connect(AddPosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnAddPos);
	menu->addAction(AddPosAction_buf);
	QAction *DeletePosAction_buf = new QAction(tr("Delete Pos"));
	QObject::connect(DeletePosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnDeletePos);
	menu->addAction(DeletePosAction_buf);
	QAction *ClearPosAction_buf = new QAction(tr("Clear Pos"));
	QObject::connect(ClearPosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnClearPos);
	menu->addAction(ClearPosAction_buf);
	menu->addSeparator();

	menu->exec(QCursor::pos());
	if (AddPosAction_buf != nullptr)	delete	AddPosAction_buf;		AddPosAction_buf = nullptr;
	if (DeletePosAction_buf != nullptr)	delete	DeletePosAction_buf;	DeletePosAction_buf = nullptr;
	if (ClearPosAction_buf != nullptr)	delete	ClearPosAction_buf;		ClearPosAction_buf = nullptr;
	if (menu != nullptr)				delete	menu;					menu = nullptr;

}

void QtProductWidgets::slotWasteContextMenuRequested(const QPoint & pos)
{
	QMenu* menu = new QMenu(this);
	QAction *AddPosAction_buf = new QAction(tr("Add Pos"));
	QObject::connect(AddPosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnAddPos);
	menu->addAction(AddPosAction_buf);
	QAction *DeletePosAction_buf = new QAction(tr("Delete Pos"));
	QObject::connect(DeletePosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnDeletePos);
	menu->addAction(DeletePosAction_buf);
	QAction *ClearPosAction_buf = new QAction(tr("Clear Pos"));
	QObject::connect(ClearPosAction_buf, &QAction::triggered, this, &QtProductWidgets::OnClearPos);
	menu->addAction(ClearPosAction_buf);
	menu->addSeparator();

	menu->exec(QCursor::pos());
	if (AddPosAction_buf != nullptr)	delete	AddPosAction_buf;		AddPosAction_buf = nullptr;
	if (DeletePosAction_buf != nullptr)	delete	DeletePosAction_buf;	DeletePosAction_buf = nullptr;
	if (ClearPosAction_buf != nullptr)	delete	ClearPosAction_buf;		ClearPosAction_buf = nullptr;
	if (menu != nullptr)				delete	menu;					menu = nullptr;
}

void QtProductWidgets::OnAddPos()
{
	QTableWidget* _pQTableWidget = nullptr;
	switch (ui.tabWidget->currentIndex()) {
	case 0: {
		_pQTableWidget = ui.tableWidget;
	}	break;
	case 2: {
		_pQTableWidget = ui.tableWasteWidget;
	}	break;
	default:	return;
		break;
	}

	int iRowCount = _pQTableWidget->rowCount();
	_pQTableWidget->setRowCount(iRowCount + 1);

	double _PosX = MotionIO::getInstance().GetAxisPos(0);
	double _PosY = MotionIO::getInstance().GetAxisPos(1);
	double _PosZ = MotionIO::getInstance().GetAxisPos(2);

	QTableWidgetItem* _pItem = new QTableWidgetItem();
	_pItem->setTextAlignment(Qt::AlignCenter);
	_pItem->setFlags(_pItem->flags() & (~Qt::ItemIsEditable));
	_pItem->setText(QString::number(iRowCount));
	_pQTableWidget->setItem(iRowCount,0, _pItem);

	QWidget* _pQWidget = nullptr;
	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
	((QDoubleSpinBox*)_pQWidget)->setValue(_PosX);
	_pQTableWidget->setCellWidget(iRowCount, 1, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setValue(_PosY);
	_pQTableWidget->setCellWidget(iRowCount, 2, _pQWidget);

	_pQWidget = new QDoubleSpinBox();
	((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
	((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
	((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
	((QDoubleSpinBox*)_pQWidget)->setValue(_PosZ);
	_pQTableWidget->setCellWidget(iRowCount, 3, _pQWidget);

	switch (ui.tabWidget->currentIndex()) {
	case 0: {
		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
		((QDoubleSpinBox*)_pQWidget)->setValue(_PosX);
		_pQTableWidget->setCellWidget(iRowCount, 4, _pQWidget);

		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setValue(_PosY);
		_pQTableWidget->setCellWidget(iRowCount, 5, _pQWidget);

		_pQWidget = new QDoubleSpinBox();
		((QDoubleSpinBox*)_pQWidget)->setMinimum(-999999);
		((QDoubleSpinBox*)_pQWidget)->setMaximum(999999);
		((QDoubleSpinBox*)_pQWidget)->setAlignment(Qt::AlignCenter);
		((QDoubleSpinBox*)_pQWidget)->setValue(_PosZ);
		_pQTableWidget->setCellWidget(iRowCount, 6, _pQWidget);

		_pQWidget = new QComboBox();
		((QComboBox*)_pQWidget)->addItem(tr("Disable"));
		((QComboBox*)_pQWidget)->addItem(tr("Enable"));
		((QComboBox*)_pQWidget)->setCurrentIndex(1);
		_pQTableWidget->setCellWidget(iRowCount, 7, _pQWidget);
	}	break;
	default:	return;
		break;
	}

	UpdateIndex();
}

void QtProductWidgets::OnDeletePos()
{
	QTableWidget* _pQTableWidget = nullptr;
	switch (ui.tabWidget->currentIndex()) {
	case 0: {
		_pQTableWidget = ui.tableWidget;
	}	break;
	case 2: {
		_pQTableWidget = ui.tableWasteWidget;
	}	break;
	default:	return;
		break;
	}

	int iRowCount = _pQTableWidget->rowCount();
	int iCurrentIndex = _pQTableWidget->currentIndex().row();
	_pQTableWidget->removeRow(iCurrentIndex);
	UpdateIndex();
}

void QtProductWidgets::OnClearPos()
{
	QTableWidget* _pQTableWidget = nullptr;
	switch (ui.tabWidget->currentIndex()) {
	case 0: {
		_pQTableWidget = ui.tableWidget;
	}	break;
	case 2: {
		_pQTableWidget = ui.tableWasteWidget;
	}	break;
	default:	return;
		break;
	}
	int iRowCount = _pQTableWidget->rowCount();
	for (size_t i = 0; i < iRowCount; i++)		_pQTableWidget->removeRow(0);
	UpdateIndex();
}

void QtProductWidgets::OnMoveModify()
{
	//if (!MotionRobot::getInstance().IsConnected())
	//{
	//	dataVar::Instance().pManager->notify(tr(" Robot Isn't Connected! Please Check Connect"), NotifyType_Error);
	//	return;
	//}
	//double _PosX = MotionRobot::getInstance().GetCurrentPos(0);
	//double _PosY = MotionRobot::getInstance().GetCurrentPos(1);
	//double _PosZ = MotionRobot::getInstance().GetCurrentPos(2);
	//int iCurrentIndex = ui.tableWidget->currentIndex().row();
	//QDoubleSpinBox* _pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 1);
	//if (_pQWidget != nullptr)	_pQWidget->setValue(_PosX);

	//_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 2);
	//if (_pQWidget != nullptr)	_pQWidget->setValue(_PosY);

	//_pQWidget = (QDoubleSpinBox*)ui.tableWidget->cellWidget(iCurrentIndex, 3);
	//if (_pQWidget != nullptr)	_pQWidget->setValue(_PosZ);

}

void QtProductWidgets::OnMouseClicked(QList<QGraphicsItem*>& item)
{
	if (item.size() > 0)
	{
		int iCurrentIndex = ((PointItem*)item[0])->index;

	}
}

void QtProductWidgets::slotChangeTab(int itab)
{
	ResizeUI();
}

void QtProductWidgets::ResizeUI()
{
	{
		int _iWidth = ui.tableWidget->width() / ui.tableWidget->columnCount();
		for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
			ui.tableWidget->setColumnWidth(i, _iWidth);
		}
	}
	{
		int _iWidth = ui.tableWasteWidget->width() / ui.tableWasteWidget->columnCount();
		for (int i = 0; i < ui.tableWasteWidget->columnCount(); i++) {
			ui.tableWasteWidget->setColumnWidth(i, _iWidth);
		}
	}
	{
		int _iWidth = ui.tableLowSpeedWidget->width() / ui.tableLowSpeedWidget->columnCount();
		for (int i = 0; i < ui.tableLowSpeedWidget->columnCount(); i++) {
			ui.tableLowSpeedWidget->setColumnWidth(i, _iWidth);
		}
	}
	{
		int _iWidth = ui.tableHighSpeedWidget->width() / ui.tableHighSpeedWidget->columnCount();
		for (int i = 0; i < ui.tableHighSpeedWidget->columnCount(); i++) {
			ui.tableHighSpeedWidget->setColumnWidth(i, _iWidth);
		}
	}

}

void QtProductWidgets::UpdateIndex()
{
	for (int iRow = 0; iRow < ui.tableWidget->rowCount(); iRow++) {
		if (ui.tableWidget->item(iRow, 0) != nullptr)
			ui.tableWidget->item(iRow, 0)->setText(QString::number(iRow));
	}
	for (int iRow = 0; iRow < ui.tableWasteWidget->rowCount(); iRow++) {
		if (ui.tableWasteWidget->item(iRow, 0) != nullptr)
			ui.tableWasteWidget->item(iRow, 0)->setText(QString::number(iRow));
	}
}

void QtProductWidgets::slotUpDateCameras()
{
	QString	_strCamName = ui.cbx_Camera->currentText();
	ui.cbx_Camera->clear();
	auto _CameraNames = PluginsManager::Instance().getCameraNames();
	ui.cbx_Camera->addItem("None");
	ui.cbx_Camera->addItems(_CameraNames);
	ui.cbx_Camera->setCurrentText(_strCamName);
}

void QtProductWidgets::slotChangeCameraParamIndex(int iIndex)
{
	ResizeUI();

}

void QtProductWidgets::moveRow(QTableWidget* pTable, int nFrom, int nTo)
{
	if (pTable == NULL) return;
	pTable->setFocus();
	if (nFrom == nTo) return;
	if (nFrom < 0 || nTo < 0) return;
	int nRowCount = pTable->rowCount();
	if (nFrom >= nRowCount || nTo > nRowCount) return;
	if (nTo < nFrom) nFrom++; pTable->insertRow(nTo);
	int nCol = pTable->columnCount();
	for (int i = 0; i < nCol; i++){
		QTableWidgetItem * currentItem = pTable->item(nFrom, i);
		QWidget* _pQWidget = pTable->cellWidget(nFrom, i);
		if (currentItem != nullptr)
			pTable->setItem(nTo, i, pTable->takeItem(nFrom, i));
		else if (_pQWidget != nullptr) {
			QString	strClass = _pQWidget->metaObject()->className();
			if (strClass == "QDoubleSpinBox") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
			else if (strClass == "QLabel") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
			else if (strClass == "QComboBox") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
		}
	}
	if (nFrom < nTo){	nTo--;	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}

void QtProductWidgets::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev);
	ResizeUI();
}

void QtProductWidgets::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
	ResizeUI();
	m_pMarkGraphViews->SetCircleSize(ui.Slider_ROI->value());

	QString strCam = ui.cbx_Camera->currentText();
	if (strCam == "None" || strCam == "")	{
		auto _CameraNames = PluginsManager::Instance().getCameraNames();
		if (_CameraNames.size() > 0)	{
			slotUpDateCameras();
			ui.cbx_Camera->setCurrentIndex(1);
		}
		strCam = ui.cbx_Camera->currentText();
	}
	m_pCamera = CameraManager::Instance().FindCamera(strCam);
	if (m_pCamera != nullptr) {
		m_pCamera->SetStream(this);
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_Continnue));
	}
	QString	strValue;
	if (m_pCamera != nullptr)
	{
		m_pCamera->GetCameraParam(CameraMaxExposure, strValue);
		ui.Slider_CodeExposure->setMaximum(strValue.toInt());
		ui.spBx_CodeExposure->setMaximum(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.Slider_CodeExposure->setMinimum(strValue.toInt());
		ui.spBx_CodeExposure->setMinimum(strValue.toInt());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_CodeGain->setMaximum(strValue.toDouble());
		ui.Slider_CodeGain->setMaximum(strValue.toDouble() * 10);

		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_CodeGain->setMinimum(strValue.toDouble());
		ui.Slider_CodeGain->setMinimum(strValue.toDouble() * 10);
	}

}

void QtProductWidgets::hideEvent(QHideEvent * ev)
{
	QtWidgetsBase::hideEvent(ev);
	if (m_pCamera != nullptr) {
		m_pCamera->SetCameraParam(CameraMode, QString::number(CameraMode_SoftTrigger));
	}
}

bool QtProductWidgets::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType) {
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent != nullptr) {
			m_pMarkGraphViews->UpdateImg();
		}
	}
	return QtWidgetsBase::event(ev);
}

int QtProductWidgets::Excute(HalconCpp::HObject& image)
{
	//m_pMarkGraphViews->SetCircleSize(m_param.iROISize);
	//HTuple Width, Height;
	//GetImageSize(image, &Width, &Height);
	//HObject Circle, ImageReduced;
	//GenCircle(&Circle, Height.D() / 2.0, Width.D() / 2.0, m_param.iROISize);
	//ReduceDomain(image, Circle, &ImageReduced);

	//switch (m_param.Color) {
	//case TargetColorWhite: {

	//}break;
	//case TargetColorBlack: {

	//}break;
	//default: {	}break;
	//}

	////识别方式
	//switch (m_param.Target) {
	//case TargetFeatureArea: {

	//}break;
	//case TargetFeatureRadius: {

	//}break;
	//default: {	}break;
	//}
	//if (m_param.bSaveImage){
	//	QString strPath = QApplication::applicationDirPath() + "/SaveImage/" 
	//		+ QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + ".bmp";
	//	WriteImage(image,"bmp",0, strPath.toStdString().c_str());
	//}

	return 0;
}
