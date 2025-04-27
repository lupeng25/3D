/****************************************************************************
**
** 
****************************************************************************/
#include "userdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>
userdialog::userdialog(QWidget *parent) :
    FBaseDialog(parent)
{
    initUI();
	LanguageChange();
}

userdialog::~userdialog()
{
	if (pLogInPage != nullptr)	delete pLogInPage;	pLogInPage = nullptr;
}

void userdialog::initUI()
{
    normalSize = QSize(600, 450);
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	pLogInPage = new LogIn(this);

	mainLayout->addSpacing(0);
    mainLayout->addWidget(pLogInPage);
	mainLayout->setContentsMargins(0, 0, 0, 20);
	connect(pLogInPage, SIGNAL(sigLogOut()), this, SIGNAL(sigLogOut()));
	connect(pLogInPage, SIGNAL(sigLogChanged(QString)), this, SIGNAL(sigLogChanged(QString)));

}

void userdialog::initConnect()
{
}

void userdialog::closeEvent(QCloseEvent * event)
{
	event->ignore();
	hide();
}

void userdialog::LanguageChange()
{
	getTitleBar()->getTitleLabel()->setText(tr("User"));
	getTitleBar()->getContentLabel()->setText(tr(""));
}

void userdialog::slotLogOut()
{
	pLogInPage->slotLogOut();
}