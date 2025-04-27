#pragma once

#include <QDialog>
#include "ui_frmImageShow.h"
//#include <opencv2\opencv.hpp>
#include "HalconCpp.h"
using namespace HalconCpp;


#include <QButtonGroup>
#include "mytitlebar.h"
#include "../../ImageShow/QGraphicsViews.h"
#include "ShowImage.h"
#include <QMap>
#include <QMutex>

class QtVariantProperty;
class QtProperty;

#include "frmBaseTool.h"
class frmImageShow : public frmBaseTool
{
	Q_OBJECT
public:
	frmImageShow(QWidget* parent = Q_NULLPTR);
	~frmImageShow();

private:
	Ui::frmImageShowClass ui;
public:
	EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
	void ShowImg();
private:
	void initTitleBar();
	void initConnect();
public:		//和窗口相关
	void ClearParam1D();
	void UpDataCameraParams(bool bCameraopened = false);

private slots:
	void on_btnAdd_clicked();
	void on_btnDelete_clicked();
protected:
	QString				toolTitleName;
	QButtonGroup*		btnGroupRadio = nullptr;
private slots:
	void valueChanged(QtProperty *property, const QVariant &value);
	void slot_btnGroupClicked(int );
private:
	int						time_out = 10000;
	//用来显示相关的图形
	HObject					m_srcImage;
	HObject					m_dstImage;
private:
	void addProperty(QtVariantProperty *property, const QString &id);

	class QtVariantPropertyManager	*variantManager;
	class QtTreePropertyBrowser		*propertyEditor;
	class QtVariantEditorFactory	*variantFactory;

	QMap<QtProperty *, QString> propertyToId;
	QMap<QString, QtVariantProperty *> idToProperty;
};