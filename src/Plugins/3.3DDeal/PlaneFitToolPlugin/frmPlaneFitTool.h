#pragma once

#include <QDialog>
#include "ui_frmPlaneFitTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "PlaneFitTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"
#include "frmBaseTool.h"

class QtVariantProperty;
class QtProperty;

class frmPlaneFitTool : public frmBaseTool
{
	Q_OBJECT

//必然包含的组件函数和变量
public:
	frmPlaneFitTool(QWidget* parent = Q_NULLPTR);
	~frmPlaneFitTool();
private:
	Ui::frmPlaneFitToolClass ui;
public:
	int	PrExecute();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	virtual int	ExecuteComplete();
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initShapeProperty();
	void initResultProperty();
	void initShowProperty();
	void initConnect();
	void initData();
	void ClearROIProperty();
//控件信号函数
private slots:
	void slot_MouseClicked(QList<QGraphicsItem *>& item);
	void slot_MouseMouve(QList<QGraphicsItem *>& item);
private:		
	QButtonGroup* btnGroupRadio = nullptr;

private slots:
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();

	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();

	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);
private slots:
	void on_UpDateCheckType(int Index);
	void slot_valueChanged(QtProperty *property, const QVariant &value);
	void slot_Result_valueChanged(QtProperty *property, const QVariant &value);
	void slot_ChangeMatchImg_Index(int Index);
protected:
	void resizeEvent(QResizeEvent * ev) override;
private:
	QVector<BaseItem*>	m_VecBaseItem;
	HObject				ModelROIRegion;			//ROI形状区域
	HObject				SearchROIRegion;	//ROI形状区域
private:	//结果控件
	class QtVariantPropertyManager*		m_ResultvariantManager = nullptr;
	class QtTreePropertyBrowser	*		m_ResultpropertyEditor = nullptr;
	class QtVariantEditorFactory*		m_ResultvariantFactory = nullptr;
	QMap<QtProperty*, QString>			m_ResultpropertyToId;
	QMap<QString, QtVariantProperty*>	m_ResultidToProperty;
	void addResultProperty(QtVariantProperty *property, const QString &id);
private:	//
	class QtVariantPropertyManager*		variantManager = nullptr;
	class QtTreePropertyBrowser*		propertyEditor = nullptr;
	class QtVariantEditorFactory*		variantFactory = nullptr;
	QMap<QtProperty*, QString>			propertyToId;
	QMap<QString, QtVariantProperty*>	idToProperty;
	void addProperty(QtVariantProperty *property, const QString &id);
};