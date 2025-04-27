#pragma once

#include <QDialog>
#include "ui_frmOutLineDetectTool.h"
#include <QButtonGroup>
#include <QWidgetAction>
#include "mytitlebar.h"
#include "OutLineDetectTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;
#include <QButtonGroup>
#include "ImageShow/QGraphicsViews.h"
#include "../../frmBaseTool.h"
#include <mutex>

class QtVariantProperty;
class QtProperty;
#define	showregion		"showregion"
#define	showDrawData	"showDrawData"
enum EnumPortCheckType
{
	EnumTypePoint,
	EnumTypePointPointCenter,
	EnumTypeLineCross,
	EnumTypeLineContourCross,
	EnumTypeLine,
	EnumTypeHorizontalLine,		//水平直线
	EnumTypeLineLineCross,
	EnumTypeCircle,
	EnumTypePointPointLine,
	EnumTypeverticalLine,			//垂线
	EnumTypepointStroke,			//垂点
	EnumTypeLineCircleCross,
	EnumTypeHeightDifference,		//高度差
	EnumTypeLineContourDistance,	//线到轮廓的距离
	EnumTypeWidth,
	EnumTypeSectionalArea,		//截面积
	EnumTypePointPointDistance,
	EnumTypePointLineDistance,
	EnumTypeLineLineAngle,
	EnumTypeLineContourCrossNum,	//线和轮廓的交点总数

	EnumTypeConnect,	//线和轮廓的交点总数
	EnumTypeLink,	//线和轮廓的交点总数
	EnumTypeUpLimit,
	EnumTypeDownLimit,
	EnumTypeCalculateMode,
};


class frmOutLineDetectTool : public frmBaseTool
{
	Q_OBJECT
	//必然包含的组件函数和变量
public:
	frmOutLineDetectTool(QWidget* parent = Q_NULLPTR);
	~frmOutLineDetectTool();
private:
	void initCustomplot();
	void initContourParam();
	void initTitleBar();
	void initRoiShapeProperty();
	void initResultProperty();
	void initParamProperty();
	void initJudgeProperty();
	void initConnect();
	void initData();
	void ChangeLanguage();
private:
	Ui::frmOutLineDetectToolClass ui;
	std::mutex			m_Lock;
	QButtonGroup*		btnGroupRadio = nullptr;
public:
	int Load();
	int Save();
	int Execute(const QString toolname = "");
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	//控件信号函数
private slots:
	void slot_MouseClicked(QList<QGraphicsItem *>& item);
	void slot_MouseMouve(QList<QGraphicsItem *>& item);
public:
	void ChangeGraphicsItemInFo(QList<QGraphicsItem *>& item);

private slots:
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();
	void on_btnLinkContent_clicked();
	void on_btnDelLinkContent_clicked();
	//检测
	void on_btnSub_clicked();
	void on_btnMoveUp_clicked();
	void on_btnMoveDown_clicked();
	void onChannelClicked(int Index);
	void onRoiWidthChanged(int Index);
	void slot_OutLineContextMenuRequested(const QPoint &pos);
private slots:
	void slot_Contour_PopMenu();
	void on_UpDateCheckType(int Index);
	void slot_Roi_valueChanged(QtProperty *property, const QVariant &value);
	void slot_Param_valueChanged(QtProperty *property, const QVariant &value);
	void slot_JudgeLimit_valueChanged(QtProperty *property, const QVariant &value);
	void slot_ChangeMatchImg_Index(int Index);
	//绘图
	void slot_UpdateImgContour(QList<QGraphicsItem*>& items);
	void slot_customContextMenuRequested(const QPoint &pos);
	void slot_customplot_FitAxis();
	void slot_customplot_ZoomMode();
	void slot_customplot_DragMode();
	void slot_customplot_SelectMode();
	void slot_customplot_SetParamMode();
	void slot_customplot_ResetLimit();
	void slot_customplot_XxwTraceRect(XxwTraceRect*traceRect);
	//创建
	void slot_Menu_ADDAction();
	void slot_TableWidget_cellClicked(int row, int column);
	void slot_ROITableWidget_cellClicked(int row, int column);
	void slot_TableWidget_pressed(const QModelIndex &index);
	void slot_ROITableWidget_pressed(const QModelIndex &index);
	void slot_TableWidget_ChangedTraceRect(XxwTraceRect*traceRect);
	void slot_TableWidget_ChangedItemLine(XxwDetectItemLine*ItemLine);
	void slot_ItemEnter(QTableWidgetItem* item);
private:
	void ChangeJudgeLimitProperty(QString strName = "", QString strData = "");
	void ChangeParamProperty(QString strName = "",QString strData = "");
	void UpdateMeasureResult();
	void ClearROIProperty();
	void ClearParamProperty();
	void ClearJudgeProperty();

	int CheckImgContourPoint(int iCheckType,double xValue,double xRange,QVector<double>& x,QVector<double>& y,int iIndex = 0);
	int CheckImgContourLine(int iCheckType, double xValue, double xRange,QLineF& line, int iIndex = 0);
	int CheckImgContourCircle(int iCheckType, double xValue, double xRange, MCircle& circle, int iIndex = 0);
private:	//当前测量结果分别区分出来
	int CheckImgDetectPoint(QString strName, int Currentrow, QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray,	QStringList&strLimitValues, QStringList&strLimitParams, QPointF & pt);
	int CheckImgDetectLine(QString strName, int Currentrow, QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray,	QStringList&strLimitValues, QStringList&strLimitParams, QLineF & line);
	int CheckImgDetectCircle(QString strName, int Currentrow, QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams, MCircle & line);
private:
	int GetReferencePointResult(QString strName, int Currentrow, QStringList strLst, int iParamIndex, QPointF & pt);	//获取关联点的结果
	int GetReferenceLineResult(QString strName, int Currentrow, QStringList strLst, int iParamIndex, QLineF & pt);	//获取关联点的结果
	int GetReferenceCircleResult(QString strName,int Currentrow,QStringList strLst,int iParamIndex,MCircle& ptCircle);	//获取关联点的结果
private:
	bool CheckSafetableWidget(int row,int col);
	QStringList GetTableWidgetFilterNames(QString strName,int Currentrow);
	int ChangeCurrentTableWidgetText(int irow,QStringList strParamArray,bool iIsLimit,int iRegionType,QStringList strLimitArray, QStringList&strLimitValues, QStringList strLimitParams);
	int GetTableWidgetText(int irow, QStringList&strParamArray,bool&iIsLimit,int&iRegionType,QStringList&strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams);
	QString GetTableWidgetText(int iRow);

	int GetTableWidgetText(QString strText, QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray, QStringList&strLimitValues,QStringList&strLimitParams);
	int SetTableWidgetText(QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams, QString& strText);
	int GetTableWidgetNameToData(QString strName,int& iRow,QString& strData);
	void ChangeLimitParam(int iIndex, XxwTraceRect::LimitType type, bool bReset, QStringList _strLimitArray);
	void GetLimitParam(int iIndex, int& iRegionType, bool& iIsLimit, QStringList& strLimitArray);
private:
	void ChangeOutLineData(int irow);
	void DeleteAllOutLineData();
	void DeleteSelectedOutLineData();

protected:
	void resizeEvent(QResizeEvent * ev) override;
	virtual void showEvent(QShowEvent *event);
	virtual void hideEvent(QHideEvent *event);
	void ResizeUI();
public:
	static QMap<EnumPortCheckType, QString> PortCheckTypeMap;
private:	
	QVector<CheckOutLineData>			m_VecCheckOutLineData;		//检测
private:
	QWidgetAction*						m_btnHintAction		= nullptr;		//指士线
	QPushButton*						m_btnHintAction_buf = nullptr;		//指士线
	QVector<BaseItem*>					m_VecBaseItem;
	HObject								ModelRoiRegion;					//ROI形状区域
	HObject								SearchRoiRegion;				//ROI形状区域
private:	//正在运行的参数
	HTuple								m_hRow, m_hCol, m_hGrayval;		//线上的参数
	bool								m_bRemovePoint;					//去除杂点
	bool								CheckPoint(double &fValue);
private:
	CheckManager						m_CheckManager;
	void								UpdateCheckManager();
private:	//参数判定条件
	class QtVariantPropertyManager*		m_JudgeLimitvariantManager = nullptr;
	class QtTreePropertyBrowser*		m_JudgeLimitpropertyEditor = nullptr;
	class QtVariantEditorFactory*		m_JudgeLimitvariantFactory = nullptr;
	QMap<QtProperty*,QString>			m_JudgeLimitpropertyToId;
	QMap<QString,QtVariantProperty*>	m_JudgeLimitidToProperty;
	QVector<QString>					m_JudgeLimitpropertyID;
	void addJudgeLimitProperty(QtVariantProperty *property, const QString &id);
private:	//参数控件
	class QtVariantPropertyManager*		m_ParamvariantManager = nullptr;
	class QtTreePropertyBrowser*		m_ParampropertyEditor = nullptr;
	class QtVariantEditorFactory*		m_ParamvariantFactory = nullptr;
	QMap<QtProperty*,QString>			m_ParampropertyToId;
	QMap<QString,QtVariantProperty*>	m_ParamidToProperty;
	QVector<QString>					m_VecParampropertyID;
	//QVector<QString>					m_VecData;
	void ChangeParamValue(QString strParam, QString strValue);
	void InsertParamValue(int index,QString strParam,QStringList strValues, QtProperty *proper);
	void DeleteParamProperty(QString strParam);
	void addParamProperty(QtVariantProperty *property, const QString &id);
	QMap<QString, QVector<int>>			m_MapVectorParam;	//用作创建工具防止重复
	bool bPreventReaptUpdateMeasure;						//用作重复刷新结果
private:	//ROI
	class QtVariantPropertyManager*		m_variantManager = nullptr;
	class QtTreePropertyBrowser*		m_propertyEditor = nullptr;
	class QtVariantEditorFactory*		m_variantFactory = nullptr;
	QMap<QtProperty*, QString>			m_propertyToId;
	QMap<QString, QtVariantProperty*>	m_idToProperty;
	void addProperty(QtVariantProperty *property, const QString &id);
};