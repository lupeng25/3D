#pragma once
#include "../ImageShow/QGraphicsViews.h"
#include <QThread>
#include "Port.h"
#include "HalconCpp.h"
using namespace HalconCpp;

class _BASEITEM_API_ ImgGraphicsViews : public QGraphicsViews
{
	Q_OBJECT
public:
	ImgGraphicsViews(QWidget* parent = nullptr);
	~ImgGraphicsViews();
public:		//显示
	QImage	HObject2QImage(const HObject& hObject);
	void	HObjectToQImage(const HObject& hObject, QImage& img);

	static	void scale_image_range(HObject ho_Image, HObject *ho_ImageScaled);
public slots:		//显示
	void slot_MouseMoveInImg(QPoint pos, QString & imgtype, QString & strInfo);

public:		//显示
	void ClearShowAllImage();
	void CloneParams(MiddleParam& params);
	static void HObject2ToQImage(const HalconCpp::HObject &Hobj, QImage& image); //Halcon中的HObject类型转QImage类型

	virtual	bool event(QEvent *event) override;
public:
	QMutex			m_ShowLock;
private:
	QString			m_strShowImg;
	int				m_iShowIndex;
};