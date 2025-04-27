#include "QtKeyWidgets.h"
#include <QProcess>
#include "Data/dataVar.h"

QtKeyWidgets::QtKeyWidgets(QWidget *parent)
	: FBaseDialog(parent)
{
	normalSize = QSize(1000, 600);
	ui.setupUi(this);
	//ui.gridLayout->setParent(nullptr);
	QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
	//ClearLayout(mainLayout);
	//mainLayout->addWidget(getTitleBar());
	//mainLayout->setContentsMargins(1, 1, 1, 1);
	//ui.gridLayout->setParent(this);
	//mainLayout->addWidget(getTitleBar());
	mainLayout->addLayout(ui.gridLayout);
	initUIData(); 
	LanguageChange();
}

QtKeyWidgets::~QtKeyWidgets()
{

}

void QtKeyWidgets::ClearLayout(QLayout * layout)
{
	QLayoutItem *item = nullptr;
	while ((item = layout->takeAt(0)))
	{
		if (item->layout())
		{
			ClearLayout(item->layout());
		}
		if (item->widget())
		{
			delete item->widget();
		}
		delete item;
	}
}

void QtKeyWidgets::initUIData()
{
	QProcess process;
	// 获取硬盘序列号
	process.start("wmic diskdrive get SerialNumber");
	process.waitForFinished();
	QString hddSerial = process.readAllStandardOutput();
	hddSerial = hddSerial.replace("SerialNumber", "").simplified();
	auto Serial = hddSerial.split(" ");
	hddSerial = Serial[0];
	ui.lEditMechineCode->setText(hddSerial);
}

EnumLicenceRetn QtKeyWidgets::CheckSoftDog(double &dRemainderDays)
{
	EnumLicenceRetn eRetn = dataVar::Instance().m_licence.GetRemainderLicenceDays(dRemainderDays);
	QString sRetn = dataVar::Instance().m_licence.GetRegisterKey(-1);
	switch (eRetn)
	{
	case ELicenceRetn_MechineKeyNotRight:
		ui.lbl_InFor->setText(tr("MechineKey Not Correct"));
		ui.lbl_InFor->setToolTip("MechineKey Not Correct");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_AlreadyRegister:
		ui.lbl_InFor->setText(tr("Already Register"));
		ui.lbl_InFor->setToolTip("Already Register");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_RegisterModifyTime:
		ui.lbl_InFor->setText(tr("Register Modify System Time"));
		ui.lbl_InFor->setToolTip("Register Modify System Time");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_InitModifyTimeOut:
		ui.lbl_InFor->setText(tr("Init Modify System Time"));
		ui.lbl_InFor->setToolTip("Init Modify System Time");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_InitDayTimeOut:
		ui.lbl_InFor->setText(tr("Init Time Out"));
		ui.lbl_InFor->setToolTip("Init Time Out");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_InitError:
		ui.lbl_InFor->setText(tr("Init Error"));
		ui.lbl_InFor->setToolTip("Init Error");
		ui.lbl_RemainderInFor->setText("");
		show();
		break;
	case EnumLicenceRetn_NotInit:
		ui.lbl_InFor->setText(tr("Not Inited"));
		ui.lbl_InFor->setToolTip("Not Inited");
		ui.lbl_RemainderInFor->setText("");
		//ui.lbl_InFor->setVisible(true);
		show();
		break;
	case EnumLicenceRetn_Permanent_Inited:
		ui.lbl_InFor->setText(tr("Inited "));
		ui.lbl_InFor->setToolTip("Inited ");
		ui.lbl_RemainderInFor->setText( tr("Permanent Registered") );
		//ui.lbl_InFor->setVisible(true);
		break;
	case EnumLicenceRetn_Inited:
		ui.lbl_InFor->setText(tr("Inited"));
		ui.lbl_InFor->setToolTip("Inited");
		if (dRemainderDays > 0)
		{
			ui.lbl_RemainderInFor->setText(tr("Days Remaining ") + QString::number(dRemainderDays, 'f', 2) + tr(" day"));
		}
		break;
	default:
		break;
	}
	return eRetn;
}

void QtKeyWidgets::LanguageChange()
{
	ui.retranslateUi(this);
	getTitleBar()->setContentLabel(tr("Register"));
}

void QtKeyWidgets::mousePressEvent(QMouseEvent * ev)
{
	if (ev->button() == Qt::LeftButton) {
		isDragging = true;
		dragPos = ev->globalPos() - frameGeometry().topLeft();
		ev->accept();
	}
}

void QtKeyWidgets::mouseReleaseEvent(QMouseEvent * ev)
{
	if (isDragging) {
		isDragging = false;
		ev->accept();
	}
}

void QtKeyWidgets::mouseMoveEvent(QMouseEvent * ev)
{
	if (isDragging) {
		move(ev->globalPos() - dragPos);
		ev->accept();
	}
}

void QtKeyWidgets::closeEvent(QCloseEvent * ev)
{
	ev->ignore();
	this->hide();
}

void QtKeyWidgets::on_btnRegister_clicked()
{
	//获取注册码
	EnumLicenceRetn retn = dataVar::Instance().m_licence.RegisterKey(ui.lEditRegister->text());
	switch (retn)
	{
	case ELicenceRetn_MechineKeyNotRight:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Not Correct!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Not Correct!";
		break;
	case EnumLicenceRetn_AlreadyRegister:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Already Init!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Already Init!";
		break;
	case EnumLicenceRetn_RegisterModifyTime:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Modify System Time!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Modify System Time!";
		break;
	case EnumLicenceRetn_InitModifyTimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Modify System Time!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Modify System Time!";
		break;
	case EnumLicenceRetn_InitDayTimeOut:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey TimeOut!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey TimeOut!";
		break;
	case EnumLicenceRetn_InitError:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Error!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Error!";
		break;
	case EnumLicenceRetn_NotInit:
		dataVar::Instance().pManager->notify(tr("Error"), tr("RegisterKey Not Init!"), NotifyType_Error);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Error: " + "RegisterKey Not Init!";
		break;
	case EnumLicenceRetn_Inited:
		dataVar::Instance().pManager->notify(tr("Info"), tr("RegisterKey Successed"), NotifyType_Info);
		qInfo() << QObject::tr("RegisterKey ") + ui.lEditRegister->text() + " Successed" ;
		break;
	default:
		break;
	}
	double _dRemainderDays;
	CheckSoftDog(_dRemainderDays);
}

