#pragma once

#include <QDialog>
#include "ui_frmScaleTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"

#include <QTableWidget>
#include "ScaleTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"

#include "../../frmBaseTool.h"
class frmScaleTool : public frmBaseTool
{
	Q_OBJECT
public:
	frmScaleTool(QWidget* parent = Q_NULLPTR);
   ~frmScaleTool();
private:
	Ui::frmScaleToolClass ui;
public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initConnect();
	void ResizeUI();

protected:
	virtual void resizeEvent(QResizeEvent *ev);
private slots:
	void on_btnAddROI_clicked();
	void onChannelClicked(int Index);
private slots:
	void slot_ChangeMatchImg_Index(int Index);
	void slot_MoveUp();
	void slot_MoveDown();
	void slot_DeleteName();
	void slot_CustomContextMenuRequested(const QPoint &pos);

private:
	QColor				m_SelectedColor;
	HObject				ROIRegion;			//ROI形状区域


};