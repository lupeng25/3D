#pragma once

#include <QWidget>
#include "ui_QtSaveWidgets.h"

class QtSaveWidgets : public QWidget
{
	Q_OBJECT

public:
	QtSaveWidgets(QWidget *parent = Q_NULLPTR);
	~QtSaveWidgets();
public slots:
	void on_btnSave_clicked();
	void on_btnCancer_clicked();
private:
	Ui::QtSaveWidgets ui;
};
