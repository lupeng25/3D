#pragma once

#include <QDialog>
#include "ui_frmImageGetSource.h"
//#include <opencv2\opencv.hpp>
#include "HalconCpp.h"
using namespace HalconCpp;

#include <QButtonGroup>
#include "mytitlebar.h"
#include "../../ImageShow/QGraphicsViews.h"
#include "GetImage.h"
#include <QMap>
#include <QMutex>

#include "frmBaseTool.h"

class QtVariantProperty;
class QtProperty;

class frmImageGetSource : public frmBaseTool
{
	Q_OBJECT
public:
	frmImageGetSource(QWidget* parent = Q_NULLPTR);
	~frmImageGetSource();
	struct ImgResult {
		EnumNodeResult	eResult;
		QString			strError;
	};
private:
	Ui::frmImageGetSourceClass ui;
public:
	virtual int	PrExecute();
	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);
	virtual int	ExecuteComplete();
public:
	int Load();
	int Save();
public:
	void InitCamera();
	void SetCameraConnected();
	void ShowImg();
private:
	void initTitleBar();
	void initCameraProperty();
	void initConnect();
	void initData();
	void ClearCameraProperty();
	void ResetView(bool bVisiable = false);
public:	//和窗口相关
	void refreshAll(bool only2D = false);
private slots:
	void onButton3DShowClicked();
	void onChannelClicked(int Index);
	void UpdateCloudPoint();
	void UpdateLimitCloudPoint(QWidget*parent);
	void slot_Save();
protected:
	virtual bool event(QEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);
private slots:
	void on_btnConnect_clicked();
	void on_btnSelectImage_clicked();
	void on_btnSelectPath_clicked();
	void on_btnUpdata_clicked();
	void on_comboCamera_currentIndexChanged(int index);
	void contextMenuEvent(QContextMenuEvent *event)override;
	void UpDateColorClound();		//显示颜色
	void initialVtkWidget();	//初始化控件
private slots:
	void valueChanged(QtProperty *property, const QVariant &value);
	void slot_btnGroupClicked(int);
	void slot_MouseMenuClicked(QString strkey);
	void slot_camShowChanged(int index);
	void slot_action_reset_triggered();
private:
	void UpDataCameraParams(bool bCameraopened = false);
	void UpDateCameraView();
private:	
	QWidget*				pViewWidget				= nullptr;
	QButtonGroup*			pBtnGroupRadio			= nullptr;

	QPushButton*			m_actionShowColors		= nullptr;
	QPushButton*			actionShowNormals		= nullptr;
	QPushButton*			actionShowMaterials		= nullptr;

	QPushButton*			actionShowScalarField	= nullptr;
	QPushButton*			actionShowColorRamp		= nullptr;
private:					//限位
	bool					m_bEnableUpLimit	= true;
	double					m_iUpLimit			= 100;
	bool					m_bEnableDownLimit	= true;
	double					m_iDownLimit		= -100;
	double					m_dScaleX			= 1;
	double					m_dScaleY			= 1;
	double					m_dScaleZ			= 1;
	double					m_dCurrentScaleX	= 1;
	double					m_dCurrentScaleY	= 1;
	double					m_dCurrentScaleZ	= 1;

private:
	QStringList				mImgNames;
	int						imgIndex;
	int						time_out = 10000;
	//用来显示相关的图形
	HObject					dstHeightImage;

private:
	//int	m_iChanal;
	bool m_bWndDelete = false;
	bool m_bNeedChangeCloud = false;
private:			//相机参数
	class QtVariantPropertyManager*		variantManager;
	class QtTreePropertyBrowser*		propertyEditor;
	class QtVariantEditorFactory*		variantFactory;
	QMap<QtProperty*, QString>			propertyToId;
	QMap<QString,QtVariantProperty*>	idToProperty;
	void addProperty(QtVariantProperty *property, const QString &id);
public:
	Camerainterface*	m_PtrCamera = nullptr;
public:
	std::mutex			m_Lock;
};