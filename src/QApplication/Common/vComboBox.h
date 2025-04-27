#pragma once

#include <QComboBox>
#include <QListWidget>

class vComboBox : public QComboBox
{
	Q_OBJECT

public:
	vComboBox(QWidget *parent = nullptr);
	~vComboBox();

public:
	virtual void showPopup();
	virtual void hidePopup();
	virtual bool eventFilter(QObject *obj, QEvent *event);

	bool IsPressed();
protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *e);

	bool m_bIshidePressed;
signals:
	void sigPopup();
	void sigHidePopup();
	void mouseSingleClickd();

};