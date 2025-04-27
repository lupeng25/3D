#include "mainwindow.h"
#if !SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
#include "SAFramelessHelper.h"
#endif
#include "SARibbonApplicationButton.h"
#include "SARibbonBar.h"
#include "SARibbonButtonGroupWidget.h"
#include "SARibbonCategory.h"
#include "SARibbonCheckBox.h"
#include "SARibbonColorToolButton.h"
#include "SARibbonComboBox.h"
#include "SARibbonCustomizeDialog.h"
#include "SARibbonCustomizeWidget.h"
#include "SARibbonElementManager.h"
#include "SARibbonGallery.h"
#include "SARibbonLineEdit.h"
#include "SARibbonMenu.h"
#include "SARibbonPannel.h"
#include "SARibbonQuickAccessBar.h"
#include "SARibbonToolButton.h"
#include "colorWidgets/SAColorGridWidget.h"
#include "colorWidgets/SAColorPaletteGridWidget.h"
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFontComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QWidgetAction>
#include "QFramer/ftoolbutton.h"
#include "Common/settingmenu.h"
#include "Common/CLinguist.h"
#include "Common/MyMessageBox.h"
#include "Data/dataVar.h"
#include "databaseVar.h"

MainWindow::MainWindow(QWidget* par)
    : SARibbonMainWindow(par)
	, m_pMenuApplicationBtn(nullptr)
	, m_bFirstCheck(true)
{
	m_startTime = QDateTime::currentDateTime();
    SAFramelessHelper* helper = framelessHelper();
    helper->setRubberBandOnResize(false);
}

MainWindow* MainWindow::getInstance()
{
	// TODO: 在此处插入 return 语句
	static MainWindow* instance;
	if (instance == nullptr) {
		instance = new MainWindow();
	}
	return instance;
}

void MainWindow::Init()
{
	dataVar::Instance().m_pWindow = this;
	databaseVar::Instance().m_pWindow = this;
	
	dataVar::Instance().pQtWaitWidgetsClass = new QtWaitWidgetsClass(this);
	dataVar::Instance().pUserdialog			= new userdialog(this);
	dataVar::Instance().pSettingDialog		= new SettingDialog(this);
	dataVar::Instance().pManager			= new NotifyManager(this);
	dataVar::Instance().pLogPage			= new LogPage(this);
	dataVar::Instance().pQtKeyWidgets		= new QtKeyWidgets(this);
	databaseVar::Instance().pManager		= dataVar::Instance().pManager;
	
	dataVar::Instance().pQtWaitWidgetsClass->hide();
	dataVar::Instance().pUserdialog->hide();
	dataVar::Instance().pLogPage->hide();
	dataVar::Instance().pSettingDialog->hide();
	dataVar::Instance().pQtKeyWidgets->hide();

	m_pNetWorkdialog = new NetWorkdialog(this);
	m_pNetWorkdialog->hide();
	m_pfrmGlobalVariable = new frmGlobalVariable(this);
	m_pfrmGlobalVariable->hide();
	dataVar::Instance().pfrmImageLayout = new frmImageLayout(this);
	dataVar::Instance().pfrmImageLayout->hide();

	setCentralWidget((m_pQtCenterWidgets = new QtCenterWidgets()));
	m_pQtCenterWidgets->initUI();
	setStatusBar(new QStatusBar());
	statusBar()->setFixedHeight(40);

	SARibbonBar* ribbon = ribbonBar();
	ribbon->setMinimumHeight(50);
	ribbon->setContentsMargins(1, 0, 5, 0);
	ribbonBar()->showMinimumModeButton(true);  // 显示ribbon最小化按钮
	ribbonBar()->setEnableWordWrap(true);
	ribbonBar()->setRibbonStyle(SARibbonBar::RibbonStyle::RibbonStyleCompactTwoRow);
	setRibbonTheme(SARibbonMainWindow::RibbonTheme::RibbonThemeOffice2013);
	createRibbonApplicationButton();

	SARibbonCategory* categoryMain = ribbon->addCategoryPage(tr("Main"));
	categoryMain->setObjectName(("Main"));
	createCategoryMain(categoryMain);
	m_pVecCategory.push_back(categoryMain);

	SARibbonCategory* categoryView = ribbon->addCategoryPage(tr("View"));
	categoryView->setObjectName(("View"));
	createCategoryView(categoryView);
	m_pVecCategory.push_back(categoryView);

	SARibbonCategory* categoryMotion = ribbon->addCategoryPage(tr("Motion"));
	categoryMotion->setObjectName(("Motion"));
	createCategoryMotion(categoryMotion);
	m_pVecCategory.push_back(categoryMotion);

	SARibbonCategory* categoryVision = ribbon->addCategoryPage(tr("Vision"));
	categoryVision->setObjectName(("Vision"));
	createCategoryVision(categoryVision);
	m_pVecCategory.push_back(categoryVision);

	SARibbonCategory* categoryOther = ribbon->addCategoryPage(tr("Other"));
	categoryOther->setObjectName(("Other"));
	createCategoryOther(categoryOther);
	m_pVecCategory.push_back(categoryOther);

	SARibbonCategory* categoryAbout = ribbon->addCategoryPage(tr("About"));
	categoryAbout->setObjectName(("About"));
	createCategoryAbout(categoryAbout);
	m_pVecCategory.push_back(categoryAbout);

	SARibbonQuickAccessBar* quickAccessBar = ribbon->quickAccessBar();
	createQuickAccessBar(quickAccessBar);
	createRightButtonGroup();
	createActionsManager();
	setMinimumWidth(1200);
	setMinimumHeight(900);

	m_pLblUser = new QToolButton(this);
	m_pLblUser->setObjectName("User");
	m_pLblUser->setMinimumWidth(150);
	m_pLblUser->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_pLblUser->setIcon(QIcon(":/skin/icon/icon/icons/light/appbar.user.png"));
	m_pLblUser->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	m_pLblUser->setMinimumHeight(statusBar()->height());
	statusBar()->addPermanentWidget(m_pLblUser);
	connect(m_pLblUser, &FToolButton::clicked, this, [=]() {
		MainWindow::getInstance()->show();
		dataVar::Instance().CenterWindow(this, dataVar::Instance().pUserdialog);
		dataVar::Instance().pUserdialog->show();
		dataVar::Instance().pUserdialog->raise();
	});

	//Time
	m_pLblTime = new QLabel(this);
	m_pLblTime->setObjectName("Time");
	m_pLblTime->setMinimumWidth(200);
	m_pLblTime->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	m_pLblTime->setAlignment(Qt::AlignCenter);
	m_pLblTime->setMinimumHeight(statusBar()->height());
	statusBar()->addPermanentWidget(m_pLblTime);

	//Error
	m_pErrorMsgButton = new FToolButton(this);
	m_pErrorMsgButton->setFocusPolicy(Qt::NoFocus);
	m_pErrorMsgButton->setMinimumWidth(50);
	m_pErrorMsgButton->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	m_pErrorMsgButton->setObjectName("ErrorMsg");
	m_pErrorMsgButton->setToolTip(tr("ErrorMsg"));
	m_pErrorMsgButton->setIconSize(QSize(40, 40));
	m_pErrorMsgButton->setIcon(QIcon(":/skin/icon/icon/icons/light/Message.png"));
	connect(m_pErrorMsgButton, &FToolButton::clicked, this, [=]() {
		MainWindow::getInstance()->show();
		dataVar::Instance().CenterWindow(this, dataVar::Instance().pLogPage);
		dataVar::Instance().pLogPage->show();
		dataVar::Instance().pLogPage->raise();
	});
	statusBar()->addWidget(m_pErrorMsgButton);

	m_pCurrentProjectName = new QLabel(this);
	m_pCurrentProjectName->setObjectName("view_project");
	m_pCurrentProjectName->setProperty("type", "xblack");
	statusBar()->addWidget(m_pCurrentProjectName);


	ribbonBar()->updateRibbonGeometry();
	setWindowTitle(dataVar::Instance().software_name);
	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));//每隔1秒刷新
	m_QDateTime = QDateTime::currentDateTime();
}

int MainWindow::initData(QString & strError)
{
	return 0;
}

int MainWindow::initUIData(QString & strError)
{
	dataVar::Instance().pSettingDialog->LoadData();
	dataVar::Instance().pSettingDialog->ResetData();
	if (dataVar::Instance().pLogPage)	dataVar::Instance().pLogPage->Init();

	//语言
	if(m_pLanguageLst != nullptr)
	for (auto& pItem : m_pLanguageLst->actions()) {
		int _iLanguage = pItem->property("Language").toInt();
		if (CLinguist::getInstance().m_CurrentLanguage == _iLanguage) { pItem->setChecked(true); }
		else { pItem->setChecked(false); }
	}

	m_pQtCenterWidgets->initUIData(strError);

	OpenProject(dataVar::Instance().projectName);
	ChangeProjectStatus();

	m_pTimer->start(500);
	return 0;
}

void MainWindow::Exit()
{
	dataVar::Instance().pManager->ClearAll();
}

void MainWindow::animationClose()
{
	QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
	connect(animation, SIGNAL(finished()), this, SLOT(close()));
	animation->setDuration(1000);
	animation->setStartValue(1);
	animation->setEndValue(0);
	animation->start();
}

void MainWindow::createRibbonApplicationButton()
{
    SARibbonBar* ribbon = ribbonBar();
    if (!ribbon) {
        return;
    }
    QAbstractButton* btn = ribbon->applicationButton();
    if (!btn) {
        btn = new SARibbonApplicationButton(this);
        ribbon->setApplicationButton(btn);
    }
	ribbonBar()->applicationButton()->setText(tr(" File "));  // 文字两边留有间距，好看一点
	ribbonBar()->applicationButton()->setIconSize(QSize(30,30));
	ribbonBar()->applicationButton()->setIcon(QIcon(":/image/icon/images/File.png"));
    if (!m_pMenuApplicationBtn) {
        m_pMenuApplicationBtn = new SARibbonMenu(this);

		QToolButton* NewAction_buf = new QToolButton(m_pMenuApplicationBtn);
		NewAction_buf->setObjectName("New");
		NewAction_buf->setText(tr("New"));
		NewAction_buf->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
		NewAction_buf->setIconSize(QSize(40, 40));
		NewAction_buf->setMinimumSize(QSize(120, 40));
		m_vecAllQPushButton.push_back(NewAction_buf);
		NewAction_buf->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.page.new.png"));
		QWidgetAction* NewAction = new QWidgetAction(this);
		NewAction->setDefaultWidget(NewAction_buf);
		connect(NewAction_buf, &QToolButton::clicked, this, &MainWindow::onActionNewTriggered);
		connect(NewAction_buf, &QPushButton::clicked, m_pMenuApplicationBtn, &QMenu::close);
		m_pMenuApplicationBtn->addAction(NewAction);

		//QAction *_pAction = createAction(tr("New"), ":/skin/icon/icon/icons/light/appbar.page.new.png", "New");
		//connect(_pAction, &QAction::triggered, this, &MainWindow::onActionNewTriggered);
  //      m_pMenuApplicationBtn->addAction(_pAction);
		QToolButton* OpenAction_buf = new QToolButton(m_pMenuApplicationBtn);
		OpenAction_buf->setObjectName("Open");
		OpenAction_buf->setText(tr("Open"));
		OpenAction_buf->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
		OpenAction_buf->setIconSize(QSize(40, 40));
		OpenAction_buf->setMinimumSize(QSize(120, 40));
		m_vecAllQPushButton.push_back(OpenAction_buf);
		OpenAction_buf->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.folder.open.png"));
		QWidgetAction* OpenAction = new QWidgetAction(this);
		OpenAction->setDefaultWidget(OpenAction_buf);
		connect(OpenAction_buf, &QToolButton::clicked, this, &MainWindow::onActionOpenTriggered);
		connect(OpenAction_buf, &QPushButton::clicked, m_pMenuApplicationBtn, &QMenu::close);
		m_pMenuApplicationBtn->addAction(OpenAction);

		//_pAction = createAction(tr("Open"), ":/skin/icon/icon/icons/light/appbar.folder.open.png", "Open");
		//connect(_pAction, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
		//m_pMenuApplicationBtn->addAction(_pAction);

		m_pMenuApplicationBtn->addSeparator();

		QToolButton* SaveAction_buf = new QToolButton(m_pMenuApplicationBtn);
		SaveAction_buf->setObjectName("Save");
		SaveAction_buf->setText(tr("Save"));
		SaveAction_buf->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
		SaveAction_buf->setIconSize(QSize(40, 40));
		SaveAction_buf->setMinimumSize(QSize(120, 40));
		m_vecAllQPushButton.push_back(SaveAction_buf);
		SaveAction_buf->setIcon(QIcon(":/skin/icon/icon/icons/dark/appbar.save.png"));
		QWidgetAction* SaveAction = new QWidgetAction(this);
		SaveAction->setDefaultWidget(SaveAction_buf);
		connect(SaveAction_buf, &QToolButton::clicked, this, &MainWindow::onActionSaveTriggered);
		connect(SaveAction_buf, &QPushButton::clicked, m_pMenuApplicationBtn, &QMenu::close);
		m_pMenuApplicationBtn->addAction(SaveAction);

		//_pAction = createAction(tr("Save"), ":/skin/icon/icon/icons/light/appbar.save.png", "Save");
		//connect(_pAction, &QAction::triggered, this, &MainWindow::onActionSaveTriggered);
		//m_pMenuApplicationBtn->addAction(_pAction);

		QToolButton* SaveAsAction_buf = new QToolButton(m_pMenuApplicationBtn);
		SaveAsAction_buf->setObjectName("SaveAs");
		SaveAsAction_buf->setText(tr("SaveAs"));
		SaveAsAction_buf->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
		SaveAsAction_buf->setIconSize(QSize(40, 40));
		SaveAsAction_buf->setMinimumSize(QSize(120, 40));
		m_vecAllQPushButton.push_back(SaveAsAction_buf);
		SaveAsAction_buf->setIcon(QIcon(":/skin/icon/icon/icons/dark/SaveAs.png"));
		QWidgetAction* SaveAsAction = new QWidgetAction(this);
		SaveAsAction->setDefaultWidget(SaveAsAction_buf);
		connect(SaveAsAction_buf, &QToolButton::clicked, this, &MainWindow::onActionSaveAsTriggered);
		connect(SaveAsAction_buf, &QPushButton::clicked, m_pMenuApplicationBtn, &QMenu::close);
		m_pMenuApplicationBtn->addAction(SaveAsAction);

		//_pAction = createAction(tr("SaveAs"), ":/skin/icon/icon/icons/light/SaveAs.png", "SaveAs");
		//connect(_pAction, &QAction::triggered, this, &MainWindow::onActionSaveAsTriggered);
		//m_pMenuApplicationBtn->addAction(_pAction);
    }
    SARibbonApplicationButton* appBtn = qobject_cast< SARibbonApplicationButton* >(btn);
    if (!appBtn) {
        return;
    }
    appBtn->setMenu(m_pMenuApplicationBtn);
}

void MainWindow::onActionHelpTriggered()
{
	AboutDialog* aboutDialog = new AboutDialog(dataVar::Instance().m_pWindow);
	aboutDialog->exec();
}

void MainWindow::onFontComWidgetCurrentFontChanged(const QFont& f)
{
    ribbonBar()->setFont(f);
    update();
}

void MainWindow::onCheckBoxAlignmentCenterClicked(bool checked)
{
    if (checked) {
        ribbonBar()->setRibbonAlignment(SARibbonAlignment::AlignCenter);
    } else {
        ribbonBar()->setRibbonAlignment(SARibbonAlignment::AlignLeft);
    }
}

void MainWindow::slotTimeOut()
{
	if (dataVar::Instance().auto_log_save_days > 0) {
		QDateTime stopTime = QDateTime::currentDateTime();
		double elapsed = (double)m_startTime.secsTo(stopTime);
		if ((int)elapsed >= 5 * 60) {
			m_startTime = QDateTime::currentDateTime();
			FolderOperation::FindFileForDeleteLog(dataVar::Instance().soft_key,
				dataVar::Instance().applicationPath + "/Log/", dataVar::Instance().auto_log_save_days);
		}
	}
	//自动登出
	if (dataVar::Instance().iLoginLevel > 0) {
		QDateTime stopTime = QDateTime::currentDateTime();
		qint64 elapsed = dataVar::Instance().startTime.secsTo(stopTime);
		if (elapsed >= dataVar::Instance().auto_login_out_Time) {
			dataVar::Instance().pUserdialog->slotLogOut();
			dataVar::Instance().startTime = QDateTime();
			slotLogOut();
			dataVar::Instance().iLoginLevel = 0;
		}
		if (m_pLblUser != nullptr) m_pLblUser->setText(dataVar::Instance().strUserName + QString(" %1%2")
			.arg(QString::number(dataVar::Instance().auto_login_out_Time - elapsed)).arg(tr("s")));
	}
	else if (m_pLblUser != nullptr) m_pLblUser->setText(dataVar::Instance().strUserName);
	if (dataVar::Instance().pQtKeyWidgets != nullptr) {
		if (m_SoftDogTime.secsTo(QDateTime::currentDateTime()) > 10 || m_bFirstCheck) {
			m_SoftDogTime = QDateTime::currentDateTime();
			double _dRemainderDays;
			EnumLicenceRetn _eRetn = dataVar::Instance().pQtKeyWidgets->CheckSoftDog(_dRemainderDays);
			if (!(_eRetn == EnumLicenceRetn_Inited || _eRetn == EnumLicenceRetn_Permanent_Inited)) {	//	未注册
				dataVar::Instance().pQtKeyWidgets->show();
				if (dataVar::Instance().iLoginLevel > 0) {
					dataVar::Instance().pUserdialog->slotLogOut();
				}
			}
			else if (_eRetn == EnumLicenceRetn_Inited) {
				if (dataVar::Instance().bEnableRegistrationAlart) {
					if ((_dRemainderDays > 0 && _dRemainderDays < dataVar::Instance().iRegistrationDays)) {
						dataVar::Instance().pQtKeyWidgets->show();
					}
				}
			}
		}
	}
	if (m_pLblTime != nullptr) m_pLblTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	if (dataVar::Instance().pSettingDialog) dataVar::Instance().pSettingDialog->ScanTime();
	if (m_bClose) {
		if (m_CloseTime.secsTo(QDateTime::currentDateTime()) > 10) {
			QCoreApplication::quit();
			exit(0);
		}
		return;
	}

}

void MainWindow::slotLogOut()
{
	dataVar::Instance().pSettingDialog->hide();
}

void MainWindow::slotChangeLanguage(int iLanguage)
{
	CLinguist::getInstance().ChangeLanguage((Language)iLanguage);

	for (auto& pItem : m_pLanguageLst->actions()) {
		int _iLanguage = pItem->property("Language").toInt();
		if (iLanguage == _iLanguage) { pItem->setChecked(true); }
		else { pItem->setChecked(false); }
	}

	QSettings settings(QApplication::applicationDirPath() + "/Config/config.ini", QSettings::IniFormat);
	settings.beginGroup("General");
	settings.setValue("language", iLanguage);
	settings.endGroup();
}

void MainWindow::onActionNetCommunicateTriggered()
{
	m_pNetWorkdialog->show();
	m_pNetWorkdialog->raise();
}

void MainWindow::onActionGlobalValTriggered()
{
	m_pfrmGlobalVariable->onButtonRestoreClicked();
	m_pfrmGlobalVariable->show();
	m_pfrmGlobalVariable->raise();
}

void MainWindow::onActionImageLayoutTriggered()
{
	dataVar::Instance().pfrmImageLayout->onButtonRestoreClicked();
	dataVar::Instance().pfrmImageLayout->show();
	dataVar::Instance().pfrmImageLayout->raise();

}

void MainWindow::onActionStartTriggered()
{

}

void MainWindow::onActionStopTriggered()
{

}

void MainWindow::onActionNewTriggered()
{
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("新建项目"), dataVar::Instance().projectName, "*.cfg");
	if (!file_name.isNull())	{
		dataVar::Instance().projectName = file_name;
		QApplication::processEvents();
		ChangeProjectStatus();
		QFile file(file_name);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		file.write("");
		file.close();
	}
}

void MainWindow::onActionOpenTriggered()
{
	QString file_name = QFileDialog::getOpenFileName(this,
		tr("打开项目"), dataVar::Instance().projectName, "*.cfg");
	if (!file_name.isNull())	{
		QApplication::processEvents();
		OpenProject(file_name);
		dataVar::Instance().projectName = file_name;
		ChangeProjectStatus();
		dataVar::Instance().pManager->notify(tr("项目打开成功"), NotifyType_Warn);
	}
}

void MainWindow::onActionSaveTriggered()
{
	if (dataVar::Instance().projectName == QString())	{
		dataVar::Instance().pManager->notify(tr("请先新建项目"), NotifyType_Warn);
		return;
	}
	SaveProject(dataVar::Instance().projectName);
	dataVar::Instance().pManager->notify(tr("保存项目成功"), NotifyType_Warn);
}

void MainWindow::onActionSaveAsTriggered()
{
	if (dataVar::Instance().projectName == QString())	{
		dataVar::Instance().pManager->notify(tr("项目另存为时，请先新建项目"), NotifyType_Warn);
		return;
	}
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("项目另存为"), dataVar::Instance().projectName, tr("Config Files (*.cfg)"));
	if (!fileName.isNull())	{
		SaveProject(fileName);
		dataVar::Instance().pManager->notify(tr("保存项目成功"), NotifyType_Warn);
	}
}

void MainWindow::onActionResetTriggered()
{

}

void MainWindow::closeEvent(QCloseEvent* e)
{
	auto res = MyMessageBox::question(this, tr("Current System Is Running. Are You Sure Want To Exit This Program?"), tr("Alart"));
	if (res == MyMessageBox::Yes) {
		dataVar::Instance().pManager->ClearAll();
		m_bClose = true;
		m_CloseTime = QDateTime::currentDateTime();
		QtWaitWidgetsClass::WaitForMsgShowFunction([]() {
			Sleep(500);



			return  StandardMsgResult::MsgResult_OK;
		}, tr("System Is Closing"));
		e->accept();
	}
	else {	e->ignore();	}
}

void MainWindow::changeEvent(QEvent * ev)
{
	SARibbonMainWindow::changeEvent(ev);
	if (NULL != ev) {
		switch (ev->type()) {
		case QEvent::LanguageChange: {
			LanguageChange();
		}	break;
		default:	break;
		}
	}
}

void MainWindow::LanguageChange()
{
	if(m_pSettingLst != nullptr)	m_pSettingLst->ChangeLanguage();
	for (auto iter : m_pVecCategory)	{
		if (iter != nullptr)	iter->setCategoryName(tr(iter->objectName().toStdString().c_str()));
	}
	if (m_pErrorMsgButton != nullptr) m_pErrorMsgButton->setText(tr(m_pErrorMsgButton->objectName().toStdString().c_str()));
	ribbonBar()->applicationButton()->setText(tr(" File "));  // 文字两边留有间距，好看一点
	for (auto iter : m_pMenuApplicationBtn->actions())
		if(iter!= nullptr) iter->setText(tr(iter->objectName().toStdString().c_str()));
	setWindowTitle(dataVar::Instance().software_name);
	if (m_themeMenu) m_themeMenu->setTitle(tr(m_themeMenu->objectName().toStdString().c_str()));
	if (m_themeMenu) m_themeMenu->setToolTip(tr(m_themeMenu->objectName().toStdString().c_str()));
	for (auto iter : m_vecAllAction) {
		if (iter != nullptr)	iter->setText(tr(iter->objectName().toStdString().c_str()));
		if (iter != nullptr)	iter->setToolTip(tr(iter->objectName().toStdString().c_str()));
	}
	for (auto iter : m_vecAllQPushButton) {
		if (iter != nullptr)	iter->setText(tr(iter->objectName().toStdString().c_str()));
		if (iter != nullptr)	iter->setToolTip(tr(iter->objectName().toStdString().c_str()));
	}
	if (m_pCurrentProjectName != nullptr) {
		m_pCurrentProjectName->setText(" " + tr("Current Project") + ": " + dataVar::Instance().projectName + " ");
		m_pCurrentProjectName->update();
	}
}

void MainWindow::createCategoryMain(SARibbonCategory* page)
{
	page->setMinimumWidth(600);
	page->setGeometry(0,0,600,height());
	SARibbonPannel* BtnPannel = page->addPannel(("Mechine Pannel"));
	BtnPannel->setMinimumWidth(600);

	QAction* actionResetAppBtn = createAction(tr("Reset"), ":/skin/icon/icon/icons/light/Reset.png", "Reset");
	connect(actionResetAppBtn, &QAction::triggered, this, &MainWindow::onActionResetTriggered);
	BtnPannel->addLargeAction(actionResetAppBtn);

	QAction* actionStartAppBtn = createAction(tr("Start"), ":/image/icon/images/Start.png","Start");
	connect(actionStartAppBtn, &QAction::triggered, this, &MainWindow::onActionStartTriggered);
	BtnPannel->addLargeAction(actionStartAppBtn);

	QAction* actionStopAppBtn = createAction(tr("Stop"), ":/image/icon/images/Stop.png", "Stop");
	connect(actionStopAppBtn, &QAction::triggered, this, &MainWindow::onActionStopTriggered);
	BtnPannel->addLargeAction(actionStopAppBtn);

}

void MainWindow::createCategoryView(SARibbonCategory* page)
{


}

void MainWindow::createCategoryMotion(SARibbonCategory * page)
{

}

void MainWindow::createCategoryVision(SARibbonCategory * page)
{
	page->setMinimumWidth(600);
	page->setGeometry(0, 0, 600, height());
	SARibbonPannel* BtnPannel = page->addPannel(("Communication Pannel"));
	BtnPannel->setMinimumWidth(600);

	QAction* actionNetAppBtn = createAction(tr("Net"), ":/skin/icon/icon/icons/light/appbar.network.port.png", "Net");
	connect(actionNetAppBtn, &QAction::triggered, this, &MainWindow::onActionNetCommunicateTriggered);
	BtnPannel->addLargeAction(actionNetAppBtn);

	QAction* actionGlobalValueAppBtn = createAction(tr("GlobalVal"), ":/skin/icon/icon/icons/light/GlobalVar.png", "GlobalVal");
	connect(actionGlobalValueAppBtn, &QAction::triggered, this, &MainWindow::onActionGlobalValTriggered);
	BtnPannel->addLargeAction(actionGlobalValueAppBtn);

	QAction* actionImageLayoutAppBtn = createAction(tr("ImageLayout"), ":/skin/icon/icon/icons/light/appbar.layout.sidebar.png", "ImageLayout");
	connect(actionImageLayoutAppBtn, &QAction::triggered, this, &MainWindow::onActionImageLayoutTriggered);
	BtnPannel->addLargeAction(actionImageLayoutAppBtn);
}

void MainWindow::createCategoryOther(SARibbonCategory* page)
{

}

void MainWindow::createCategoryAbout(SARibbonCategory * page)
{
	page->setMinimumWidth(600);
	page->setGeometry(0, 0, 600, height());
	SARibbonPannel* BtnPannel = page->addPannel(("About Pannel"));
	BtnPannel->setMinimumWidth(600);

	QAction* actionResetAppBtn = createAction(tr("About"), ":/skin/icon/icon/icons/light/appbar.information.png", "About");
	connect(actionResetAppBtn, &QAction::triggered, this, [=]() {
		MainWindow::getInstance()->show();
		AboutDialog* aboutDialog = new AboutDialog(dataVar::Instance().m_pWindow);
		aboutDialog->exec();
	});
	BtnPannel->addLargeAction(actionResetAppBtn);

}

void MainWindow::createQuickAccessBar(SARibbonQuickAccessBar* quickAccessBar)
{
	//QAction* actionSave = createAction("save", ":/icon/icon/save.svg", "save");
	//actionSave->setShortcut(QKeySequence("Ctrl+S"));
	//actionSave->setShortcutContext(Qt::ApplicationShortcut);
	//quickAccessBar->addAction(actionSave);
}

void MainWindow::createRightButtonGroup()
{
    SARibbonBar* ribbon = ribbonBar();
    if (!ribbon) {
        return;
    }
    SARibbonButtonGroupWidget* rightBar = ribbon->rightButtonGroup();
	SARibbonApplicationButton* actionLanguage	= createActionButton(tr("Language"),	":/skin/icon/icon/icons/light/Language.png", "Language");
	SARibbonApplicationButton *actionSet		= createActionButton(tr("set"),			":/skin/icon/icon/icons/light/Setting.png", "set");
	SARibbonApplicationButton *actionSkin		= createActionButton(tr("skin"),		":/skin/icon/icon/icons/light/skin.png", "skin");
	m_pSettingLst			= new SettingMenu(this);
	m_themeMenu				= new ThemeMenu(this);
	m_pLanguageLst			= new QMenu(this);
	m_themeMenu->Load();
	{
		QAction* _pAction = new QAction();
		//_pAction->setIcon(QIcon(":/images/skin/images/Chinese.png"));
		_pAction->setText(tr("Chinese"));
		_pAction->setToolTip(tr("Chinese"));
		_pAction->setObjectName(("Chinese"));
		_pAction->setProperty("Language", QString::number(Language_CN));
		_pAction->setCheckable(true);
		connect(_pAction, &QAction::triggered, this, [=]() { slotChangeLanguage(_pAction->property("Language").toInt());	});
		m_pLanguageLst->addAction(_pAction);
	}
	{
		QAction* _pAction = new QAction();
		//_pAction->setIcon(QIcon(":/images/skin/images/Chinese.png"));
		_pAction->setText(tr("English"));
		_pAction->setToolTip(tr("English"));
		_pAction->setObjectName(("English"));
		_pAction->setProperty("Language", QString::number(Language_EN));
		_pAction->setCheckable(true);
		connect(_pAction, &QAction::triggered, this, [=]() { slotChangeLanguage(_pAction->property("Language").toInt());	});
		m_pLanguageLst->addAction(_pAction);
	}

	actionLanguage->setMenu(m_pLanguageLst);
	actionSet->setMenu(m_pSettingLst);
	actionSkin->setMenu(m_themeMenu);

	rightBar->addWidget(actionLanguage);
	rightBar->addWidget(actionSet);
	rightBar->addWidget(actionSkin);
}

void MainWindow::createActionsManager()
{
    // 添加其他的action，这些action并不在ribbon管理范围，主要用于SARibbonCustomizeWidget自定义用
	m_pActionsManager = new SARibbonActionsManager(ribbonBar());  // 申明过程已经自动注册所有action
    // 以下注册特别的action
	m_pActionsManager->setTagName(SARibbonActionsManager::CommonlyUsedActionTag, tr("in common use"));  //
}

QAction* MainWindow::createAction(const QString& text, const QString& iconurl, const QString& objName)
{
    QAction* act = new QAction(this);
    act->setText(text);
    act->setIcon(QIcon(iconurl));
    act->setObjectName(objName);
	m_vecAllAction.push_back(act);
    return act;
}

QAction* MainWindow::createAction(const QString& text, const QString& iconurl)
{
    QAction* act = new QAction(this);
    act->setText(text);
    act->setIcon(QIcon(iconurl));
    act->setObjectName(text);
	m_vecAllAction.push_back(act);
    return act;
}

SARibbonApplicationButton * MainWindow::createActionButton(const QString & text, const QString & iconurl, const QString & objName)
{
	SARibbonApplicationButton* act = new SARibbonApplicationButton(this);
	act->setToolTip(text);
	act->setIcon(QIcon(iconurl));
	act->setObjectName(text);
	act->setStyleSheet("background-color:transparent;");
	act->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
	return act;
}

void MainWindow::ChangeProjectStatus()
{
	qInfo() << tr("切换 ") << dataVar::Instance().projectName;
	m_pCurrentProjectName->setText(" " + tr("Current Project") + ": " + dataVar::Instance().projectName + " ");
	m_pCurrentProjectName->update();
}

int MainWindow::SaveProject(QString strFilePath)
{
	QJsonObject root;
	if (m_pQtCenterWidgets != nullptr)		m_pQtCenterWidgets->GetData(root);
	if (m_pNetWorkdialog != nullptr)		m_pNetWorkdialog->GetData(root);
	if (m_pfrmGlobalVariable != nullptr)	m_pfrmGlobalVariable->GetData(root);
	if (dataVar::Instance().pfrmImageLayout != nullptr)	dataVar::Instance().pfrmImageLayout->GetData(root);
	QJsonParseError err_rpt;
	QJsonDocument	root_document;

	root_document.setObject(root);
	QByteArray root_string_compact = root_document.toJson(QJsonDocument::Compact);
	QString	strData = QString(root_string_compact);

	QFile file(strFilePath);
	if (!file.exists())	{
		qCritical() << tr("不存在") << strFilePath;
		dataVar::Instance().pManager->notify(tr("不存在") + strFilePath, NotifyType_Error);
		return -1;
	}
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	file.write(strData.toUtf8());
	file.close();

	SaveProjectPath();
	return 0;
}

void MainWindow::OpenProject(QString strFilePath)
{
	QApplication::processEvents();
	QString file_name = strFilePath;

	QString strData;
	QFile file(file_name);
	if (!file.exists())
	{
		qCritical() << tr("不存在") << file_name;
		dataVar::Instance().pManager->notify(tr("不存在") + file_name, NotifyType_Error);
		return;
	}
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QByteArray t = file.readAll();

	strData = QString(t);
	file.close();

	// 开始解析 解析成功返回QJsonDocument对象否则返回null
	QJsonParseError err_rpt;
	QJsonDocument root_document = QJsonDocument::fromJson(strData.toUtf8(), &err_rpt);
	if (err_rpt.error != QJsonParseError::NoError && !root_document.isNull())	{
		qCritical() << tr("不存在") << file_name;
		dataVar::Instance().pManager->notify(tr("不存在") + file_name, NotifyType_Error);
		return;
	}
	dataVar::Instance().projectName = file_name;
	// 获取根节点
	QJsonObject root = root_document.object();
	if (m_pQtCenterWidgets != nullptr)		m_pQtCenterWidgets->SetData(root);
	if (m_pNetWorkdialog != nullptr)		m_pNetWorkdialog->SetData(root);
	if (m_pfrmGlobalVariable != nullptr)	m_pfrmGlobalVariable->SetData(root);
	if (dataVar::Instance().pfrmImageLayout != nullptr)	dataVar::Instance().pfrmImageLayout->SetData(root);
	SaveProjectPath();
}

void MainWindow::SaveProjectPath()
{	
	//切换语言
	QSettings settings(dataVar::Instance().applicationPath + "/config/config.ini", QSettings::IniFormat);
	settings.setValue(QString("General/projectName"), dataVar::Instance().projectName);
}
