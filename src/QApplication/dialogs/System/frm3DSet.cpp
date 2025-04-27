#include "frm3DSet.h"
//#include "ui_frm3DSet.h"
#include <QDateTime>
//#include "Data/dataVar.h"
#include "databaseVar.h"

frm3DSet::frm3DSet(QWidget* parent) :
	frmSetBase(parent)
{
	ui.setupUi(this);
}

frm3DSet::~frm3DSet()
{
}

int frm3DSet::GetData(QJsonObject & strData)
{
	QJsonObject CommonParam;
	CommonParam.insert("spinInvalidateValue", QString::number(ui.spinInvalidateValue->value()));
	strData.insert("frm3DSet", CommonParam);
	return 0;
}

int frm3DSet::SetData(QJsonObject & strData)
{
	QJsonObject CommonParam = strData.find("frm3DSet").value().toObject();
	if (CommonParam.contains("spinInvalidateValue")) { ui.spinInvalidateValue->setValue(CommonParam["spinInvalidateValue"].toString().toInt()); }

	return 0;
}

int frm3DSet::NewProject()
{
	return 0;
}

int frm3DSet::UpdateUI()
{
	databaseVar::Instance().m_mapParam["spinInvalidateValue"]	= QString::number(ui.spinInvalidateValue->value());
	return 0;
}

void frm3DSet::RetranslationUi()
{
	ui.retranslateUi(this);
}

QVector<QPair<QString, QString>> frm3DSet::GetVecSetName()
{
	QVector<QPair<QString, QString>> vecNames;
	vecNames.push_back(QPair<QString, QString>("3DParam", QObject::tr("3DParam")));
	vecNames.push_back(QPair<QString, QString>("frm3DSet", QObject::tr("frm3DSet")));
	return vecNames;
}
