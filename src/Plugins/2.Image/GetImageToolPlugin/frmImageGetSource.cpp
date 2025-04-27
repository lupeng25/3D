#include "frmImageGetSource.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QThread>
#include <QElapsedTimer>
#include <QPainter>
#include <QtConcurrent>
#include <QWidgetAction>
#include <QToolBar>
#include <QToolButton>

#include <QFileDialog>

#include "databaseVar.h"
#include "PluginsManager.h"
#include "CameraManager.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"

frmImageGetSource::frmImageGetSource(QWidget* parent)
	: frmBaseTool(parent) {

	ui.setupUi(GetCenterWidget());
	m_iChanal = 0;
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop = QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	this->setWindowIcon(QIcon(":/skin/icon/icon/icons/light/GetImage.png"));
	//初始化标题栏
	initTitleBar();
	//图像显示
	m_view = new QGraphicsViews;
	connect(m_view, &QGraphicsViews::sig_MouseMoveInImg, this, &frmImageGetSource::slot_MouseMoveInImg);
	ui.imageShowLayout->addWidget(m_view);
	imgIndex = 0;
	initCameraProperty();
	initConnect();
	initialVtkWidget();	//初始化控件
	initData();
	ResetView(false);
	setAttribute(Qt::WA_TranslucentBackground, false);
}

frmImageGetSource::~frmImageGetSource()
{
	m_bWndDelete							= true;
	m_bNeedChangeCloud						= true;
	if (m_view != nullptr)					delete m_view;				m_view = nullptr;
	if (pViewWidget != nullptr)				delete pViewWidget;			pViewWidget = nullptr;
	if (m_titleBar != nullptr)				delete m_titleBar;			m_titleBar = nullptr;

	if (pBtnGroupRadio != nullptr)			delete pBtnGroupRadio;		pBtnGroupRadio = nullptr;
	if (propertyEditor != nullptr)			delete propertyEditor;		propertyEditor = nullptr;
	if (variantManager != nullptr)			delete variantManager;		variantManager = nullptr;
	if (variantFactory != nullptr)			delete variantFactory;		variantFactory = nullptr;
	this->deleteLater();
}

void frmImageGetSource::initTitleBar()
{
	m_titleBar->setTitleIcon(":/skin/icon/icon/icons/light/GetImage.png");
	//m_titleBar->setButtonType(MIN_MAX_BUTTON);
}

void frmImageGetSource::initCameraProperty()
{
	variantManager = new QtVariantPropertyManager(this);			//相机属性
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(valueChanged(QtProperty *, const QVariant &)));
	variantFactory = new QtVariantEditorFactory(this);
	propertyEditor = new QtTreePropertyBrowser(ui.tab_Camera);
	propertyEditor->setFactoryForManager(variantManager, variantFactory);
	ui.gridLayout_camera->addWidget(propertyEditor);
}

void frmImageGetSource::initConnect()
{
	//连接信号与槽
	connect(ui.cbx_Camerachannal,	SIGNAL(activated(int)),					this, SLOT(onChannelClicked(int)));
	connect(ui.btn3DShow,			&QCheckBox::clicked,					this, &frmImageGetSource::onButton3DShowClicked);
	m_view->RegisteredMenu("3DShow",	tr("Show 3D"),":/QtApplication/Bitmaps/3DView.png");
	connect(m_view,					&QGraphicsViews::sig_MouseMenuClicked,	this, &frmImageGetSource::slot_MouseMenuClicked);
	connect(ui.btnSelectImage,		&QPushButton::clicked,					this, &frmImageGetSource::on_btnSelectImage_clicked);
	connect(ui.btnSelectPath,		&QPushButton::clicked,					this, &frmImageGetSource::on_btnSelectPath_clicked);
	connect(ui.btnConnect,			&QPushButton::clicked,					this, &frmImageGetSource::on_btnConnect_clicked);
	connect(ui.btnUpdata,			&QPushButton::clicked,					this, &frmImageGetSource::on_btnUpdata_clicked);
}

void frmImageGetSource::initData()
{
	ui.btn3DShow->setCheckable(true);
	ui.btn3DShow->setChecked(false);
	ui.stackedWidget->setCurrentIndex(0);
	ui.tabWidget->setCurrentIndex(0);
	onButton3DShowClicked();

	//选项按钮
	pBtnGroupRadio = new QButtonGroup(this);
	pBtnGroupRadio->addButton(ui.radioImage, 0);
	pBtnGroupRadio->addButton(ui.radioFolder, 1);
	pBtnGroupRadio->addButton(ui.radioCamera, 2);
	connect(pBtnGroupRadio, SIGNAL(buttonClicked(int)), this, SLOT(slot_btnGroupClicked(int)));
	ui.radioImage->click();

	connect(ui.cbx_CameraView, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_camShowChanged(int)));
}

void frmImageGetSource::ClearCameraProperty()
{
	for (auto iter : idToProperty)	if (iter != nullptr)	delete iter;	idToProperty.clear();
	propertyToId.clear();
}

void frmImageGetSource::ResetView(bool bVisiable)
{
	if (bVisiable)	{
		ui.btn3DShow->setChecked(false);
		ui.btn3DShow->setVisible(true);
	}
	else	{
		ui.btn3DShow->setChecked(false);
		ui.btn3DShow->setVisible(false);
	}
	ui.widget_Img->setVisible(true);
}

void frmImageGetSource::refreshAll(bool only2D)
{
	std::lock_guard<std::mutex> _lock(m_Lock);

}

void frmImageGetSource::UpdateCloudPoint()
{
	try	{
		if (!m_dstImage.IsInitialized())	return;
		HTuple channel, Type;
		HalconCpp::CountChannels(m_dstImage, &channel);
		HalconCpp::GetImageType(m_dstImage, &Type);
		if (Type.S() == HString(ImgType_Byte.toStdString().c_str()))	return;
		//获取点云数据然后设置
		if (Type.S() == HString("real"))
		{
			switch (channel.I())
			{
			default:
			case 1:
			{
				HImage	himg(m_dstImage);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				BYTE*	pSrc = static_cast<BYTE*>(ptr);
				const int floatByte = sizeof(float);
				const int floatWidthByte = sizeof(float) * width;
				//QString	__strData;
				for (int row = 0; row < height; row++)
				{
					BYTE* _ptrLine = pSrc + row * floatWidthByte;
					for (int col = 0; col < width; col++)
					{

					}
				}
			}	break;
			case 2:
			{
				HObject	hHeightimgChannal;
				HObject	hGrayimgChannal;
				HTuple hHeightType;
				HTuple hGrayType;
				{
					HObject	hHeightimg;
					HObject	hGrayimg;
					AccessChannel(m_dstImage, &hHeightimg, m_iChanal + 1);
					AccessChannel(m_dstImage, &hGrayimg, m_iChanal + 2);
					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hGrayimg, &hGrayType);

					hHeightimgChannal = hHeightimg;
					hGrayimgChannal = hGrayimg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hGrayimg(hGrayimgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrGray = hGrayimg.GetImagePointer1(&type, &width, &height);
				std::string	strGray = std::string(hGrayType.S());
				std::string	strHeight = std::string(hHeightType.S());
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcGray = static_cast<BYTE*>(ptrGray);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrGrayLine = pSrcGray;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrGrayByte = pSrcGray;
				int ByteHeightLenth = sizeof(float);
				int LineHeightLenth = sizeof(float) * width;
				int ByteGrayLenth = sizeof(float);
				int LineGrayLenth = sizeof(float) * width;
				if (hGrayType.S() == HString("real")){
					ByteGrayLenth = sizeof(float);
					LineGrayLenth = sizeof(float) * width;
				}
				else if (hGrayType.S() == HString("byte")){
					ByteGrayLenth = sizeof(BYTE);
					LineGrayLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hHeightType.S() == HString("real")){
					ByteHeightLenth = sizeof(float);
					LineHeightLenth = sizeof(float) * width;
				}
				else if (hHeightType.S() == HString("byte")){
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else return;
				for (int row = 0; row < height; row++){
					_ptrHeightLine = pSrcHeight + row * LineHeightLenth;
					_ptrGrayLine = pSrcGray + row * LineGrayLenth;
					for (int col = 0; col < width; col++){
						_ptrHeightByte = _ptrHeightLine + col * ByteHeightLenth;
						_ptrGrayByte = _ptrGrayLine + col * ByteGrayLenth;

					}
				}
			}	break;
			//4通道
			case 4:
			{
				HObject	hHeightimgChannal;
				HObject	hRImgChannal;
				HObject	hGImgChannal;
				HObject	hBImgChannal;
				HTuple	hHeightType;
				HTuple	hRType, hGType, hBType;
				{
					HObject	hHeightimg;
					HObject	hRImg, hGImg, hBImg;
					AccessChannel(m_dstImage, &hHeightimg, m_iChanal + 1);
					AccessChannel(m_dstImage, &hRImg, m_iChanal + 2);
					AccessChannel(m_dstImage, &hGImg, m_iChanal + 3);
					AccessChannel(m_dstImage, &hBImg, m_iChanal + 4);

					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hRImg, &hRType);
					HalconCpp::GetImageType(hGImg, &hGType);
					HalconCpp::GetImageType(hBImg, &hBType);

					hHeightimgChannal = hHeightimg;
					hRImgChannal = hRImg;
					hGImgChannal = hGImg;
					hBImgChannal = hBImg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hRImg(hRImgChannal);
				HImage	hGImg(hGImgChannal);
				HImage	hBImg(hBImgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrRImg = hRImg.GetImagePointer1(&type, &width, &height);
				void*	ptrGImg = hGImg.GetImagePointer1(&type, &width, &height);
				void*	ptrBImg = hBImg.GetImagePointer1(&type, &width, &height);
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcRImg = static_cast<BYTE*>(ptrRImg);
				BYTE* pSrcGImg = static_cast<BYTE*>(ptrGImg);
				BYTE* pSrcBImg = static_cast<BYTE*>(ptrBImg);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrRImgLine = pSrcRImg;
				BYTE* _ptrGImgLine = pSrcGImg;
				BYTE* _ptrBImgLine = pSrcBImg;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrRImgByte = pSrcRImg;
				BYTE* _ptrGImgByte = pSrcGImg;
				BYTE* _ptrBImgByte = pSrcBImg;
				int ByteHeightLenth = sizeof(float);
				int LineHeightLenth = sizeof(float) * width;
				int ByteRImgLenth = sizeof(float);
				int LineRImgLenth = sizeof(float) * width;
				int ByteGImgLenth = sizeof(float);
				int LineGImgLenth = sizeof(float) * width;
				int ByteBImgLenth = sizeof(float);
				int LineBImgLenth = sizeof(float) * width;
				if (hRType.S() == HString("real")){
					ByteRImgLenth = sizeof(float);
					LineRImgLenth = sizeof(float) * width;
				}
				else if (hRType.S() == HString("byte"))		{
					ByteRImgLenth = sizeof(BYTE);
					LineRImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hGType.S() == HString("real"))			{
					ByteGImgLenth = sizeof(float);
					LineGImgLenth = sizeof(float) * width;
				}
				else if (hGType.S() == HString("byte"))		{
					ByteGImgLenth = sizeof(BYTE);
					LineGImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hBType.S() == HString("real"))			{
					ByteBImgLenth = sizeof(float);
					LineBImgLenth = sizeof(float) * width;
				}
				else if (hBType.S() == HString("byte"))		{
					ByteBImgLenth = sizeof(BYTE);
					LineBImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hHeightType.S() == HString("real"))		{
					ByteHeightLenth = sizeof(float);
					LineHeightLenth = sizeof(float) * width;
				}
				else if (hHeightType.S() == HString("byte")){
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else return;
				for (int row = 0; row < height; row++)		{
					_ptrHeightLine = pSrcHeight + row * LineHeightLenth;
					_ptrRImgLine = pSrcRImg + row * LineRImgLenth;
					_ptrGImgLine = pSrcGImg + row * LineGImgLenth;
					_ptrBImgLine = pSrcBImg + row * LineBImgLenth;
					for (int col = 0; col < width; col++)
					{

					}
				}

			}	break;
			}
		}
		else if (Type.S() == HString("uint2"))
		{
			switch (channel.I())
			{
			default:
			case 1:
			{
				HImage	himg(m_dstImage);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				BYTE*	pSrc = static_cast<BYTE*>(ptr);
				const int floatByte			= sizeof(unsigned short);
				const int floatWidthByte	= sizeof(unsigned short) * width;
				//QString	__strData;
				for (int row = 0; row < height; row++)
				{
					BYTE* _ptrLine = pSrc + row * floatWidthByte;
					for (int col = 0; col < width; col++)
					{

					}
				}
			}	break;
			case 2:
			{
				HObject	hHeightimgChannal;
				HObject	hGrayimgChannal;
				HTuple hHeightType;
				HTuple hGrayType;
				{
					HObject	hHeightimg;
					HObject	hGrayimg;
					AccessChannel(m_dstImage, &hHeightimg, m_iChanal + 1);
					AccessChannel(m_dstImage, &hGrayimg, m_iChanal + 2);
					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hGrayimg, &hGrayType);

					hHeightimgChannal = hHeightimg;
					hGrayimgChannal = hGrayimg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hGrayimg(hGrayimgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrGray = hGrayimg.GetImagePointer1(&type, &width, &height);
				std::string	strGray = std::string(hGrayType.S());
				std::string	strHeight = std::string(hHeightType.S());
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcGray = static_cast<BYTE*>(ptrGray);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrGrayLine = pSrcGray;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrGrayByte = pSrcGray;
				int ByteHeightLenth = sizeof(unsigned int);
				int LineHeightLenth = sizeof(unsigned int) * width;
				int ByteGrayLenth = sizeof(unsigned int);
				int LineGrayLenth = sizeof(unsigned int) * width;
				if (hGrayType.S() == HString("uint2"))
				{
					ByteGrayLenth = sizeof(unsigned short);
					LineGrayLenth = sizeof(unsigned short) * width;
				}
				else if (hGrayType.S() == HString("byte"))
				{
					ByteGrayLenth = sizeof(BYTE);
					LineGrayLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hHeightType.S() == HString("uint2"))
				{
					ByteHeightLenth = sizeof(unsigned short);
					LineHeightLenth = sizeof(unsigned short) * width;
				}
				else if (hHeightType.S() == HString("byte"))
				{
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else return;
				for (int row = 0; row < height; row++)
				{
					_ptrHeightLine = pSrcHeight + row * LineHeightLenth;
					_ptrGrayLine = pSrcGray + row * LineGrayLenth;
					for (int col = 0; col < width; col++)
					{
						_ptrHeightByte = _ptrHeightLine + col * ByteHeightLenth;
						_ptrGrayByte = _ptrGrayLine + col * ByteGrayLenth;

					}
				}
			}	break;
			//4通道
			case 4:
			{
				HObject	hHeightimgChannal;
				HObject	hRImgChannal;
				HObject	hGImgChannal;
				HObject	hBImgChannal;
				HTuple	hHeightType;
				HTuple	hRType, hGType, hBType;
				{
					HObject	hHeightimg;
					HObject	hRImg, hGImg, hBImg;
					AccessChannel(m_dstImage, &hHeightimg, m_iChanal + 1);
					AccessChannel(m_dstImage, &hRImg, m_iChanal + 2);
					AccessChannel(m_dstImage, &hGImg, m_iChanal + 3);
					AccessChannel(m_dstImage, &hBImg, m_iChanal + 4);

					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hRImg, &hRType);
					HalconCpp::GetImageType(hGImg, &hGType);
					HalconCpp::GetImageType(hBImg, &hBType);

					hHeightimgChannal = hHeightimg;
					hRImgChannal = hRImg;
					hGImgChannal = hGImg;
					hBImgChannal = hBImg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hRImg(hRImgChannal);
				HImage	hGImg(hGImgChannal);
				HImage	hBImg(hBImgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrRImg = hRImg.GetImagePointer1(&type, &width, &height);
				void*	ptrGImg = hGImg.GetImagePointer1(&type, &width, &height);
				void*	ptrBImg = hBImg.GetImagePointer1(&type, &width, &height);
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcRImg = static_cast<BYTE*>(ptrRImg);
				BYTE* pSrcGImg = static_cast<BYTE*>(ptrGImg);
				BYTE* pSrcBImg = static_cast<BYTE*>(ptrBImg);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrRImgLine = pSrcRImg;
				BYTE* _ptrGImgLine = pSrcGImg;
				BYTE* _ptrBImgLine = pSrcBImg;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrRImgByte = pSrcRImg;
				BYTE* _ptrGImgByte = pSrcGImg;
				BYTE* _ptrBImgByte = pSrcBImg;
				int ByteHeightLenth = sizeof(unsigned int);
				int LineHeightLenth = sizeof(unsigned int) * width;
				int ByteRImgLenth = sizeof(unsigned int);
				int LineRImgLenth = sizeof(unsigned int) * width;
				int ByteGImgLenth = sizeof(unsigned int);
				int LineGImgLenth = sizeof(unsigned int) * width;
				int ByteBImgLenth = sizeof(unsigned int);
				int LineBImgLenth = sizeof(unsigned int) * width;
				if (hRType.S() == HString("uint2"))
				{
					ByteRImgLenth = sizeof(unsigned short);
					LineRImgLenth = sizeof(unsigned short) * width;
				}
				else if (hRType.S() == HString("byte"))
				{
					ByteRImgLenth = sizeof(BYTE);
					LineRImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hGType.S() == HString("uint2"))
				{
					ByteGImgLenth = sizeof(unsigned short);
					LineGImgLenth = sizeof(unsigned short) * width;
				}
				else if (hGType.S() == HString("byte"))
				{
					ByteGImgLenth = sizeof(BYTE);
					LineGImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hBType.S() == HString("uint2"))
				{
					ByteBImgLenth = sizeof(unsigned short);
					LineBImgLenth = sizeof(unsigned short) * width;
				}
				else if (hBType.S() == HString("byte"))
				{
					ByteBImgLenth = sizeof(BYTE);
					LineBImgLenth = sizeof(BYTE) * width;
				}
				else return;
				if (hHeightType.S() == HString("uint2"))
				{
					ByteHeightLenth = sizeof(unsigned short);
					LineHeightLenth = sizeof(unsigned short) * width;
				}
				else if (hHeightType.S() == HString("byte"))
				{
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else return;
				for (int row = 0; row < height; row++)
				{
					_ptrHeightLine = pSrcHeight + row * LineHeightLenth;
					_ptrRImgLine = pSrcRImg + row * LineRImgLenth;
					_ptrGImgLine = pSrcGImg + row * LineGImgLenth;
					_ptrBImgLine = pSrcBImg + row * LineBImgLenth;
					for (int col = 0; col < width; col++)
					{
						_ptrHeightByte = _ptrHeightLine + col * ByteHeightLenth;
						_ptrRImgByte = _ptrRImgLine + col * ByteRImgLenth;
						_ptrGImgByte = _ptrGImgLine + col * ByteGImgLenth;
						_ptrBImgByte = _ptrBImgLine + col * ByteBImgLenth;

					}
				}

			}	break;
			}
		}
		else if (Type.S() == HString("int4"))
		{
			switch (channel.I())
			{
			default:
			case 1:
			{
				HImage	himg(m_dstImage);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptr = himg.GetImagePointer1(&type, &width, &height);
				BYTE*	pSrc = static_cast<BYTE*>(ptr);
				const int floatByte = sizeof(int);
				const int floatWidthByte = sizeof(int) * width;
				//QString	__strData;
				for (int row = 0; row < height; row++)
				{
					BYTE* _ptrLine = pSrc + row * floatWidthByte;
					for (int col = 0; col < width; col++)
					{

					}
				}
			}	break;
			case 2:
			{
				HObject	hHeightimgChannal;
				HObject	hGrayimgChannal;
				HTuple hHeightType;
				HTuple hGrayType;
				{
					HObject	hHeightimg;
					HObject	hGrayimg;
					AccessChannel(m_dstImage, &hHeightimg, 0 + 1);
					AccessChannel(m_dstImage, &hGrayimg, 0 + 2);
					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hGrayimg, &hGrayType);

					hHeightimgChannal = hHeightimg;
					hGrayimgChannal = hGrayimg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hGrayimg(hGrayimgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrGray = hGrayimg.GetImagePointer1(&type, &width, &height);
				std::string	strGray = std::string(hGrayType.S());
				std::string	strHeight = std::string(hHeightType.S());
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcGray = static_cast<BYTE*>(ptrGray);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrGrayLine = pSrcGray;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrGrayByte = pSrcGray;
				int ByteHeightLenth = sizeof(unsigned int);
				int LineHeightLenth = sizeof(unsigned int) * width;
				int ByteGrayLenth = sizeof(unsigned int);
				int LineGrayLenth = sizeof(unsigned int) * width;
				if (hGrayType.S() == HString("uint2"))
				{
					ByteGrayLenth = sizeof(unsigned int);
					LineGrayLenth = sizeof(unsigned int) * width;
				}
				else if (hGrayType.S() == HString("byte"))
				{
					ByteGrayLenth = sizeof(BYTE);
					LineGrayLenth = sizeof(BYTE) * width;
				}
				else if (hGrayType.S() == HString("int4"))
				{
					ByteGrayLenth = sizeof(int);
					LineGrayLenth = sizeof(int) * width;
				}
				else return;
				if (hHeightType.S() == HString("uint2"))
				{
					ByteHeightLenth = sizeof(unsigned int);
					LineHeightLenth = sizeof(unsigned int) * width;
				}
				else if (hHeightType.S() == HString("byte"))
				{
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else if (hHeightType.S() == HString("int4"))
				{
					ByteHeightLenth = sizeof(int);
					LineHeightLenth = sizeof(int) * width;
				}
				else return;
				for (int row = 0; row < height; row++)
				{
					_ptrHeightLine	= pSrcHeight + row * LineHeightLenth;
					_ptrGrayLine	= pSrcGray + row * LineGrayLenth;
					for (int col = 0; col < width; col++)
					{
						_ptrHeightByte	= _ptrHeightLine + col * ByteHeightLenth;
						_ptrGrayByte	= _ptrGrayLine	 + col * ByteGrayLenth;

					}
				}
			}	break;
			//4通道
			case 4:
			{
				HObject	hHeightimgChannal;
				HObject	hRImgChannal;
				HObject	hGImgChannal;
				HObject	hBImgChannal;
				HTuple	hHeightType;
				HTuple	hRType, hGType, hBType;
				{
					HObject	hHeightimg;
					HObject	hRImg, hGImg, hBImg;
					AccessChannel(m_dstImage, &hHeightimg, m_iChanal + 1);
					AccessChannel(m_dstImage, &hRImg, m_iChanal + 2);
					AccessChannel(m_dstImage, &hGImg, m_iChanal + 3);
					AccessChannel(m_dstImage, &hBImg, m_iChanal + 4);

					HalconCpp::GetImageType(hHeightimg, &hHeightType);
					HalconCpp::GetImageType(hRImg, &hRType);
					HalconCpp::GetImageType(hGImg, &hGType);
					HalconCpp::GetImageType(hBImg, &hBType);

					hHeightimgChannal = hHeightimg;
					hRImgChannal = hRImg;
					hGImgChannal = hGImg;
					hBImgChannal = hBImg;
				}
				HImage	hHeightimg(hHeightimgChannal);
				HImage	hRImg(hRImgChannal);
				HImage	hGImg(hGImgChannal);
				HImage	hBImg(hBImgChannal);
				Hlong	width;
				Hlong	height;
				HString type;
				void*	ptrHeight = hHeightimg.GetImagePointer1(&type, &width, &height);
				void*	ptrRImg = hRImg.GetImagePointer1(&type, &width, &height);
				void*	ptrGImg = hGImg.GetImagePointer1(&type, &width, &height);
				void*	ptrBImg = hBImg.GetImagePointer1(&type, &width, &height);
				BYTE* pSrcHeight = static_cast<BYTE*>(ptrHeight);
				BYTE* pSrcRImg = static_cast<BYTE*>(ptrRImg);
				BYTE* pSrcGImg = static_cast<BYTE*>(ptrGImg);
				BYTE* pSrcBImg = static_cast<BYTE*>(ptrBImg);
				BYTE* _ptrHeightLine = pSrcHeight;
				BYTE* _ptrRImgLine = pSrcRImg;
				BYTE* _ptrGImgLine = pSrcGImg;
				BYTE* _ptrBImgLine = pSrcBImg;
				BYTE* _ptrHeightByte = pSrcHeight;
				BYTE* _ptrRImgByte = pSrcRImg;
				BYTE* _ptrGImgByte = pSrcGImg;
				BYTE* _ptrBImgByte = pSrcBImg;
				int ByteHeightLenth = sizeof(unsigned int);
				int LineHeightLenth = sizeof(unsigned int) * width;
				int ByteRImgLenth = sizeof(unsigned int);
				int LineRImgLenth = sizeof(unsigned int) * width;
				int ByteGImgLenth = sizeof(unsigned int);
				int LineGImgLenth = sizeof(unsigned int) * width;
				int ByteBImgLenth = sizeof(unsigned int);
				int LineBImgLenth = sizeof(unsigned int) * width;
				if (hRType.S() == HString("uint2"))
				{
					ByteRImgLenth = sizeof(unsigned int);
					LineRImgLenth = sizeof(unsigned int) * width;
				}
				else if (hRType.S() == HString("byte"))
				{
					ByteRImgLenth = sizeof(BYTE);
					LineRImgLenth = sizeof(BYTE) * width;
				}
				else if (hRType.S() == HString("int4"))
				{
					ByteRImgLenth = sizeof(int);
					LineRImgLenth = sizeof(int) * width;
				}
				else return;
				if (hGType.S() == HString("uint2"))
				{
					ByteGImgLenth = sizeof(unsigned int);
					LineGImgLenth = sizeof(unsigned int) * width;
				}
				else if (hGType.S() == HString("byte"))
				{
					ByteGImgLenth = sizeof(BYTE);
					LineGImgLenth = sizeof(BYTE) * width;
				}
				else if (hGType.S() == HString("int4"))
				{
					ByteGImgLenth = sizeof(int);
					LineGImgLenth = sizeof(int) * width;
				}
				else return;
				if (hBType.S() == HString("uint2"))
				{
					ByteBImgLenth = sizeof(unsigned int);
					LineBImgLenth = sizeof(unsigned int) * width;
				}
				else if (hBType.S() == HString("byte"))
				{
					ByteBImgLenth = sizeof(BYTE);
					LineBImgLenth = sizeof(BYTE) * width;
				}
				else if (hBType.S() == HString("int4"))
				{
					ByteBImgLenth = sizeof(int);
					LineBImgLenth = sizeof(int) * width;
				}
				else return;
				if (hHeightType.S() == HString("uint2"))
				{
					ByteHeightLenth = sizeof(unsigned int);
					LineHeightLenth = sizeof(unsigned int) * width;
				}
				else if (hHeightType.S() == HString("byte"))
				{
					ByteHeightLenth = sizeof(BYTE);
					LineHeightLenth = sizeof(BYTE) * width;
				}
				else if (hHeightType.S() == HString("int4"))
				{
					ByteHeightLenth = sizeof(int);
					LineHeightLenth = sizeof(int) * width;
				}
				else return;
				for (int row = 0; row < height; row++)
				{
					_ptrHeightLine = pSrcHeight + row * LineHeightLenth;
					_ptrRImgLine = pSrcRImg + row * LineRImgLenth;
					_ptrGImgLine = pSrcGImg + row * LineGImgLenth;
					_ptrBImgLine = pSrcBImg + row * LineBImgLenth;
					for (int col = 0; col < width; col++)
					{
						_ptrHeightByte = _ptrHeightLine + col * ByteHeightLenth;
						_ptrRImgByte = _ptrRImgLine + col * ByteRImgLenth;
						_ptrGImgByte = _ptrGImgLine + col * ByteGImgLenth;
						_ptrBImgByte = _ptrBImgLine + col * ByteBImgLenth;

					}
				}

			}	break;
			}
		}
		refreshAll();
	}
	catch (...)	{	}
}

void frmImageGetSource::slot_Save()
{
	UpDateCameraView();

}

void frmImageGetSource::UpdateLimitCloudPoint(QWidget*parent)
{

}

//进行3D显示
void frmImageGetSource::onButton3DShowClicked()
{
	if (!ui.btn3DShow->isChecked())	{
		ui.stackedWidget->setCurrentIndex(0);
		ui.widget_Img->setVisible(true);
		ui.label_Item->setVisible(false);
		m_view->setVisible(true);
		ui.cbx_CameraView->setVisible(false);
		//ui.btn3DShow->setChecked(false);
	}
	else	{
		ui.widget_Img->setVisible(false);
		ui.stackedWidget->setCurrentIndex(1);
		ui.label_Item->setVisible(true);
		m_view->setVisible(false);
		ui.cbx_CameraView->setVisible(true);
		//ui.btn3DShow->setChecked(true);
		UpDateCameraView();
	}
}

void frmImageGetSource::onChannelClicked(int Index)
{
	if (Index < 0)	return;

	try {
		m_iChanal = Index;
		//QImage img(Hal2QImage(m_srcImage));
		if (m_view != nullptr)	m_view->ClearImgShow();
		if (m_iChanal == 0) {
			m_dstImage.Clear();
			m_dstImage = m_srcImage.Clone();
		}
		else AccessChannel(m_srcImage,	&m_dstImage, m_iChanal);
		if (m_view != nullptr) {
			m_view->ClearImgShow();
			UpdateQGraphicsViews();
		}
	}
	catch (...)	{	}
}

int frmImageGetSource::PrExecute()
{
	ui.btn3DShow->setVisible(true);
	ui.btn3DShow->setEnabled(false);
	ui.cbx_CameraView->setVisible(false);
	ui.stackedWidget->setCurrentIndex(0);
	ui.widget_Img->setVisible(true);
	ui.label_Item->setVisible(false);
	ui.cbx_Camerachannal->clear();
	m_view->setVisible(true);
	m_bNeedChangeCloud = true;
	m_iChanal = 0;
	ui.btn3DShow->setChecked(false);
	onButton3DShowClicked();

	return 0;
}

EnumNodeResult frmImageGetSource::Execute(MiddleParam& param, QString& strError)
{	
	switch (pBtnGroupRadio->checkedId()) {
	case 0: {
		QString _strFilePath = ui.txtImagePath->text();
		if (_strFilePath.isEmpty()) {
			sDrawText				_strText;
			_strText.bControl		= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText		= tr("The Current Path File Is Empty!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_ParamError;
		}
		HalconCpp::ReadImage(&m_dstImage, _strFilePath.toLocal8Bit().toStdString().data());
		m_srcImage = m_dstImage.Clone();
	}	break;
	case 1: {
		QString _strFilePath;
		if (mImgNames.size() > 0) {
			if (imgIndex >= mImgNames.size())
				imgIndex = 0;
			QString _strImgPath = mImgNames[imgIndex];
			imgIndex++;
			if (_strImgPath.isEmpty()) {
				sDrawText				_strText;
				_strText.bControl = true;
				if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = tr("The Current Path File Is Empty!");
				param.VecDrawText.push_back(_strText);
				return NodeResult_Error;
			}
			_strFilePath = ui.txtFolderPath->text() + "/" + _strImgPath;
			HalconCpp::ReadImage(&m_dstImage, _strFilePath.toLocal8Bit().toStdString().data());
			m_srcImage = m_dstImage.Clone();
		}
		else {
			sDrawText				_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("The Current Path File Is Empty!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}
	}	break;
	case 2: {
		QString _strCamera = ui.comboCamera->currentText();
		if (_strCamera.isEmpty()) {
			sDrawText				_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("当前的没有相机!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}
		Camerainterface	* _ptrCamera = CameraManager::Instance().FindCamera(_strCamera);
		if (_ptrCamera == nullptr) {
			sDrawText				_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("No Camera Is Found Currently!!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}
		if (!_ptrCamera->IsCamConnect()) {
			_ptrCamera->Init();
			_ptrCamera->Start();
			_ptrCamera->StartVedioLive();
		}
		if (!_ptrCamera->IsCamConnect()) {
			sDrawText				_strText;
			_strText.bControl = true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = tr("The Current Camera Isn't Connected!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}
		((GetImage*)m_pTool)->m_hTrrigerEvent.RstEvent();
		//设置相机参数
		for (auto iter = ((GetImage*)m_pTool)->m_mapParam.begin(); iter != ((GetImage*)m_pTool)->m_mapParam.end(); iter++)
			m_PtrCamera->SetCameraParam(iter.key(), iter.value());
		_ptrCamera->Trigger();
		if (((GetImage*)m_pTool)->m_hTrrigerEvent.WaitEvent(time_out) < 0) {
			sDrawText				_strText;
			_strText.bControl		= true;
			if (m_ShowidToProperty.count(FontSize) > 0)	_strText.iDrawLenth = m_ShowidToProperty[FontSize]->value().toInt();
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText		= tr("The Current Camera Accept TimeOut!");
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}
		if (((GetImage*)m_pTool)->m_hGrayImage.IsInitialized())
			m_dstImage = ((GetImage*)m_pTool)->m_hGrayImage.Clone();
		if (((GetImage*)m_pTool)->m_hHeightImage.IsInitialized())
			AppendChannel(((GetImage*)m_pTool)->m_hHeightImage, m_dstImage, &m_dstImage);
		m_srcImage = m_dstImage.Clone();
	}	break;
	default: {			}break;
	}

	m_bNeedChangeCloud = false;
	m_titleBar->setEnabled(true);
	ui.btn3DShow->setEnabled(true);

	return NodeResult_OK;
}

int frmImageGetSource::ExecuteComplete()
{
	HTuple	hType, hChannel;
	HalconCpp::GetImageType(m_dstImage, &hType);
	HalconCpp::CountChannels(m_dstImage, &hChannel);
	ui.cbx_Camerachannal->addItem("All");
	for (int i = 0; i < hChannel.I(); i++)
		ui.cbx_Camerachannal->addItem(QString::number(i + 1));
	ui.cbx_Camerachannal->setCurrentIndex(m_iChanal);

	QFuture<void> value = QtConcurrent::run([=] {
		UpdateCloudPoint();
		if (m_bWndDelete)	return;
		UpDateColorClound();
		if (m_bWndDelete)	return;
	});
	while (!value.isFinished()) {
		if (m_bWndDelete)	return NodeResult_OK;
		QApplication::processEvents();
	}
	ui.btn3DShow->setEnabled(true);
	m_view->update();
	UpDateCameraView();
	return NodeResult_OK;
}

int frmImageGetSource::Load()
{
	m_titleBar->setTitleContent(((GetImage*)m_pTool)->GetItemId());
	switch (((GetImage*)m_pTool)->m_iPathType){
	case 0:	ui.radioImage->click();		break;
	case 1:	ui.radioFolder->click();	break;
	case 2:	ui.radioCamera->click();	break;
	}
	ui.txtImagePath->setText(((GetImage*)m_pTool)->m_strImgPath);
	ui.txtFolderPath->setText(((GetImage*)m_pTool)->m_strFolderPath);
	if (ui.comboCamera->count() <= 0)	ui.comboCamera->addItem(((GetImage*)m_pTool)->m_strCamera);
	ui.comboCamera->setCurrentText(((GetImage*)m_pTool)->m_strCamera);
	ui.cbx_ImgOutPut->setCurrentIndex((int)((GetImage*)m_pTool)->m_eOutPutImg);

	if (((GetImage*)m_pTool)->m_hGrayImage.IsInitialized() && ((GetImage*)m_pTool)->m_hHeightImage.IsInitialized()) {
		switch (((GetImage*)m_pTool)->m_eRetn){
		case eImgType_Gray:	{	m_dstImage = ((GetImage*)m_pTool)->m_hGrayImage.Clone();		}break;
		case eImgType_Height:{	m_dstImage = ((GetImage*)m_pTool)->m_hHeightImage.Clone();	}break;
		case eImgType_HeightAndGray:{	Compose2(((GetImage*)m_pTool)->m_hGrayImage, ((GetImage*)m_pTool)->m_hHeightImage, &m_dstImage);}break;
		default: {	}	break;	}
		if (m_dstImage.IsInitialized())	{
			m_srcImage = m_dstImage.Clone();
			switch (((GetImage*)m_pTool)->m_iPathType){
			case 0: {	if (((GetImage*)m_pTool)->m_strImgPath.isEmpty())		goto GoTonext;	} break;
			case 1: {	if (((GetImage*)m_pTool)->m_strFolderPath.isEmpty())		goto GoTonext;	}break;
			case 2: {	if (((GetImage*)m_pTool)->m_strCamera.isEmpty())			goto GoTonext;	}break;}
			QImage img(Hal2QImage(m_dstImage));
			m_view->DispImage(img, true);
			HTuple	hType, hChannel,hLength;
			HalconCpp::GetImageType(m_dstImage, &hType);
			HalconCpp::CountChannels(m_dstImage, &hChannel);
			ui.cbx_Camerachannal->addItem("All");
			HalconCpp::TupleLength(hChannel,&hLength);
			if (hLength.I() <=0)	goto GoTonext;
			for (int i = 0; i < hChannel.I(); i++)	ui.cbx_Camerachannal->addItem(QString::number(i + 1));
		}
	}
GoTonext:

	QDir dir(((GetImage*)m_pTool)->m_strFolderPath);
	if (!dir.exists()) { return NodeResult_Error; }
	dir.setFilter(QDir::Files);
	dir.setSorting(QDir::Name);
	dir.setNameFilters(QString("*.jpg;*.png;*.jpeg;*.bmp;*.tif;*.tiff").split(";"));
	mImgNames.clear();
	mImgNames = dir.entryList();

	Camerainterface*_ptrCamera				= CameraManager::Instance().FindCamera(((GetImage*)m_pTool)->m_strCamera);
	m_PtrCamera								= _ptrCamera;

	SetCameraConnected();
	UpDataCameraParams(false);

	return 0;
}

int frmImageGetSource::Save()
{
	((GetImage*)m_pTool)->m_iPathType				= pBtnGroupRadio->checkedId();
	((GetImage*)m_pTool)->m_strImgPath				= ui.txtImagePath->text();
	((GetImage*)m_pTool)->m_strFolderPath			= ui.txtFolderPath->text();
	((GetImage*)m_pTool)->m_strCamera				= ui.comboCamera->currentText();

	((GetImage*)m_pTool)->m_eOutPutImg				= (eImgType)ui.cbx_ImgOutPut->currentIndex();

	return 0;
}

void frmImageGetSource::InitCamera()
{
	if (ui.comboCamera->count() <= 0)			return;
	if (ui.comboCamera->currentIndex() < 0)		return;
	QString _strCameraName						= ui.comboCamera->currentText();
	if (_strCameraName.isEmpty())				return;

	Camerainterface*_ptrCamera					= nullptr;
	if (((GetImage*)m_pTool)->InitCamera(_strCameraName, _ptrCamera) != NodeResult_OK)	{	}
	else {	}
	if (m_PtrCamera != nullptr && _ptrCamera == nullptr) {	//初始化
		m_PtrCamera->StopVedioLive();
		m_PtrCamera->Stop();
		m_PtrCamera->Exit();
		delete m_PtrCamera;
	}
	m_PtrCamera = _ptrCamera;
	m_strCamera = _strCameraName;

	UpDataCameraParams();
	SetCameraConnected();
}

void frmImageGetSource::SetCameraConnected()
{
	if (m_PtrCamera != nullptr)
		if (m_PtrCamera->IsCamConnect())	{
			ui.btnConnect->setText(tr("Stop"));
			ui.btnConnect->setToolTip(tr("Stop"));
			switch (pBtnGroupRadio->checkedId())	{
			case 0: {	}break;
			case 2: {		}break;
			}
			return;
		}
	switch (pBtnGroupRadio->checkedId()){
	case 0:	break;
	case 2:	break;
	}
	ui.btnConnect->setText(tr("Connect"));
	ui.btnConnect->setToolTip(tr("Connect"));
}

void frmImageGetSource::ShowImg()
{
	QApplication::postEvent(this, new MsgEvent(1, 0, ""));
}

void frmImageGetSource::valueChanged(QtProperty *property, const QVariant &value)
{
	if (!propertyToId.contains(property))	return;
	QString id = propertyToId[property];
	if (m_PtrCamera != nullptr)
		if (m_PtrCamera->IsCamConnect())	{
			m_PtrCamera->SetCameraParam(id, value.toString());
			if (	id == CameraSerial 
				||	id == CameraModels 
				||	id == CameraLicenseKey)	{
				return;
			}
			((GetImage*)m_pTool)->m_mapParam.insert(id, value.toString());
		}
}

void frmImageGetSource::slot_btnGroupClicked(int value)
{
	switch (value)	{
	case 0: {
		ui.txtImagePath->setEnabled(true);
		ui.btnSelectImage->setEnabled(true);
		ui.txtFolderPath->setEnabled(false);
		ui.btnSelectPath->setEnabled(false);
		ui.comboCamera->setEnabled(false);
		ui.btnConnect->setEnabled(false);
		ui.btnUpdata->setEnabled(false);
	}	break;
	case 1: {
		ui.txtImagePath->setEnabled(false);
		ui.btnSelectImage->setEnabled(false);
		ui.txtFolderPath->setEnabled(true);
		ui.btnSelectPath->setEnabled(true);
		ui.comboCamera->setEnabled(false);
		ui.btnConnect->setEnabled(false);
		ui.btnUpdata->setEnabled(false);
	}	break;
	case 2: {
		ui.txtImagePath->setEnabled(false);
		ui.btnSelectImage->setEnabled(false);
		ui.txtFolderPath->setEnabled(false);
		ui.btnSelectPath->setEnabled(false);
		ui.comboCamera->setEnabled(true);
		ui.btnConnect->setEnabled(true);
		ui.btnUpdata->setEnabled(true);
		if (ui.btnConnect->isChecked())	{
		}
		else {
		}
	}	break;
	default:{	}	break;
	}
}

void frmImageGetSource::slot_MouseMenuClicked(QString strkey)
{
	if (strkey == "3DShow"	|| strkey == "2DShow")			{
		if (ui.btn3DShow->isChecked())	{
			ui.stackedWidget->setCurrentIndex(0);
			ui.widget_Img->setVisible(true);
			ui.label_Item->setVisible(false);
			ui.btn3DShow->setChecked(false);
			ui.cbx_CameraView->setVisible(false);
			m_view->setVisible(true);
		}
		else {
			ui.widget_Img->setVisible(false);
			ui.stackedWidget->setCurrentIndex(1);
			ui.label_Item->setVisible(true);
			ui.btn3DShow->setChecked(true);
			m_view->setVisible(false);
			ui.cbx_CameraView->setVisible(true);
			UpDateCameraView();
		}
		ui.btn3DShow->setVisible(true);
	}
}

void frmImageGetSource::UpDataCameraParams(bool bCameraopened)
{
	variantManager->clear();
	propertyEditor->clear();
	ClearCameraProperty();
	//设置相机参数
	if (m_PtrCamera != nullptr)
		if (m_PtrCamera->IsCamConnect()) {
			QVector<QPair<QString, QString>> mapParam;
			m_PtrCamera->GetAllCameraParam(mapParam);
			QtVariantProperty *_ptrproperty = nullptr;
			for (auto iter = mapParam.begin(); iter != mapParam.end(); iter++) {
				int				iEnable; 
				EnumParamType	iType;
				QString			Param;
				QStringList		strEnumLst;
				QString			LowParam;
				QString			HighParam;
				QString			strLang;
				if (ValueToParam((*iter).second, strLang, iEnable, iType, Param, strEnumLst, LowParam, HighParam) == 0) {
					switch (iType)			{
					case ParamType_Bool:	{
						_ptrproperty = variantManager->addProperty(QVariant::Bool, (*iter).first);
					}	break;
					case ParamType_Int:		{
						_ptrproperty = variantManager->addProperty(QVariant::Int, (*iter).first);
					}	break;
					case ParamType_Double:	{
						_ptrproperty = variantManager->addProperty(QVariant::Double, (*iter).first);
					}	break;
					case ParamType_String:	{
						_ptrproperty = variantManager->addProperty(QVariant::String, (*iter).first);
					}	break;
					case ParamType_Enum:	{
						_ptrproperty = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), (*iter).first);
						_ptrproperty->setAttribute(QLatin1String("enumNames"), strEnumLst);
					}	break;
					case ParamType_Other:	{
						_ptrproperty = variantManager->addProperty(QVariant::Double, (*iter).first);
					}	break;
					default:{		}break;
					}
					_ptrproperty->setValue(Param);
					_ptrproperty->setEnabled(iEnable == 1? true : false);
					_ptrproperty->setAttribute(QLatin1String("decimals"), databaseVar::Instance().form_System_Precision);
					if (!LowParam.isEmpty())	_ptrproperty->setAttribute(QLatin1String("minimum"), LowParam);
					if (!HighParam.isEmpty())	_ptrproperty->setAttribute(QLatin1String("maximum"), HighParam);
					addProperty(_ptrproperty, QLatin1String((*iter).first.toStdString().c_str()));
				}
				else {
					_ptrproperty = variantManager->addProperty(QVariant::Double, (*iter).first);
					_ptrproperty->setValue((*iter).second);
					addProperty(_ptrproperty, QLatin1String((*iter).first.toStdString().c_str()));
				}
				_ptrproperty->setPropertyName(strLang);
			}
			return;
		}
}

void frmImageGetSource::UpDateCameraView()
{
	slot_camShowChanged(ui.cbx_CameraView->currentIndex());
}

void frmImageGetSource::on_comboCamera_currentIndexChanged(int index)
{

}

void frmImageGetSource::contextMenuEvent(QContextMenuEvent *ev)
{
	QDialog::contextMenuEvent(ev);
	QPoint pos = ev->pos();
	pos = this->mapToGlobal(pos);
	if (ui.btn3DShow->isChecked())
	{
		QMenu* menu = new QMenu(this);
		QPushButton*	RangeAction_buf	= new QPushButton(menu);
		QWidgetAction*	RangeAction		= new QWidgetAction(this);
		{
			//设定点云范围
			RangeAction_buf->setText(QString(tr("设定取值范围")));
			RangeAction_buf->setIconSize(QSize(22, 22));
			RangeAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/fit.png"));
			RangeAction->setDefaultWidget(RangeAction_buf);
			connect(RangeAction_buf, &QPushButton::clicked, menu, &QMenu::close);	
			connect(RangeAction_buf, &QPushButton::clicked, this, [=]() {

			});		//相机参数
			//相机参数
			menu->addAction(RangeAction);
			menu->addSeparator();
		}
		QPushButton*	presetAction_buf	= new QPushButton(menu);
		QWidgetAction*	presetAction		= new QWidgetAction(this);
		{
			presetAction_buf->setText(tr("Show 2D"));
			presetAction_buf->setIconSize(QSize(22, 22));
			presetAction_buf->setIcon(QIcon(":/QtApplication/Bitmaps/2DView.png"));
			presetAction->setDefaultWidget(presetAction_buf);
			connect(presetAction_buf, &QPushButton::clicked, this, [=]() {	slot_MouseMenuClicked("2DShow");	});
			connect(presetAction_buf, &QPushButton::clicked, menu, &QMenu::close);
			menu->addAction(presetAction);
			menu->addSeparator();
		}
		menu->exec(pos);

		if (RangeAction_buf != nullptr)			delete RangeAction_buf;		RangeAction_buf		= nullptr;
		if (RangeAction != nullptr)				delete RangeAction;			RangeAction			= nullptr;
		if (menu != nullptr)					delete menu;				menu = nullptr;
	}
}

void frmImageGetSource::UpDateColorClound()
{
	try
	{	
		//显示相关的参数
		refreshAll();
	}
	catch (...)	{	}
}

void frmImageGetSource::initialVtkWidget()
{

}

void frmImageGetSource::on_btnSelectImage_clicked()
{
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getOpenFileName(this, tr("打开图片"), path_C, "*.bmp *png *jpg *.jpeg *.tif *.tiff");
	if (false == dirPath.isEmpty())	{		
		ui.txtImagePath->setText(dirPath);
	}
}

bool frmImageGetSource::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType)	{
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent != nullptr)				{
			if (MyEvent->iIndex == 0)		{
				ui.btn3DShow->setEnabled(true);
				m_titleBar->setEnabled(true);
			}
			else if (MyEvent->iIndex == 1)	{
				if (((GetImage*)m_pTool)->m_hGrayImage.IsInitialized())
					m_dstImage = ((GetImage*)m_pTool)->m_hGrayImage.Clone();
				if (((GetImage*)m_pTool)->m_hHeightImage.IsInitialized())
					Union2(((GetImage*)m_pTool)->m_hHeightImage, m_dstImage, &m_dstImage);


			}
		}
	}
	return frmBaseTool::event(ev);
}

void frmImageGetSource::resizeEvent(QResizeEvent * ev)
{
	frmBaseTool::resizeEvent(ev);
}

void frmImageGetSource::on_btnConnect_clicked()
{
	ClearCameraProperty();
	if (ui.comboCamera->count() <= 0)	return;
	if (ui.comboCamera->currentIndex() < 0)	return;
	QString _strCameraName				= ui.comboCamera->currentText();
	if (_strCameraName.isEmpty())		return;
	ui.btnConnect->setChecked(true);
	ui.btnConnect->setEnabled(false);
	QFuture<bool> value = QtConcurrent::run([=] {
		if (!PluginsManager::Instance().FindCamera(_strCameraName)) {	}
		Camerainterface*_ptrCamera			= CameraManager::Instance().FindCamera(_strCameraName);
		if (_ptrCamera != nullptr)			{
			if (_ptrCamera->IsCamConnect())	{
				_ptrCamera->StopVedioLive();
				_ptrCamera->Stop();
				_ptrCamera->Exit();
				if(_ptrCamera != nullptr)	delete	_ptrCamera;		_ptrCamera		= nullptr;
				m_PtrCamera		= nullptr;
				CameraManager::Instance().DeleteCamera(_strCameraName);
				return true;
			}
		}
		if (((GetImage*)m_pTool)->InitCamera(_strCameraName, _ptrCamera) != NodeResult_OK) {}
		else {}
		if (m_PtrCamera != nullptr && _ptrCamera == nullptr) {	//初始化
			m_PtrCamera->StopVedioLive();
			m_PtrCamera->Stop();
			m_PtrCamera->Exit();
			delete m_PtrCamera;
		}
		m_PtrCamera = _ptrCamera;
		m_strCamera = _strCameraName;
		return false;
	});
	while (!value.isFinished()) {	QApplication::processEvents();	}
	if (!value.result())	{
		UpDataCameraParams();
		SetCameraConnected();
	}
	else {
		switch (pBtnGroupRadio->checkedId()){
		case 0:	break;
		case 2:		break;
		}
		SetCameraConnected();
		UpDataCameraParams(false);
	}
	ui.btnConnect->setChecked(false);
	ui.btnConnect->setEnabled(true);
}

void frmImageGetSource::on_btnSelectPath_clicked()
{
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("打开图片文件夹"), path_C);
	if (false == dirPath.isEmpty())	{
		imgIndex = 0;		
		ui.txtFolderPath->setText(dirPath);
		QDir dir(dirPath);
		if (!dir.exists()) { return ; }
		dir.setFilter(QDir::Files);
		dir.setSorting(QDir::Name);
		dir.setNameFilters(QString("*.jpg;*.png;*.jpeg;*.bmp;*.tif;*.tiff").split(";"));
		mImgNames.clear();
		mImgNames = dir.entryList();
	}
}

void frmImageGetSource::on_btnUpdata_clicked()
{
	ui.comboCamera->clear();
	for (auto iter : PluginsManager::Instance().getVecCameras()){
		ui.comboCamera->addItem(iter.strCameraName);
	}
}

void frmImageGetSource::addProperty(QtVariantProperty * property, const QString & id)
{
	propertyToId[property] = id;
	idToProperty[id] = property;
	QtBrowserItem *item = propertyEditor->addProperty(property);
	propertyEditor->setExpanded(item, true);
}

void frmImageGetSource::slot_camShowChanged(int index)
{


}

//重设视角
void frmImageGetSource::slot_action_reset_triggered()
{

}