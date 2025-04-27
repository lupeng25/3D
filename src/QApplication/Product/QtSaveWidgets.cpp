#include "QtSaveWidgets.h"
#include "mainwindow/mainwindow.h"

QtSaveWidgets::QtSaveWidgets(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

QtSaveWidgets::~QtSaveWidgets()
{

}

void QtSaveWidgets::on_btnSave_clicked()
{
	MainWindow::getInstance()->SaveData();
	MainWindow::getInstance()->SaveSystemData();
}

void QtSaveWidgets::on_btnCancer_clicked()
{
	MainWindow::getInstance()->LoadData();
	MainWindow::getInstance()->LoadSystemData();
}
