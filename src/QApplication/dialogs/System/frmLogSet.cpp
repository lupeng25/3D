#include "frmLogSet.h"
#include "ui_frmLogSet.h"
#include <QDateTime>
#include "Data/dataVar.h"

frmLogSet::frmLogSet(QWidget* parent) :
	frmSetBase(parent)
{
	ui.setupUi(this);

}

frmLogSet::~frmLogSet()
{
}

int frmLogSet::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("spinLogOutTime", QString::number(ui.spinLogSaveDays->value()));
	strData.insert("frmLogSet", CommonParam);
	return 0;
}

int frmLogSet::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("frmLogSet").value().toObject();
	if (CommonParam.contains("spinLogSaveDays")) { ui.spinLogSaveDays->setValue(CommonParam["spinLogSaveDays"].toString().toInt()); }

	return 0;
}

int frmLogSet::NewProject()
{
	return 0;
}

int frmLogSet::UpdateUI()
{
	dataVar::Instance().auto_log_save_days		= ui.spinLogSaveDays->value();
	return 0;
}

void frmLogSet::RetranslationUi()
{
	ui.retranslateUi(this);
}

QVector<QPair<QString, QString>> frmLogSet::GetVecSetName()
{
	QVector<QPair<QString, QString>> vecNames;
	vecNames.push_back(QPair<QString, QString>("Normal", tr("Normal")));
	vecNames.push_back(QPair<QString, QString>("LogSet", tr("LogSet")));
	return vecNames;
}
