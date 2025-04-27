#pragma once

#include <QDialog>
#include "ui_frmGlueMeasureTool.h"
#include <QButtonGroup>
#include "mytitlebar.h"
#include "GlueMeasureTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"
#include "../../frmBaseTool.h"

class QtVariantProperty;
class QtProperty;

class frmGlueMeasureTool : public frmBaseTool
{
	Q_OBJECT

	//必然包含的组件函数和变量
public:
	frmGlueMeasureTool(QWidget* parent = Q_NULLPTR);
	~frmGlueMeasureTool();
private:
	Ui::frmGlueMeasureToolClass ui;
public:
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
public:
	int Load();
	int Save();
	int	PrExecute();
	int	ExecuteComplete();
private:
	void initCustomplot();
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

	void slot_mousePressEvent	(QMouseEvent* ev);
	void slot_mouseMoveEvent	(QMouseEvent* ev);
	void slot_mouseReleaseEvent	(QMouseEvent* ev);
private:		
	QButtonGroup* btnGroupRadio = nullptr;
	QVector<QVector<MeasureRect2>> ShapeToLine(QVector<BaseItem*>& );

private slots:
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();

	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();

	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);
	void onCheckTypeChanged(int Index);

	void onROIRectChanged(int Index);
private slots:
	void on_UpDateCheckType(int Index);
	void slot_valueChanged(QtProperty *property, const QVariant &value);
	void slot_Result_valueChanged(QtProperty *property, const QVariant &value);
	void slot_ChangeMatchImg_Index(int Index);
	void slot_customContextMenuRequested(const QPoint &pos);

	void slot_customplot_FitAxis();
	void slot_customplot_ZoomMode();
	void slot_customplot_DragMode();
	void slot_customplot_SelectMode();
	void slot_customplot_SetParamMode();
	void slot_customplot_ResetLimit();

	void onChangeCustomplotContour(int iIndex, QVector<MeasureRect2>&	_lines);
	QList<QPointF> lineRectIntersection(const QLineF &line, const QRectF &rect);


protected:
	void resizeEvent(QResizeEvent * ev) override;
private:
	//int					m_iChanal = 0;
	QVector<BaseItem*>	m_VecBaseItem;
	//QGraphicsViews*		m_view = nullptr;
	HObject				ModelROIRegion;			//ROI形状区域
	HObject				SearchROIRegion;	//ROI形状区域
	//QList<QLineF>		m_BezierLine;
	QMenu*				m_QMenu = nullptr;	
	QWidgetAction*		m_btnHintAction = nullptr;		//指士线
	QPushButton*		m_btnHintAction_buf = nullptr;		//指士线

	HTuple				m_hRow, m_hCol, m_hGrayval;		//线上的参数

private:	//结果控件
	class QtVariantPropertyManager*		m_ResultvariantManager = nullptr;
	class QtTreePropertyBrowser	*		m_ResultpropertyEditor = nullptr;
	class QtVariantEditorFactory*		m_ResultvariantFactory = nullptr;
	QMap<QtProperty*, QString>			m_ResultpropertyToId;
	QMap<QString, QtVariantProperty*>	m_ResultidToProperty;
private:	//
	class QtVariantPropertyManager*		variantManager = nullptr;
	class QtTreePropertyBrowser*		propertyEditor = nullptr;
	class QtVariantEditorFactory*		variantFactory = nullptr;

	QMap<QtProperty*, QString>			propertyToId;
	QMap<QString, QtVariantProperty*>	idToProperty;

//Q_SIGNALS:
//	void SigAddResultProperty(QtVariantProperty *property, const QString &id);
//	void SigAddProperty(QtVariantProperty *property, const QString &id);
private slots:
	//void addResultProperty(QtVariantProperty *property, const QString &id);
	void addProperty(QtVariantProperty *property, const QString &id);
};