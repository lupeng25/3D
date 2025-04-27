#include "frmSaveImageTool.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPainter>
#include <QThread>
#include <QElapsedTimer>
#include "databaseVar.h"
#include "HalconCpp.h"
//#include "NodeManager.h"

#include "Port.h"
using namespace HalconCpp;

frmSaveImageTool::frmSaveImageTool(QWidget* parent)
	: frmBaseTool(parent)
{
	ui.setupUi(GetCenterWidget());
	//FramelessWindowHint属性设置窗口去除边框
	//WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//设置窗体在屏幕中间位置
	QDesktopWidget* desktop		= QApplication::desktop();
	move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);
	//设置窗口背景透明
	//setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowIcon(QIcon(":/icon/icon/save.svg"));
	//初始化标题栏
	initTitleBar();	

	initConnect();
	//选项按钮
	pBtnGroupRadio = new QButtonGroup(this);
	pBtnGroupRadio->addButton(ui.radioImage, 0);
	pBtnGroupRadio->addButton(ui.radioFolder, 1);
	ui.radioImage->click();
}

frmSaveImageTool::~frmSaveImageTool()
{
	if (pBtnGroupRadio != nullptr)				delete pBtnGroupRadio;				pBtnGroupRadio	= nullptr;
	if (m_titleBar != nullptr)					delete m_titleBar;					m_titleBar		= nullptr;
	this->deleteLater();
}

void frmSaveImageTool::initTitleBar()
{
	m_titleBar->setTitleIcon(":/icon/icon/save.svg");
}

void frmSaveImageTool::initConnect()
{
	connect(ui.btnSelectImage,	&QPushButton::pressed, this, &frmSaveImageTool::on_btnSelectImage_clicked);
	connect(ui.btnSelectPath,	&QPushButton::pressed, this, &frmSaveImageTool::on_btnSelectPath_clicked);

}

void frmSaveImageTool::on_btnSelectImage_clicked()
{
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getSaveFileName(this, tr("打开图片"), path_C, "*.bmp *png *jpg *.jpeg");
	if (false == dirPath.isEmpty())
	{
		ui.txtImagePath->setText(dirPath);
	}
}

void frmSaveImageTool::on_btnSelectPath_clicked()
{
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("打开图片文件夹"), path_C);
	if (false == dirPath.isEmpty())
	{
		ui.txtFolderPath->setText(dirPath);
	}
}

EnumNodeResult frmSaveImageTool::Execute(MiddleParam & param, QString & strError)
{
	HTuple hType;
	if (((SaveImageTool*)m_pTool)->m_PMiddleParam == nullptr) { return NodeResult_Error; }

	HObject _Object;
	if (((SaveImageTool*)m_pTool)->m_PMiddleParam->MapPImgVoid.count(ui.cbx_ImgInPut->currentText()) > 0)
		_Object = ((HObject*)((SaveImageTool*)m_pTool)->m_PMiddleParam->MapPImgVoid[ui.cbx_ImgInPut->currentText()])->Clone();
	else return NodeResult_Error;

	GetImageType(_Object, &hType);
	std::string strExName = ((SaveImageTool*)m_pTool)->GetFileExName((EnumImgType)ui.txtSaveFileEx->currentIndex()).toLocal8Bit();
	std::string strFilePath = "";
	if (hType.S() == HString("real"))	strExName = "tiff";

	switch (pBtnGroupRadio->checkedId())
	{
	case 0:		//保存文件路径
	{
		if (ui.txtImagePath->text().isEmpty()) return NodeResult_Error;
		strFilePath = ui.txtImagePath->text().toLocal8Bit();
		WriteImage(_Object, strExName.c_str(), 0, strFilePath.c_str());
	}	break;
	default:	//保存文件夹路径
	{
		QString _strFile = ui.txtFolderPath->text();
		if (ui.txtSaveFileExTension->currentIndex() != 0 && ui.txtSaveFileExTension->currentIndex() != 2) {
			_strFile = QString("%1/%2").arg(ui.txtFolderPath->text()).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
			QDir dir(_strFile);
			if (!dir.exists())
				dir.mkdir(_strFile);
		}
		strFilePath = (QString("%1/%2").arg(_strFile).arg(((SaveImageTool*)m_pTool)->GetFileExFileName(ui.txtSaveFileExTension->currentIndex()))).toLocal8Bit();
		if (strFilePath.empty()) { return NodeResult_Error; }

		WriteImage(_Object, strExName.c_str(), 0, strFilePath.c_str());
	}	break;
	}
	return NodeResult_OK;
}

int frmSaveImageTool::Load()
{
	m_titleBar->setTitleContent(((SaveImageTool*)m_pTool)->GetItemId());

	ui.cbx_ImgInPut->clear();
	if (MiddleParam::MapMiddle().size() > ((SaveImageTool*)m_pTool)->m_iFlowIndex) {
		for (auto iter : MiddleParam::MapMiddle()[((SaveImageTool*)m_pTool)->m_iFlowIndex].MapPImgVoid) {	//图像
			if (!iter.first.contains(((SaveImageTool*)m_pTool)->GetItemId())) {
				ui.cbx_ImgInPut->addItem(iter.first);
			}
		}
	}
	ui.cbx_ImgInPut->setCurrentText(((SaveImageTool*)m_pTool)->m_strInImg);

	switch (((SaveImageTool*)m_pTool)->m_iToolType)
	{
	case 0:	ui.radioImage->click();		break;
	default:
	case 1:	ui.radioFolder->click();	break;
	}
	ui.txtImagePath->setText(((SaveImageTool*)m_pTool)->m_strImgPath);
	ui.txtFolderPath->setText(((SaveImageTool*)m_pTool)->m_strFolderPath);

	ui.txtSaveFileEx->setCurrentIndex(((SaveImageTool*)m_pTool)->m_EnumImgType);
	ui.txtSaveFileExTension->setCurrentIndex(((SaveImageTool*)m_pTool)->m_iSaveExType);
	return 0;
}

int frmSaveImageTool::Save()
{
	((SaveImageTool*)m_pTool)->m_strInImg			= ui.cbx_ImgInPut->currentText();
	((SaveImageTool*)m_pTool)->m_iToolType			= pBtnGroupRadio->checkedId();
	((SaveImageTool*)m_pTool)->m_strImgPath			= ui.txtImagePath->text();
	((SaveImageTool*)m_pTool)->m_strFolderPath		= ui.txtFolderPath->text();
	
	((SaveImageTool*)m_pTool)->m_EnumImgType		= (EnumImgType)ui.txtSaveFileEx->currentIndex();
	((SaveImageTool*)m_pTool)->m_iSaveExType		= ui.txtSaveFileExTension->currentIndex();

	return 0;
}
