#include "vComboBox.h"
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>

vComboBox::vComboBox(QWidget *parent)
	: QComboBox(parent)
{
	this->installEventFilter(this);
}

vComboBox::~vComboBox()
{

}

void vComboBox::showPopup()
{
	m_bIshidePressed = false;
	emit sigPopup();
	QComboBox::showPopup();
}

void vComboBox::hidePopup()
{
	m_bIshidePressed = true;
	emit sigHidePopup();
	QComboBox::hidePopup();

}

bool vComboBox::eventFilter(QObject * obj, QEvent * event)
{
	if (obj == this) {
		if (event->type() == QEvent::Wheel) {
			return true;	//禁用下拉框的滚轮改变项的功能
		}
	}
	return QComboBox::eventFilter(obj, event);
}

bool vComboBox::IsPressed()
{
	return m_bIshidePressed;
}



void vComboBox::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit mouseSingleClickd();  //触发clicked信号
	}

	QComboBox::mousePressEvent(event);  //将该事件传给父类处理，这句话很重要，如果没有，父类无法处理本来的点击事件
}

void vComboBox::mouseReleaseEvent(QMouseEvent * e)
{
	QComboBox::mouseReleaseEvent(e);  //将该事件传给父类处理，这句话很重要，如果没有，父类无法处理本来的点击事件
}
