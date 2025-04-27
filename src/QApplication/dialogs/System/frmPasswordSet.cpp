#include "frmPasswordSet.h"
#include "ui_frmLogSet.h"
#include <QDateTime>
#include "Data/dataVar.h"

frmPasswordSet::frmPasswordSet(QWidget* parent) :
	frmSetBase(parent)
{
	ui.setupUi(this);

}

frmPasswordSet::~frmPasswordSet()
{
}

bool frmPasswordSet::CheckPageAvalible()
{
	if (dataVar::Instance().iLoginLevel <= 8) {
		dataVar::Instance().pManager->notify(tr("Warnning"), tr("The Current Permissions Are Insufficient!"), NotifyType_Warn);
		return false;
	}
	return true;
}

int frmPasswordSet::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("cbx_UseDefaultPassward",	QString::number(ui.cbx_UseDefaultPassward->isChecked()));
	CommonParam.insert("lineEdit",					QString(ui.lineEdit->text()));
	strData.insert("frmPasswordSet", CommonParam);
	return 0;
}

int frmPasswordSet::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("frmPasswordSet").value().toObject();
	if (CommonParam.contains("cbx_UseDefaultPassward")) { ui.cbx_UseDefaultPassward->setChecked(CommonParam["cbx_UseDefaultPassward"].toString().toInt()); }
	if (CommonParam.contains("lineEdit"))				{ ui.lineEdit->setText(CommonParam["lineEdit"].toString()); }

	return 0;
}

int frmPasswordSet::NewProject()
{
	return 0;
}

int frmPasswordSet::UpdateUI()
{
	dataVar::Instance().bUseDefaultPassward = ui.cbx_UseDefaultPassward->isChecked();
	dataVar::Instance().strDefaultPassward	= ui.lineEdit->text();

	return 0;
}

void frmPasswordSet::RetranslationUi()
{
	ui.retranslateUi(this);
}

QVector<QPair<QString, QString>> frmPasswordSet::GetVecSetName()
{
	QVector<QPair<QString, QString>> vecNames;
	vecNames.push_back(QPair<QString, QString>("Normal",		tr("Normal")));
	vecNames.push_back(QPair<QString, QString>("PasswordSet",	tr("PasswordSet")));
	return vecNames;
}
