/****************************************************************************
**
** 
****************************************************************************/
#include "NetWorkdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>

NetWorkdialog::NetWorkdialog(QWidget *parent) :
    FBaseDialog(parent)
{
    initUI();
	LanguageChange();
	getTitleBar()->setMaxButtonVisible(true);

}

NetWorkdialog::~NetWorkdialog()
{
	if (pfrmInstrumentation != nullptr)	delete pfrmInstrumentation;	pfrmInstrumentation = nullptr;
}

void NetWorkdialog::initUI()
{
    normalSize = QSize(1200, 900);
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	pfrmInstrumentation = new frmInstrumentation(this);

	mainLayout->addSpacing(0);
    mainLayout->addWidget(pfrmInstrumentation);
	mainLayout->setContentsMargins(0, 0, 0, 5);
}

void NetWorkdialog::initConnect()
{
}

int NetWorkdialog::GetData(QJsonObject & strData)
{
	pfrmInstrumentation->GetData(strData);
	return 0;
}

int NetWorkdialog::SetData(QJsonObject & strData)
{
	pfrmInstrumentation->SetData(strData);
	return 0;
}

void NetWorkdialog::closeEvent(QCloseEvent * event)
{
	event->ignore();
	hide();
}

void NetWorkdialog::LanguageChange()
{
	getTitleBar()->getTitleLabel()->setText(tr("NetWork"));
	getTitleBar()->getContentLabel()->setText(tr(""));
}
