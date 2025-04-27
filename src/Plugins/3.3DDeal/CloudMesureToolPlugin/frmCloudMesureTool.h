#pragma once

#include <QDialog>
#include "ui_frmCloudMesureTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "CloudMesureTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"
#include "../../frmBaseTool.h"

#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

class QtVariantProperty;
class QtProperty;

const int iMinRadius = 8;

class frmCloudMesureTool : public frmBaseTool
{
	Q_OBJECT
	//必然包含的组件函数和变量
public:
	frmCloudMesureTool(QWidget* parent = Q_NULLPTR);
	~frmCloudMesureTool();

	using MesureRegion = tagCloudMesure;
private:
	Ui::frmCloudMesureToolClass ui;
public:
	int	PrExecute();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	int	ExecuteComplete();
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
	//ROI1
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();
	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();

	//ROI2
	void on_btnAddROI1_clicked();
	void on_btnDeleteROI1Select_clicked();
	void on_btnDeleteROI1All_clicked();
	void on_btnLink1Content_clicked();
	void on_btnDelLink1Content_clicked();
	//更改
	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);
private slots:
	void on_UpDateCheckType(int Index);
	void slot_valueChanged(QtProperty *property, const QVariant &value);
	void slot_Result_valueChanged(QtProperty *property, const QVariant &value);
	void slot_ChangeMatchImg_Index(int Index);
	void slot_ChangeROITabIndex(int Index);
	void slot_tabWidget_ChangIndex(int Index);
	void slot_ChangeRegionTabIndex(int Index);
	void slot_ChangeALargrimType(int Index);
	void slot_ChangeSpanSlider(int lower, int upper);
private:
	void ChangeRoiIndex(int iIndex);
	void ChangeParamIndex(int iIndex);
protected:
	void resizeEvent(QResizeEvent * ev) override;
private:
	//int								m_iChanal = 0;
	QVector<BaseItem*>					m_VecBaseItem;	//ROI
	QVector<BaseItem*>					m_VecROI2Item;	//ROI2
	//QGraphicsViews*						m_view = nullptr;
	HObject								ModelROIRegion;		//ROI形状区域
	HObject								ModelROI1Region;	//ROI形状区域
	HObject								SearchROIRegion;	//ROI形状区域
	QVector<MesureRegion>				m_vecMesureRegion;
private:	//结果控件
	class QtVariantPropertyManager*		m_ResultvariantManager		= nullptr;
	class QtEnumPropertyManager*		m_ResultEnumPropertyManager = nullptr;
	class QtTreePropertyBrowser	*		m_ResultpropertyEditor		= nullptr;
	class QtVariantEditorFactory*		m_ResultvariantFactory		= nullptr;
	class QtEnumEditorFactory*			m_ResultEnumEditorFactory	= nullptr;
	QMap<QtProperty*, QString>			m_ResultpropertyToId;
	QMap<QString, QtVariantProperty*>	m_ResultidToProperty;
	QMap<QString, bool>					m_ResultidToExpanded;
	void addResultProperty(QtVariantProperty *property, const QString &id);
	bool m_bValueChange = true;
private:	//
	class QtVariantPropertyManager*		variantManager = nullptr;
	class QtTreePropertyBrowser*		propertyEditor = nullptr;
	class QtVariantEditorFactory*		variantFactory = nullptr;

	QMap<QtProperty*, QString>			propertyToId;
	QMap<QString, QtVariantProperty*>	idToProperty;

	void addProperty(QtVariantProperty *property, const QString &id);
};