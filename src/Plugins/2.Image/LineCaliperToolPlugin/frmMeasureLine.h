#pragma once

#include <QDialog>
#include "ui_frmMeasureLine.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "MeasureLine.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"
#include "../../frmBaseTool.h"

class QtVariantProperty;
class QtProperty;

class frmMeasureLine : public frmBaseTool
{
	Q_OBJECT

//必然包含的组件函数和变量
public:
	frmMeasureLine(QWidget* parent = Q_NULLPTR);
	~frmMeasureLine();

private:
	Ui::frmMeasureLineClass ui;

//流程函数
public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
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
	void ClearShape();
	void ClearROIProperty();
public:

public:
	QString m_strModelIndex = "";
	QString m_strTypeVariable = "";
	QString	m_strValueName = "";
public:
	static QVector<LineCaliperP> QJsonToShapeData(QJsonArray &RegionData);
private slots:
	void on_btnSetLine_clicked();
	void on_btnDeleteSelect_clicked();
	void on_btnClearLine_clicked();

	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();

	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);

private slots:
	void slot_valueChanged(QtProperty *property, const QVariant &value);
	void slot_ChangeMatchImg_Index(int);

	void slot_MouseMouve(QList<QGraphicsItem *>& item);
	void slot_MouseClicked(QList<QGraphicsItem *>& item);
protected:
	void resizeEvent(QResizeEvent * ev) override;
private:
	QVector<BaseItem*>	m_VecSearchItem;

	HObject				m_ROIRegion;	//ROI形状区域
	HTuple				m_Measurehandle;

private:	//结果控件
	class QtVariantPropertyManager*		m_ResultvariantManager = nullptr;
	class QtTreePropertyBrowser	*		m_ResultpropertyEditor = nullptr;
	class QtVariantEditorFactory*		m_ResultvariantFactory = nullptr;
	QMap<QtProperty*, QString>			m_ResultpropertyToId;
	QMap<QString, QtVariantProperty*>	m_ResultidToProperty;
	bool m_bValueChange = true;

private:	//
	class QtVariantPropertyManager*		variantManager = nullptr;
	class QtTreePropertyBrowser*		propertyEditor = nullptr;
	class QtVariantEditorFactory*		variantFactory = nullptr;

	QMap<QtProperty *, QString>			propertyToId;
	QMap<QString, QtVariantProperty *>	idToProperty;
	void addProperty(QtVariantProperty *property, const QString &id);
};