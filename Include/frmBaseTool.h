#pragma once

#include <QDialog>
#include <QButtonGroup>

#include "basemainwindow.h"
#include "ImageShow/QGraphicsViews.h"
#include <QTableWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>

#include "FlowchartTool.h"
#include "Port.h"

class QtVariantPropertyManager;
class QtTreePropertyBrowser;
class QtVariantEditorFactory;
class QtProperty;
class QtVariantProperty;

#define	FontSize		"FontSize"
#define	showregion		"showregion"
#define	showResult		"showResult"
#define	showDrawResult	"showDrawResult"
#define	UseFollow		"UseFollow"
#define	txtLink			"txtLink"

#include "HalconCpp.h"
using namespace HalconCpp;

namespace Ui {
    class frmBaseToolClass;
} // namespace Ui

class _FLOWCHARTTOOL_API_ frmBaseTool : public BaseWindow
{
	Q_OBJECT
public:
	//void InitUI(QWidget* pWidget = Q_NULLPTR);
	static void scale_image_range(HalconCpp::HObject ho_Image, HalconCpp::HObject *ho_ImageScaled);
public:
	int ChangeTableToData(QTableWidget* pTable, QJsonArray& strData);
	int ChangeDataToTable(QJsonArray& strData, QTableWidget* pTable);
	int ShapeToRegion(BaseItem*, HalconCpp::HObject &Regions);
	int ShapeToRegion(QVector<BaseItem*>&, HalconCpp::HObject &Regions);
	int ShapeToData(QVector<BaseItem*>&, QVector<CommonData> &RegionData);
	int DataToShape(QVector<CommonData> &RegionData, QVector<BaseItem*>&vecItems);
	static int AffineShapeTrans(QVector<BaseItem*>&vecItems, const HTuple& HomMat2D, MiddleParam & param,bool bUse = false);
	static int AffineShapeTrans(QVector<CommonData>&vecRegions,const HTuple& HomMat2D,MiddleParam & param, bool bUse = false);

	static int ShapeToQJson(QVector<BaseItem*>&, QJsonArray &RegionData);
	static int QJsonToShape(QJsonArray &RegionData, QVector<BaseItem*>&);

	QString GetShapeInfo(BaseItem* Item);
	void moveRow(QTableWidget* pTable, int nFrom, int nTo);

public:		//显示
	static QImage	HObject2QImage(const HalconCpp::HObject& hObject);
	static void		HObjectToQImage(const HalconCpp::HObject& hObject, QImage& img);
	QImage			Hal2QImage(const HalconCpp::HObject& hobj);
	int				m_iChanal;
//必然包含的组件函数和变量
public:
	frmBaseTool(QWidget* parent = Q_NULLPTR);
	~frmBaseTool();

	QWidget* GetCenterWidget();

private:
	void initTitleBar();
protected:	//图像
	virtual	bool event(QEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);

	//控件信号函数
private slots:
	void onButtonCloseClicked();
	void on_btnExecute_clicked();
	void on_btnSave_clicked();
public slots:
	void slot_MouseMoveInImg(QPoint pos, QString & imgtype, QString & strInfo);
protected:	//图像
	HalconCpp::HObject	m_srcImage;
	HalconCpp::HObject	m_dstImage;
	QString				m_strCamera;
public:
	QString				m_strModelIndex = "";
	QString				m_strTypeVariable = "";
	QString				m_strValueName = "";
public:
	virtual int Load();
	virtual int Save();
	virtual int	NewProject();
	virtual int	PrExecute();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError) = 0;
	virtual int	ExecuteComplete();
	virtual void ChangeLanguage();
	void UpdateQGraphicsViews();

	FlowchartGraphicsItem*				m_pTool = nullptr;
	QGraphicsViews*						m_view = nullptr;
protected:	//结果控件
	class QtVariantPropertyManager  *	m_ShowvariantManager = nullptr;
	class QtTreePropertyBrowser     *	m_ShowpropertyEditor = nullptr;
	class QtVariantEditorFactory	*	m_ShowvariantFactory = nullptr;
	QMap<QtProperty*, QString>			m_ShowpropertyToId;
	QMap<QString, QtVariantProperty *>	m_ShowidToProperty;
	void addShowProperty(QtVariantProperty*property, const QString&id, const QString&strLanguage);
	void initShowEditorProperty(QWidget* parent, QLayout* layout);
private:
	Ui::frmBaseToolClass* ui;
	MiddleParam m_Middle_Param;
	QString m_strError;
};