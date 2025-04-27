#pragma once

#include <QWidget>
#include <QImage>
#include "ui_QtVisionWidget.h"
#include "Pages/QtWidgetsBase.h"
#include "ImageShow/QGraphicsViews.h"
#include <QtConcurrent>
#include "Log/MyEvent.h"
#include "Data/dataVar.h"

#include "Camerainterface.h"
#include <QRunnable>
#include <QThreadPool>
#include "halconcpp\HalconCpp.h"
using namespace HalconCpp;
class QtVisionWidget;

class VisionTask : public QRunnable {
public:
	QtVisionWidget*	m_ptrVision{	nullptr	};
	HalconCpp::HObject		m_hImage;
	int	iImgWidth;
	int	iImgHeight;
	void run() override;
public:
	int RecieveImgBuffer(QString strCamName, unsigned char* gray, int iwidth, int iheight, int ibit, QString imgtype, int icount);
};
struct VisionStruct
{
	int	iCodeType;
	int iCode;
	int iExposure;
	double dGain;
};
struct ZIFUVisionStruct
{
	double dScore;
};


class QtVisionWidget : public QtWidgetsBase,public CameraStream
{
	Q_OBJECT
public:
	QtVisionWidget(QWidget *parent = Q_NULLPTR);
	~QtVisionWidget();
public:
	QImage ByteToQImage(unsigned char* bytes, int width, int height, int bytesPerLine, QImage::Format imageFormat);
	
	virtual int RecieveBuffer(QString strCamName, unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

	//当使用当前接口时8为使用Rgray 使用 其他都应该Ggray Bgray为nullptr 
	virtual int RecieveRGBBuffer(QString strCamName, unsigned char* Rgray, unsigned char* Ggray, unsigned char* Bgray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount);

public:
	void CheckConConnected();
	void ConnectCamera();
	virtual void initUI();
	virtual bool CheckPageAvalible();
	virtual int CloseUI();
	virtual int initData(QString& strError);

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);
	virtual int	NewProject();
	virtual void InitConnect();

	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);
	bool CheckCylinderSafety();

	int ShapesToRegion(QVector<BaseItem*>&, HObject &Regions);
	int ShapeToRegion(BaseItem*&, HObject &Regions);
	void TableToStruct(QTableWidget* table,QVector<VisionStruct>& vecVision);
	void TableToZIFUStruct(QTableWidget* table, QVector<ZIFUVisionStruct>& vecVision);

signals:
	void SigSetValue(int index,double dRobotX, double dRobotY, double dImgX, double dImgY );
public slots :
	void slotMouseClicked(QList<QGraphicsItem *>& item);

private slots :
	void on_btnTest_clicked();
	void on_btnConnect_clicked();
	void on_btnTrrigerlight_clicked();
	void slotUpDateCameras();
	void slotUpDateExposure(int value);
	void slotUpDateGain(int value);
	void slotUpDateLight();

	void slotUpDateParam(int index);
	void UpDateParam();

	void ClearMatchShapeModel();
	void ClearMatchNCCModel();
	void ClearTableWidget();

	void slotSetValue(int index, double dRobotX, double dRobotY, double dImgX, double dImgY);
private slots :
	void on_btnAddROI_clicked();
	void on_btnDeleteROISelect_clicked();
	void on_btnDeleteROIAll_clicked();

	void on_btnAddSearchROI_clicked();
	void on_btnDeleteSearchROISelect_clicked();
	void on_btnDeleteSearchROIAll_clicked();

	void on_btnCreateModel_clicked();
	void on_btnFindModel_clicked();

	void slot_ChangeMatchType(int);
	void slot_btnGroupClicked(int);
	void slot_ChangeMarkModelTab(int iTab);

	void onRoiWidthChanged(int Index);
	void slot_SetGreedyChanged(double value);
	void slot_SetMinOverLapChanged(double value);
	void slot_SetMinScoreChanged(double value);

	void slot_ChangeTab(int iTab);
private slots :
	void on_btnAddCodeROI_clicked();
	void on_btnDeleteCodeROISelect_clicked();
	void on_btnDeleteCodeROIAll_clicked();
	void on_btnCodeIdentify_clicked();
	void slotChangeCodeType(int value);
	void slot_cellClicked(int row, int column);
private slots :
	void on_btnAddZIFUROI_clicked();
	void on_btnDeleteZIFUROISelect_clicked();
	void on_btnDeleteZIFUROIAll_clicked();
	void on_btnCreateZIFUIdentify_clicked();
	void on_btnZIFUIdentify_clicked();
protected:
	virtual	bool event(QEvent *ev) override;
	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);
	void ResizeUI();
	void UpdateIndex();

private:
	Ui::QtVisionWidgetClass ui;
	Camerainterface*		m_pCamera{ nullptr };
	QGraphicsViews*			m_pMarkGraphViews{ nullptr };

private: //MarkA
	QButtonGroup*			m_pBtnGroupRadio{ nullptr };
	QVector<BaseItem*>		m_VecBaseItem;
	QVector<BaseItem*>		m_VecSearchBaseItem;
	QVector<BaseItem*>		m_VecCodeBaseItem;
	QVector<BaseItem*>		m_VecZIFUBaseItem;
	QVector<sDrawCross>		m_VecCreateCross;

	HObject					m_ModelROIRegion;		//ROI形状区域
	HObject					m_SearchROIRegion;		//ROI形状区域
	HTuple					m_hShapeModel;
	HTuple					m_hNCCModel;
	QVector<CommonData>		m_vecZIFUData;		//公共数据ROI
	QVector<CommonData>		m_vecCodeData;			//公共数据ROI
	QVector<CommonData>		m_vecCommonData;		//公共数据ROI
	QVector<CommonData>		m_vecSearchData;		//公共数据ROI
	ResultPos				m_MarkAResultPos;
	QCreateEvent			m_QCreateEvent;


public:
	int Excute(HalconCpp::HObject& image);
	int						m_iIndex;
	CameraParam				m_param;
	ResultRetn				m_ResultRetn;
	HalconCpp::HTuple		m_hSuckHomMat;
	HObject					m_dstImage;

	CameraResultParam		m_MatchResult;
	CameraParam				m_MarkBparam;	
	//Point3D				m_MarkCalibrationPos;


public:
	bool m_bDealVisionMode = false;
	void SetDealMode(bool flag = false) {	m_bDealVisionMode = flag;	}
};