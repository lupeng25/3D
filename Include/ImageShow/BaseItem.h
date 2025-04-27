#pragma once

#include <QObject>
#include <QList>
#include <mutex>
#include "Comm.h"

#include <QObject>
#include <QAbstractGraphicsShapeItem>
#include <QPointF>
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QKeyEvent>
#include <QList>
#include <QDebug>
#include <QtMath>

#ifndef _BASEITEM_EXPORT_
#define _BASEITEM_API_ _declspec(dllexport)
#else
#define _BASEITEM_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_


//******基类******
class _BASEITEM_API_ BaseItem : public QObject, public QGraphicsItemGroup
{
	Q_OBJECT
public:
	enum EnumBaseItemStr
	{
		EnumBaseItemStr_sSigma,
		EnumBaseItemStr_sThreshold,
		EnumBaseItemStr_sSelect,
		EnumBaseItemStr_sTransition,
		EnumBaseItemStr_sActiveNum,
		EnumBaseItemStr_sStartAngle,
		EnumBaseItemStr_sEndAngle,
		EnumBaseItemStr_sRadius,
	};
	BaseItem(QPointF center, ShapeItemType type);
	virtual ~BaseItem();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	virtual bool updatePos(int index, QPointF pt) = NULL;
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b,int& index);
	virtual	void SetDrawEnd();
	virtual	bool IsDrawEnd();
public:
	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue);
public:
	void PointRotate(QPointF pt,QPointF ptCenter,double dAngle,QPointF& ptResult);
	//获取贝塞尔曲线的法线
	QList<QLineF> get_beziernormalline_curves(QList<QPointF> srcPts, QList<QPointF>&dstPts, float fstep, float fEverylenth, float fEveryHeight);
public:
	qreal* scale;
	void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
	ShapeItemType GetItemType();
	void setIndex(int index) { m_Index = index; };
	int getIndex() { return m_Index; };
	EnumItemRegionType m_eRegionType = RegionType_Add;
	void SelectItem(bool bFlag = false);
	void SetLinePath(QPointF P1, QPointF P2, float flenth, QPainterPath& path);

	//获取法线
	void GetNormalLine(QPointF P1, QPointF P2, float flenth, QPointF& ResP1, QPointF& ResP2);
	void GetStartEndNormalLine(QPointF PStart, QPointF P2, float flenth, QPointF& ResP1, QPointF& ResP2);

	//获取旋转形状
	void GetLineArrowPoint(QPointF startP1, QPointF EndP2, float flenth,float fAngle, QPointF& ResP1, QPointF& ResP2);

	virtual QRectF boundingRect() ;
	static void createNBezierCurve(const QList<QPointF> &src, QList<QPointF> &dest, qreal precision);
public:
	bool	m_bSelected = false;
	bool	m_bShowMoveSelected;	//是否显示选中

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
protected:
	static QMap<EnumBaseItemStr, QString>	mapKeyString;
	QMap<QString, QString>	mapLangString;

	std::mutex		m_Lock;
	QPointF			center;
	ShapeItemType	types;
	QPen			SelectedPen;
	QPen			noSelectedPen;
	QColor			fillColor;
	QPen			thisPen;
	qreal			LineWidth = 2;  //控制点初始尺寸
	QString			ItemDiscrib = QString::fromLocal8Bit("");
	int				m_Index;
	QVector<QPointF> m_vecPoint;
};

//******绘制结果数据******
class _BASEITEM_API_ DrawItem : public BaseItem
{
	Q_OBJECT
public:
	DrawItem();
	virtual~DrawItem();
	void AddLineRect(QVector<sColorLineRect>& Plist, QColor color = QColor(0, 255, 0))		{ std::lock_guard<std::mutex> _lock(m_Lock); RectangleList.append(Plist);	DrawColor	= color;	};
	void AddRRectangle(QVector<sColorRRectangle>& Plist,QColor color = QColor(0, 255, 0))	{ std::lock_guard<std::mutex> _lock(m_Lock); RRectangleList.append(Plist);	DrawColor	= color;	};
	void AddLine(QVector<sColorLine>& Plist,	QColor color = QColor(0, 255, 0))			{ std::lock_guard<std::mutex> _lock(m_Lock); LineList.append(Plist);		DrawColor	= color;	};
	void AddRegion(QVector<QPointF>& Plist,		QColor color = QColor(0, 255, 0))			{ std::lock_guard<std::mutex> _lock(m_Lock); ROIList.push_back(Plist);		DrawColor	= color;	};
	void AddCross(QVector<sDrawCross>& Plist,	QColor color = QColor(0, 255, 0))			{ std::lock_guard<std::mutex> _lock(m_Lock); CrossList.append(Plist);		};
	void AddTextList(QVector<sDrawText>& Plist)												{ std::lock_guard<std::mutex> _lock(m_Lock); TextList.append(Plist);		};
	void AddColorPoints(QVector<sColorPoint>& Plist)										{ std::lock_guard<std::mutex> _lock(m_Lock); ColorList.append(Plist);		};
	void AddColorCircles(QVector<sColorCircle>& Plist)										{ std::lock_guard<std::mutex> _lock(m_Lock); CircleList.append(Plist); };
	void AddColorEllipse(QVector<sColorRRectangle>& Plist)									{ std::lock_guard<std::mutex> _lock(m_Lock); REllipseList.append(Plist); };
	void ClearAll()		{
		std::lock_guard<std::mutex> _lock(m_Lock);
		ColorList.clear();
		LineList.clear(); 
		RRectangleList.clear();
		RectangleList.clear();
		CrossList.clear();
		TextList.clear();
		ROIList.clear();
		CircleList.clear();
		REllipseList.clear();
	};
	void Refresh();
	void SetDispROI(QRectF& Rec) { returnRec = Rec; };
protected:
	int type() const override { return 100; };
	bool updatePos(int index, QPointF pt) { return true; };
	virtual QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
private:
	QColor	DrawColor;
	QPen	DrawPen;
	QRectF	returnRec;
	QVector<sColorPoint>		ColorList;
	QVector<sColorLine>			LineList;
	QVector<sColorRRectangle>	RRectangleList;
	QVector<sColorRRectangle>	REllipseList;
	QVector<sColorLineRect>		RectangleList;
	QVector<QVector<QPointF>>	ROIList;
	QVector<sDrawCross>			CrossList;
	QVector<sDrawText>			TextList;
	QVector<sColorCircle>		CircleList;
};

//******点******
class _BASEITEM_API_ PointItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	PointItem(qreal x, qreal y, qreal R);
	virtual ~PointItem();
	void GetCircle(MCircle& Cir);
	void SetCircle(MCircle& Cir);
	bool circle_init_state = false;
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);
	ShapeItemType m_showShape;

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	qreal Radius;
};

//******矩形******
class _BASEITEM_API_ RectangleItem : public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	RectangleItem(qreal x, qreal y, qreal width, qreal height);
	virtual~RectangleItem();
	void SetRect(MRectangle MRect);
	void GetRect(MRectangle& MRect);	
	bool rect_init_state = false;
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
};

//******旋转矩形******
class _BASEITEM_API_ RectangleRItem : public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	RectangleRItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi);
	virtual~RectangleRItem();
	void GetRotatedRect(MRotatedRect& MRRect);
	void SetRotatedRect(MRotatedRect& MRRect);
	bool rrect_init_state = false;

	void UpdateLinePath();
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;

protected:
	qreal angle = 0;
	qreal lenth1 = 0;
	qreal lenth2 = 0;
	QPointF Pa1;
	QPointF Pa2;
	QPointF Pa3;
	QPointF Pa4;
	QPointF PArrow;
	QPainterPath	QPPath;							// 箭头曲线	
};

//******多边形******
class _BASEITEM_API_ PolygonItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	PolygonItem();
	virtual ~PolygonItem();
	QPointF getCentroid(QList<QPointF> list);
	QPointF getCentroid(QVector<QPointF>& list);
	void getMaxLength();
	void GetPolygon(MPolygon& mpolygon);
	virtual void SetPolygon(MPolygon& mpolygon);
	bool polygon_init_state = false;
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual	void SetDrawEnd();
	virtual	bool IsDrawEnd();

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
public slots:
	//void pushPoint(QPointF p, QList<QPointF> list, bool isCenter);
	void PushPos(QPointF pos);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
protected:
	void UpdateLinePath();
protected:
	qreal Radius;
	bool bDrawFinished;
	QPainterPath	QPPath;							// 箭头曲线	
};

//******椭圆******
class _BASEITEM_API_ EllipseItem : public RectangleRItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	EllipseItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi);
	virtual ~EllipseItem();
public:
	virtual void SetRotatedRect(MRotatedRect& MRRect);
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
protected:
	void UpdateLinePath();
};

//******圆******
class _BASEITEM_API_ CircleItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	CircleItem(qreal x, qreal y, qreal R);
	virtual ~CircleItem();
	void GetCircle(MCircle& Cir);
	void SetCircle(MCircle& Cir);
	bool circle_init_state = false;
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
	qreal Radius;
};

//******同心圆******
class _BASEITEM_API_ ConcentricCircleItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:	
	ConcentricCircleItem(qreal x, qreal y, qreal RadiusMin, qreal RadiusMax);
	virtual ~ConcentricCircleItem();
	void GetConcentricCircle(CCircle& CCir);
	void SetConcentricCircle(CCircle& CCir);
	bool concentric_circle_init_state = false;	
	int segment_line_num = 0;
	std::vector<QPointF> ring_small_points = std::vector<QPointF>(1000);
	std::vector<QPointF> ring_big_points = std::vector<QPointF>(1000);	
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
	qreal RadiusMin;
	qreal RadiusMax;
	QPainterPath	QPPath;							// 箭头曲线	
	void UpdateLinePath();
};

//******直线******
class _BASEITEM_API_ LineItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	LineItem(qreal x1, qreal y1, qreal x2, qreal y2);
	virtual ~LineItem();
	void GetLineData(LineData&line);
	void SetLineData(LineData&line);
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
protected:
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
	QPointF P1;
	QPointF P2;
	QPainterPath	linePath;							// 箭头曲线
	void UpdateLinePath();
};

//******方向直线******
class _BASEITEM_API_ LineArrowItem :public BaseItem
{
	Q_OBJECT
public:
	LineArrowItem(qreal x1, qreal y1, qreal x2, qreal y2);
	virtual ~LineArrowItem();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

public:
	void GetLineData(LineData&line);
	void SetLineData(LineData&line);
public:
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);

protected:
	virtual QRectF boundingRect() const override;
protected:
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
private:
	void UpdateLinePath();
	QPointF P1;
	QPointF P2;
	QPainterPath	linePath;							// 箭头曲线
	QPainterPath	ArrowlinePath1;						// 箭头曲线
	QPainterPath	ArrowlinePath2;						// 箭头曲线
};

//******自由直线******
class _BASEITEM_API_ BezierLineItem :public BaseItem
{
	Q_OBJECT
public:
	BezierLineItem();
	virtual ~BezierLineItem();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

	static int ChangeParamTovecMeasure(QJsonObject &vecData, QVector<MeasureRect2>& vecMeasure);
public:
	QPointF getCentroid(QList<QPointF> list);
	QPointF getCentroid(QVector<QPointF>& list);
	void GetPolygon(MPolygon& mpolygon);
	virtual void SetPolygon(MPolygon& mpolygon);
	bool polygon_init_state = false;
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual	void SetDrawEnd();
	virtual	bool IsDrawEnd();

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);

	QList<QLineF> GetDrawLines(QList<QPointF>& points);
	QVector<MeasureRect2> GetDrawLines();

public slots:
	//void pushPoint(QPointF p, QList<QPointF> list, bool isCenter);
	void PushPos(QPointF pos);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
public:
	void UpdateLinePath();
	void UpdateMeasurePath(bool	bToScene = false);
public:
	bool bDrawFinished;
	bool bDrawNormalLine;							//Draw Normal Line

	float	fTwoLineLenth;							//单个卡尺中心距离
	float	fNormalLineHeight;
	float	fNormalLineWidth;
	bool	bNeedCheckDistance;
protected:
	QPainterPath			QNormalLinePath;			// 法线
	QPainterPath			QPPath;						// 箭头曲线
	QPainterPath			QPArrowPath;				// 箭头曲线
	double					m_dSigma;					//平滑度
	double					m_dThreshold;				//阈值
	QString					m_strTransition;			//极性
	QString					m_strSelect;				//选择
	int						m_iSelectedIndex;
	QMap<QString, QString>	m_mapParam;					//其他参数
	QVector<MeasureRect2>	m_Measurelines;
};

//******卡尺******
class _BASEITEM_API_ Caliper :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	Caliper(qreal x1, qreal y1, qreal x2, qreal y2, qreal height);
	virtual ~Caliper();
	void SetCaliper(QPointF p1, QPointF p2);
	void GetCaliper(CaliperP& CP);
	bool caliper_init_state = false;
	int segment_line_num = 0;
	std::vector<QPointF> line_small_points	= std::vector<QPointF>(1000);
	std::vector<QPointF> line_big_points	= std::vector<QPointF>(1000);
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);

	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter,const QStyleOptionGraphicsItem* option,QWidget* widget) override;
	QPointF P1;
	QPointF P2;
	qreal Height;
	qreal angle;
	qreal Lenth;
};

//******直线卡尺******
class _BASEITEM_API_ LineCaliperItem :public BaseItem
{
	Q_OBJECT
public:
	LineCaliperItem();
	LineCaliperItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi);
	LineCaliperItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, qreal height);
	virtual ~LineCaliperItem();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	void GetLineData(LineCaliperP&line);
	void SetLineData(LineCaliperP&line);
	int		segment_line_num = 10;
	qreal	segment_line_width = 10;		//高度

public:
	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);

protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	bool UpDate(int index);
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,QWidget* widget) override;
protected:
	void UpdateLinePath();
	qreal angle = 0;
	qreal lenth1 = 0;
	qreal lenth2 = 0;		//高度
	QPointF Pa1;
	QPointF Pa2;
	QPointF Pa3;
	QPointF Pa4;
	QPointF PArrow;
	QPainterPath	linePath;							// 箭头曲线

	double	m_dSigma = 1;						//平滑度
	double	m_dThreshold = 30;					//阈值
	QString	m_strTransition = "all";			//极性
	QString	m_strSelect = "all";				//选择
	qreal	m_ActiveNum = 6;

};

//******圆弧卡尺******
class _BASEITEM_API_ ArcCaliperItem :public BaseItem
{
	Q_OBJECT
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);
public:
	ArcCaliperItem(qreal x, qreal y, qreal Radius);
	ArcCaliperItem();
	virtual ~ArcCaliperItem();
	void GetArcCaliper(ArcCaliperP& CCir);
	void SetArcCaliper(ArcCaliperP& CCir);
	bool concentric_circle_init_state = false;
	int segment_line_num = 4;
	void UpDateCenter();
public:
	virtual	bool IsMouseInItem(QPointF& pt, DIRECTION & b, int& index);
	virtual	void SetDrawPenWidth(qreal lenth);
	virtual int	SetParam(QString key, QString value);
	virtual int	GetParam(QString key, QString&value);
	virtual int	GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue);
protected:
	virtual QRectF boundingRect() const override;
	bool updatePos(int index, QPointF pt) override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	QPointF GetAnglePos(qreal Radius, qreal angle);
	qreal CalculateAngle(const QPointF &p1, const QPointF &p2, const QPointF &p3);
	QPainterPath	QPPath;							// 箭头曲线	
	void UpdateLinePath();
	qreal	m_Radius;
	qreal	m_Height	= 40;
	qreal	m_Width		= 40;
	qreal	m_dStartAngle	= 30;						//平滑度
	qreal	m_dEndAngle		= 270;						//平滑度

	qreal	m_dSigma = 1;						//平滑度
	qreal	m_dThreshold = 30;					//阈值
	QString	m_strTransition = "all";			//极性
	QString	m_strSelect = "all";				//选择
	qreal	m_ActiveNum = 6;
};
