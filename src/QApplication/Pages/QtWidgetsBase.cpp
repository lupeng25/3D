#include "QtWidgetsBase.h"

QtWidgetsBase::QtWidgetsBase(QWidget *parent)
	: QWidget(parent)
{

}

void QtWidgetsBase::changeEvent(QEvent * ev)
{
	switch (ev->type())
	{
	case QEvent::LanguageChange: {
		ChangeLanguage();
	}	break;
	default:
		break;
	}
}

int QtWidgetsBase::GetData(QJsonObject & strData)
{
	return 0;
}

int QtWidgetsBase::SetData(QJsonObject & strData)
{
	return 0;
}

int QtWidgetsBase::CompleteUpDateData()
{
	return 0;
}

int QtWidgetsBase::NewProject()
{
	return 0;
}

int QtWidgetsBase::ChangeProjectComplete()
{
	return 0;
}

int QtWidgetsBase::GetSystemData(QJsonObject & strData)
{
	return 0;
}

int QtWidgetsBase::SetSystemData(QJsonObject & strData)
{
	return 0;
}

int QtWidgetsBase::SCanTimerUI()
{
	return 0;
}

void QtWidgetsBase::ChangeLanguage()
{
}

QVector<QPair<QString, QString>> QtWidgetsBase::GetVecSetName()
{
	return QVector<QPair<QString, QString>>();
}
