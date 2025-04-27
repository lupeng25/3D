#include "LogIn.h"
#include <QMessageBox>
#include "DataBase/QWorld.h"
#include "Data/dataVar.h"
//#include "mainwindow/MyMessageBox.h"

LogIn::LogIn(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_StyledBackground, true);
	ui.btnRegister->setVisible(false);
	initData();
	initUI();
	initConnect();
	dataVar::Instance().strUserName = QString(tr("operator"));
	dataVar::Instance().m_strUserName = QString(tr("operator"));
}

LogIn::~LogIn()
{

}

void LogIn::initData()
{
	ui.stackedWidget->setCurrentIndex(0);
	//SQLite数据库操作	
	QFileInfo file(dataVar::Instance().ConfigFolder + "MyDataBase.db");
	if (!file.exists())
	{
		dataVar::Instance().bol_connect = dataVar::Instance().sqlite.CreateConnectDB(dataVar::Instance().soft_key, 
			dataVar::Instance().ConfigFolder + "MyDataBase.db");
		if (dataVar::Instance().bol_connect == true)
		{
			dataVar::Instance().sqlite.CreateTable(dataVar::Instance().soft_key, "LoginTable");
			dataVar::Instance().sqlite.InsertData(dataVar::Instance().soft_key, "LoginTable", "operator", "666666");
			dataVar::Instance().sqlite.InsertData(dataVar::Instance().soft_key, "LoginTable", "adminstaotor", "666666", "9");
		}
	}
	else
	{
		dataVar::Instance().bol_connect = dataVar::Instance().sqlite.CreateConnectDB(dataVar::Instance().soft_key,
			dataVar::Instance().ConfigFolder + "MyDataBase.db");
	}
	updatenames();
}

void LogIn::initUI()
{
	if (ui.txtUserName->lineEdit())		ui.txtUserName->lineEdit()->setAlignment(Qt::AlignCenter);
	if (ui.txtUserNameR->lineEdit())	ui.txtUserNameR->lineEdit()->setAlignment(Qt::AlignCenter);
	if (ui.txtUserNameM->lineEdit())	ui.txtUserNameM->lineEdit()->setAlignment(Qt::AlignCenter);
}

void LogIn::initConnect()
{
	//connect(ui.btnLogin,	SIGNAL(pressed()), this, SLOT(onLogInClicked()));
	connect(ui.btnLogout,	SIGNAL(pressed()), this, SLOT(onLogOutClicked()));
	connect(ui.btnRegister, SIGNAL(pressed()), this, SLOT(onRegisterClicked()));
	connect(ui.btnModify,	SIGNAL(pressed()), this, SLOT(onModifyClicked()));
}

void LogIn::RetranslationUi()
{
	ui.retranslateUi(this);
}

void LogIn::changeEvent(QEvent * ev)
{
	if (NULL != ev) {
		switch (ev->type()) {
		case QEvent::LanguageChange: {
			RetranslationUi();
		}	break;
		default:
			break;
		}
	}
	QWidget::changeEvent(ev);
}

void LogIn::on_btnLogin_clicked()
{
	if (dataVar::Instance().bol_connect == false)
	{
		dataVar::Instance().bol_connect = dataVar::Instance().sqlite.CreateConnectDB(dataVar::Instance().soft_key,
			dataVar::Instance().ConfigFolder + "MyDataBase.db");
	}
	switch (ui.stackedWidget->currentIndex())
	{
	case 0:{	//登录
		if (dataVar::Instance().bol_connect == true)
		{
			if (ui.txtUserName->currentText() == "" || ui.txtUserPW->text() == "")
			{
				dataVar::Instance().pManager->notify(tr("Warnning"), tr("The username or user password cannot be empty!"), NotifyType_Warn);

				return;
			}
			else
			{
				QString password;
				int _iLevel = 0;
				bool _bquery = dataVar::Instance().sqlite.QueryUserData(dataVar::Instance().soft_key, ui.txtUserName->currentText(), password, _iLevel);
				if (_bquery == true && password == ui.txtUserPW->text() && password != "")
				{
					dataVar::Instance().iLoginLevel = _iLevel;
					if (!ui.cbxRemeber->isChecked())
						ui.txtUserPW->setText("");

					dataVar::Instance().strUserName = ui.txtUserName->currentText();
					dataVar::Instance().startTime = QDateTime::currentDateTime();

					dataVar::Instance().m_strUserName	= ui.txtUserName->currentText();
					dataVar::Instance().m_iLevel		= _iLevel;
					//emit sigUserChange();
					emit sigUserLogIn();
					emit sigLogChanged(dataVar::Instance().strUserName);
					qInfo() << dataVar::Instance().strUserName << tr(" Login Successed");
					dataVar::Instance().pManager->notify(tr("Info"), dataVar::Instance().strUserName + tr(" Login Successed"), NotifyType_Info);

					dataVar::Instance().pUserdialog->hide();
				}
				else
				{
					dataVar::Instance().pManager->notify(tr("Error"), tr("Incorrect username or user password!"), NotifyType_Error);
				}
			}
		}
		else
		{
			dataVar::Instance().pManager->notify(tr("Error"), tr("Database connection failed!"), NotifyType_Error);
		}
	}	break;
	case 1: {	//注册
		if (ui.txtUserPWN->text() == ui.txtUserPWCN->text())
		{
			if (dataVar::Instance().bol_connect == true)
			{
				if (ui.txtUserNameR->currentText() == "" || ui.txtUserPWN->text() == "")
				{
					dataVar::Instance().pManager->notify(tr("Warnning"), tr("The username or user password cannot be empty!"), NotifyType_Warn);
					return;
				}
				QStringList names = getnames();
				if (!names.contains(ui.txtUserNameR->currentText()))
				{
					dataVar::Instance().sqlite.InsertData(dataVar::Instance().soft_key, "LoginTable", 
						ui.txtUserNameR->currentText().toStdString().c_str(),
						ui.txtUserPWN->text().toStdString().c_str() ,"1");
					qInfo() << ui.txtUserNameR->currentText() << tr(" Registration Successed");
					dataVar::Instance().pManager->notify(tr("Info"), ui.txtUserNameR->currentText() + tr(" Registration Successed"), NotifyType_Info);
					updatenames();
				}
				else
				{
					dataVar::Instance().pManager->notify(tr("Error"), tr("The username repeat!"), NotifyType_Error);
					//MyMessageBox::warning(this, tr("The username repeat!"), tr("Warnning"));
					return;
				}
			}
			else
			{
				dataVar::Instance().pManager->notify(tr("Error"), tr("Database connection failed!"), NotifyType_Error);
			}
		}
		else //密码不一样
		{
			dataVar::Instance().pManager->notify(tr("Warnning"), tr("Passward Isn't Correct"), NotifyType_Warn);
			//MyMessageBox::warning(this, tr("Passward Isn't Correct"), tr("Warnning"));
		}
	}	break;
	case 2: {	//修改密码
		if (ui.txtUserPWN->text() == ui.txtUserPWCN->text())
		{
			if (dataVar::Instance().bol_connect == true)
			{
				if (ui.txtUserNameM->currentText() == "" || ui.txtUserPWM->text() == "")
				{
					dataVar::Instance().pManager->notify(tr("Warnning"), tr("The username or user password cannot be empty!"), NotifyType_Warn);
					//MyMessageBox::warning(this, tr("The username or user password cannot be empty!"), tr("Warnning"));
					return;
				}
				QString password;
				int _iLevel = 0;
				bool _bquery = dataVar::Instance().sqlite.QueryUserData(dataVar::Instance().soft_key, ui.txtUserNameM->currentText(), password, _iLevel);
				if (_bquery == true && password == ui.txtUserPWM->text() && password != "")
				{
					dataVar::Instance().sqlite.ModifyUserData(dataVar::Instance().soft_key,
						ui.txtUserNameM->currentText().toStdString().c_str(),
						ui.txtUserPWNM->text().toStdString().c_str(), _iLevel);

					dataVar::Instance().iLoginLevel = _iLevel;

					dataVar::Instance().strUserName = ui.txtUserNameM->currentText();
					dataVar::Instance().startTime = QDateTime::currentDateTime();

					emit sigLogChanged(dataVar::Instance().strUserName);
					qInfo() << dataVar::Instance().strUserName << tr(" Change Password Successed");
					dataVar::Instance().pManager->notify(tr("Info"), dataVar::Instance().strUserName + tr(" Change Password Successed"), NotifyType_Info);
				}
				else
				{
					dataVar::Instance().pManager->notify(tr("Error"), tr("Incorrect username or user password!"), NotifyType_Error);
					//MyMessageBox::critical(this, tr("Incorrect username or user password!"), tr("Error"));
				}
			}
			else
			{
				dataVar::Instance().pManager->notify(tr("Error"), tr("Database connection failed!"), NotifyType_Error);
				//MyMessageBox::critical(this, tr("Database connection failed!"), tr("Error"));
			}
		}
		else //密码不一样
		{
			dataVar::Instance().pManager->notify(tr("Warnning"), tr("Passward Isn't Correct"), NotifyType_Warn);
			//MyMessageBox::warning(this, tr("Passward Isn't Correct"), tr("Warnning"));
			return;
		}
	}	break;
	default: {	}break;
	}
}

void LogIn::onLogOutClicked()
{
	switch (ui.stackedWidget->currentIndex())
	{
	case 0: {	//登录
		slotLogOut();
		dataVar::Instance().m_strUserName = dataVar::Instance().strUserName;
		dataVar::Instance().m_iLevel = 0;
		emit sigUserChange();
	}	break;
	case 1: {	//注册
		ui.stackedWidget->setCurrentIndex(0);
	}	break;
	case 2: {	//修改密码
		ui.stackedWidget->setCurrentIndex(0);
	}	break;
	default: {	}break;
	}

	ui.label->setText(tr("User Login"));
	ui.btnLogin->setText(tr("Login"));
	ui.btnLogin->setToolTip(tr("Login"));
	ui.btnLogout->setText(tr("LogOut"));
	ui.btnLogout->setToolTip(tr("LogOut"));
}

void LogIn::onRegisterClicked()
{
	ui.stackedWidget->setCurrentIndex(1);
	ui.label->setText(tr("User Registrate"));
	ui.btnLogout->setText(tr("Cancer"));
	ui.btnLogout->setToolTip(tr("Cancer"));

	ui.btnLogin->setText(tr("Registrate"));
	ui.btnLogin->setToolTip(tr("Registrate"));
}

void LogIn::onModifyClicked()
{
	ui.stackedWidget->setCurrentIndex(2);
	ui.label->setText(tr("Change Password"));
	ui.btnLogout->setText(tr("Cancer"));
	ui.btnLogout->setToolTip(tr("Cancer"));

	ui.btnLogin->setText(tr("Modify"));
	ui.btnLogin->setToolTip(tr("Modify"));
}

void LogIn::slotLogOut()
{
	if (dataVar::Instance().iLoginLevel >= 1)
	{
		dataVar::Instance().startTime = QDateTime();
		emit sigLogOut();
		dataVar::Instance().iLoginLevel = 0;

		qInfo() << dataVar::Instance().strUserName << tr(" Logged Out");
		dataVar::Instance().pManager->notify(tr("Info"), dataVar::Instance().strUserName + tr(" Logged Out"), NotifyType_Info);
		dataVar::Instance().strUserName = QString("operator");

		emit sigLogChanged(dataVar::Instance().strUserName);
	}
	else
	{
		dataVar::Instance().startTime = QDateTime();
		emit sigLogOut();
		dataVar::Instance().iLoginLevel = 0;
		qInfo() << dataVar::Instance().strUserName << tr(" Logged Out");
		dataVar::Instance().pManager->notify(tr("Info"), dataVar::Instance().strUserName + tr(" Logged Out"), NotifyType_Info);
		dataVar::Instance().strUserName = QString("operator");
	}
}

void LogIn::keyPressEvent(QKeyEvent * ev)
{
	if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
		emit ui.btnLogin->clicked();
		if (dataVar::Instance().iLoginLevel > 0)	{
			dataVar::Instance().pUserdialog->hide();
		}
	}
	else {
		QWidget::keyPressEvent(ev);
	}

}

void LogIn::showEvent(QShowEvent * ev)
{
	QWidget::showEvent(ev);
	if (dataVar::Instance().bUseDefaultPassward 
		&& ui.txtUserPW->text().isEmpty()
		&& m_bFirstShow)	
	{
		ui.txtUserPW->setText(dataVar::Instance().strDefaultPassward);
		m_bFirstShow = false;
	}
	ui.txtUserPW->setFocus();
}

void LogIn::updatenames()
{
	ui.txtUserName->clear();
	ui.txtUserNameR->clear();
	ui.txtUserNameM->clear();

	QFileInfo file(dataVar::Instance().ConfigFolder + "MyDataBase.db");
	if (file.exists())
	{
		dataVar::Instance().bol_connect = dataVar::Instance().sqlite.CreateConnectDB(dataVar::Instance().soft_key,
			dataVar::Instance().ConfigFolder + "MyDataBase.db");
		if (dataVar::Instance().bol_connect == true)
		{
			QSqlQuery query(dataVar::Instance().sqlite.sqldatabase);
			query.exec("SELECT * FROM LoginTable");
			QStringList namelst;
			while (query.next())
			{
				QString user_name = query.value(0).toString();
				QString password = query.value(1).toString();
				int iLevel = query.value(1).toInt();
				namelst.push_back(user_name);
			}

			ui.txtUserName->addItems(namelst);
			ui.txtUserNameR->addItems(namelst);
			ui.txtUserNameM->addItems(namelst);

			setTextAlignment(ui.txtUserName);
			setTextAlignment(ui.txtUserNameR);
			setTextAlignment(ui.txtUserNameM);
		}
	}
}

QStringList LogIn::getnames()
{
	QFileInfo file(dataVar::Instance().ConfigFolder + "MyDataBase.db");
	if (file.exists())
	{
		dataVar::Instance().bol_connect = dataVar::Instance().sqlite.CreateConnectDB(dataVar::Instance().soft_key,
			dataVar::Instance().ConfigFolder + "MyDataBase.db");
		if (dataVar::Instance().bol_connect == true)
		{
			QSqlQuery query(dataVar::Instance().sqlite.sqldatabase);
			query.exec("SELECT * FROM LoginTable");
			QStringList namelst;
			while (query.next())
			{
				QString user_name = query.value(0).toString();
				QString password = query.value(1).toString();
				int iLevel = query.value(1).toInt();
				namelst.push_back(user_name);
			}
			return namelst;
		}
	}
	return QStringList();
}

void LogIn::setTextAlignment(QComboBox* cbx,Qt::Alignment alignment)
{
	auto* model = dynamic_cast<QStandardItemModel*>(cbx->view()->model());
	if (Q_NULLPTR == model)
		return;
	for (int index = 0, size = model->rowCount(); index < size; ++index)	{
		if (Q_NULLPTR != model->item(index))	{
			model->item(index)->setTextAlignment(alignment);
		}
	}
}