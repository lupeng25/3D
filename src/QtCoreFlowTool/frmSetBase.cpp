#include "frmSetBase.h"
#include <QEvent>

QVector<frmSetBase*> frmSetBase::m_vecFrm = QVector<frmSetBase*>();
frmSetBase::frmSetBase(QWidget *parent) :
	QWidget(parent)
{

}

frmSetBase::~frmSetBase()
{

}
void frmSetBase::changeEvent(QEvent * event)
{
	if (NULL != event) {
		switch (event->type()) {
		case QEvent::LanguageChange: {
			RetranslationUi();
		}	break;
		default:
			break;
		}
	}
}
bool frmSetBase::CheckPageAvalible()
{
	return true;
}
int frmSetBase::GetData(QJsonObject & strData)
{
	return 0;
}

int frmSetBase::SetData(QJsonObject & strData)
{
	return 0;
}

int frmSetBase::ResetData()
{
	return 0;
}

int frmSetBase::NewProject()
{
	return 0;
}

int frmSetBase::UpdateUI()
{
	return 0;
}

int frmSetBase::ScanTime()
{
	return 0;
}

void frmSetBase::ChangesetTitleColor()
{

}

void frmSetBase::RetranslationUi()
{

}

QVector<QPair<QString, QString>> frmSetBase::GetVecSetName()
{
	return QVector<QPair<QString, QString>>();
}

int frmSetBase::AddWndSet(frmSetBase * widget)
{
	if(widget == nullptr)	return -1;
	for (auto iter = m_vecFrm.begin(); iter != m_vecFrm.end(); iter++) {
		if (*iter == widget){
			return -1;
		}
	}
	m_vecFrm.append(widget);
	return 0; 
}
