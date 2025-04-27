#pragma once

#include <QObject>
#include <QList>
#include <QUuid>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <string>
#include <memory>
using namespace std;

#ifndef _BASEREGION_EXPORT_
#define _BASEREGION_API_ _declspec(dllexport)
#else
#define _BASEREGION_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

//图像类型
const QString cstItem							= "ComItem";
const QString cstItemRadius						= "Radius";
const QString cstItemCenterPos					= "ItemCenterPos";
const QString cstItemCenterX					= "CenterX";
const QString cstItemCenterY					= "CenterY";
const QString cstItemPosX						= "PosX";
const QString cstItemPosY						= "PosY";
const QString cstItemLine						= "ItemLine";
const QString cstItemPosX1						= "X1";
const QString cstItemPosY1						= "Y1";
const QString cstItemPosX2						= "X2";
const QString cstItemPosY2						= "Y2";
const QString cstItemAngle						= "Angle";
const QString cstItemWidth						= "Width";
const QString cstItemHeight						= "Height";
const QString cstItemNumber						= "Number";
const QString cstItemLenth1						= "Lenth1";
const QString cstItemLenth2						= "Lenth2";
const QString cstItemPhi						= "Phi";
const QString cstItemSmallRadius				= "SmallRadius";
const QString cstItemBigRadius					= "BigRadius";
const QString cstItemShapeType					= "ShapeType";
const QString cstItemRegionType					= "RegionType";
const QString cstItemPos						= "Pos";
const QString cstItemPosition					= "Position";
const QString cstItemEnableNormalLine			= "EnableNormalLine";
const QString cstItemNormalLineLenth			= "NormalLineLenth";
const QString cstItemNormalLineHeight			= "NormalLineHeight";		//法线高度
const QString cstItemNormalLineWidth			= "NormalLineWidth";
const QString cstItemCheckDistance				= "CheckDistance";
const QString cstItemNormalIndex				= "NormalIndex";
const QString cstItemTwoLineLenth				= "TwoLineLenth";

const QString cstItemNormalSigma				= "NormalSigma";
const QString cstItemNormalThreshold			= "NormalThreshold";
const QString cstItemNormalTransition			= "NormalTransition";
const QString cstItemNormalSelection			= "NormalSelection";
const QString cstItemNormalSelectedIndex		= "NormalSelectedIndex";
const QString cstItemSplit						= "|";

#ifndef M_PI
#define  M_PI  3.14159265358979323846
#endif
enum ItemType
{
	Null = 0,		// 空
	Nomal,			// 代表全局常态
	Link,			// 连接线
	Rect,			// 流程
	Condition,		// 判定
};
struct FlowchartItemType
{
	ItemType	type = ItemType::Null;
};

enum FlowchartCursor
{
	ArrowCursor = 0,
	DrawLinkCursor,
	SizeAllCurSor,
	OpenHandCursor,
	ClosedHandCursor,
	CrossCursor,
	MoveCursor,
};

enum LINE_HEAD_TYPE
{
	NOLINEARROW,
	LINEARROW
};

enum DIRECTION
{
	NONE,
	NORTHWEST,
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	INRECT,
	STARTPOINT,
	ENDPOINT
};

// 图元样式信息
struct FlowchartStyleBase
{
	QPen							pen_;					// 基本画笔样式--背景相关
	QBrush							brush_;					// 基本画刷样式--背景相关
	QPen							text_pen_;				// 文本画笔样式
	QFont							font_;					// 字体样式
	FlowchartStyleBase()
	{
		pen_ = QPen();
		pen_.setColor(QColor(65, 113, 156));
		pen_.setWidth(2);

		brush_ = QBrush(QColor(89, 152, 209));

		text_pen_ = QPen();;
		text_pen_.setColor(QColor(254, 255, 255));
		text_pen_.setWidth(1);

		font_ = QFont("Microsoft YaHei", 10, 2);
	}
};

// 图元数据信息
struct FlowchartContentBase
{
	QString							id_;										// 图元id
	QString							content_;									// 图元内容
	QString							tooltip_;									// 图元提示信息

	FlowchartContentBase()
	{
		id_ = QUuid::createUuid().toString();
		content_ = "";
		tooltip_ = "";
	}
};

// 图元结构体基类
struct FlowchartInforBase
{
	int								m_iID;
	double							position_x_, position_y_, width_, height_;
	FlowchartStyleBase				item_style_;
	FlowchartContentBase			item_content_;
	FlowchartItemType				item_type_;

	FlowchartInforBase()
	{
		m_iID = 0;
		position_x_ = 0.0;
		position_y_ = 0.0;
		width_ = 120.0;
		height_ = 40.0;
		item_style_ = FlowchartStyleBase();
		item_content_ = FlowchartContentBase();
	};
	FlowchartInforBase(double _x, double _y, double _width = 120.0, double _height = 40.0) {
		m_iID = 0;
		position_x_ = _x;
		position_y_ = _y;
		width_ = _width;
		height_ = _height;
		item_style_ = FlowchartStyleBase();
		item_content_ = FlowchartContentBase();
	};
};
typedef std::vector<FlowchartInforBase>	FlowchartInforBases;

enum ShapeItemType
{
	ItemAutoItem = 0,        // 自定义类型
	ItemPoint,               // 点
	ItemCircle,              // 圆
	ItemEllipse,             // 椭圆
	ItemConcentric_Circle,   // 同心圆			
	ItemRectangle,           // 矩形
	ItemRectangleR,          // 旋转矩形
	ItemSquare,              // 正方形
	ItemPolygon,             // 多边形		
	ItemLineObj,             // 直线
	ItemArrowLineObj,        // 箭头直线
	ItemBezierLineObj,       // 贝塞尔曲线
	LineCaliperObj,			// 直线卡尺
	ArcCaliperObj,			// 圆弧卡尺
};

enum EnumItemRegionType
{
	RegionType_Add,//合并
	RegionType_Differe,//差集
};

struct sColorPoint
{
	std::vector<QPointF> vecPoints;
	QColor				 DrawQColor;
	qreal				 LineWidth;  //控制点初始尺寸
	sColorPoint()
	{
		vecPoints.clear();
		DrawQColor = QColor(0, 255, 0);
		LineWidth = 2;
	}
	sColorPoint(std::vector<QPointF> vecPt, qreal linewidth = 2, QColor color = QColor(0, 255, 0))
	{
		vecPoints = vecPt;
		DrawQColor = color;
		LineWidth = linewidth;
	}
};

struct sColorCircle
{
	QPointF				CenterPoints;
	qreal				qRadius;  //控制点初始尺寸
	QColor				DrawQColor;
	qreal				LineWidth;  //控制点初始尺寸
	void Rst() {
		CenterPoints = QPointF();
		DrawQColor = QColor(0, 255, 0);
		LineWidth = -1;
		qRadius = 25;
	}
	sColorCircle()
	{
		Rst();
	}
	sColorCircle(
		QPointF centerPt,
		qreal radius,
		qreal linewidth = -1,
		QColor color = QColor(0, 255, 0)) {
		CenterPoints = centerPt;
		qRadius = radius;
		DrawQColor = color;
		LineWidth = linewidth;
	}
};

struct sColorLineRect
{
	QLineF				Dline;
	qreal				DWidth;
	QColor				DrawQColor;
	qreal				LineWidth;  //控制点初始尺寸
	void Rst() {
		Dline = QLineF();
		DWidth = 10;
		DrawQColor = QColor(0, 255, 0);
		LineWidth = -1;
	}
	sColorLineRect() {
		Rst();
	}
	sColorLineRect(QLineF line, qreal width, qreal linewidth = -1, QColor color = QColor(0, 255, 0))
	{
		Dline = line;
		DrawQColor = color;
		LineWidth = linewidth;
	}
};

struct sColorRRectangle
{
	QPointF				Center;
	qreal				DHeight;
	qreal				DWidth;
	qreal				Angle;
	QColor				DrawQColor;
	qreal				LineWidth;  //控制点初始尺寸
	void Rst() {
		Center = QPointF();
		DHeight = 10;
		DWidth = 10;
		Angle = 0;
		DrawQColor = QColor(0, 255, 0);
		LineWidth = -1;
	}
	sColorRRectangle() {
		Rst();
	}
	sColorRRectangle(QPointF center, qreal height, qreal width, qreal linewidth = -1, QColor color = QColor(0, 255, 0))
	{
		Center = center;
		DHeight = height;
		DWidth = width;
		DrawQColor = color;
		LineWidth = linewidth;
	}
};

struct sColorLine
{
	QLineF				Dline;
	QColor				DrawQColor;
	qreal				LineWidth;  //控制点初始尺寸
	void Rst() {
		Dline = QLineF();
		LineWidth = -1;
		DrawQColor = QColor(0, 255, 0);
	}
	sColorLine() {
		Rst();
	}
	sColorLine(QLineF line, qreal linewidth = -1, QColor color = QColor(0, 255, 0))
	{
		Dline = line;
		DrawQColor = color;
		LineWidth = linewidth;
	}
};

struct sDrawText
{
	QPointF Center;
	bool	bControl;
	int		iDrawLenth;
	QColor	DrawColor;
	QString	strText;

	void Rst() {
		Center		= QPointF(0, 0);
		bControl	= true;
		iDrawLenth	= 50;
		DrawColor	= QColor(0, 255, 0);
		strText.clear();
	}
	sDrawText(){
		Rst();
	}
};

struct sDrawCross
{
	QPointF Center;
	float	fAngle;
	QString	strText;
	int		iDrawLenth;
	QColor	DrawColor;
	void Rst() {
		Center = QPointF(0, 0);
		fAngle = 0;
		iDrawLenth = 50;
		DrawColor = QColor(0, 0, 255);
		strText.clear();
	}
	sDrawCross()
	{
		Rst();
	}
};

struct sRadPoint
{
	float fCenterX;
	float fCenterY;
	float fRad;
	sRadPoint()
	{
		fRad = 0;
		fCenterX = 0;
		fCenterY = 50;
	}
	sRadPoint(float x, float y, float frad)
	{
		fRad		= frad;
		fCenterX	= x;
		fCenterY	= y;
	}
};
struct sResultCross
{
	qreal	LineCenterX;
	qreal	LineCenterY;
	qreal	LineRad;  //弧度
	sResultCross()
	{
		LineCenterX = 50;
		LineCenterY = 50;
		LineRad = 0;
	}
	sResultCross(QPointF crossPt, float fRad = 0)
	{
		LineCenterX = crossPt.x();
		LineCenterY = crossPt.y();
		LineRad = fRad;
	}
	sResultCross(float crossPtX, float crossPtY, float fRad = 0)
	{
		LineCenterX = crossPtX;
		LineCenterY = crossPtY;
		LineRad = fRad;
	}
};
//线
struct LineData
{
	LineData()
	{
	}

	LineData(float x1, float y1, float x2, float y2)
	{
		row		= y1;
		col		= x1;
		row1	= y2;
		col1	= x2;
	}
	float row;
	float col;
	float row1;
	float col1;
};

//矩形
struct MRectangle
{
	MRectangle()
	{
	}

	MRectangle(float x, float y, float Width, float Height)
	{
		row = y;
		col = x;
		width = Width;
		height = Height;
	}
	float row;
	float col;
	float width;
	float height;
};

//旋转矩形
struct MRotatedRect
{
	MRotatedRect()
	{
	}

	MRotatedRect(float x, float y, float Phi, float Lenth1, float Lenth2)
	{
		row = y;
		col = x;
		phi = Phi;
		lenth1 = Lenth1;
		lenth2 = Lenth2;
	}
	float row;
	float col;
	float phi;
	float lenth1;
	float lenth2;
};

//圆
struct MCircle
{
	MCircle()
	{
	}

	MCircle(float x, float y, float Radius,int index = 0)
	{
		row = y;
		col = x;
		radius = Radius;
		iIndex = index;
	}
	float row;
	float col;
	float radius;
	int	iIndex;
};

//多边形
struct MPolygon
{
	MPolygon()
	{
	}

	MPolygon(QList<QPointF> Points, QList<QPointF> List_P, QList<QList<QPointF>> List_Ps)
	{
		points = Points;
		list_p = List_P;		
		list_ps = List_Ps;
	}
	QList<QPointF> points;
	QList<QPointF> list_p;
	QList<QList<QPointF>> list_ps;
};

//同心圆
struct CCircle
{
	CCircle()
	{
	}

	CCircle(float x, float y, float RadiusMin, float RadiusMax)
	{
		row = y;
		col = x;
		small_radius = RadiusMin;
		big_radius = RadiusMax;
	}
	float row;
	float col;
	float small_radius;
	float big_radius;
};

//卡尺
struct LineCaliperP
{
	qreal x1;
	qreal y1;
	qreal x2;
	qreal y2;
	qreal num;
	qreal height;
	qreal width;
	qreal sSigma;
	qreal sThreshold;
	QString sSelect;
	QString sTransition;			//极性
	qreal	sActiveNum;
	LineCaliperP()
	{
		height = 30;
		width = 40;
		num = 10;
		sSigma = 1;
		sSelect = "all";
		sTransition = "all";
		sActiveNum = 6;
		sThreshold = 30;
	}
	LineCaliperP(qreal X1, qreal Y1, qreal X2, qreal Y2, qreal Height, qreal Width,
		qreal Num = 10, qreal sigma = 1, qreal threshold = 30, 
		QString strSelect = "all", QString strTransition = "all",qreal activeNum = 30)
	{
		x1 = X1;
		y1 = Y1;
		x2 = X2;
		y2 = Y2;
		height = Height;
		width = Width;
		num = Num;
		sSigma = sigma;
		sSelect = strSelect;
		sTransition = strTransition;
		sActiveNum = activeNum;
	}
};

//卡尺
struct ArcCaliperP
{
	qreal	x;
	qreal	y;
	qreal	Radius;
	qreal	dStartAngle;						//平滑度
	qreal	dEndAngle;						//平滑度
	qreal	num;
	qreal	height;
	qreal	width;
	qreal	sSigma;
	qreal	sThreshold;
	QString sSelect;
	QString sTransition;			//极性
	qreal	sActiveNum;
	ArcCaliperP()
	{
		height			= 30;
		width			= 40;
		num				= 10;
		sSigma			= 1;
		sSelect			= "all";
		sTransition		= "all";
		sActiveNum		= 6;
		sThreshold		= 30;
		Radius			= 20;
		dStartAngle		= 0;
		dEndAngle		= 360;
	}
	ArcCaliperP(qreal X, qreal Y, qreal sRadius,qreal Height, qreal Width,
		qreal start = 0, qreal end = 360, qreal Num = 10, qreal sigma = 1, qreal threshold = 30,
		QString strSelect = "all", QString strTransition = "all", qreal activeNum = 30)
	{
		x				= X;
		y				= Y;
		Radius			= sRadius;
		dStartAngle		= start;
		dEndAngle		= end;
		height			= Height;
		width			= Width;
		num				= Num;
		sSigma			= sigma;
		sSelect			= strSelect;
		sTransition		= strTransition;
		sActiveNum		= activeNum;
	}
};
//卡尺
struct CaliperP
{
	CaliperP()
	{
	}

	CaliperP(float x, float y, float Len1, float Len2, float Angle, qreal Height, QPointF PP1, QPointF PP2)
	{
		//row = y;
		//col = x;
		//len1 = Len1;
		//len2 = Len2;
		//angle = Angle;
		//x1 = X1;
		//y1 = Y1;
		//x2 = X2;
		//y2 = Y2;
		height = Height;
		pp1 = PP1;
		pp2 = PP2;
	}
	//float row;
	//float col;
	//float len1;
	//float len2;
	//float angle;
	//qreal x1;
	//qreal y1;
	//qreal x2;
	//qreal y2;
	qreal height;
	qreal width;
	QPointF pp1;
	QPointF pp2;
};

struct MeasureRect2
{
	QLineF					line;
	double					dLastLineWidth;			//距离上个线的宽度
	double					dNormalLineWidth;		//法线的宽度
	double					dNormalLineHeight;		//法线的高度
	double					dSigma;					//平滑度
	double					dThreshold;				//阈值
	QString					strTransition;			//极性
	QString					strSelect;				//选择
	bool					bNeedCheckDistance;		//是否检测胶路点
	QMap<QString, QString>	mapParam;
	void Rst() {
		dLastLineWidth		= 2;
		dNormalLineWidth	= 2;
		dNormalLineHeight	= 2;
		dSigma				= 1;
		dThreshold			= 0.2;
		strTransition		= "all";
		strSelect			= "all";
		mapParam.clear();
		bNeedCheckDistance = false;
	}
	MeasureRect2() {
		Rst();
	}
};


struct CommonData
{
	ShapeItemType		type;
	EnumItemRegionType	eRegionType;
	QString				strData;
	CommonData()
	{
		type			= ItemCircle;
		eRegionType		= RegionType_Add;
		strData			= "";
	}
};


enum EnumParamType
{
	ParamType_Bool,
	ParamType_Int,
	ParamType_Double,
	ParamType_String,
	ParamType_Enum,
	ParamType_Other,

};

//QString WINAPI	ParamToValue(int iEnable, EnumParamType iType, QString Param);
//int WINAPI	ValueToParam(QString value, int& iEnable, EnumParamType& iType, QString& Param);

static QString ParamToValue(QString strLang,int iEnable, EnumParamType iType, QString Param, QStringList strEnumLst = QStringList(), QString LowParam = QString(), QString HighParam = QString())
{
	QString _strLimit;
	for (int i = 0; i < strEnumLst.size(); i++) {
		if (i >= (strEnumLst.size() - 1))
			_strLimit.append(strEnumLst[i]);
		else
			_strLimit.append(strEnumLst[i]).append("|");
	}
	return QString("%1_%2_%3_%4_%5_%6_%7").arg(strLang).arg(QString::number(iEnable)).arg(QString::number(iType)).arg(Param).arg(_strLimit).arg(LowParam).arg(HighParam);
}

static int ValueToParam(QString value, QString& strLang,int& iEnable, EnumParamType& iType, QString& Param, QStringList& strEnumLst, QString& LowParam, QString& HighParam)
{
	QStringList params = value.split("_");
	if (params.size() > 0)	strLang = params[0];
	if (params.size() > 1)	iEnable = params[1].toInt();
	else return -1;
	if (params.size() > 2)	iType = (EnumParamType)params[2].toInt();
	else return -1;
	if (params.size() > 3)	Param = params[3];
	else return -1;
	if (params.size() > 4) { strEnumLst = params[4].split("|"); }
	else return -1;
	if (params.size() > 5) { LowParam = params[5]; }
	else return -1;
	if (params.size() > 6) { HighParam = params[6]; }
	else return -1;
	return 0;
}
