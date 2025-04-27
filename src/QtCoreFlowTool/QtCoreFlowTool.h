#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtCoreFlowTool.h"

class QtCoreFlowTool : public QMainWindow
{
	Q_OBJECT

public:
	QtCoreFlowTool(QWidget *parent = Q_NULLPTR);

private:
	Ui::QtCoreFlowToolClass ui;
};
