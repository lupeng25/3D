#include "frmBase.h"
frmBase::frmBase(QWidget* parent)
	: BaseWindow(parent)
{
}

frmBase::~frmBase()
{
	this->deleteLater();
}

int frmBase::GetData(QJsonObject & strData)
{
	return 0;
}

int frmBase::SetData(QJsonObject & strData)
{
	return 0;
}

int frmBase::NewProject()
{
	return 0;
}
