#include "QtSplashScreen.h"
#include "QDebug"
#include<QMouseEvent>

QtSplashScreen::QtSplashScreen(QPixmap pixmap, QWidget *parent) :
	QSplashScreen(parent, pixmap)
{
	installEventFilter(this);//设置事件发送
	m_progressBar = new QProgressBar(this);
	m_progressBar->setAlignment(Qt::AlignCenter);
	m_progressBar->setGeometry(0, pixmap.height() + 30, pixmap.width(), 30);   // 设置进度条的位置
	m_progressBar->setStyleSheet("QProgressBar{height:22px; text-align:center; font-size:20px; color:white; border-radius:11px;}");
	m_progressBar->setValue(0);

	m_labelText = new QLabel(this);
	m_labelText->setGeometry(0, pixmap.height() - 105, pixmap.width(), 25);   // 设置进度条的位置
	m_labelText->setAlignment(Qt::AlignCenter);
	m_labelText->setText("Start");
	m_labelText->setStyleSheet("background:transparent;color:white;");

	m_Visionlabel = new QLabel(this);
	m_Visionlabel->setGeometry(0, pixmap.height() - 55, pixmap.width(), 25);   // 设置进度条的位置
	m_Visionlabel->setAlignment(Qt::AlignCenter);
	m_Visionlabel->setText("0.0.0.0");
	m_Visionlabel->setStyleSheet("background:transparent;color:white;");

	m_SoftVisionlabel = new QLabel(this);
	m_SoftVisionlabel->setGeometry(0, pixmap.height() - 80, pixmap.width(), 25);   // 设置进度条的位置
	m_SoftVisionlabel->setAlignment(Qt::AlignCenter);
	m_SoftVisionlabel->setText(QSplashScreen::tr("SoftVision"));
	m_SoftVisionlabel->setStyleSheet("background:transparent;color:white;");

	m_Otherlabel = new QLabel(this);
	m_Otherlabel->setGeometry(0, pixmap.height() - 55, pixmap.width(), 25);   // 设置进度条的位置
	m_Otherlabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	m_Otherlabel->setText("...");
	m_Otherlabel->setStyleSheet("background:transparent;color:white;");

	QFont font = this->font();
	font.setPointSizeF(20);
	this->setFont(font);
	showAppText("QtApplication");
}

QtSplashScreen::~QtSplashScreen()
{
	//delete m_progressBar;
	//delete m_Visionlabel;
	//delete m_Otherlabel;
	//delete m_SoftVisionlabel;
	if (m_progressBar != nullptr)		delete m_progressBar;		m_progressBar = nullptr;
	if (m_labelText != nullptr)			delete m_labelText;			m_labelText = nullptr;
	if (m_Visionlabel != nullptr)		delete m_Visionlabel;		m_Visionlabel = nullptr;
	if (m_Otherlabel != nullptr)		delete m_Otherlabel;		m_Otherlabel = nullptr;
	if (m_SoftVisionlabel != nullptr)	delete m_SoftVisionlabel;	m_SoftVisionlabel = nullptr;

}

void QtSplashScreen::setRange(int min, int max)
{
	m_progressBar->setRange(min, max);
}

void QtSplashScreen::updateNum(int n)
{
	m_progressBar->setValue(n);//更新进度条进度
	//    mySleep(10);//停止10ms
}

void QtSplashScreen::showAppText(QString str)
{	
	Qt::Alignment topRight = Qt::AlignHCenter | Qt::AlignTop;  //字体显示位置
	showMessage(str, topRight, Qt::white);
}

void QtSplashScreen::showMessageText(QString str)
{
	m_labelText->setText(str);
}

void QtSplashScreen::showVisionText(QString str)  //设置软件版本
{
	m_Visionlabel->setText(str);
}

void QtSplashScreen::showOtherText(QString str)
{
	m_Otherlabel->setAlignment(Qt::AlignCenter);
	m_Otherlabel->setText(str);
}

void QtSplashScreen::changeEvent(QEvent * ev)
{
	QSplashScreen::changeEvent(ev);
	if (ev->type() == QEvent::Type::LanguageChange)
	{
		m_SoftVisionlabel->setText(QSplashScreen::tr("SoftVision"));
	}
}
