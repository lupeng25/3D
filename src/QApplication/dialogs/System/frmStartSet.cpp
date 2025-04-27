#include "frmStartSet.h"
#include <QDateTime>
#include <QFileDialog>
#include <QScreen>
#include "Data/dataVar.h"

frmStartSet::frmStartSet(QWidget* parent) :
	frmSetBase(parent)
{
	ui.setupUi(this);
	slotUpDateStartImages();
	if (ui.cbx_SoftImgPath->lineEdit())		ui.cbx_SoftImgPath->lineEdit()->setAlignment(Qt::AlignCenter);
	setTextAlignment(ui.cbx_SoftImgPath);
	connect(ui.cbx_SoftImgPath, SIGNAL(sigPopup()), this, SLOT(slotUpDateStartImages()));
	UpdateMinMax();
}

frmStartSet::~frmStartSet()
{

}

int frmStartSet::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("lineEdit",			QString(ui.lineEdit->text()));
	CommonParam.insert("lineEdit_Company",	QString(ui.lineEdit_Company->text()));
	CommonParam.insert("lEdit_OtherInfo",	QString(ui.lEdit_OtherInfo->toPlainText()));
	CommonParam.insert("cbx_SoftImgPath",	QString(ui.cbx_SoftImgPath->currentText()));
	CommonParam.insert("spinBox_Width",		QString::number(ui.spinBox_Width->value()));
	CommonParam.insert("spinBox_Height",	QString::number(ui.spinBox_Height->value()));
	strData.insert("frmStartSet", CommonParam);

	QSettings settings(QApplication::applicationDirPath() + "/Config/config.ini", QSettings::IniFormat);
	settings.beginGroup("General");
	settings.setValue(QString("software_name"),			ui.lineEdit->text());
	settings.setValue(QString("software_StartImg"),		ui.cbx_SoftImgPath->currentText());
	settings.setValue(QString("software_OtherInfo"),	ui.lEdit_OtherInfo->toPlainText());
	settings.setValue(QString("software_CompanyName"),	ui.lineEdit_Company->text());
	settings.setValue(QString("spinBox_Width"),			QString::number(ui.spinBox_Width->value()));
	settings.setValue(QString("spinBox_Height"),		QString::number(ui.spinBox_Height->value()));
	settings.endGroup();

	return 0;
}

int frmStartSet::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("frmStartSet").value().toObject();	
	if (CommonParam.contains("lineEdit"))			{ ui.lineEdit->setText(CommonParam["lineEdit"].toString()); }
	if (CommonParam.contains("lEdit_OtherInfo"))	{ ui.lEdit_OtherInfo->setText(CommonParam["lEdit_OtherInfo"].toString()); }
	if (CommonParam.contains("lineEdit_Company"))	{ ui.lineEdit_Company->setText(CommonParam["lineEdit_Company"].toString()); }
	if (CommonParam.contains("spinBox_Width"))		{ ui.spinBox_Width->setValue(CommonParam["spinBox_Width"].toString().toDouble()); }
	if (CommonParam.contains("spinBox_Height"))		{ ui.spinBox_Height->setValue(CommonParam["spinBox_Height"].toString().toDouble()); }

	if (CommonParam.contains("cbx_SoftImgPath")) { 
		for (int iIndex = 0; iIndex < ui.cbx_SoftImgPath->count(); iIndex++)	{
			auto strText = ui.cbx_SoftImgPath->itemText(iIndex);
			if (strText == CommonParam["cbx_SoftImgPath"].toString())	{
				ui.cbx_SoftImgPath->setCurrentIndex(iIndex);	break;
			}
		}
		ui.cbx_SoftImgPath->setCurrentText(CommonParam["cbx_SoftImgPath"].toString()); 
	}

	return 0;
}

int frmStartSet::NewProject()
{
	return 0;
}

void frmStartSet::slotUpDateStartImages()
{
	QString str = ui.cbx_SoftImgPath->currentText();
	ui.cbx_SoftImgPath->clear();
	QString strPath = QApplication::applicationDirPath() + "/startimage/";
	QDir dir(strPath );
	QFileInfoList fileList = dir.entryInfoList();
	for (QFileInfo f : fileList) {
		if (f.baseName() == "") {
			continue;
		}
		ui.cbx_SoftImgPath->addItem(QIcon(strPath + f.fileName()) ,f.fileName());
	}
	setTextAlignment(ui.cbx_SoftImgPath);
	ui.cbx_SoftImgPath->setCurrentText(str);
}

int frmStartSet::UpdateUI()
{

	dataVar::Instance().software_name			= ui.lineEdit->text();
	dataVar::Instance().software_CompanyName	= ui.lineEdit_Company->text();
	dataVar::Instance().software_StartImg		= ui.cbx_SoftImgPath->currentText();
	dataVar::Instance().software_OtherInfo		= ui.lEdit_OtherInfo->toPlainText();

	QSettings settings(QApplication::applicationDirPath() + "/Config/config.ini", QSettings::IniFormat);
	settings.beginGroup("General");
	dataVar::Instance().software_name			= settings.value(QString("software_name"), QString("QApplication")).toString();
	dataVar::Instance().software_StartImg		= settings.value(QString("software_StartImg"), QString("default")).toString();
	dataVar::Instance().software_OtherInfo		= settings.value(QString("software_OtherInfo"), QString("")).toString();
	dataVar::Instance().software_CompanyName	= settings.value(QString("software_CompanyName"), QString("")).toString();
	dataVar::Instance().SetDlgSize.setWidth(	settings.value(QString("spinBox_Width"), QString("-1")).toString().toDouble());
	dataVar::Instance().SetDlgSize.setHeight(	settings.value(QString("spinBox_Height"), QString("-1")).toString().toDouble());
	settings.endGroup();
	return 0;
}

void frmStartSet::RetranslationUi()
{
	ui.retranslateUi(this);
}

QVector<QPair<QString, QString>> frmStartSet::GetVecSetName()
{
	QVector<QPair<QString, QString>> vecNames;
	vecNames.push_back(QPair<QString, QString>("Normal",tr("Normal")));
	vecNames.push_back(QPair<QString, QString>("StartSet", tr("StartSet")));
	return vecNames;
}

void frmStartSet::on_btnSoftWareImg_clicked()
{
	QString path_C = QDir::currentPath();
	QString dirPath = QFileDialog::getOpenFileName(this, tr("打开图片"), path_C, "*.bmp;*png;*jpg *.jpeg;*.tif;*.tiff");
	if (false == dirPath.isEmpty()) {
		auto strFileNameLst = dirPath.split("/");
		if (strFileNameLst.size() > 0)	{
			QString strPath = QApplication::applicationDirPath() + "/startimage/" + strFileNameLst[strFileNameLst.size() - 1];
			if (QFile::exists(dirPath)) {
				if (QFile::copy(dirPath, strPath)) {
					SetCurrentName(strFileNameLst[strFileNameLst.size() - 1]);
				}
				else {
					dataVar::Instance().pManager->notify(tr("复制失败(可能目标文件已存在或路径无效)"), NotifyType_Error);

				}
			}
			else {
				dataVar::Instance().pManager->notify(tr("源文件不存在"), NotifyType_Error);
			}
		}
	}
}

void frmStartSet::setTextAlignment(QComboBox * cbx, Qt::Alignment alignment)
{
	auto* model = dynamic_cast<QStandardItemModel*>(cbx->view()->model());
	if (Q_NULLPTR == model)
		return;
	for (int index = 0, size = model->rowCount(); index < size; ++index) {
		if (Q_NULLPTR != model->item(index)) {
			model->item(index)->setTextAlignment(alignment);
		}
	}
}

void frmStartSet::SetCurrentName(QString strName)
{
	QString str = ui.cbx_SoftImgPath->currentText();
	ui.cbx_SoftImgPath->clear();
	QString strPath = QApplication::applicationDirPath();
	QDir dir(strPath + "/startimage/");
	QFileInfoList fileList = dir.entryInfoList();
	for (QFileInfo f : fileList) {
		if (f.baseName() == "") {
			continue;
		}
		ui.cbx_SoftImgPath->addItem(QIcon(strPath + f.fileName()), f.fileName());
	}
	setTextAlignment(ui.cbx_SoftImgPath);
	ui.cbx_SoftImgPath->setCurrentText(strName);
}

void frmStartSet::UpdateMinMax()
{
	// 获取主屏幕
	QScreen *screen = QGuiApplication::primaryScreen();
	// 获取屏幕的尺寸
	QSize screenSize = screen->size();
	int screenWidth = screenSize.width();
	int screenHeight = screenSize.height();
	ui.spinBox_Width->setMaximum(screenWidth);
	ui.spinBox_Height->setMaximum(screenHeight);
}

void frmStartSet::showEvent(QShowEvent * event)
{
	frmSetBase::showEvent(event);

	UpdateMinMax();
}

void frmStartSet::hideEvent(QHideEvent * event)
{
	frmSetBase::hideEvent(event);
}
