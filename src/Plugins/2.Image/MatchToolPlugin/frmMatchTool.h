#pragma once

#include <QDialog>
#include "ui_frmMatchTool.h"
#include <QButtonGroup>
#include "MatchTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"

class QtVariantProperty;
class QtProperty;

#include "../../frmBaseTool.h"
class frmMatchTool : public frmBaseTool
{
	Q_OBJECT

public:
	frmMatchTool(QWidget* parent = Q_NULLPTR);
	~frmMatchTool();
private:
	Ui::frmMatchToolClass ui;
public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
private:
	void initTitleBar();
	void initShapeProperty();
	void initResultProperty();
	void initConnect();
	void initData();
	void ClearROIProperty();
	void ClearMatchShapeModel();
	void ClearMatchNCCModel();
//控件信号函数
private slots:
	void slot_MouseClicked(QList<QGraphicsItem *>& item);
	void slot_MouseMouve(QList<QGraphicsItem *>& item);

	int slot_CreateShapeModel(MiddleParam& param, QString& strError);
	int slot_FindShapeModel(MiddleParam& param, QString& strError);

private slots:
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();

	void on_btnAddSearchROI_clicked();
	void on_btnDeleteSearchROISelect_clicked();
	void on_btnDeleteSearchROIAll_clicked();

	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);

	void slot_SetLowContrastChanged(int Index);
	void slot_SetHighContrastChanged(int Index);

	void slot_SetGreedyChanged(double value);
	void slot_SetMinOverLapChanged(double value);
	void slot_SetMinScoreChanged(double value);

	void slot_ChangeTab(int iTab);
	void slot_btnGroupClicked(int);

	void slot_ChangeMatchType(int);

private slots:
	void slot_ChangeMatchImg_Index(int Index);
	void slot_valueChanged(QtProperty *property, const QVariant &value);

private:
	QButtonGroup*		btnGroupRadio;
	QVector<sDrawCross>	m_VecCreateCross;
	QVector<BaseItem*>	m_VecBaseItem;
	QVector<BaseItem*>	m_VecSearchBaseItem;
	QColor				m_SelectedColor;
	HObject				ModelROIRegion;		//ROI形状区域
	HObject				SearchROIRegion;	//ROI形状区域
	HTuple				m_hShapeModel;
	HTuple				m_hNCCModel;
private:
	class QtVariantPropertyManager	*variantManager;
	class QtTreePropertyBrowser		*propertyEditor;
	class QtVariantEditorFactory	*variantFactory;

	QMap<QtProperty *, QString>			propertyToId;
	QMap<QString, QtVariantProperty *>	idToProperty;
	void addProperty(QtVariantProperty *property, const QString &id);
};