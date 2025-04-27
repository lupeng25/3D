#include "QtStartWidgets.h"
#include <QDesktopWidget>

QtStartWidgets::QtStartWidgets(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint); // 设置窗口无边框

	int screenWidth = QApplication::desktop()->width();
	int screenHeight = QApplication::desktop()->height();

	// 获取子窗口的宽度和高度
	int windowWidth = this->width();
	int windowHeight = this->height();

	// 计算子窗口的左上角坐标
	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	// 将子窗口移动到屏幕中央
	this->move(x, y);
}

QtStartWidgets::~QtStartWidgets()
{
}
void QtStartWidgets::setRange(int min, int max)
{
	ui.progressBar->setRange(min, max);
}

void QtStartWidgets::updateNum(int n)
{
	ui.progressBar->setValue(n);//更新进度条进度
}

void QtStartWidgets::showAppText(QString str)
{
	ui.lblApplition->setText(str);
}

void QtStartWidgets::showAppOtherInfo(QString str)
{
	ui.lbl_OtherInfo->setText(str);
}

void QtStartWidgets::showAppImg(QString str)
{
	QString strPath = QApplication::applicationDirPath() + "/startimage/" + str;
	QPixmap pix;
	pix.load(strPath);
	ui.lblImg->setScaledContents(true);
	ui.lblImg->setPixmap(pix);
}

void QtStartWidgets::showMessageText(QString str)
{
	ui.lblText->setText(str);
}

void QtStartWidgets::showVisionText(QString str)  //设置软件版本
{
	ui.lblVision->setText(str);
}

void QtStartWidgets::finish(QWidget * parent)
{
	if (parent)
	{
		this->close();
		//parent->show();
	}
}

void QtStartWidgets::changeEvent(QEvent * ev)
{
	QWidget::changeEvent(ev);
	if (ev->type() == QEvent::Type::LanguageChange)
	{
		ui.retranslateUi(this);
	}
}
