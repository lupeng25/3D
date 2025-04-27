#include "QtWaitWidgetsClass.h"
#include <QDesktopWidget>
#include "Windows.h"
#include "Data/dataVar.h"
//#include "QtMainPages.h"
#include "mainwindow.h"

QtWaitWidgetsClass::QtWaitWidgetsClass(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ClearVecBtn();	
	m_thread		= new KQTaskTread(this);
	//this->moveToThread(m_thread);
	hide();
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	//SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);
	m_timer			= new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));//每隔1秒刷新

	m_progressBar = new QProgressBar(this);
	m_progressBar->setAlignment(Qt::AlignCenter);
	m_progressBar->setGeometry(0, height() - 30, width(), 30);   // 设置进度条的位置
	m_progressBar->setStyleSheet("QProgressBar{height:22px; text-align:center; font-size:20px; color:white; border-radius:11px;}");
	m_progressBar->setValue(0);
	m_progressBar->hide();

	ui.widget->start();
	pBtnClose = new QPushButton(this);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(pBtnClose->sizePolicy().hasHeightForWidth());
	pBtnClose->setSizePolicy(sizePolicy);
	pBtnClose->setMinimumSize(QSize(80, 60));
	pBtnClose->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/skin/images/close.png"), QSize(), QIcon::Normal, QIcon::Off);
	pBtnClose->setIcon(icon);
	pBtnClose->setIconSize(QSize(20, 20));
	//pBtnClose->setStyleSheet("QPushButton{ background-color: transparent;border:0px;}\
	//			QPushButton::hover{ background-color: rgb(33, 50, 70,0.5);border:1px solid white;}");
	connect(pBtnClose,		SIGNAL(pressed()), this, SLOT(slotBtnClose()));
	setEnabled(true);
}

QtWaitWidgetsClass::~QtWaitWidgetsClass()
{
	ui.widget->stop();
}

void QtWaitWidgetsClass::ClearVecBtn()
{
	for (auto iter : pvecBtn2D)	 if (iter)	delete iter;
	pvecBtn2D.clear();
}

void QtWaitWidgetsClass::changeEvent(QEvent * ev)
{
}

void QtWaitWidgetsClass::resizeEvent(QResizeEvent * ev)
{
	if (pBtnClose != nullptr) pBtnClose->setGeometry(QRect(width() - pBtnClose->width(), 0, pBtnClose->width(), pBtnClose->height()));
	QWidget::resizeEvent(ev);
}

void QtWaitWidgetsClass::ShowMessage(QString strMsg)
{
	ui.lbl_Message->setText(strMsg);
}

void QtWaitWidgetsClass::ShowQProgressBar(bool bshow)
{
	if (bshow){	m_progressBar->show();	}
	else{	m_progressBar->hide();	}
}

StandardMsgResult QtWaitWidgetsClass::WaitForShow(const std::function<StandardMsgResult()>& task, const QString & label, StandardMsgType msg )
{
	dataVar::Instance().CenterMainWindow(this);
	if (m_thread->isRunning())	m_thread->terminate();
	m_thread->m_MsgResult		= MsgResult_OK;
	if (m_thread)				m_thread->SetTask(task);
	if (m_thread != nullptr)	m_thread->start();
	setStandardButtons(msg);
	this->show();
	raise();
	m_bBreak					= false;
	setEnabled(true);
	ShowMessage(label);
	pBtnClose->setEnabled(true);
	while ( this->isVisible() || m_thread->isRunning())
	{
		qApp->processEvents();
		if (m_bBreak)	break;
	}
	this->hide();
	if (m_thread->isRunning())	m_thread->terminate();
	return 	m_thread->m_MsgResult;
}

StandardMsgResult QtWaitWidgetsClass::WaitForExec(const std::function<StandardMsgResult()>& task, const QString & label, StandardMsgType msg )
{
	dataVar::Instance().CenterMainWindow(this);
	if(m_thread->isRunning())	m_thread->quit();
	m_thread->m_MsgResult		= MsgResult_OK;
	if (m_thread)				m_thread->SetTask(task);
	if (m_thread != nullptr)	m_thread->start();
	setStandardButtons(msg);
	m_bBreak = false;
	//m_iRetn = MsgResult_OK;
	ShowMessage(label);
	raise();
	this->exec();
	if (m_thread->isRunning())	m_thread->terminate();
	while (this->isVisible() || m_thread->isRunning())	{
		qApp->processEvents();
		if (m_bBreak)	break;
	}
	//close();
	setEnabled(true);
	if (parentWidget())	parentWidget()->setEnabled(true);
	return m_thread->m_MsgResult;
}

StandardMsgResult QtWaitWidgetsClass::WaitForAfterShow(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, const QString & label, StandardMsgType msg)
{
	dataVar::Instance().CenterMainWindow(this);
	if (m_thread->isRunning())	m_thread->terminate();
	m_thread->m_MsgResult = MsgResult_OK;
	if (m_thread)				m_thread->SetTask(task);
	if (m_thread != nullptr)	m_thread->start();
	setStandardButtons(msg);
	this->show();
	raise();
	m_bBreak = false;
	setEnabled(true);
	ShowMessage(label);
	pBtnClose->setEnabled(true);
	while (this->isVisible() || m_thread->isRunning())	{
		qApp->processEvents();
		if (m_bBreak)	break;
	}
	this->hide();
	if (m_thread->isRunning())	m_thread->terminate();
	return 	m_thread->m_MsgResult;
}

StandardMsgResult QtWaitWidgetsClass::WaitForAfterExec(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, const QString & label, StandardMsgType msg)
{
	dataVar::Instance().CenterMainWindow(this);
	if (m_thread->isRunning())	m_thread->quit();
	m_thread->m_MsgResult = MsgResult_OK;
	if (m_thread)				m_thread->SetTask(task);
	if (m_thread)				m_thread->SetEndTask(aftertask);
	if (m_thread != nullptr)	m_thread->start();
	setStandardButtons(msg);
	m_bBreak = false;
	//m_iRetn = MsgResult_OK;
	ShowMessage(label);
	raise();
	//if (parentWidget())	parentWidget()->setEnabled(true);
	pBtnClose->setEnabled(true);
	this->exec();
	while (this->isVisible() || m_thread->isRunning()) {
		qApp->processEvents();
		if (m_bBreak)	break;
	}
	auto _result = m_thread->m_MsgResult;

	//结束后的
	if (m_thread->isRunning())	m_thread->terminate();
	if (m_thread->isRunning())	m_thread->quit();
	m_thread->wait();
	if (_result == MsgResult_Close){
		close();
		setEnabled(true);
		if (parentWidget())	parentWidget()->setEnabled(true);
		return _result;
	}

	if (m_thread)				m_thread->SetTask(aftertask);
	if (m_thread != nullptr)	m_thread->start();
	while (this->isVisible() || m_thread->isRunning()) {
		qApp->processEvents();
		if (m_bBreak)	break;
	}
	//close();
	setEnabled(true);
	if (parentWidget())	parentWidget()->setEnabled(true);
	return _result;
}

StandardMsgResult QtWaitWidgetsClass::WaitForFunction(const std::function<StandardMsgResult()>& task, QString label, StandardMsgType msg, QWidget * parent)
{
	QtWaitWidgetsClass* gQtWaitWidgetsClass = new QtWaitWidgetsClass;
	gQtWaitWidgetsClass->setAttribute(Qt::WA_DeleteOnClose);
	gQtWaitWidgetsClass->show();
	gQtWaitWidgetsClass->raise();
	dataVar::Instance().CenterMainWindow(gQtWaitWidgetsClass);
	if (parent != nullptr)
		gQtWaitWidgetsClass->setParent(parent);
	//gQtWaitWidgetsClass.setStandardButtons(msg);
	return	gQtWaitWidgetsClass->WaitForExec(task, label, msg);
}

StandardMsgResult QtWaitWidgetsClass::WaitForShowFunction(const std::function<StandardMsgResult()>& task, QString label, StandardMsgType msg, QWidget * parent)
{
	QtWaitWidgetsClass gQtWaitWidgetsClass;
	gQtWaitWidgetsClass.show();
	gQtWaitWidgetsClass.raise();
	if (parent != nullptr)
		gQtWaitWidgetsClass.setParent(parent);
	//gQtWaitWidgetsClass.setStandardButtons(msg);
	return	gQtWaitWidgetsClass.WaitForShow(task, label, msg);
}

StandardMsgResult QtWaitWidgetsClass::WaitForMsgShowFunction(const std::function<StandardMsgResult()>& task, QString label, StandardMsgType msg, QWidget * parent)
{
	dataVar::Instance().CenterMainWindow(dataVar::Instance().pQtWaitWidgetsClass);
	dataVar::Instance().pQtWaitWidgetsClass->show();
	dataVar::Instance().pQtWaitWidgetsClass->raise();
	if(parent != nullptr)
		dataVar::Instance().pQtWaitWidgetsClass->setParent(parent);
	return dataVar::Instance().pQtWaitWidgetsClass->WaitForShow(task, label, msg);
}

StandardMsgResult QtWaitWidgetsClass::WaitForMsgExecFunction(const std::function<StandardMsgResult()>& task, QString label, StandardMsgType msg, QWidget * parent)
{
	dataVar::Instance().CenterMainWindow(dataVar::Instance().pQtWaitWidgetsClass);

	dataVar::Instance().pQtWaitWidgetsClass->show();
	dataVar::Instance().pQtWaitWidgetsClass->raise();
	if (parent != nullptr)
		dataVar::Instance().pQtWaitWidgetsClass->setParent(parent);
	return	dataVar::Instance().pQtWaitWidgetsClass->WaitForExec(task, label, msg);
}

void QtWaitWidgetsClass::SetStandardMsgResult(StandardMsgResult Result)
{
	dataVar::Instance().pQtWaitWidgetsClass->m_thread->m_MsgResult = Result;
	//dataVar::Instance().pQtWaitWidgetsClass->m_iRetn = Result;
}

StandardMsgResult QtWaitWidgetsClass::WaitForMsgAfterShowFunction(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, QString label, StandardMsgType msg, QWidget * parent)
{
	dataVar::Instance().CenterMainWindow(dataVar::Instance().pQtWaitWidgetsClass);
	dataVar::Instance().pQtWaitWidgetsClass->show();
	dataVar::Instance().pQtWaitWidgetsClass->raise();
	if (parent != nullptr)	dataVar::Instance().pQtWaitWidgetsClass->setParent(parent);
	return	dataVar::Instance().pQtWaitWidgetsClass->WaitForAfterShow(task, aftertask,label, msg);
}

StandardMsgResult QtWaitWidgetsClass::WaitForMsgAfterExecFunction(const std::function<StandardMsgResult()>& task,const std::function<StandardMsgResult()>& aftertask, QString label, StandardMsgType msg, QWidget * parent)
{
	dataVar::Instance().CenterMainWindow(dataVar::Instance().pQtWaitWidgetsClass);
	dataVar::Instance().pQtWaitWidgetsClass->show();
	dataVar::Instance().pQtWaitWidgetsClass->raise();
	if (parent != nullptr)	dataVar::Instance().pQtWaitWidgetsClass->setParent(parent);
	return	dataVar::Instance().pQtWaitWidgetsClass->WaitForAfterExec(task, aftertask,label, msg);
}

void QtWaitWidgetsClass::mousePressEvent(QMouseEvent * ev)
{
	if (ev->button() == Qt::LeftButton) {
		isDragging = true;
		dragPos = ev->globalPos() - frameGeometry().topLeft();
		ev->accept();
	}
}

void QtWaitWidgetsClass::mouseReleaseEvent(QMouseEvent * ev)
{
	if (isDragging) {
		isDragging = false;
		ev->accept();
	}
}

void QtWaitWidgetsClass::mouseMoveEvent(QMouseEvent * ev)
{
	if (isDragging) {
		move(ev->globalPos() - dragPos);
		ev->accept();
	}
}

void QtWaitWidgetsClass::closeEvent(QCloseEvent *ev)
{
	if (parentWidget())	parentWidget()->setEnabled(true);
	QDialog::closeEvent(ev);
	m_bBreak = true;
}

void QtWaitWidgetsClass::showEvent(QShowEvent * ev)
{
	if (parentWidget())		parentWidget()->setEnabled(false);
	QDialog::showEvent(ev);	
	setEnabled(true); 
	raise();
	pBtnClose->setEnabled(true);
}

void QtWaitWidgetsClass::setStandardButtons(StandardMsgType buttons)
{
	switch (buttons)
	{
	case QtWaitWidgetsClass::MsgType_NoNe: {
		pBtnClose->hide();
	} break;
	case QtWaitWidgetsClass::MsgType_Close: {
		pBtnClose->show();
	}	break;
	default:
		break;
	}
}

void QtWaitWidgetsClass::slotbtnOKClicked()
{
	m_bBreak = true;
}

void QtWaitWidgetsClass::slotbtnCancerClicked()
{
	m_bBreak = true;
	//m_thread->m_MsgResult = StandardMsgResult::MsgResult_Cancer;
}

void QtWaitWidgetsClass::slotTimeOut()
{
	m_thread->terminate();
	close();
}

void QtWaitWidgetsClass::slotBtnClose()
{
	this->close();
	m_thread->m_MsgResult	= MsgResult_Close;
	if (parentWidget())		parentWidget()->setEnabled(true);
	m_bBreak = true;
}

LoadingDialog::LoadingDialog(QWidget *parent) : QDialog(parent)
{
	//如果需要显示任务栏对话框则删除Qt::Tool
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	initUi();
}

/**
* @brief LoadingDialog::initUi UI元素初始化
*/
void LoadingDialog::initUi()
{
	this->setFixedSize(250, 250);
	m_pCenterFrame = new QFrame(this);
	m_pCenterFrame->setGeometry(10, 10, 230, 230);

	//加载Loading动画
	m_pLoadingMovie = new QMovie(":/image/image/wait.gif");
	m_pLoadingMovie->setScaledSize(QSize(120, 120));
	m_pMovieLabel = new QLabel(m_pCenterFrame);
	m_pMovieLabel->setGeometry(55, 10, 120, 120);
	m_pMovieLabel->setScaledContents(true);
	m_pMovieLabel->setMovie(m_pLoadingMovie);
	m_pLoadingMovie->start();

	//提示文本
	m_pTipsLabel = new QLabel(m_pCenterFrame);
	m_pTipsLabel->setGeometry(5, 130, 220, 50);
	m_pTipsLabel->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
	m_pTipsLabel->setObjectName("tips");
	m_pTipsLabel->setText("加载中,请稍候...");
	m_pTipsLabel->setStyleSheet("QLabel#tips{font-family:\"Microsoft YaHei\";font-size: 15px;color: #333333;}");

	//取消按钮
	m_pCancelBtn = new QPushButton(m_pCenterFrame);
	m_pCancelBtn->setObjectName("cancelBtn");
	m_pCancelBtn->setText("取消等待");
	m_pCancelBtn->setStyleSheet("QPushButton#cancelBtn{"
		"background-color: #edeef6;"
		"border-radius: 4px;"
		"font-family: \"Microsoft YaHei\";"
		"font-size: 14px;"
		"color: #333333;"
		"}"
		"QPushButton#cancelBtn::hover{"
		"background:#dcdeea"
		"}");
	m_pCancelBtn->setGeometry(25, 180, 180, 35);
	m_pCancelBtn->setEnabled(true);
	connect(m_pCancelBtn, &QPushButton::clicked, this, &LoadingDialog::cancelBtnClicked);

	//实例阴影shadow
	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor(32, 101, 165));
	shadow->setBlurRadius(10);
	this->setGraphicsEffect(shadow);
}

/**
* @brief LoadingDialog::setTipsText 设置提示文本
* @param strTipsText 提示文本
*/
void LoadingDialog::setTipsText(QString strTipsText)
{
	m_pTipsLabel->setText(strTipsText);
}

/**
* @brief LoadingDialog::setCanCancel 设置是够允许用户点击取消等待按钮
* @param bCanCancel 是够允许
*/
void LoadingDialog::setCanCancel(bool bCanCancel)
{
	m_pCancelBtn->setEnabled(bCanCancel);
}

/**
* @brief LoadingDialog::moveToCenter 移动对话框到指定窗口中间
* @param pParent 指定窗口指针
*/
void LoadingDialog::moveToCenter(QWidget *pParent)
{
	if (pParent != nullptr && pParent != NULL)
	{
		int nParentWidth = pParent->width();
		int nParentHeigth = pParent->height();

		int nWidth = this->width();
		int nHeight = this->height();

		int nParentX = pParent->x();
		int nParentY = pParent->y();

		int x = (nParentX + (nParentWidth - nWidth) / 2);
		int y = (nParentY + (nParentHeigth - nHeight) / 2);

		this->move(x, y);
	}
}

/**
* @brief LoadingDialog::cancelBtnClicked 取消按钮槽函数
*/
void LoadingDialog::cancelBtnClicked()
{
	emit cancelWaiting();
	this->done(USER_CANCEL);
}

/**
* @brief LoadingDialog::paintEvent 界面绘制
* @param event
*/
void LoadingDialog::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); //反锯齿
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(Qt::transparent);
	QRect rect = this->rect();
	rect.setLeft(9);
	rect.setTop(9);
	rect.setWidth(rect.width() - 9);
	rect.setHeight(rect.height() - 9);
	painter.drawRoundedRect(rect, 8, 8);
	QWidget::paintEvent(event);
}

LoadingDialog::~LoadingDialog()
{
	delete m_pLoadingMovie;
	delete m_pMovieLabel;
	delete m_pTipsLabel;
	delete m_pCancelBtn;
	delete m_pCenterFrame;
}

KQTaskTread::KQTaskTread(QWidget* widget, const std::function<StandardMsgResult()>& task)
{
	this->m_task = task;
	m_wait_ui = widget;
	if (this->m_wait_ui != nullptr) {
		connect(this, SIGNAL(taskFinish()), m_wait_ui, SLOT(close()));
	}
}

KQTaskTread::KQTaskTread(QWidget* widget)
{
	m_wait_ui = widget;
	if (this->m_wait_ui != nullptr) {
		connect(this, SIGNAL(taskFinish()), m_wait_ui, SLOT(close()));
	}
}

void KQTaskTread::SetTask(const std::function<StandardMsgResult()>& task)
{
	this->m_task = task;
}

void KQTaskTread::SetEndTask(const std::function<StandardMsgResult()>& task)
{
	this->m_endtask = task;
}

void KQTaskTread::run()
{
	if (this->m_task != nullptr)
		m_MsgResult = this->m_task();

	//if (m_endtask != nullptr)	m_endtask();
	emit taskFinish();
}

#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include <QDebug>
#include <QWindow>

KQProgressDisplayControl::KQProgressDisplayControl(KQWaitDlg* waitDlg) :m_waitDlg(waitDlg)
{
	if (m_waitDlg != nullptr)
	{
		connect(this, &KQProgressDisplayControl::setSteped,		m_waitDlg, &KQWaitDlg::setStep);
		connect(this, &KQProgressDisplayControl::setStepMsged,	m_waitDlg, &KQWaitDlg::setStepMsg);
		connect(this, &KQProgressDisplayControl::setUpdate,		m_waitDlg, &KQWaitDlg::setUpdate);
	}
}
void KQProgressDisplayControl::setStep(int nStep)
{
	emit setSteped(nStep);
}

void KQProgressDisplayControl::setStepMsg(QString sMsg)
{
	emit setStepMsged(sMsg);
}

KQWaitDlg::KQWaitDlg(QString label, QWidget *parent) : QDialog(parent)
{
#ifdef  __GNUC__
														   //this->setStyleSheet("QDialog{background-color: rgba(E0,FF,FF,1);}");// linux下设置为灰色背景
	setAttribute(Qt::WA_TranslucentBackground, true);//透明需要开启透明 终端使用命令 marco -c --replace
#else
	this->setAttribute(Qt::WA_TranslucentBackground, true);  //window下 设置背景透明
#endif

	m_Layout = new QVBoxLayout(this);
	m_Layout->setMargin(0);
	m_Layout->setSpacing(0);
	this->setLayout(m_Layout);

	m_progressType = RadiusProgress;
	m_sMsg = label;

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateMsg()));//每隔1秒刷新
	//connect(m_timer, SIGNAL(timeout()), this, SLOT(setUpdate()));//每隔1秒刷新
	m_nDotjs				= 0;
	m_nStep					= 0;
	m_label					= new QLabel(this);
	m_ringsProgress			= new KQRingsProgressBar(this);
	m_radiusProgress		= new KQRadiusProgressBar(this);
	m_thread				= new KQTaskTread(this);
	m_pSpotsShrinkCircle	= new SpotsShrinkCircle(this);
	m_pSpotsCircle			= new SpotsCircle(this);
	m_pStickCircle			= new StickCircle(this);
	m_pTubeCircle			= new TubeCircle(this);
	m_pTextCircle			= new TextCircle("Loading...",this);
	m_pTubeBallCircle		= new TubeBallCircle(this);
	m_pSpotsShrinkCircle->hide();
	m_pSpotsCircle->hide();
	m_pStickCircle->hide();
	m_pTubeCircle->hide();
	m_pTextCircle->hide();
	m_pTubeBallCircle->hide();
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

}

void KQWaitDlg::WaitFor(const std::function<StandardMsgResult()>& task, ProgressType nType)
{
	int w = 200;
	int h = 100;
	m_label->setText(m_sMsg);
	m_label->setWordWrap(false);
	m_label->setAlignment(Qt::AlignCenter);
	//设置字号
	QFont ft = m_label->font();
	ft.setPointSize(10);
	ft.setBold(true);
	m_label->setFont(ft);
	//设置颜色
	QPalette pa;
	pa.setColor(QPalette::WindowText, QColor(30, 144, 255));
	m_label->setPalette(pa);

	if (RingsProgress == nType) {	//环形
		m_progressType = RingsProgress;
		m_Layout->addWidget(m_ringsProgress);
		m_ringsProgress->setFixedSize(300, 100);
		w = 300;
		h = 160;
		m_label->setFixedHeight(60);
		m_label->setAlignment(Qt::AlignCenter);
	}
	else if (RadiusProgress == nType) {
		m_progressType = RadiusProgress;
		m_Layout->addWidget(m_radiusProgress);
	}
	else if (RSpotsShrinkCircle == nType) {        //条形
		w = m_pSpotsShrinkCircle->width();
		h = m_pSpotsShrinkCircle->height() + 40;
		m_progressType = RSpotsShrinkCircle;
		m_Layout->addWidget(m_pSpotsShrinkCircle);
		m_pSpotsShrinkCircle->start();
		m_pSpotsShrinkCircle->show();

		m_label->setFixedHeight(m_pSpotsShrinkCircle->height());
	}
	else if (RSpotsCircle == nType) {        //条形
		w = m_pSpotsCircle->width();
		h = m_pSpotsCircle->height() + 40;
		m_progressType = RSpotsCircle;
		m_Layout->addWidget(m_pSpotsCircle);
		m_pSpotsCircle->start();
		m_pSpotsCircle->show();
		m_label->setFixedHeight(m_pSpotsCircle->height());
	}
	else if (RStickCircle == nType) {        //条形
		w = m_pStickCircle->width();
		h = m_pStickCircle->height() + 40;
		m_progressType = RStickCircle;
		m_Layout->addWidget(m_pStickCircle);
		m_pStickCircle->start();
		m_pStickCircle->show();
		m_label->setFixedHeight(m_pStickCircle->height());
	}
	else if (RTubeCircle == nType) {        //条形
		w = m_pTubeCircle->width();
		h = m_pTubeCircle->height() + 40;
		m_progressType = RTubeCircle;
		m_Layout->addWidget(m_pTubeCircle);
		m_pTubeCircle->start();
		m_pTubeCircle->show();
		m_label->setFixedHeight(m_pTubeCircle->height());
	}
	else if (RTextCircle == nType) {        //条形
		w = m_pTextCircle->width();
		h = m_pTextCircle->height() + 40;
		m_progressType = RTextCircle;
		m_Layout->addWidget(m_pTextCircle);
		m_pTextCircle->start();
		m_pTextCircle->show();
		m_label->setFixedHeight(m_pTextCircle->height());
	}
	else if (RTubeBallCircle == nType) {        //条形
		w = m_pTubeBallCircle->width();
		h = m_pTubeBallCircle->height() + 40;
		m_progressType = RTubeBallCircle;
		m_Layout->addWidget(m_pTubeBallCircle);
		m_pTubeBallCircle->start();
		m_pTubeBallCircle->show();
		m_label->setFixedHeight(m_pTubeBallCircle->height());
	}
	else {        //条形
		w = m_pSpotsShrinkCircle->width();
		h = m_pSpotsShrinkCircle->height() + 40;
		m_progressType = RSpotsShrinkCircle;
		m_Layout->addWidget(m_pSpotsShrinkCircle);
		m_pSpotsShrinkCircle->start();
		m_pSpotsShrinkCircle->show();
		
		m_label->setFixedHeight(m_pSpotsShrinkCircle->height());
	}

	m_Layout->addWidget(m_label);

	//设置显示位置
	this->setFixedSize(w, h);
	QDesktopWidget* deskWidget = QApplication::desktop();
	QRect deskRect = deskWidget->availableGeometry();//QDesktopWidget
	double x = (deskRect.width() - w) / 2;   //设置对话框居中于桌面
	double y = (deskRect.height() - h) / 2;

	//查看桌面窗口的儿子
	foreach(QObject *object, deskWidget->children()) {
		if (object->isWidgetType())
		{
			qDebug() << "widget";
			QWidget *w = static_cast<QWidget*>(object);
			qDebug() << w->geometry();
		}
		if (object->isWindowType())
		{
			qDebug() << "window";
			QWindow *w = static_cast<QWindow*>(object);
			qDebug() << w->geometry();
		}
	}

	/*int desktop_width = QApplication::desktop()->c;
	int desktop_high = QApplication::desktop()->height();
	this->resize (desktop_width, desktop_high);
	this->setWindowFlags (Qt::FramelessWindowHint);*/ // hide title pannel
	this->move(x, y);

	if (m_thread)	m_thread->SetTask(task);
	//this->m_task = task;
	this->setWindowModality(Qt::ApplicationModal);
	m_timer->start(1000);//每隔1秒刷新

	if (m_thread != nullptr)
		m_thread->start();
	this->exec();
	//if (parentWidget())
	//	parentWidget()->setEnabled(true);
}

void KQWaitDlg::WaitFor(const std::function<StandardMsgResult()>& task, const QString & label, ProgressType nType)
{
	int w = 200;
	int h = 100;
	setStepMsg(label);
	m_label->setText(m_sMsg);
	m_label->setWordWrap(false);
	m_label->setAlignment(Qt::AlignCenter);
	//设置字号
	QFont ft = m_label->font();
	ft.setPointSize(10);
	ft.setBold(true);
	m_label->setFont(ft);
	//设置颜色
	QPalette pa;
	pa.setColor(QPalette::WindowText, QColor(30, 144, 255));
	m_label->setPalette(pa);

	if (RingsProgress == nType) {	//环形
		m_progressType = RingsProgress;
		m_Layout->addWidget(m_ringsProgress);
		m_ringsProgress->setFixedSize(300, 100);
		w = 300;
		h = 160;
		m_label->setFixedHeight(60);
		m_label->setAlignment(Qt::AlignCenter);
	}
	else if (RadiusProgress == nType) {
		m_progressType = RadiusProgress;
		m_Layout->addWidget(m_radiusProgress);
	}
	else if (RSpotsShrinkCircle == nType) {        //条形
		w = m_pSpotsShrinkCircle->width();
		h = m_pSpotsShrinkCircle->height() + 40;
		m_progressType = RSpotsShrinkCircle;
		m_Layout->addWidget(m_pSpotsShrinkCircle);
		m_pSpotsShrinkCircle->start();
		m_pSpotsShrinkCircle->show();

		m_label->setFixedHeight(m_pSpotsShrinkCircle->height());
	}
	else if (RSpotsCircle == nType) {        //条形
		w = m_pSpotsCircle->width();
		h = m_pSpotsCircle->height() + 40;
		m_progressType = RSpotsCircle;
		m_Layout->addWidget(m_pSpotsCircle);
		m_pSpotsCircle->start();
		m_pSpotsCircle->show();
		m_label->setFixedHeight(m_pSpotsCircle->height());
	}
	else if (RStickCircle == nType) {        //条形
		w = m_pStickCircle->width();
		h = m_pStickCircle->height() + 40;
		m_progressType = RStickCircle;
		m_Layout->addWidget(m_pStickCircle);
		m_pStickCircle->start();
		m_pStickCircle->show();
		m_label->setFixedHeight(m_pStickCircle->height());
	}
	else if (RTubeCircle == nType) {        //条形
		w = m_pTubeCircle->width();
		h = m_pTubeCircle->height() + 40;
		m_progressType = RTubeCircle;
		m_Layout->addWidget(m_pTubeCircle);
		m_pTubeCircle->start();
		m_pTubeCircle->show();
		m_label->setFixedHeight(m_pTubeCircle->height());
	}
	else if (RTextCircle == nType) {        //条形
		w = m_pTextCircle->width();
		h = m_pTextCircle->height() + 40;
		m_progressType = RTextCircle;
		m_Layout->addWidget(m_pTextCircle);
		m_pTextCircle->start();
		m_pTextCircle->show();
		m_label->setFixedHeight(m_pTextCircle->height());
	}
	else if (RTubeBallCircle == nType) {        //条形
		w = m_pTubeBallCircle->width();
		h = m_pTubeBallCircle->height() + 40;
		m_progressType = RTubeBallCircle;
		m_Layout->addWidget(m_pTubeBallCircle);
		m_pTubeBallCircle->start();
		m_pTubeBallCircle->show();
		m_label->setFixedHeight(m_pTubeBallCircle->height());
	}
	else {        //条形
		w = m_pSpotsShrinkCircle->width();
		h = m_pSpotsShrinkCircle->height() + 40;
		m_progressType = RSpotsShrinkCircle;
		m_Layout->addWidget(m_pSpotsShrinkCircle);
		m_pSpotsShrinkCircle->start();
		m_pSpotsShrinkCircle->show();

		m_label->setFixedHeight(m_pSpotsShrinkCircle->height());
	}

	m_Layout->addWidget(m_label);

	//设置显示位置
	this->setFixedSize(w, h);
	QDesktopWidget* deskWidget = QApplication::desktop();
	QRect deskRect = deskWidget->availableGeometry();//QDesktopWidget
	double x = (deskRect.width() - w) / 2;   //设置对话框居中于桌面
	double y = (deskRect.height() - h) / 2;

	//查看桌面窗口的儿子
	foreach(QObject *object, deskWidget->children()) {
		if (object->isWidgetType())
		{
			qDebug() << "widget";
			QWidget *w = static_cast<QWidget*>(object);
			qDebug() << w->geometry();
		}
		if (object->isWindowType())
		{
			qDebug() << "window";
			QWindow *w = static_cast<QWindow*>(object);
			qDebug() << w->geometry();
		}
	}

	/*int desktop_width = QApplication::desktop()->c;
	int desktop_high = QApplication::desktop()->height();
	this->resize (desktop_width, desktop_high);
	this->setWindowFlags (Qt::FramelessWindowHint);*/ // hide title pannel
	this->move(x, y);

	if (m_thread)	m_thread->SetTask(task);
	//this->m_task = task;
	this->setWindowModality(Qt::ApplicationModal);
	m_timer->start(1000);//每隔1秒刷新

	if (m_thread != nullptr)
		m_thread->start();
	//if (parentWidget())
	//	parentWidget()->setEnabled(false);
	this->exec();
	//if (parentWidget())
	//	parentWidget()->setEnabled(true);
}

void KQWaitDlg::WaitFor(const std::function<StandardMsgResult()>& task, std::function<StandardMsgResult()> endtask, ProgressType nType)
{
	if (m_thread != nullptr)	m_thread->SetEndTask(endtask);
	WaitFor(task, nType);
}

void KQWaitDlg::WaitFor(const std::function<StandardMsgResult()>& task, std::function<StandardMsgResult()> endtask, const QString & label, ProgressType nType)
{
	if (m_thread != nullptr)	m_thread->SetEndTask(endtask);
	WaitFor(task, nType);
}

void KQWaitDlg::Start(KQWaitDlg::ProgressType type)
{

}

void KQWaitDlg::WaitForFunction(const std::function<StandardMsgResult()>& task, QString label, QWidget * parent, ProgressType nType)
{
	KQWaitDlg* gKQWaitDlg = new KQWaitDlg;
	gKQWaitDlg->setParent(parent);
	gKQWaitDlg->show();
	gKQWaitDlg->raise();
	gKQWaitDlg->WaitFor(task, label, nType);
	delete gKQWaitDlg;
}

void KQWaitDlg::setStep(int nStep)
{
	if (m_progressType == RadiusProgress && m_radiusProgress != nullptr)
	{
		m_radiusProgress->setValue(nStep);
	}
	else if (m_progressType == RingsProgress && m_ringsProgress != nullptr)
	{
		m_ringsProgress->setPercent(nStep);
	}
	update();//会调用
}

void KQWaitDlg::setStepMsg(QString sMsg)//修改文本
{
	m_sMsg = sMsg;
	m_label->setText(m_sMsg);
	if (m_pTextCircle != nullptr)	m_pTextCircle->setText(sMsg);
}

void KQWaitDlg::setUpdate()
{
	//刷新上面圆环或滑动条
	m_nStep += 5;
	if (m_nStep >= 100) m_nStep = 0;
	if (m_progressType == RadiusProgress && m_radiusProgress != nullptr)
	{
		m_radiusProgress->setValue(m_nStep);
	}
	else if (m_progressType == RingsProgress && m_ringsProgress != nullptr)
	{
		m_ringsProgress->setPercent(m_nStep);
	}
	//刷新下面文字
	QString sMsg = m_sMsg;
	for (int i = 0; i<m_nDotjs; i++) {
		sMsg += ".";
	}
	++m_nDotjs;
	if (m_nDotjs>3)	m_nDotjs = 0;
	m_label->setText(sMsg);



	update();
}

void KQWaitDlg::updateMsg()//每隔一秒刷新界面
{
	QString sMsg = m_sMsg;
	for (int i = 0; i<m_nDotjs; i++) {
		sMsg += ".";
	}
	++m_nDotjs;
	if (m_nDotjs>3)	m_nDotjs = 0;
	m_label->setText(sMsg);
	update();
}

void KQWaitDlg::showEvent(QShowEvent *event)
{
	if (parentWidget())
		parentWidget()->setEnabled(false);
	//if (m_thread != nullptr)
	//	m_thread->start();
}

void KQWaitDlg::keyPressEvent(QKeyEvent *event)
{
}

void KQWaitDlg::closeEvent(QCloseEvent * ev)
{
	QDialog::closeEvent(ev);
	if (parentWidget())
		parentWidget()->setEnabled(true);
}

#include  <QPainter>
#include  <QPixmap>
KQRadiusProgressBar::KQRadiusProgressBar(QWidget *parent) : QProgressBar(parent)
{
	setMinimum(0);
	setMaximum(100);
	setValue(0);
	setFixedHeight(40);
}

void KQRadiusProgressBar::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QRect rect = QRect(0, 0, width(), height() / 2);
	QRect textRect = QRect(0, height() / 2, width(), height() / 2);

	const double k = (double)(value() - minimum()) / (maximum() - minimum());
	int x = (int)(rect.width() * k);
	QRect fillRect = rect.adjusted(0, 0, x - rect.width(), 0);

	QString valueStr = QString("");//QString("%1%").arg(QString::number(value()));
	QPixmap buttomMap = QPixmap(":/Res/waitdialog/radius_back.png");
	QPixmap fillMap = QPixmap(":/Res/waitdialog/radius_front.png");

	//画进度条
	p.drawPixmap(rect, buttomMap);
	p.drawPixmap(fillRect, fillMap, fillRect);

	//画文字
	QFont f = QFont("Microsoft YaHei", 15, QFont::Bold);
	p.setFont(f);
	p.setPen(QColor("#555555"));
	p.drawText(textRect, Qt::AlignCenter, valueStr);
}

#include <QPainter>

KQRingsProgressBar::KQRingsProgressBar(QWidget *parent) : QWidget(parent),
	m_rotateAngle(0),
	m_percent(0) {

}

void KQRingsProgressBar::setPercent(int percent)
{
	if (percent != m_percent)	{
		m_percent = percent;
		update();
	}
}

void KQRingsProgressBar::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	m_rotateAngle = 360 * m_percent / 100;
	int side = qMin(width(), height());
	QRectF outRect(100, 0, side, side);
	QRectF inRect(120, 20, side - 40, side - 40);
	QString valueStr = QString("%1%").arg(QString::number(m_percent));//QString(""),QString("%1%").arg(QString::number(m_percent));

	//画外圆
	p.setPen(Qt::NoPen);
	p.setBrush(QBrush(QColor(97, 117, 118)));
	p.drawEllipse(outRect);
	p.setBrush(QBrush(QColor(0, 205, 102)));
	p.drawPie(outRect, (90 - m_rotateAngle) * 16, m_rotateAngle * 16);
	//画遮罩
	p.setBrush(palette().window().color());
	p.drawEllipse(inRect);
	//画文字
	QFont f = QFont("Microsoft YaHei", 13, QFont::Bold);
	p.setFont(f);
	//p.setFont(f);
	p.setPen(QColor(30, 144, 255));
	p.drawText(inRect, Qt::AlignCenter, valueStr);
}