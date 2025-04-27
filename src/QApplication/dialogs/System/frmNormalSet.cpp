#include "frmNormalSet.h"
//#include "ui_frmNormal.h"
#include <QDateTime>
#include "Data/dataVar.h"

frmNormalSet::frmNormalSet(QWidget* parent) :
	frmSetBase(parent)
{
	ui.setupUi(this);

}

frmNormalSet::~frmNormalSet()
{
}

int frmNormalSet::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("cbx_StartWhenWinStart",				QString::number(ui.cbx_StartWhenWinStart->isChecked()));
	CommonParam.insert("cbx_AutoFullScreen",				QString::number(ui.cbx_AutoFullScreen->isChecked()));
	CommonParam.insert("cbx_FlowStartWhenProcessStart",		QString::number(ui.cbx_FlowStartWhenProcessStart->isChecked()));
	CommonParam.insert("spinLogOutTime",					QString::number(ui.spinLogOutTime->value()));
	//CommonParam.insert("cbx_Language",						QString::number(ui.cbx_Language->currentIndex()));
	CommonParam.insert("spx_RegistrationDays",				QString::number(ui.spx_RegistrationDays->value()));
	CommonParam.insert("cbx_EnableRegistrationAlart",		QString::number(ui.cbx_EnableRegistrationAlart->isChecked()));
	CommonParam.insert("spinMotionTimeOut",					QString::number(ui.spinMotionTimeOut->value()));
	CommonParam.insert("cbx_UseThreeLight",					QString::number(ui.cbx_UseThreeLight->isChecked()));

	strData.insert("frmNormalSet", CommonParam);
	//ChangeSetLanguage((Language)ui.cbx_Language->currentIndex());
	return 0;
}

int frmNormalSet::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("frmNormalSet").value().toObject();
	if (CommonParam.contains("cbx_StartWhenWinStart"))			ui.cbx_StartWhenWinStart->setChecked(CommonParam["cbx_StartWhenWinStart"].toString().toInt());
	if (CommonParam.contains("cbx_AutoFullScreen"))				ui.cbx_AutoFullScreen->setChecked(CommonParam["cbx_AutoFullScreen"].toString().toInt());
	if (CommonParam.contains("cbx_FlowStartWhenProcessStart"))	ui.cbx_FlowStartWhenProcessStart->setChecked(CommonParam["cbx_FlowStartWhenProcessStart"].toString().toInt());
	if (CommonParam.contains("spinLogOutTime"))					ui.spinLogOutTime->setValue(CommonParam["spinLogOutTime"].toString().toInt());
	//if (CommonParam.contains("cbx_Language"))					ui.cbx_Language->setCurrentIndex(CommonParam["cbx_Language"].toString().toInt());
	if (CommonParam.contains("spx_RegistrationDays"))			ui.spx_RegistrationDays->setValue(CommonParam["spx_RegistrationDays"].toString().toInt());
	if (CommonParam.contains("cbx_EnableRegistrationAlart"))	ui.cbx_EnableRegistrationAlart->setChecked(CommonParam["cbx_EnableRegistrationAlart"].toString().toInt());
	
	if (CommonParam.contains("spinMotionTimeOut"))				ui.spinMotionTimeOut->setValue(CommonParam["spinMotionTimeOut"].toString().toInt());
	if (CommonParam.contains("cbx_UseThreeLight"))				ui.cbx_UseThreeLight->setChecked(CommonParam["cbx_UseThreeLight"].toString().toInt());

	//static bool _bFirstChangeLanguage = false;
	//if (!_bFirstChangeLanguage)		_bFirstChangeLanguage = true;
	//else
	//{
	//	ChangeSetLanguage((Language)ui.cbx_Language->currentIndex());
	//}
	return 0;
}

//设置程序自启动appPath程序路径
void frmNormalSet::SetProcessAutoRunSelf(const QString& appPath)
{
	//注册表路径需要使用双反斜杠，如果是32位系统，要使用QSettings::Registry32Format
	QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::Registry64Format);
	//以程序名称作为注册表中的键，根据键获取对应的值(程序路径)
	QFileInfo fInfo(appPath);
	QString name = fInfo.baseName();
	QString path = settings.value(name).toString();
	QString newPath = QDir::toNativeSeparators(appPath);
	if (path != newPath)
	{
		settings.setValue(name, newPath);
	}
}

//移除程序自启动appPath程序路径
void frmNormalSet::RemoveProcessAutoRunSelf(const QString& appPath)
{
	//注册表路径需要使用双反斜杠，如果是32位系统，要使用QSettings::Registry32Format
	QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::Registry64Format);
	//以程序名称作为注册表中的键，根据键获取对应的值(程序路径)
	QFileInfo fInfo(appPath);
	QString name = fInfo.baseName();
	settings.remove(name);
}

int frmNormalSet::NewProject()
{
	return 0;
}

int frmNormalSet::UpdateUI()
{
	if (ui.cbx_StartWhenWinStart->isChecked()) { SetProcessAutoRunSelf(qApp->applicationFilePath()); }
	else { RemoveProcessAutoRunSelf(qApp->applicationFilePath()); }

	dataVar::Instance().auto_login_out_Time			= ui.spinLogOutTime->value();
	dataVar::Instance().auto_run_system_start		= ui.cbx_FlowStartWhenProcessStart->isChecked();
	dataVar::Instance().auto_run_system_FullScreen	= ui.cbx_AutoFullScreen->isChecked();
	dataVar::Instance().bEnableRegistrationAlart	= ui.cbx_EnableRegistrationAlart->isChecked();
	dataVar::Instance().iRegistrationDays			= ui.spx_RegistrationDays->value();
	dataVar::Instance().iMotionTimeOut				= ui.spinMotionTimeOut->value();

	return 0;
}

void frmNormalSet::RetranslationUi()
{
	ui.retranslateUi(this);
}
QVector<QPair<QString, QString>> frmNormalSet::GetVecSetName()
{
	QVector<QPair<QString, QString>> vecNames;
	vecNames.push_back(QPair<QString, QString>("Normal", tr("Normal")));
	vecNames.push_back(QPair<QString, QString>("NormalSet", tr("NormalSet")));
	return vecNames;
}