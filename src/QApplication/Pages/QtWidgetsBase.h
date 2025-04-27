#pragma once

#include <QWidget>
#include <QButtonGroup>


#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#define OfflineDebug 1
#include "ImageShow/ImageItem.h"
#include "ImageShow/QGraphicsScenes.h"
#include "ImageShow/QGraphicsViews.h"

#include "HalconCpp.h"
using namespace HalconCpp;

#include "Common/MyMessageBox.h"

enum EnumMatchModeType
{
	EnumMatchType_Shape,
	EnumMatchType_NCC,
};

struct ResultPos
{
	float	fMatchX;
	float	fMatchY;
	float	fMatchAngle;
	void Rst() {
		fMatchX = 0;
		fMatchY = 0;
		fMatchAngle = 0;
	}
	ResultPos() {
		Rst();
	}
};

struct CameraResultParam
{
	int iMatchNumber;
	QVector<ResultPos> vecPos;
	int iMatchCalNumber;
	QVector<ResultPos> vecCalPos;
	int	iImgWidth;
	int	iImgHeight;
	int iMatchResultNumber;

	void Rst() {
		iMatchCalNumber = 0;
		iMatchNumber = 0;
		iMatchResultNumber = 0;
		iImgWidth = -1;
		iImgHeight = -1;
		vecPos.clear();
		vecCalPos.clear();
	}
	CameraResultParam() {
		Rst();
	}
};


struct CameraParam
{
	bool					bSaveImage;
	bool					bSaveNGImage;
	EnumMatchModeType		m_eMatchtype;
	float					m_fStartAngle;
	float					m_fEndAngle;
	float					m_fMinScore;
	int						m_iMatchNumber;
	float					m_fMatchGreedy;
	float					m_fMatchOverLap;
	int						m_iSubPixType;
	HObject					m_ROISearchRegion;	//将ROI参数转化为形状区域
	HTuple					m_hNCCModel;
	HTuple					m_hShapeModel;
	HTuple					m_hHomMat2D;
	int						iExposure;
	double					dGain;
	QVector<BaseItem*>		m_VecBaseItem;
	QVector<BaseItem*>		m_VecSearchBaseItem;
	ResultPos				m_MarkAResultPos;

	void ClearMatchShapeModel() {
		try { ClearShapeModel(m_hShapeModel); }	catch (...) {}
	}
	void ClearMatchNCCModel() {
		try { ClearNccModel(m_hNCCModel); }	catch (...) {}
	}
	void ClearAllROIModel(QGraphicsViews* view) {
		QList<QGraphicsItem *> shapeLst = (view->scene())->items();
		for (auto iter : shapeLst)		{
			if (iter != nullptr) if (iter->type() == 10) {
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecBaseItem.size(); i++)				{
					if (m_VecBaseItem[i] == iter)					{
						_bFinded = true;
					}
				}
				if (_bFinded)				{
					m_VecBaseItem.removeOne((BaseItem*)iter);
					(view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
				}
			}
		}
		for (size_t i = 0; i < m_VecBaseItem.size(); i++)	m_VecBaseItem[i]->setIndex(i);
	}
	void ClearAllSearchROIModel(QGraphicsViews* view) {
		QList<QGraphicsItem *> shapeLst = (view->scene())->items();
		for (auto iter : shapeLst) {
			if (iter != nullptr) if (iter->type() == 10) {
				bool	_bFinded = false;
				for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++) {
					if (m_VecSearchBaseItem[i] == iter) {
						_bFinded = true;
					}
				}
				if (_bFinded) {
					m_VecSearchBaseItem.removeOne((BaseItem*)iter);
					(view->scene())->removeItem(iter);	delete iter;	iter = nullptr;
				}
			}
		}
		for (size_t i = 0; i < m_VecSearchBaseItem.size(); i++)	m_VecSearchBaseItem[i]->setIndex(i);
	}
	void Rst() {
		bSaveImage		= false;
		bSaveNGImage	= false;
		m_eMatchtype	= EnumMatchType_Shape;
		m_fStartAngle	= -90;
		m_fEndAngle		= 180;
		m_fMinScore		= 0.6;
		m_iMatchNumber	= 1;
		m_fMatchGreedy	= 0.5;
		m_fMatchOverLap = 0.5;
		m_iSubPixType	= 0;
		iExposure		= 20;
		dGain			= 1;
		m_ROISearchRegion.Clear();
		ClearMatchShapeModel();
		ClearMatchNCCModel();
		m_MarkAResultPos.Rst();
	}
	CameraParam() {
		Rst();
	}
};

class QtWidgetsBase : public QWidget
{
	Q_OBJECT
public:
	QtWidgetsBase(QWidget *parent = Q_NULLPTR);
//	virtual ~QtWidgetsBase();

protected:
	void changeEvent(QEvent * ev);
public:
	virtual bool CheckPageAvalible() = 0;
	virtual int CloseUI() = 0;
	virtual int initData(QString& strError) = 0;

	virtual	int GetData(QJsonObject& strData);
	virtual	int SetData(QJsonObject& strData);
	virtual	int CompleteUpDateData();
	virtual int	NewProject();
	virtual int	ChangeProjectComplete();

	//virtual bool CheckNextPageAvalible() = 0;
	virtual	int GetSystemData(QJsonObject& strData);
	virtual	int SetSystemData(QJsonObject& strData);

	virtual int SCanTimerUI();
	virtual void ChangeLanguage();
	virtual	QVector<QPair<QString, QString>> GetVecSetName();
};