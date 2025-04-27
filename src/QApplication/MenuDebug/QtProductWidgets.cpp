#include "QtProductWidgets.h"
#include <QMenu>
#include "Pages/QtWaitWidgetsClass.h"
#include "Motion\MOtionIO.h"
#include "Log/MyEvent.h"
#include "Vision/PluginsManager.h"
#include "Vision/CameraManager.h"
#include "Data/dataVar.h"
#include "Common/MyMessageBox.h"
#include "ImageShow/ImageItem.h"
#include "ImageShow/QGraphicsScenes.h"
#include <QFileDialog>
#include <QElapsedTimer>

void VisionProductTask::run()
{
	if (m_ptrVision != nullptr) {
		if (m_ptrVision->m_bDealVisionMode)	{
			m_ptrVision->Excute(m_strCamName, m_hImage);
		}
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
	if (m_ptrVision != nullptr) {
		m_ptrVision->m_dstImage = m_hImage.Clone();;
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
	connect(m_pMarkGraphViews, SIGNAL(sig_MouseClicked(QList<QGraphicsItem*>&)), this, SLOT(OnMouseClicked(QList<QGraphicsItem*>&)));

	ui.CameraLayout->addWidget(m_pMarkGraphViews);

	connect(ui.Slider_Exposure, SIGNAL(sliderReleased()),					this, SLOT(slotUpDateExposure()));
	connect(ui.Slider_Gain,		SIGNAL(sliderReleased()),					this, SLOT(slotUpDateGain()));
	connect(ui.Slider_Light,	SIGNAL(sliderReleased()),					this, SLOT(slotUpDateLight()));
	connect(ui.cbx_Camera,		SIGNAL(currentIndexChanged(int)),			this, SLOT(slotChangeCamera(int)));
	connect(ui.cbx_Camera,		SIGNAL(sigPopup()),							this, SLOT(slotUpDateCameras()));
	connect(ui.tabCameraWidget,	SIGNAL(currentChanged(int)),				this, SLOT(slotChangeCameraParamIndex(int)));
	connect(this,				&QtProductWidgets::sigSetQTableWidget,		this, &QtProductWidgets::slot_SetQTableWidget, Qt::QueuedConnection);
	m_pQCreateEvent = new QCreateEvent();
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
		_pTask->m_strCamName = strCamName;

		if (m_pMarkGraphViews != nullptr) {
			m_pMarkGraphViews->DispImage(ByteToQImage(gray, iwidth, iheight, iwidth * (ibit == 24 ? 3 : 1), ((ibit == 24) ? QImage::Format_RGB32 : QImage::Format_Indexed8)), false);
		}
		_pTask->RecieveImgBuffer(strCamName,gray, iwidth, iheight, ibit, imgtype, icount);

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
	int	_iRowCount = ui.tableWidget->rowCount();
	for (size_t i = 0; i < _iRowCount; i++)		ui.tableWidget->removeRow(0);
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidget, &QTableWidget::customContextMenuRequested, this, &QtProductWidgets::slotContextMenuRequested);
	ui.tableWidget->horizontalHeader()->setVisible(true);
	connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slot_PoscellClicked(int, int)));

	ui.tabWidget->setCurrentIndex(0);
	ui.tabCameraWidget->setCurrentIndex(0);

	connect(ui.Slider_ROI, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_ROI->setValue(value);
	});
	connect(ui.spBx_ROI, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_ROI->setValue(ui.spBx_ROI->value());
	});
	connect(ui.Slider_Light, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_Light->setValue(value);
		slotUpDateLight();
	});
	connect(ui.spBx_Light, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_Light->setValue(ui.spBx_Light->value());
		slotUpDateLight();
	});
	connect(ui.Slider_Exposure, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_Exposure->setValue(value);
		slotUpDateExposure();
	});
	connect(ui.spBx_Exposure, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_Exposure->setValue(ui.spBx_Exposure->value());
		slotUpDateExposure();
	});
	connect(ui.Slider_Gain, &QSlider::valueChanged, this, [=](int value) {
		ui.spBx_Gain->setValue(value * 0.1);
		slotUpDateGain();
	});
	connect(ui.spBx_Gain, &QDoubleSpinBox::editingFinished, this, [=]() {
		ui.Slider_Gain->setValue(ui.spBx_Gain->value() * 10);
		slotUpDateGain();
	});
	return 0;
}

int QtProductWidgets::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;

	CommonParam.insert("spBx_ROI",						QString::number(ui.spBx_ROI->value()));
	CommonParam.insert("spBx_Light",					QString::number(ui.spBx_Light->value()));
	CommonParam.insert("cBx_Channel",					QString::number(ui.cBx_Channel->currentIndex()));
	CommonParam.insert("Slider_Exposure",				QString::number(ui.Slider_Exposure->value()));
	CommonParam.insert("Slider_Gain",					QString::number(ui.spBx_Gain->value()));
	//位置参数


	{
		QJsonArray PositionParam;
		ChangeTableToData(ui.tableWidget, PositionParam);
		CommonParam.insert("PositionParam", PositionParam);
	}


	strData.insert("QtProductWidgets", CommonParam);
	UpDateUI();
	return 0;
}

int QtProductWidgets::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("QtProductWidgets").value().toObject();

	if (CommonParam.contains("spBx_ROI"))						ui.spBx_ROI->setValue(CommonParam["spBx_ROI"].toString().toInt());
	if (CommonParam.contains("spBx_Light"))						ui.spBx_Light->setValue(CommonParam["spBx_Light"].toString().toInt());
	if (CommonParam.contains("cBx_Channel"))					ui.cBx_Channel->setCurrentIndex(CommonParam["cBx_Channel"].toString().toInt());
	if (CommonParam.contains("Slider_Exposure"))				ui.Slider_Exposure->setValue(CommonParam["Slider_Exposure"].toString().toInt());
	if (CommonParam.contains("Slider_Gain"))					ui.spBx_Gain->setValue(CommonParam["Slider_Gain"].toString().toInt());


	{
		QJsonArray PositionParam = CommonParam.find("PositionParam").value().toArray();
		ChangeDataToTable(PositionParam,ui.tableWidget);
	}

	ResizeUI();

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

}

bool QtProductWidgets::CheckCylinderSafety()
{
	return true;
}

void QtProductWidgets::on_btnTest_clicked()
{
	QString strCam = ui.cbx_Camera->currentText();
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

		switch (hImageObject.CountChannels().I()) {
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
			hImageObject.GetImagePointer3(&PointerRed, &PointerGreen, &PointerBlue, &Type, &Width, &Height);
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

void QtProductWidgets::on_btnTrriger_clicked()
{
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
//
//void QtProductWidgets::on_btnPhotoLearn_clicked()
//{
//	auto res = MyMessageBox::question(this, tr(" Are You Sure Want To Change Position?"), tr("Alart"));
//	if (res == MyMessageBox::Yes) {
//		double _PosX = MotionIO::getInstance().GetAxisPos(0);
//		double _PosY = MotionIO::getInstance().GetAxisPos(1);
//		double _PosZ = MotionIO::getInstance().GetAxisPos(2);
//
//	}
//}
//
//void QtProductWidgets::on_btnPhotoMove_clicked()
//{
//	double _PosX = MotionIO::getInstance().GetAxisPos(0);
//	double _PosY = MotionIO::getInstance().GetAxisPos(1);
//	double _PosZ = MotionIO::getInstance().GetAxisPos(2);
//
//	QString _strMsg = tr(" MoveTo Waste Pannel");
//	StandardMsgResult result = QtWaitWidgetsClass::WaitForMsgExecFunction([=]() {
//
//		return  StandardMsgResult::MsgResult_OK;
//	}, _strMsg + tr(" Now !"), QtWaitWidgetsClass::MsgType_Close);
//	switch (result) {
//	case MsgResult_TimeOut:
//		dataVar::Instance().pManager->notify(tr("Error"), _strMsg + tr(" Time Out!"), NotifyType_Error);
//		break;
//	case MsgResult_Close:
//		dataVar::Instance().pManager->notify(tr("Cancer"), _strMsg + tr(" Cancer!"), NotifyType_Info);
//		break;
//	case MsgResult_NG:
//	case MsgResult_OK:
//	default: { }	break;
//	}
//}

void QtProductWidgets::slotUpDateExposure()
{
	switch (ui.tabCameraWidget->currentIndex())
	{
	default:
	case 1:
	case 2: {
		int _iValue = ui.Slider_Exposure->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraExposure, QString::number(_iValue));
		}
	}break;
	}
}

void QtProductWidgets::slotUpDateGain()
{
	switch (ui.tabCameraWidget->currentIndex())
	{
	default:
	case 1: {
		int _iValue = ui.spBx_Gain->value();
		if (m_pCamera != nullptr) {
			m_pCamera->SetCameraParam(CameraGain, QString::number(_iValue));
		}
	}break;
	}
}

void QtProductWidgets::slotUpDateLight()
{
	switch (ui.tabCameraWidget->currentIndex()){
	default:
	case 1: {
		int _iChannel = ui.cBx_Channel->currentIndex();
		int _iValue = ui.Slider_Light->value();
		//MotionSerialPort::getInstance().WriteLightValue(_iChannel, _iValue);
	}break;
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
		ui.Slider_Exposure->setMaximum(strValue.toDouble());
		ui.spBx_Exposure->setMaximum(strValue.toDouble());

		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.Slider_Exposure->setMinimum(strValue.toDouble());
		ui.spBx_Exposure->setMinimum(strValue.toDouble());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_Gain->setMaximum(strValue.toDouble());
		ui.Slider_Gain->setMaximum(strValue.toDouble() * 10);

		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_Gain->setMinimum(strValue.toDouble());
		ui.Slider_Gain->setMinimum(strValue.toDouble() * 10);
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
}

void QtProductWidgets::slotChangeAxisSpeedParam()
{


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
	
}

void QtProductWidgets::OnDeletePos()
{

}

void QtProductWidgets::OnClearPos()
{

}

void QtProductWidgets::OnMoveModify()
{

}

void QtProductWidgets::OnMouseClicked(QList<QGraphicsItem*>& items)
{

}

void QtProductWidgets::slotChangeTab(int itab)
{
	ResizeUI();
}

void QtProductWidgets::on_btnPortIdentify_clicked()
{

}

void QtProductWidgets::on_btnWorkZifuTestPos_clicked()
{

}

void QtProductWidgets::ResizeUI()
{


}

void QtProductWidgets::UpdateIndex()
{

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

void QtProductWidgets::slot_PoscellClicked(int row, int column)
{
	int _iCurrentIndex = ui.tableWidget->currentIndex().row();
	if (_iCurrentIndex >= 0)
	{
		QString	strText = ui.tableWidget->item(_iCurrentIndex, 9)->text();
		// 开始解析 解析成功返回QJsonDocument对象否则返回null
		QJsonParseError err_rpt;
		QJsonDocument root_document = QJsonDocument::fromJson(strText.toUtf8(), &err_rpt);
		if (err_rpt.error != QJsonParseError::NoError && !root_document.isNull()) {
			dataVar::Instance().pManager->notify(tr("Error"), tr("Not Exist"), NotifyType_Error);
			return;
		}
		// 获取根节点
		QJsonObject root = root_document.object();

		if (root.contains("spBx_Exposure"))		ui.spBx_Exposure->setValue(root["spBx_Exposure"].toString().toDouble());
		if (root.contains("spBx_Gain"))			ui.spBx_Gain->setValue(root["spBx_Gain"].toString().toDouble());
		if (root.contains("spBx_ROI"))			ui.spBx_ROI->setValue(root["spBx_ROI"].toString().toDouble());
		if (m_pCamera != nullptr) {	m_pCamera->SetCameraParam(CameraExposure, QString::number(ui.spBx_Exposure->value()));	}
		if (m_pCamera != nullptr) { m_pCamera->SetCameraParam(CameraGain, QString::number(ui.spBx_Gain->value()));	}

	}
}

void QtProductWidgets::slotChangeCameraParamIndex(int iIndex)
{
	ResizeUI();

}

void QtProductWidgets::on_btnWorkApplyPos_clicked()
{
	int _iCurrentIndex = ui.tableWidget->currentIndex().row();
	if (_iCurrentIndex >= 0)	{
		QJsonObject	Json;
		QJsonDocument	root_document;
		Json.insert("spBx_Exposure",	QString::number(ui.spBx_Exposure->value()));
		Json.insert("spBx_Gain",		QString::number(ui.spBx_Gain->value()));
		Json.insert("spBx_ROI",			QString::number(ui.spBx_ROI->value()));

		root_document.setObject(Json);
		QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
		QString	strText = QString(root_string_compact);

		ui.tableWidget->item(_iCurrentIndex, 9)->setText(strText);
		ui.tableWidget->selectRow(_iCurrentIndex);
	}
}

void QtProductWidgets::slot_SetQTableWidget(int iRow, int iCol, QWidget * _pQWidget, QTableWidget * pTable)
{
	if (_pQWidget != nullptr) {
		pTable->setCellWidget(iRow, iCol, _pQWidget);
	}
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
			else if (strClass == "QSpinBox") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
			else if (strClass == "QLabel") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
			else if (strClass == "QComboBox") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
			else if (strClass == "QCheckBox") {
				pTable->setCellWidget(nTo, i, _pQWidget);
			}
		}
	}
	if (nFrom < nTo){	nTo--;	}
	pTable->removeRow(nFrom); pTable->selectRow(nTo);
}

int QtProductWidgets::ChangeTableToData(QTableWidget * pTable, QJsonArray & strData)
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

int QtProductWidgets::ChangeDataToTable(QJsonArray & strData, QTableWidget * pTable)
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

void QtProductWidgets::resizeEvent(QResizeEvent * ev)
{
	QtWidgetsBase::resizeEvent(ev);
	ResizeUI();
}

void QtProductWidgets::showEvent(QShowEvent * ev)
{
	QtWidgetsBase::showEvent(ev);
	ResizeUI();

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
		ui.Slider_Exposure->setMaximum(strValue.toDouble());
		ui.spBx_Exposure->setMaximum(strValue.toDouble());

		m_pCamera->GetCameraParam(CameraMinExposure, strValue);
		ui.Slider_Exposure->setMinimum(strValue.toDouble());
		ui.spBx_Exposure->setMinimum(strValue.toDouble());

		m_pCamera->GetCameraParam(CameraMaxGain, strValue);
		ui.spBx_Gain->setMaximum(strValue.toDouble());
		ui.Slider_Gain->setMaximum(strValue.toDouble() * 10);

		m_pCamera->GetCameraParam(CameraMinGain, strValue);
		ui.spBx_Gain->setMinimum(strValue.toDouble());
		ui.Slider_Gain->setMinimum(strValue.toDouble() * 10);
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

int QtProductWidgets::Excute(QString strCamName, HalconCpp::HObject& image)
{
	return 0;
}
