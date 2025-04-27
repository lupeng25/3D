#include "ImageShow/BaseItem.h"
#include <math.h>
#include <QtDebug>
#include <QThread>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#define _PI 3.14159265358979323846
QMap<BaseItem::EnumBaseItemStr, QString>	BaseItem::mapKeyString = {
	{ EnumBaseItemStr_sSigma,		"sSigma"		},
	{ EnumBaseItemStr_sThreshold,	"sThreshold"	},
	{ EnumBaseItemStr_sSelect,		"sSelect"		},
	{ EnumBaseItemStr_sTransition,	"sTransition"	},
	{ EnumBaseItemStr_sActiveNum,	"sActiveNum"	},
	{ EnumBaseItemStr_sStartAngle,	"sStartAngle"	},
	{ EnumBaseItemStr_sEndAngle,	"sEndAngle"		},
	{ EnumBaseItemStr_sRadius,		"sRadius" },

};

/// <summary>
/// 计算组合数公式
/// </summary>
/// <param name="n"></param>
/// <param name="k"></param>
/// <returns></returns>
static ulong calc_combination_number(int n, int k)
{
	ulong* result = new ulong[n + 1];
	for (int i = 1; i <= n; i++)
	{
		result[i] = 1;
		for (int j = i - 1; j >= 1; j--)
			result[j] += result[j - 1];
		result[0] = 1;
	}
	return result[k];
}
/// <summary>
/// n阶贝塞尔曲线插值计算函数
/// 根据起点，n个控制点，终点 计算贝塞尔曲线插值
/// </summary>
/// <param name="t">当前插值位置0~1 ，0为起点，1为终点</param>
/// <param name="points">起点，n-1个控制点，终点</param>
/// <param name="count">n+1个点</param>
/// <returns></returns>
static QPointF bezier_interpolation_func(float t, QList<QPointF> points, int count)
{
	QPointF PointF;
	float* part = new float[count];
	float sum_x = 0, sum_y = 0;
	for (int i = 0; i < count; i++)
	{
		ulong tmp;
		int n_order = count - 1;    // 阶数
		tmp = calc_combination_number(n_order, i);
		sum_x += (float)(tmp * points[i].x() * pow((1 - t), n_order - i) * pow(t, i));
		sum_y += (float)(tmp * points[i].y() * pow((1 - t), n_order - i) * pow(t, i));
	}
	PointF.setX(sum_x);
	PointF.setY(sum_y);
	return PointF;
}

static QList<QPointF> draw_bezier_curves(QList<QPointF> points,float step)
{
	QList<QPointF> bezier_curves_points;
	float t = 0;
	do
	{
		QPointF temp_point = bezier_interpolation_func(t, points, points.size());    // 计算插值点
		t += step;
		bezier_curves_points.push_back(temp_point);
	} while (t <= 1 && points.size() > 1);    // 一个点的情况直接跳出.
	return bezier_curves_points;  // 曲线轨迹上的所有坐标点
}

static QPoint bezier_interpolationPoint_func(float t, QList<QPoint> points, int count)
{
	QPoint PointF;
	float* part = new float[count];
	float sum_x = 0, sum_y = 0;
	for (int i = 0; i < count; i++)
	{
		ulong tmp;
		int n_order = count - 1;    // 阶数
		tmp = calc_combination_number(n_order, i);
		sum_x += (float)(tmp * points[i].x() * pow((1 - t), n_order - i) * pow(t, i));
		sum_y += (float)(tmp * points[i].y() * pow((1 - t), n_order - i) * pow(t, i));
	}
	PointF.setX(sum_x);
	PointF.setY(sum_y);
	return PointF;
}

static QList<QPoint> CalculateNormalPoints(QPointF pt1, QPointF pt2,double dLenth) {
	QPointF center = QPointF( (pt1.x() + pt2.x()) / 2.0, (pt1.y() + pt2.y()) / 2.0 );



}

//决策步长
double getStep(const QList<QPointF> &vertexs) {
	double sum = 1;
	for (size_t i = 1; i < vertexs.size(); i++) {
		sum += sqrt((	vertexs[i].x() - vertexs[i - 1].x()	)*(	vertexs[i].x() - vertexs[i - 1].x()	) + (	vertexs[i].y() - vertexs[i - 1].y()	)*(	vertexs[i].y() - vertexs[i - 1].y()	));
	}
	return 1 / (sum * 1.5);
}

static QList<QPointF> draw_bezierPoint_curves(QList<QPointF> srcPts, QList<QPointF>&dstPts, float fstep)
{
	QList<QPointF> bezier_curves_points;
	size_t n = srcPts.size();
	dstPts.clear();
	if (n <= 0)	return bezier_curves_points;
	double **_pArrayX = new double*[n]; //中间计算过程需要用到的二维数组
	double **_pArrayY = new double*[n]; //中间计算过程需要用到的二维数组
	for (size_t i = 0; i < n; i++) {
		_pArrayX[i] = new double[n];
		_pArrayY[i] = new double[n];
	}
	//以一定步长生成曲线上的离散点
	double STEP = getStep(srcPts);
	double _dLenth = 0;
	int _iIndex = -1;
	QPointF _temp;
	for (double u = 0.0; u <= 1.0; u += STEP) {
		for (size_t i = 0; i < n; i++) {
			_pArrayX[i][0] = srcPts[i].x();
			_pArrayY[i][0] = srcPts[i].y();
		}
		//依次降阶计算（类似动态规划的计算过程）
		for (size_t r = 1; r < n; r++) {
			for (size_t i = 0; i < n - r; i++) {
				//P[i][r]=(1-u)*P[i][r-1]+u*P[i+1][r-1]; //递推式 - 关键
				_pArrayX[i][r] = (1 - u)*_pArrayX[i][r - 1] + u * _pArrayX[i + 1][r - 1]; //注：此出处计算应该全程使用double类型，最终取点的时候再取整，否则误差很大，曲线不够光滑
				_pArrayY[i][r] = (1 - u)*_pArrayY[i][r - 1] + u * _pArrayY[i + 1][r - 1];
			}
		}
		_temp = QPointF(/*qRound*/(_pArrayX[0][n - 1]), /*qRound*/(_pArrayY[0][n - 1]));
		if (bezier_curves_points.size() > 0) {
			int _iCurrentIndex = bezier_curves_points.size() - 1;
			_dLenth = _dLenth + sqrt((bezier_curves_points[_iCurrentIndex].x() - _temp.x())	* (bezier_curves_points[_iCurrentIndex].x() - _temp.x())
				+ (bezier_curves_points[_iCurrentIndex].y() - _temp.y())	* (bezier_curves_points[_iCurrentIndex].y() - _temp.y()));
		}
		int _ScaleIndex = u / fstep;
		if (_iIndex != _ScaleIndex) {
			_iIndex = _ScaleIndex;
			bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
		}
		dstPts.push_back(QPointF(_temp.x(), _temp.y()));
	}
	if ((_iIndex * fstep) < 1.0){
		bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
	}
	//释放空间
	for (size_t i = 0; i < n; i++) {
		delete[] _pArrayX[i];
		delete[] _pArrayY[i];
	}
	delete[] _pArrayX;
	delete[] _pArrayY;
	return bezier_curves_points;  // 曲线轨迹上的所有坐标点
}

//贝塞尔曲线法线
static QList<QLineF> get_beziernormalline_curves(QList<QPointF> srcPts,QList<QPointF>&dstPts,float fstep,float fEverylenth)
{
	QList<QLineF>				bezier_normallines;
	QList<QPointF>				bezier_curves_points;
	int n = srcPts.size();
	dstPts.clear();
	if (n <= 0)					return bezier_normallines;
	double **_pArrayX			= new double*[n]; //中间计算过程需要用到的二维数组
	double **_pArrayY			= new double*[n]; //中间计算过程需要用到的二维数组
	for (int i = 0; i < n; i++) {
		_pArrayX[i]				= new double[n];
		_pArrayY[i]				= new double[n];
	}
	//以一定步长生成曲线上的离散点
	double STEP					= getStep(srcPts);
	double _dCurvesLenth		= 0;
	int _iIndex					= -1;
	int _iCurrentLenthRow		= 0;
	QPointF _temp;
	for (double u = 0.0; u <= 1.0; u += STEP) {
		for (int i = 0; i < n; i++) {
			_pArrayX[i][0]		= srcPts[i].x();
			_pArrayY[i][0]		= srcPts[i].y();
		}
		//依次降阶计算（类似动态规划的计算过程）
		for (int r = 1; r < n; r++) {
			for (int i = 0; i < n - r; i++) {
				//P[i][r]=(1-u)*P[i][r-1]+u*P[i+1][r-1]; //递推式 - 关键
				_pArrayX[i][r]	= (1 - u)*_pArrayX[i][r - 1] + u * _pArrayX[i + 1][r - 1]; //注：此出处计算应该全程使用double类型，最终取点的时候再取整，否则误差很大，曲线不够光滑
				_pArrayY[i][r]	= (1 - u)*_pArrayY[i][r - 1] + u * _pArrayY[i + 1][r - 1];
			}
		}
		_temp = QPointF(/*qRound*/(_pArrayX[0][n - 1]), /*qRound*/(_pArrayY[0][n - 1]));
		if (bezier_curves_points.size() > 0) {
			int _iCurrentIndex	= bezier_curves_points.size() - 1;
			_dCurvesLenth		= _dCurvesLenth + sqrt((bezier_curves_points[_iCurrentIndex].x() - _temp.x())	* (bezier_curves_points[_iCurrentIndex].x() - _temp.x())
				+ (bezier_curves_points[_iCurrentIndex].y() - _temp.y()) * (bezier_curves_points[_iCurrentIndex].y() - _temp.y()));
			int _iCurrentRow		= _dCurvesLenth / fEverylenth;
			if (_iCurrentLenthRow	!= _iCurrentRow){
				_iCurrentLenthRow	= _iCurrentRow;
				//计算当前线的法线


			}
		}
		int _ScaleIndex = u / fstep;
		if (_iIndex != _ScaleIndex) {
			_iIndex = _ScaleIndex;
			bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
		}
		dstPts.push_back(QPointF(_temp.x(), _temp.y()));
	}
	if ((_iIndex * fstep) < 1.0) {
		if (bezier_curves_points.size() > 0) {
			int _iCurrentIndex = bezier_curves_points.size() - 1;
			_dCurvesLenth = _dCurvesLenth + sqrt((bezier_curves_points[_iCurrentIndex].x() - _temp.x())	* (bezier_curves_points[_iCurrentIndex].x() - _temp.x())
				+ (bezier_curves_points[_iCurrentIndex].y() - _temp.y())	* (bezier_curves_points[_iCurrentIndex].y() - _temp.y()));
			int _iCurrentRow		=	_dCurvesLenth / fEverylenth;
			if (_iCurrentLenthRow	!=	_iCurrentRow) {
				_iCurrentLenthRow	=	_iCurrentRow;
				//计算当前线的法线


			}
		}
		bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
	}
	//释放空间
	for (size_t i = 0; i < n; i++) {
		delete[] _pArrayX[i];
		delete[] _pArrayY[i];
	}
	delete[] _pArrayX;
	delete[] _pArrayY;

	return bezier_normallines;  // 曲线轨迹上的所有坐标点
}

static QList<QPointF> draw_bezierPoint_curves(QList<QPointF> points, float fstep)
{
	QList<QPointF> bezier_curves_points;
	size_t n = points.size();
	if (n <= 0)	return bezier_curves_points;
	double **_pArrayX = new double*[n]; //中间计算过程需要用到的二维数组
	double **_pArrayY = new double*[n]; //中间计算过程需要用到的二维数组
	for (size_t i = 0; i < n; i++) {
		_pArrayX[i] = new double[n];
		_pArrayY[i] = new double[n];
	}
	//以一定步长生成曲线上的离散点
	double STEP = getStep(points);
	double _dLenth = 0;
	int _iIndex = -1;
	QPointF _temp;
	for (double u = 0.0; u <= 1.0; u += STEP) {
		for (size_t i = 0; i < n; i++) {
			_pArrayX[i][0] = points[i].x();
			_pArrayY[i][0] = points[i].y();
		}
		//依次降阶计算（类似动态规划的计算过程）
		for (size_t r = 1; r < n; r++) {
			for (size_t i = 0; i < n - r; i++) {
				//P[i][r]=(1-u)*P[i][r-1]+u*P[i+1][r-1]; //递推式 - 关键
				_pArrayX[i][r] = (1 - u)*_pArrayX[i][r - 1] + u * _pArrayX[i + 1][r - 1]; //注：此出处计算应该全程使用double类型，最终取点的时候再取整，否则误差很大，曲线不够光滑
				_pArrayY[i][r] = (1 - u)*_pArrayY[i][r - 1] + u * _pArrayY[i + 1][r - 1];
			}
		}
		_temp = QPointF(/*qRound*/(_pArrayX[0][n - 1]), /*qRound*/(_pArrayY[0][n - 1]));
		if (bezier_curves_points.size() > 0) {
			int _iCurrentIndex = bezier_curves_points.size() - 1;
			_dLenth = _dLenth + sqrt((bezier_curves_points[_iCurrentIndex].x() - _temp.x())	* (bezier_curves_points[_iCurrentIndex].x() - _temp.x())
				+ (bezier_curves_points[_iCurrentIndex].y() - _temp.y())	* (bezier_curves_points[_iCurrentIndex].y() - _temp.y()));
		}
		int _ScaleIndex = u / fstep;
		if (_iIndex != _ScaleIndex)	{
			_iIndex = _ScaleIndex;
			bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
		}
	}
	if ((_iIndex * fstep) < 1.0)
	{
		bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
	}
	//释放空间
	for (size_t i = 0; i < n; i++) {
		delete[] _pArrayX[i];
		delete[] _pArrayY[i];
	}
	delete[] _pArrayX;
	delete[] _pArrayY;
	return bezier_curves_points;  // 曲线轨迹上的所有坐标点
}

BaseItem::BaseItem(QPointF center, ShapeItemType type) 
	: center(center)
	, types(type)
	, m_bShowMoveSelected(false)
	, m_bSelected(false)
{
	m_vecPoint.clear();
	setHandlesChildEvents(false);//设置后才能将事件传递到子元素
	noSelectedPen.setColor(QColor(0, 100, 200));
	noSelectedPen.setWidth(LineWidth);
	SelectedPen.setColor(QColor(0, 180, 0));
	SelectedPen.setWidth(LineWidth);
	fillColor	= QColor(0, 160, 230, 50); //填充颜色
	thisPen		= noSelectedPen;

	mapLangString = {
		{ "sSigma"						,	tr("sSigma") },
		{ "sThreshold"					,	tr("sThreshold") },
		{ "sSelect"						,	tr("sSelect") },
		{ "sTransition"					,	tr("sTransition") },
		{ "sActiveNum"					,	tr("sActiveNum") },

		{ "ComItem"						,	tr("ComItem") },
		{ "Radius"						,	tr("Radius") },
		{ "ItemCenterPos"				,	tr("ItemCenterPos") },
		{ "CenterX"						,	tr("CenterX") },
		{ "CenterY"						,	tr("CenterY") },
		{ "PosX"						,	tr("PosX") },
		{ "PosY"						,	tr("PosY") },
		{ "ItemLine"					,	tr("ItemLine") },
		{ "PosX1"						,	tr("PosX1") },
		{ "PosY1"						,	tr("PosY1") },
		{ "PosX2"						,	tr("PosX2") },
		{ "PosY2"						,	tr("PosY2") },
		{ "Angle"						,	tr("Angle") },
		{ "Width"						,	tr("Width") },
		{ "Height"						,	tr("Height") },
		{ "Number"						,	tr("Number") },
		{ "Lenth1"						,	tr("Lenth1") },
		{ "Lenth2"						,	tr("Lenth2") },
		{ "Phi"							,	tr("Phi") },
		{ "SmallRadius"					,	tr("SmallRadius") },
		{ "BigRadius"					,	tr("BigRadius") },
		{ "ShapeType"					,	tr("ShapeType") },
		{ "RegionType"					,	tr("RegionType") },
		{ "Pos"							,	tr("Pos") },
		{ "Position"					,	tr("Position") },
		{ "EnableNormalLine"			,	tr("EnableNormalLine") },
		{ "NormalLineLenth"				,	tr("NormalLineLenth") },
		{ "NormalLineHeight"			,	tr("NormalLineHeight") },
		{ "NormalLineWidth"				,	tr("NormalLineWidth") },
		{ "CheckDistance"				,	tr("CheckDistance") },
		{ "NormalIndex"					,	tr("NormalIndex") },
		{ "TwoLineLenth"				,	tr("TwoLineLenth") },

		{ "NormalSigma"					,	tr("NormalSigma") },
		{ "NormalThreshold"				,	tr("NormalThreshold") },
		{ "NormalTransition"			,	tr("NormalTransition") },
		{ "NormalSelection"				,	tr("NormalSelection") },
		{ "NormalSelectedIndex"			,	tr("NormalSelectedIndex") },
		{ "Split"						,	tr("Split") },

		{ "sStartAngle"					,	tr("sStartAngle") },
		{ "sEndAngle"					,	tr("sEndAngle") },
		{ "sRadius"						,	tr("sRadius") },
	};
	//setAcceptHoverEvents(true);
	//setCacheMode(DeviceCoordinateCache);
	//setZValue(-1);		// 设置图元层级
	//模式选择 自定义模式用于显示亚像素轮廓和Region  不设定任何属性
	if (type != ItemAutoItem) {this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable |	QGraphicsItem::ItemIsFocusable);}
}

BaseItem::~BaseItem()
{
	m_vecPoint.clear();
}

int BaseItem::SetData(QJsonObject & strData)
{
	return 0;
}

int BaseItem::GetData(QJsonObject & strData)
{
	return 0;
}

/**
 * @brief createNBezierCurve 生成N阶贝塞尔曲线点
 * @param src 源贝塞尔控制点
 * @param dest 目的贝塞尔曲线点
 * @param precision 生成精度
 */
void BaseItem::createNBezierCurve(const QList<QPointF> &src, QList<QPointF> &dest, qreal precision)
{
	if (src.size() <= 0) return;

	//清空
	QList<QPointF>().swap(dest);

	for (qreal t = 0; t < 1.0000; t += precision) {
		int size = src.size();
		QVector<qreal> coefficient(size, 0);
		coefficient[0] = 1.000;
		qreal u1 = 1.0 - t;

		for (int j = 1; j <= size - 1; j++) {
			qreal saved = 0.0;
			for (int k = 0; k < j; k++) {
				qreal temp = coefficient[k];
				coefficient[k] = saved + u1 * temp;
				saved = t * temp;
			}
			coefficient[j] = saved;
		}
		QPointF resultPoint;
		for (int i = 0; i < size; i++) {
			QPointF point = src.at(i);
			resultPoint = resultPoint + point * coefficient[i];
		}
		dest.append(resultPoint);
	}
}

void BaseItem::SelectItem(bool bFlag)
{
	if (bFlag)	{
		thisPen = SelectedPen;
	}
	else	{
		thisPen = noSelectedPen;
	}
}

bool BaseItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	pt = mapFromScene(pt);
	dir = NONE;
	for (int i = 0; i < m_vecPoint.size(); i++) {
		auto pos		= m_vecPoint[i];
		if (	(pt.x() >= (pos.x() - 2 * LineWidth)) 
			&&	(pt.x() <= (pos.x() + 2 * LineWidth)) 
			&&	(pt.y() <= (pos.y() + 2 * LineWidth)) 
			&&	(pt.y() >= (pos.y() - 2 * LineWidth)))
		{
			index = i;
			dir = NORTH;
			return true;
		}
	}
	return false;
}

void BaseItem::SetDrawEnd()
{
}

bool BaseItem::IsDrawEnd()
{
	return false;
}

int BaseItem::SetParam(QString key, QString value)
{
	return 0;
}

int BaseItem::GetParam(QString key, QString&value)
{
	return 0;
}

int BaseItem::GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue)
{
	return 0;
}

void BaseItem::PointRotate(QPointF pt, QPointF ptCenter, double dAngle, QPointF & ptResult)
{
	//pt.x -= ptCenter.x;
	//pt.y -= ptCenter.y;
	pt.setX(pt.x() - ptCenter.x());
	pt.setY(pt.y() - ptCenter.y());

	dAngle = dAngle / 180 * _PI;
	ptResult.setX( pt.x() * cos(dAngle) - pt.y() * sin(dAngle) + ptCenter.x());
	ptResult.setY( pt.x() * sin(dAngle) + pt.y() * cos(dAngle) + ptCenter.y());
}

QList<QLineF> BaseItem::get_beziernormalline_curves(QList<QPointF> srcPts, QList<QPointF>& dstPts, float fstep, float fEverylenth,float fEveryHeight)
{
	QList<QLineF>				bezier_normallines;
	QList<QPointF>				bezier_curves_points;
	int n = srcPts.size();
	dstPts.clear();
	if (n <= 0)					return bezier_normallines;
	double **_pArrayX			= new double*[n]; //中间计算过程需要用到的二维数组
	double **_pArrayY			= new double*[n]; //中间计算过程需要用到的二维数组
	for (int i = 0; i < n; i++) {
		_pArrayX[i]				= new double[n];
		_pArrayY[i]				= new double[n];
	}
	//以一定步长生成曲线上的离散点
	double STEP					= getStep(srcPts);
	double _dCurvesLenth		= 0;
	int _iIndex					= -1;
	int _iCurrentLenthRow		= -1;
	QPointF _temp, _temp1;
	QPointF _tempRetn, _temp1Retn;
	for (double u = 0.0; u <= 1.0; u += STEP) {
		for (int i = 0; i < n; i++) {
			_pArrayX[i][0]		= srcPts[i].x();
			_pArrayY[i][0]		= srcPts[i].y();
		}
		//依次降阶计算（类似动态规划的计算过程）
		for (int r = 1; r < n; r++) {
			for (int i = 0; i < n - r; i++) {
				_pArrayX[i][r]	= (1 - u)*_pArrayX[i][r - 1] + u * _pArrayX[i + 1][r - 1]; //注：此出处计算应该全程使用double类型，最终取点的时候再取整，否则误差很大，曲线不够光滑
				_pArrayY[i][r]	= (1 - u)*_pArrayY[i][r - 1] + u * _pArrayY[i + 1][r - 1];
			}
		}
		_temp = QPointF(/*qRound*/(_pArrayX[0][n - 1]), /*qRound*/(_pArrayY[0][n - 1]));
		if (dstPts.size() > 0) {
			int _iCurrentIndex	= dstPts.size() - 1;
			_dCurvesLenth		= _dCurvesLenth + sqrt((dstPts[_iCurrentIndex].x() - _temp.x())	* (dstPts[_iCurrentIndex].x() - _temp.x())
				+ (dstPts[_iCurrentIndex].y() - _temp.y()) * (dstPts[_iCurrentIndex].y() - _temp.y()));

			int _iCurrentRow	= (_dCurvesLenth - fEverylenth / 2.0) / fEverylenth;
			if (_iCurrentLenthRow != _iCurrentRow && (_dCurvesLenth - fEverylenth / 2.0) >= 0 ) {
				_iCurrentLenthRow = _iCurrentRow;
				//计算当前线的法线
				_temp1			= dstPts[_iCurrentIndex];
				GetNormalLine(_temp, _temp1, fEveryHeight, _tempRetn, _temp1Retn);
				bezier_normallines.push_back(QLineF(_tempRetn, _temp1Retn));
			}
		}
		int _ScaleIndex = u / fstep;
		if (_iIndex != _ScaleIndex) {
			_iIndex				= _ScaleIndex;
			bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
		}
		dstPts.push_back(QPointF(_temp.x(), _temp.y()));
	}
	if ((_iIndex * fstep) < 1.0) {
		if (bezier_curves_points.size() > 0) {
			int _iCurrentIndex = dstPts.size() - 1;
			_dCurvesLenth		= _dCurvesLenth + sqrt((dstPts[_iCurrentIndex].x() - _temp.x())	* (dstPts[_iCurrentIndex].x() - _temp.x())
				+ (dstPts[_iCurrentIndex].y() - _temp.y())	* (dstPts[_iCurrentIndex].y() - _temp.y()));
			int _iCurrentRow = (_dCurvesLenth - fEverylenth / 2.0) / fEverylenth;
			if (_iCurrentLenthRow != _iCurrentRow && (_dCurvesLenth - fEverylenth / 2.0) >= 0) {
				_iCurrentLenthRow = _iCurrentRow;
				//计算当前线的法线
				_temp1			= dstPts[_iCurrentIndex];
				GetNormalLine(_temp, _temp1, fEveryHeight, _tempRetn, _temp1Retn);
				bezier_normallines.push_back(QLineF(_tempRetn, _temp1Retn));
			}
		}
		bezier_curves_points.push_back(QPointF(_temp.x(), _temp.y()));
	}
	//释放空间
	for (size_t i = 0; i < n; i++) {
		delete[] _pArrayX[i];
		delete[] _pArrayY[i];
	}
	delete[] _pArrayX;
	delete[] _pArrayY;

	return bezier_normallines;  // 曲线轨迹上的所有坐标点
}

void BaseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//缩放控制点尺寸
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont font;
	font.setPointSizeF(15 / (*scale)); //字体大小
	painter->setFont(font);
	painter->restore();
	//子类绘制时状态
	//LineWidth = 2 / (*scale);

	if (m_bSelected || m_bShowMoveSelected){
		thisPen = SelectedPen;
		for (int i = 0; i < m_vecPoint.size(); i++){
			if (i == 0) {
				const auto center_rect = QRectF((m_vecPoint[i] - QPointF(LineWidth,LineWidth)), QSizeF(LineWidth * 2, LineWidth * 2));
				painter->setPen(QPen(	QBrush(	QColor(0, 0, 255) ),1));
				painter->setBrush(QColor(255, 255, 255));
				painter->drawRect(center_rect);
			}
			else {
				const auto center_rect = QRectF((m_vecPoint[i] - QPointF(1.5 * LineWidth, 1.5 * LineWidth)), QSizeF(LineWidth * 3, LineWidth * 3));
				painter->setPen(QColor(147, 147, 147));
				painter->setBrush(QColor(255, 255, 255));
				painter->drawRect(center_rect);
			}
		}
	}
	else {	thisPen = noSelectedPen;	}

	thisPen.setWidthF(LineWidth);
	painter->setPen(thisPen);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));

}

ShapeItemType BaseItem::GetItemType()
{
	return types;
}

void BaseItem::SetDrawPenWidth(qreal lenth)
{
	LineWidth = qMax<qreal>(lenth,0);
}

void BaseItem::SetLinePath(QPointF P1, QPointF P2, float flenth, QPainterPath & path)
{
	//path = QPainterPath();
	double dx = P2.x() - P1.x();
	double dy = P2.y() - P1.y();
	double distance = sqrt(dy * dy + dx * dx);
	double devX = dx / distance * (	qMax<double>(flenth, 0.5)	);
	double devY = dy / distance * ( qMax<double>(flenth, 0.5)	);
	QPointF _ptTemp = P1 + QPointF(devX, devY);

	QVector<QPointF>	_vecPoint;
	QMatrix transform;
	{
		transform.reset();
		transform = transform.translate(P1.x(), P1.y()).rotate(90).translate(-P1.x(), -P1.y());
		_vecPoint << transform.map(_ptTemp);
	} {
		transform.reset();
		transform = transform.translate(P1.x(), P1.y()).rotate(-90).translate(-P1.x(), -P1.y());
		_vecPoint << transform.map(_ptTemp);
	}
	_ptTemp = P2 + QPointF(devX, devY);
	{
		transform.reset();
		transform = transform.translate(P2.x(), P2.y()).rotate(-90).translate(-P2.x(), -P2.y());
		_vecPoint << transform.map(_ptTemp);
	}
	{
		transform.reset();
		transform = transform.translate(P2.x(), P2.y()).rotate(90).translate(-P2.x(), -P2.y());
		_vecPoint << transform.map(_ptTemp);
	}
	_vecPoint.push_back(_vecPoint[0]);
	path.setFillRule(Qt::WindingFill);
	path.addPolygon(_vecPoint);

}

void BaseItem::GetNormalLine(QPointF P1, QPointF P2, float flenth, QPointF & ResP1, QPointF & ResP2)
{
	double dx = P2.x() - P1.x();
	double dy = P2.y() - P1.y();
	double distance = sqrt(dy * dy + dx * dx);
	double devX = dx / distance * (qMax<double>(flenth, 0.5));
	double devY = dy / distance * (qMax<double>(flenth, 0.5));

	QPointF _ptCenter;
	_ptCenter.setX((P2.x() + P1.x()) / 2.0);
	_ptCenter.setY((P2.y() + P1.y()) / 2.0);

	QVector<QPointF>	_vecPoint;
	//QMatrix transform;
	QPointF _ptTemp;
	{
		_ptTemp = _ptCenter + QPointF(devX, devY);;
		PointRotate(_ptTemp, _ptCenter,90, ResP1);
		//transform.reset();
		//transform = transform.translate(_ptCenter.x(), _ptCenter.y()).rotate(90).translate(-_ptCenter.x(), -_ptCenter.y());
		//_vecPoint << transform.map(_ptTemp);
		//ResP1 = transform.map(_ptTemp);
	} {
		_ptTemp = _ptCenter + QPointF(devX, devY);;
		PointRotate(_ptTemp, _ptCenter, -90, ResP2);
		//transform.reset();
		//transform = transform.translate(_ptCenter.x(), _ptCenter.y()).rotate(-90).translate(-_ptCenter.x(), -_ptCenter.y());
		//_vecPoint << transform.map(_ptTemp);
		//ResP2 = transform.map(_ptTemp);
	}
}

void BaseItem::GetStartEndNormalLine(QPointF PStart, QPointF P2, float flenth, QPointF & ResP1, QPointF & ResP2)
{
	double dx = P2.x() - PStart.x();
	double dy = P2.y() - PStart.y();
	double distance = sqrt(dy * dy + dx * dx);
	double devX = dx / distance * (qMax<double>(flenth, 0.5));
	double devY = dy / distance * (qMax<double>(flenth, 0.5));

	QPointF _ptCenter;
	_ptCenter.setX(PStart.x() );
	_ptCenter.setY(PStart.y() );

	QVector<QPointF>	_vecPoint;
	//QMatrix transform;
	QPointF _ptTemp;
	{
		_ptTemp = _ptCenter + QPointF(devX, devY);;
		PointRotate(_ptTemp, _ptCenter, 90, ResP1);
		//transform.reset();
		//transform = transform.translate(_ptCenter.x(), _ptCenter.y()).rotate(90).translate(-_ptCenter.x(), -_ptCenter.y());
		//_vecPoint << transform.map(_ptTemp);
		//ResP1 = transform.map(_ptTemp);
	} {
		_ptTemp = _ptCenter + QPointF(devX, devY);;
		PointRotate(_ptTemp, _ptCenter, -90, ResP2);
		//transform.reset();
		//transform = transform.translate(_ptCenter.x(), _ptCenter.y()).rotate(-90).translate(-_ptCenter.x(), -_ptCenter.y());
		//_vecPoint << transform.map(_ptTemp);
		//ResP2 = transform.map(_ptTemp);
	}
}

void BaseItem::GetLineArrowPoint(QPointF startP1, QPointF EndP2, float flenth, float fAngle, QPointF & ResP1, QPointF & ResP2)
{
	double dx = EndP2.x() - startP1.x();
	double dy = EndP2.y() - startP1.y();
	double distance = sqrt(dy * dy + dx * dx);
	double devX = dx / distance * (qMax<double>(flenth, 0.5));
	double devY = dy / distance * (qMax<double>(flenth, 0.5));
	QPointF _ptTemp = startP1 + QPointF(devX, devY);

	QVector<QPointF>	_vecPoint;
	QMatrix transform;
	//{
	//	transform.reset();
	//	transform = transform.translate(startP1.x(), startP1.y()).rotate(90).translate(-startP1.x(), -startP1.y());
	//	_vecPoint << transform.map(_ptTemp);
	//} {
	//	transform.reset();
	//	transform = transform.translate(startP1.x(), startP1.y()).rotate(-90).translate(-startP1.x(), -startP1.y());
	//	_vecPoint << transform.map(_ptTemp);
	//}
	_ptTemp = EndP2 + QPointF(devX, devY);
	{
		transform.reset();
		transform = transform.translate(EndP2.x(), EndP2.y()).rotate(-fAngle).translate(-EndP2.x(), -EndP2.y());
		_vecPoint << transform.map(_ptTemp);
	}
	{
		transform.reset();
		transform = transform.translate(EndP2.x(), EndP2.y()).rotate(fAngle).translate(-EndP2.x(), -EndP2.y());
		_vecPoint << transform.map(_ptTemp);
	}
	ResP1 = _vecPoint[0];
	ResP2 = _vecPoint[1];

}

QRectF BaseItem::boundingRect()
{
	return QRectF(0, 0, 10000, 10000);
}

void BaseItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItemGroup::mousePressEvent(event);
}

void BaseItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItemGroup::mouseMoveEvent(event);
}

void BaseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItemGroup::mouseReleaseEvent(event);
}

//******绘制结果数据******
DrawItem::DrawItem() :BaseItem(QPointF(0, 0), ItemAutoItem)
{
	returnRec = QRectF(0, 0, 10000, 10000);
	DrawColor = QColor(0, 255,0);
	ClearAll();
}

DrawItem::~DrawItem()
{

}

void DrawItem::Refresh()
{
	this->update();
}

QRectF DrawItem::boundingRect() const
{
	//当不设定Rect时超出区域会发生不显示的情况
	return returnRec;
}

void DrawItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	LineWidth = 1 / (*scale);
	DrawPen.setWidthF(LineWidth);
	DrawPen.setColor(DrawColor);
	painter->setPen(DrawPen);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	std::lock_guard<std::mutex> _lock(m_Lock);

	//直线
	for (int i = 0; i < LineList.size(); i++) {
		if(LineList[i].LineWidth > 0)	DrawPen.setWidthF(LineList[i].LineWidth); else  DrawPen.setWidthF(LineWidth);
		DrawPen.setColor(LineList[i].DrawQColor);
		painter->setPen(DrawPen);
		//GetStartEndNormalLine(LineList[i].Dline.p1(), LineList[i].Dline.p2(), LineList[i].LineWidth / 2.0, ResP[0], ResP[1]);
		//GetStartEndNormalLine(LineList[i].Dline.p2(), LineList[i].Dline.p1(), LineList[i].LineWidth / 2.0, ResP[2], ResP[3]);
		painter->drawLine(LineList[i].Dline.p1(), LineList[i].Dline.p2());
		//painter->drawLine(ResP[0], ResP[1]);
		//painter->drawLine(ResP[1], ResP[2]);
		//painter->drawLine(ResP[2], ResP[3]);
		//painter->drawLine(ResP[3], ResP[0]);
	}
	
	//计算矩形
	QPointF ResP[4];
	memset(&ResP,0,sizeof(QPointF) * 4 );
	for (int i = 0; i < RectangleList.size(); i++){
		if (RectangleList[i].LineWidth > 0)	DrawPen.setWidthF(RectangleList[i].LineWidth); else  DrawPen.setWidthF(LineWidth);
		DrawPen.setColor(RectangleList[i].DrawQColor);
		painter->setPen(DrawPen);
		GetStartEndNormalLine(RectangleList[i].Dline.p1(), RectangleList[i].Dline.p2(), RectangleList[i].DWidth / 2.0, ResP[0], ResP[1]);
		GetStartEndNormalLine(RectangleList[i].Dline.p2(), RectangleList[i].Dline.p1(), RectangleList[i].DWidth / 2.0, ResP[2], ResP[3]);
		painter->drawLine(ResP[0], ResP[1]);
		painter->drawLine(ResP[1], ResP[2]);
		painter->drawLine(ResP[2], ResP[3]);
		painter->drawLine(ResP[3], ResP[0]);
	}
	//memset(&ResP,0,sizeof(QPointF) * 4 );
	painter->save();
	for (int i = 0; i < RRectangleList.size(); i++){
		if (RRectangleList[i].LineWidth > 0)	DrawPen.setWidthF(RRectangleList[i].LineWidth); else  DrawPen.setWidthF(LineWidth);
		//DrawPen.setWidthF(RRectangleList[i].LineWidth);
		DrawPen.setColor(RRectangleList[i].DrawQColor);
		painter->setPen(DrawPen);
		painter->translate(RRectangleList[i].Center);
		painter->rotate( - RRectangleList[i].Angle/* / _PI * 180.0*/);
		painter->drawRect(QRectF(- RRectangleList[i].DHeight , -RRectangleList[i].DWidth, RRectangleList[i].DHeight * 2, RRectangleList[i].DWidth * 2.0));
	}
	//painter->restore();
	//painter->save();
	for (int i = 0; i < REllipseList.size(); i++) {
		if (REllipseList[i].LineWidth > 0)	DrawPen.setWidthF(REllipseList[i].LineWidth); else  DrawPen.setWidthF(LineWidth);
		//DrawPen.setWidthF(RRectangleList[i].LineWidth);
		DrawPen.setColor(REllipseList[i].DrawQColor);
		painter->setPen(DrawPen);
		painter->translate(REllipseList[i].Center);
		painter->rotate(-REllipseList[i].Angle/* / _PI * 180.0*/);
		painter->drawEllipse(QRectF(-REllipseList[i].DHeight, -REllipseList[i].DWidth, REllipseList[i].DHeight * 2, REllipseList[i].DWidth * 2.0));
	}
	painter->restore();

	for (auto item : ROIList){	for (auto p : item)	{	painter->drawPoint(p);	}	}
	int	_iIndex		= 0;
	long _lDrawHeight	= 0;
	for (auto& item : TextList){
		_iIndex++;
		DrawPen.setColor(item.DrawColor);
		QFont font = painter->font();
		if (item.iDrawLenth > 0)
			font.setPointSizeF(item.iDrawLenth);
		painter->setFont(font);
		painter->setPen(DrawPen);
		if (item.bControl){
			QFontMetrics	Metrics(font);
			int	_Height		= Metrics.height();
			_lDrawHeight	= _lDrawHeight + _Height;
			item.Center.setY( _lDrawHeight );
			painter->drawText(item.Center, item.strText);
		}
		else{	painter->drawText(item.Center, item.strText);	}
	}
	for (int i = 0; i < CrossList.size(); i++){
		DrawPen.setWidthF(LineWidth);
		DrawPen.setColor(CrossList[i].DrawColor);
		painter->setPen(DrawPen);
		auto&item = CrossList[i];
		//painter->setPen(QPen(item.DrawColor, 2));
		float theta = item.fAngle / _PI * 180;
		theta = 360 - theta;
		theta = theta / 180.0 * _PI;
		float x1 = item.Center.x();
		float y1 = item.Center.y();
		float x2 = item.Center.x();
		float y2 = item.Center.y();
		float x = item.Center.x() - item.iDrawLenth;
		float y = item.Center.y();
		x1 = (x - item.Center.x()) * cos(theta) - (y - item.Center.y()) * sin(theta) + item.Center.x();
		y1 = (x - item.Center.x()) * sin(theta) + (y - item.Center.y()) * cos(theta) + item.Center.y();
		x = item.Center.x() + item.iDrawLenth;
		y = item.Center.y();
		x2 = (x - item.Center.x()) * cos(theta) - (y - item.Center.y()) * sin(theta) + item.Center.x();
		y2 = (x - item.Center.x()) * sin(theta) + (y - item.Center.y()) * cos(theta) + item.Center.y();
		painter->drawLine(QPointF(x1, y1), QPointF( x2, y2));
		x = item.Center.x();
		y = item.Center.y() - item.iDrawLenth;
		x1 = (x - item.Center.x()) * cos(theta) - (y - item.Center.y()) * sin(theta) + item.Center.x();
		y1 = (x - item.Center.x()) * sin(theta) + (y - item.Center.y()) * cos(theta) + item.Center.y();
		x = item.Center.x();
		y = item.Center.y() + item.iDrawLenth;
		x2 = (x - item.Center.x()) * cos(theta) - (y - item.Center.y()) * sin(theta) + item.Center.x();
		y2 = (x - item.Center.x()) * sin(theta) + (y - item.Center.y()) * cos(theta) + item.Center.y();
		painter->drawLine(QPointF(x1, y1), QPointF(x2, y2));
	}
	for (auto item : ColorList){
		if (item.LineWidth > 0)	DrawPen.setWidthF(item.LineWidth); else  DrawPen.setWidthF(LineWidth);
		DrawPen.setColor(item.DrawQColor);
		painter->setPen(DrawPen);
		for (auto p : item.vecPoints) { painter->drawPoint(p); }
	}
	for (auto item : CircleList) {
		if (item.LineWidth > 0)	DrawPen.setWidthF(item.LineWidth); else  DrawPen.setWidthF(LineWidth);
		DrawPen.setColor(item.DrawQColor);
		painter->setPen(DrawPen);
		painter->drawEllipse(item.CenterPoints,item.qRadius, item.qRadius);
	}
}

#pragma region 点
PointItem::PointItem(qreal x, qreal y, qreal R) :BaseItem(QPointF(x, y), ItemPoint)
{
	m_showShape = ItemCircle;
	Radius = R;
	m_vecPoint.push_back(center);
	//m_vecPoint.push_back(center + QPointF(R, 0));
	//ControlList << new ControlItem(this, center, 0);
	//ControlList << new ControlItem(this, center + QPointF(R, 0), 1);
}

int PointItem::SetData(QJsonObject & strData)
{
	MCircle _data;
	if (strData.contains("col"))	_data.col		= strData["col"].toString().toDouble();
	if (strData.contains("row"))	_data.row		= strData["row"].toString().toDouble();
	if (strData.contains("radius"))	_data.radius	= strData["radius"].toString().toDouble();
	SetCircle(_data);

	return 0;
}

int PointItem::GetData(QJsonObject & strData)
{
	MCircle _data;
	GetCircle(_data);
	strData.insert("col",		QString::number(	_data.col		)	);
	strData.insert("row",		QString::number(	_data.row		)	);
	strData.insert("radius",	QString::number(	_data.radius	)	);
	return 0;
}

bool PointItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	switch (m_showShape)
	{
	default:
	case ItemPoint: {	
		double dDistance = sqrt((pt.x() - m_vecPoint[0].x()) * (pt.x() - m_vecPoint[0].x()) + (pt.y() - m_vecPoint[0].y()) * (pt.y() - m_vecPoint[0].y()));
		if (dDistance <= Radius) {
			dir = INRECT;
			return true;
		}
	}	break;
	case ItemRectangle:
	case ItemRectangleR:
	case ItemSquare: {
		QRectF _rect = QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2);
		if (_rect.contains(pt)){
			dir = INRECT;
			return true;
		}
	}	break;
	}
	return false;
}

void PointItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth); //UpdateLinePath();

	//Radius = qMax<int>(8,lenth);
	update();
}

int PointItem::SetParam(QString key, QString value)
{
	MCircle _data;
	GetCircle(_data);
	if (key == cstItemPosX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemPosY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemRadius)
	{
		_data.radius	= value.toDouble();
		Radius			= value.toDouble();
	}
	else if (key == cstItemShapeType)
	{
		switch ((ShapeItemType)value.toInt())	{
		case 0: {
			m_showShape = ItemCircle;
		}	break;
		case 1: {
			m_showShape = ItemRectangle;
		}	break;
		default:
			break;
		}
	}

	SetCircle(_data);
	return 0;
}

int PointItem::GetParam(QString key, QString&value)
{
	MCircle _data;
	GetCircle(_data);
	if (key == cstItemPosX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemPosY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemRadius)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.radius));
	}
	else if (key == cstItemShapeType)
	{
		QStringList strlst = QStringList{ "Circle" ,"Rect"};
		QStringList strLanglst = QStringList{ BaseItem::tr("Circle") ,BaseItem::tr("Rect") };
		int	_iIndex = 0;
		switch (m_showShape)	{
		default:
		case ItemPoint:	_iIndex = 0;		break;
		case ItemRectangle:
		case ItemRectangleR:
		case ItemSquare:	_iIndex = 1;	break;		}
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum, QString::number(_iIndex), strLanglst);
	}
	return 0;
}

int PointItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	MCircle _data;
	GetCircle(_data);
	QVector<QPair<QString, QString>>			mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemCenterX,		ParamToValue(
		BaseItem::tr(cstItemCenterX.toStdString().c_str()), 1,ParamType_Double, QString::number(_data.col	))));
	mapItem.push_back(QPair<QString, QString>(cstItemCenterY,		ParamToValue(
		BaseItem::tr(cstItemCenterY.toStdString().c_str()), 1,ParamType_Double, QString::number(_data.row))));
	mapItem.push_back(QPair<QString, QString>(cstItemRadius,		ParamToValue(
		BaseItem::tr(cstItemRadius.toStdString().c_str()), 1,ParamType_Double, QString::number(_data.radius))));

	QStringList strlst = QStringList{ "Circle" ,"Rect" };
	QStringList strLanglst = QStringList{ BaseItem::tr("Circle") ,BaseItem::tr("Rect") };
	int	_iIndex = 0;
	switch (m_showShape) {
	default:
	case ItemPoint:	_iIndex = 0;		break;
	case ItemRectangle:
	case ItemRectangleR:
	case ItemSquare:	_iIndex = 1;	break;
	}
	mapItem.push_back(QPair<QString, QString>(cstItemShapeType,	ParamToValue(
		BaseItem::tr(cstItemShapeType.toStdString().c_str()), 1, ParamType_Enum, QString::number(_iIndex), strLanglst)));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF PointItem::boundingRect() const
{
	//return QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2);
	return QRectF(0, 0, 10000, 10000);
}

bool PointItem::updatePos(int index, QPointF pt)
{
	m_vecPoint[index] = pt;
	//QPointF Pf = m_vecPoint[index];
	//QPointF tmp = Pf - center;
	//Radius = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	return true;
}

void PointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	switch (m_showShape)
	{
	default:
	case ItemPoint:
		painter->drawEllipse(QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2));
		break;
	case ItemRectangle:
	case ItemRectangleR:
	case ItemSquare:
		painter->drawRect(QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2));
		break;
	}
}

PointItem::~PointItem()
{

}

void PointItem::GetCircle(MCircle& Cir)
{
	QPointF GetRec = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	Cir.col = GetRec.x();
	Cir.row = GetRec.y();
	Cir.radius = Radius;
}

void PointItem::SetCircle(MCircle & Cir)
{
	QPointF ptPoit;
	ptPoit.setX(Cir.col);
	ptPoit.setY(Cir.row);
	ptPoit = this->mapFromScene(ptPoit);
	m_vecPoint[0] = ptPoit;
	//ControlList[0]->SetPoint(ptPoit);
	center.setX(ptPoit.x());
	center.setY(ptPoit.y());
	Radius = Cir.radius;

	//ptPoit.setX(center.x() + Radius);
	//m_vecPoint[1] = ptPoit;
	//ControlList[1]->SetPoint(ptPoit);
}

#pragma endregion

#pragma region 矩形
RectangleItem::RectangleItem(qreal x, qreal y, qreal width, qreal height) 
	:BaseItem(QPointF(x, y), ItemRectangle)
{
	m_vecPoint.push_back(QPointF(x - width / 2, y - height / 2));
	m_vecPoint.push_back(QPointF(x, y));
	m_vecPoint.push_back(QPointF(x + width, y));
	m_vecPoint.push_back(QPointF(x + width, y + height));
	m_vecPoint.push_back(QPointF(x, y + height));

	//ControlList << new ControlItem(this, QPointF(x + width / 2, y + height / 2), 0);
	//ControlList << new ControlItem(this, QPointF(x, y), 1);
	//ControlList << new ControlItem(this, QPointF(x + width, y), 2);
	//ControlList << new ControlItem(this, QPointF(x + width, y + height), 3);
	//ControlList << new ControlItem(this, QPointF(x, y + height), 4);
}

QRectF RectangleItem::boundingRect() const
{
	return  QRectF(m_vecPoint[1], m_vecPoint[3]);
	//return  QRectF(ControlList[1], ControlList[3]);
}

int RectangleItem::SetData(QJsonObject & strData)
{
	MRectangle _data;
	if (strData.contains("col"))	_data.col		= strData["col"].toString().toDouble();
	if (strData.contains("row"))	_data.row		= strData["row"].toString().toDouble();
	if (strData.contains("width"))	_data.width		= strData["width"].toString().toDouble();
	if (strData.contains("height"))	_data.height	= strData["height"].toString().toDouble();
	SetRect(_data);
	return 0;
}

int RectangleItem::GetData(QJsonObject & strData)
{
	MRectangle _data;
	GetRect(_data);
	strData.insert("col",		QString::number(_data.col));
	strData.insert("row",		QString::number(_data.row));
	strData.insert("width",		QString::number(_data.width));
	strData.insert("height",	QString::number(_data.height));

	return 0;
}

bool RectangleItem::updatePos(int index, QPointF pt)
{
	if (index < m_vecPoint.size())	m_vecPoint[index] = pt;
	QPointF Pf = m_vecPoint[index];
	//角点分情况
	switch (index)
	{
	case 1: {
		m_vecPoint[2] = QPointF(m_vecPoint[2].x(), Pf.y());
		m_vecPoint[4] = QPointF(Pf.x(), m_vecPoint[4].y());
	}	break;
	case 2: {
		m_vecPoint[1] = QPointF(m_vecPoint[1].x(), Pf.y());
		m_vecPoint[3] = QPointF(Pf.x(), m_vecPoint[3].y());
	}	break;
	case 3: {
		m_vecPoint[2] = QPointF(Pf.x(), m_vecPoint[2].y());
		m_vecPoint[4] = QPointF(m_vecPoint[4].x(), Pf.y());
	}	break;
	case 4: {
		m_vecPoint[1] = QPointF(Pf.x(), m_vecPoint[1].y());
		m_vecPoint[3] = QPointF(m_vecPoint[3].x(), Pf.y());
	} 	break;
	default:
		break;
	}
	m_vecPoint[0] = QPointF((m_vecPoint[1].x() + m_vecPoint[2].x()) / 2.0, (m_vecPoint[2].y() + m_vecPoint[3].y()) / 2.0	);
	//中心点
	//ControlList[0]->SetPoint(QPointF((ControlList[1].x() + ControlList[2].x()) / 2, (ControlList[2].y() + ControlList[3].y()) / 2));
	return true;
}

void RectangleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	QRectF ret(	m_vecPoint[1], m_vecPoint[3]	);
	//QRectF ret(ControlList[1], ControlList[3]);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	painter->drawRect(ret);
}

RectangleItem::~RectangleItem()
{

}

void RectangleItem::SetRect(MRectangle MRect)
{
	QPointF CenterSet = this->mapFromScene(MRect.col, MRect.row);
	QPointF set		= CenterSet - QPointF(MRect.width / 2, MRect.height / 2);
	m_vecPoint[0]	= CenterSet;
	m_vecPoint[1]	= set ;
	m_vecPoint[2]	= set + QPointF(MRect.width, 0);
	m_vecPoint[3]	= set + QPointF(MRect.width, MRect.height);
	m_vecPoint[4]	= set + QPointF(0, MRect.height);

	//ControlList[0]->SetPoint(set + QPointF(MRect.width / 2, MRect.height / 2));
	//ControlList[1]->SetPoint(set);
	//ControlList[2]->SetPoint(set + QPointF(MRect.width, 0));
	//ControlList[3]->SetPoint(set + QPointF(MRect.width, MRect.height));
	//ControlList[4]->SetPoint(set + QPointF(0, MRect.height));
	this->setFocus();
}

void RectangleItem::GetRect(MRectangle& MRect)
{
	MRect.width		= m_vecPoint[2].x() - m_vecPoint[1].x();
	MRect.height	= m_vecPoint[3].y() - m_vecPoint[1].y();
	QPointF GetRec	= this->mapToScene((m_vecPoint[1].x() + m_vecPoint[2].x()) / 2.0, (m_vecPoint[1].y() + m_vecPoint[3].y()) / 2.0);
	MRect.col		= GetRec.x();
	MRect.row		= GetRec.y();
}

bool RectangleItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {	return true;	}
	if (boundingRect().contains(pt)){
		dir = INRECT;
		return true;
	}
	return false;
}

int RectangleItem::SetParam(QString key, QString value)
{
	MRectangle _data;
	GetRect(_data);
	if (key == cstItemCenterX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemCenterY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemHeight)
	{
		_data.height = value.toDouble();
	}
	else if (key == cstItemWidth)
	{
		_data.width = value.toDouble();
	}
	SetRect(_data);
	return 0;
}

int RectangleItem::GetParam(QString key, QString&value)
{
	MRectangle _data;
	GetRect(_data);
	if (key == cstItemCenterX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemCenterY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemHeight)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.height));
	}
	else if (key == cstItemWidth)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width));
	}
	return 0;
}

int RectangleItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{	
	MRectangle _data;
	GetRect(_data);
	QVector<QPair<QString, QString>>			mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemCenterX,		ParamToValue(
		BaseItem::tr(cstItemCenterX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col ))));
	mapItem.push_back(QPair<QString, QString>(cstItemCenterY,		ParamToValue(
		BaseItem::tr(cstItemCenterY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row	))));
	mapItem.push_back(QPair<QString, QString>(cstItemHeight,		ParamToValue(
		BaseItem::tr(cstItemHeight.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.height))));
	mapItem.push_back(QPair<QString, QString>(cstItemWidth,		ParamToValue(
		BaseItem::tr(cstItemWidth.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

void RectangleItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);
	update();
}
#pragma endregion

#pragma region 旋转矩形
RectangleRItem::RectangleRItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi) :BaseItem(QPointF(x, y), ItemRectangleR)
{
	angle = Pi;
	lenth1 = Lenth1 / 2;
	lenth2 = Lenth2 / 2;
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);

	m_vecPoint.push_back(center);
	m_vecPoint.push_back(Pa1);
	m_vecPoint.push_back(Pa2);
	m_vecPoint.push_back(Pa3);
	m_vecPoint.push_back(Pa4);
	UpdateLinePath();
	////中心
	//ControlList << new ControlItem(this, center, 0);
	////中心线控制点
	//ControlList << new ControlItem(this, Pa1, 1);
	//ControlList << new ControlItem(this, Pa2, 2);
	//ControlList << new ControlItem(this, Pa3, 3);
	//ControlList << new ControlItem(this, Pa4, 4);
}

RectangleRItem::~RectangleRItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

int RectangleRItem::SetData(QJsonObject & strData)
{	
	MRotatedRect _data;
	if (strData.contains("col"))	_data.col		= strData["col"].toString().toDouble();
	if (strData.contains("row"))	_data.row		= strData["row"].toString().toDouble();
	if (strData.contains("phi"))	_data.phi		= strData["phi"].toString().toDouble();
	if (strData.contains("lenth1"))	_data.lenth1	= strData["lenth1"].toString().toDouble();
	if (strData.contains("lenth2"))	_data.lenth2	= strData["lenth2"].toString().toDouble();
	SetRotatedRect(_data);
	return 0;
}

int RectangleRItem::GetData(QJsonObject & strData)
{	
	MRotatedRect _data;
	GetRotatedRect(_data);
	strData.insert("col",		QString::number(_data.col));
	strData.insert("row",		QString::number(_data.row));
	strData.insert("phi",		QString::number(_data.phi));
	strData.insert("lenth1",	QString::number(_data.lenth1));
	strData.insert("lenth2",	QString::number(_data.lenth2));
	return 0;
}

void RectangleRItem::UpdateLinePath()
{
	QPPath = QPainterPath();
	SetLinePath(Pa1, Pa3, lenth1, QPPath);
	//QPPath.moveTo(m_vecPoint[1]);
	//for (int i = 2; i < m_vecPoint.size(); i++)	{
	//	QPPath.lineTo(m_vecPoint[i]);
	//}
	//QPPath.lineTo(m_vecPoint[1]);
	//QPPath.setFillRule(Qt::WindingFill);

	//QVector<QPointF>	_vecPoint;
	//QMatrix transform;
	//QPointF _ptTemp = QPointF();
	//{
	//	_ptTemp = QPointF(m_vecPoint[0].x() - lenth1, m_vecPoint[0].y() + lenth2);
	//	transform.reset();
	//	transform = transform.translate(m_vecPoint[0].x(), m_vecPoint[0].y()).rotate(angle).translate(-m_vecPoint[0].x(), -m_vecPoint[0].y());
	//	_vecPoint << transform.map(_ptTemp);
	//}	{
	//	_ptTemp = QPointF(m_vecPoint[0].x() + lenth1, m_vecPoint[0].y() + lenth2);
	//	transform.reset();
	//	transform = transform.translate(m_vecPoint[0].x(), m_vecPoint[0].y()).rotate(angle).translate(-m_vecPoint[0].x(), -m_vecPoint[0].y());
	//	_vecPoint << transform.map(_ptTemp);
	//}	{
	//	_ptTemp = QPointF(m_vecPoint[0].x() - lenth1, m_vecPoint[0].y() - lenth2);
	//	transform.reset();
	//	transform = transform.translate(m_vecPoint[0].x(), m_vecPoint[0].y()).rotate(angle).translate(-m_vecPoint[0].x(), -m_vecPoint[0].y());
	//	_vecPoint << transform.map(_ptTemp);
	//}	{
	//	_ptTemp = QPointF(m_vecPoint[0].x() + lenth1, m_vecPoint[0].y() - lenth2);
	//	transform.reset();
	//	transform = transform.translate(m_vecPoint[0].x(), m_vecPoint[0].y()).rotate(angle).translate(-m_vecPoint[0].x(), -m_vecPoint[0].y());
	//	_vecPoint << transform.map(_ptTemp);
	//}
	//_vecPoint.push_back(_vecPoint[0]);
	//QPPath.setFillRule(Qt::WindingFill);
	//QPPath.addPolygon(_vecPoint);
}

bool RectangleRItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	if (QPPath.contains(pt)) {
		dir = INRECT;
		return true;
	}
	return false;
}

void RectangleRItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth); 
	UpdateLinePath();
	update();
}

int RectangleRItem::SetParam(QString key, QString value)
{
	MRotatedRect _data;
	GetRotatedRect(_data);
	if (key == cstItemCenterX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemCenterY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemLenth1)
	{
		_data.lenth1 = value.toDouble();
	}
	else if (key == cstItemLenth2)
	{
		_data.lenth2 = value.toDouble();
	}
	else if (key == cstItemPhi)
	{
		_data.phi = value.toDouble();
	}
	SetRotatedRect(_data);
	return 0;
}

int RectangleRItem::GetParam(QString key, QString&value)
{
	MRotatedRect _data;
	GetRotatedRect(_data);
	if (key == cstItemCenterX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemCenterY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemLenth1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth1));
	}
	else if (key == cstItemLenth2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth2));
	}
	else if (key == cstItemPhi)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.phi));
	}
	return 0;
}

int RectangleRItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	MRotatedRect _data;
	GetRotatedRect(_data);


	QVector<QPair<QString, QString>>			mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemCenterX,	ParamToValue(
		BaseItem::tr(cstItemCenterX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col	))));
	mapItem.push_back(QPair<QString, QString>(cstItemCenterY,	ParamToValue(
		BaseItem::tr(cstItemCenterY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row ))));
	mapItem.push_back(QPair<QString, QString>(cstItemLenth1,	ParamToValue(
		BaseItem::tr(cstItemLenth1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth1))));
	mapItem.push_back(QPair<QString, QString>(cstItemLenth2,	ParamToValue(
		BaseItem::tr(cstItemLenth2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth2))));
	mapItem.push_back(QPair<QString, QString>(cstItemPhi,		ParamToValue(
		BaseItem::tr(cstItemPhi.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.phi))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF RectangleRItem::boundingRect() const
{
	qreal tmp = (lenth1 > lenth2 ? lenth2 : lenth1);
	return QRectF(center.x() - tmp, center.y() - tmp, tmp * 2, tmp * 2);
}

bool RectangleRItem::updatePos(int index, QPointF pt)
{
	if(index < m_vecPoint.size())	m_vecPoint[index] = pt;
	QPointF Pf = m_vecPoint[index];
	qreal dx = Pf.x() - center.x();
	qreal dy = Pf.y() - center.y();
	if (dx >= 0 && dy < 0){
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0){
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0){
		angle = _PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0){
		angle = _PI * 2 - atan2(dy, dx);
	}
	//角度补偿 角度方向逆时针 控制点排列方向 顺时针
	switch (index){
	case 1:
		angle += _PI / 2;
		break;
	case 3:
		angle += _PI * 3 / 2;
		break;
	case 4:
		angle += _PI;
		break;
	}
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	if (index == 2 || index == 4){
		lenth1 = sqrt(dx * dx + dy * dy);
	}
	else if (index == 1 || index == 3){
		lenth2 = sqrt(dx * dx + dy * dy);
	}
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);

	m_vecPoint[1] = Pa1;
	m_vecPoint[2] = Pa2;
	m_vecPoint[3] = Pa3;
	m_vecPoint[4] = Pa4;
	UpdateLinePath();
	//ControlList[1]->SetPoint(Pa1);
	//ControlList[2]->SetPoint(Pa2);
	//ControlList[3]->SetPoint(Pa3);
	//ControlList[4]->SetPoint(Pa4);
	return true;
}

void RectangleRItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	////画角度方向线
	//painter->drawLine(PArrow, Pa2);
	////绘制方向箭头
	//float l = 15.0;//箭头长度
	//double atn1 = atan2((PArrow.y() - Pa2.y()), (PArrow.x() - Pa2.x()));
	//double atn2 = atan2((PArrow.x() - Pa2.x()), (PArrow.y() - Pa2.y()));
	//QPointF Arrow1(PArrow.x() - l * cos(atn1 - 0.5), PArrow.y() - l * sin(atn1 - 0.5));
	//QPointF Arrow2(PArrow.x() - l * sin(atn2 - 0.5), PArrow.y() - l * cos(atn2 - 0.5));
	//if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	//painter->drawLine(PArrow, Arrow1);
	//painter->drawLine(PArrow, Arrow2);
	//painter->drawPath(QPPath);
	////绘制旋转矩形
	painter->save();
	painter->translate(center);
	painter->rotate(-angle * 180 / _PI);
	painter->drawRect(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	painter->restore();
}

void RectangleRItem::GetRotatedRect(MRotatedRect& MRRect)
{
	QPointF GetRRect = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	MRRect.col = GetRRect.x();
	MRRect.row = GetRRect.y();
	MRRect.phi = angle / _PI * 180.0;
	MRRect.lenth1 = lenth1;
	MRRect.lenth2 = lenth2;
}

void RectangleRItem::SetRotatedRect(MRotatedRect & MRRect)
{
	angle = MRRect.phi / 180.0 * _PI;
	lenth1 = MRRect.lenth1;
	lenth2 = MRRect.lenth2;
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	QPointF _center;
	_center.setX(MRRect.col);
	_center.setY(MRRect.row);
	QPointF	_Pa1 = _center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	QPointF	_Pa2 = _center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	QPointF	_Pa3 = _center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	QPointF	_Pa4 = _center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	QPointF	_PArrow = _center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);

	center = this->mapFromScene(_center);
	PArrow = this->mapFromScene(_PArrow);
	Pa1 = this->mapFromScene(_Pa1);
	Pa2 = this->mapFromScene(_Pa2);
	Pa3 = this->mapFromScene(_Pa3);
	Pa4 = this->mapFromScene(_Pa4);


	m_vecPoint[0] = this->mapFromScene(_center);
	m_vecPoint[1] = this->mapFromScene(_Pa1);
	m_vecPoint[2] = this->mapFromScene(_Pa2);
	m_vecPoint[3] = this->mapFromScene(_Pa3);
	m_vecPoint[4] = this->mapFromScene(_Pa4);
	UpdateLinePath();
	//ControlList[0]->SetPoint(this->mapFromScene(_center));
	//ControlList[1]->SetPoint(this->mapFromScene(_Pa1));
	//ControlList[2]->SetPoint(this->mapFromScene(_Pa2));
	//ControlList[3]->SetPoint(this->mapFromScene(_Pa3));
	//ControlList[4]->SetPoint(this->mapFromScene(_Pa4));
}
#pragma endregion

#pragma region 多边形
PolygonItem::PolygonItem() :BaseItem(QPointF(0, 0), ItemPolygon)
{
	m_bSelected = true;
	//init_points.clear();
	//list_ps.clear();
	m_vecPoint.push_back(center);
	//ControlList << new ControlItem(this, center, 0);
	//ControlList[0]->setVisible(false);
	bDrawFinished = false;
	UpdateLinePath();
}

PolygonItem::~PolygonItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

int PolygonItem::SetData(QJsonObject & strData)
{	
	MPolygon _data;
	QJsonArray array_Roiobject = strData.find("Polygon").value().toArray();
	if (!array_Roiobject.empty())									{
		for (size_t i = 0; i < array_Roiobject.count(); i++)		{
			QJsonArray _PointArray = array_Roiobject.at(i).toArray();
			QPointF	_ptCenter;
			if(_PointArray.count() >  0)	_ptCenter.setX(	_PointArray.at(0).toString().toDouble()	);
			if (_PointArray.count() > 1)	_ptCenter.setY( _PointArray.at(1).toString().toDouble()	);
			_data.points.push_back(_ptCenter);
			_data.list_p.push_back(_ptCenter);
		}
	}

	SetPolygon(_data);
	return 0;
}

int PolygonItem::GetData(QJsonObject & strData)
{	
	MPolygon _data;
	GetPolygon(_data);
	QJsonArray _PointConFig;
	for (int i = 0; i < _data.points.size(); i++)
	{
		QJsonArray _PointArray = {
			QString::number(_data.points[i].x()),
			QString::number(_data.points[i].y()	)
		};
		_PointConFig.append(_PointArray);
	}
	strData.insert("Polygon", _PointConFig);

	return 0;
}

QPointF PolygonItem::getCentroid(QList<QPointF> list)
{
	qreal x = 0;
	qreal y = 0;
	for (auto& temp : list)	{
		x += temp.x();
		y += temp.y();
	}
	x = x / list.size();
	y = y / list.size();
	return QPointF(x, y);
}

QPointF PolygonItem::getCentroid(QVector<QPointF>& list)
{
	qreal x = 0;
	qreal y = 0;
	for (int i = 1; i < list.size(); i++){
		x += list[i].x();
		y += list[i].y();
	}
	x = x / list.size();
	y = y / list.size();
	return QPointF(x, y);
}

void PolygonItem::getMaxLength()
{
	QVector<qreal> vec;
	vec.reserve(200);
	vec.clear();
	for (int i = 1; i < m_vecPoint.size(); i++)	{
		qreal dis = sqrt(pow(center.x() - m_vecPoint[i].x(), 2) + pow(center.y() - m_vecPoint[i].y(), 2));
		vec.append(dis);
	}
	qreal ret = 0;
	for (auto& temp : vec)	{
		if (temp > ret) {
			ret = temp;
		}
	}
	Radius = ret;
}

void PolygonItem::PushPos(QPointF pos)
{
	if (!bDrawFinished) {
		if (m_vecPoint.size() > 1)
		{
			if (   (pos.x() >= (m_vecPoint[1].x() - 2 * LineWidth))
				&& (pos.x() <= (m_vecPoint[1].x() + 2 * LineWidth))
				&& (pos.y() <= (m_vecPoint[1].y() + 2 * LineWidth))
				&& (pos.y() >= (m_vecPoint[1].y() - 2 * LineWidth))) 
			{
				bDrawFinished = true;
			}
			else	{
				m_vecPoint.push_back(pos);
			}
		}
		else	{
			m_vecPoint.push_back(pos);
		}

		center = getCentroid(m_vecPoint);
		m_vecPoint[0] = center;
		this->update();
	}
	UpdateLinePath();
}

QRectF PolygonItem::boundingRect() const
{
	return QRectF(0, 0, 10000, 10000);
}

bool PolygonItem::updatePos(int index, QPointF pt)
{
	Q_UNUSED(index);
	QList<QPointF> list;
	list.reserve(200);
	list.clear();
	m_vecPoint[index] = pt;
	for (int i = 1; i < m_vecPoint.size(); i++)	{
		list << m_vecPoint[i];
	}
	center = getCentroid(list);
	m_vecPoint[0] = center;
	//ControlList[0]->SetPoint(center);
	return true;
}

void PolygonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	BaseItem::paint(painter, option, widget);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	thisPen.setWidthF(LineWidth);
	painter->setPen(thisPen);
	painter->setBrush(QBrush(fillColor));
	if (bDrawFinished)	{
		for (int i = 1; i < m_vecPoint.size() - 1; i++)		{
			painter->drawLine(m_vecPoint[i], m_vecPoint[i + 1]);
		}
		painter->drawLine(m_vecPoint[m_vecPoint.size() - 1], m_vecPoint[1]);
	}
	else{
		for (int i = 1; i < m_vecPoint.size() - 1; i++)	{
			painter->drawLine(m_vecPoint[i], m_vecPoint[i + 1]);
		}
	}
}

void PolygonItem::UpdateLinePath()
{
	QPPath = QPainterPath();
	if (bDrawFinished) {
		for (int i = 1; i < m_vecPoint.size() - 1; i++) {
			SetLinePath(m_vecPoint[i], m_vecPoint[i + 1], LineWidth, QPPath);
		}
		SetLinePath(m_vecPoint[m_vecPoint.size() - 1], m_vecPoint[1], LineWidth, QPPath);
	}
	else {
		for (int i = 1; i < m_vecPoint.size() - 1; i++) {
			SetLinePath(m_vecPoint[i], m_vecPoint[i + 1], LineWidth, QPPath);
		}
	}
}

void PolygonItem::GetPolygon(MPolygon& mpolygon)
{
	QList<QPointF> list_p;
	list_p.reserve(200);
	list_p.clear();
	for (int i = 1; i < m_vecPoint.size(); i++)
	{
		list_p.append(m_vecPoint[i]);
	}
	mpolygon.list_p = list_p;
	mpolygon.points = list_p;
	//mpolygon.list_ps = list_ps;
}

void PolygonItem::SetPolygon(MPolygon & mpolygon)
{
	QPointF _ptCenter = getCentroid(mpolygon.points);
	m_vecPoint.clear();
	m_vecPoint.push_back(_ptCenter);
	for (int i = 0; i < mpolygon.points.size(); i++) {
		m_vecPoint.push_back(mpolygon.points[i]);
	}
}

bool PolygonItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	if (QPPath.contains(pt) ) {
		dir = INRECT;
		return true;
	}
	return false;
}

void PolygonItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth); //UpdateLinePath();
	update();
}

int PolygonItem::SetParam(QString key, QString value)
{
	return 0;
}

int PolygonItem::GetParam(QString key, QString&value)
{
	return 0;
}

int PolygonItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	return 0;
}

void PolygonItem::SetDrawEnd()
{
	bDrawFinished = true;
}

bool PolygonItem::IsDrawEnd()
{
	return bDrawFinished;
}
#pragma endregion

#pragma region 椭圆
EllipseItem::EllipseItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi) 
	:RectangleRItem(x, y, Lenth1, Lenth2, Pi)
{
	types = ItemEllipse; 
	UpdateLinePath();
}

EllipseItem::~EllipseItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

int EllipseItem::SetData(QJsonObject & strData)
{
	MRotatedRect _data;
	if (strData.contains("col"))	_data.col = strData["col"].toString().toDouble();
	if (strData.contains("row"))	_data.row = strData["row"].toString().toDouble();
	if (strData.contains("phi"))	_data.phi = strData["phi"].toString().toDouble();
	if (strData.contains("lenth1"))	_data.lenth1 = strData["lenth1"].toString().toDouble();
	if (strData.contains("lenth2"))	_data.lenth2 = strData["lenth2"].toString().toDouble();
	SetRotatedRect(_data);

	return 0;
}

int EllipseItem::GetData(QJsonObject & strData)
{	
	MRotatedRect _data;
	GetRotatedRect(_data);
	strData.insert("col",		QString::number(_data.col));
	strData.insert("row",		QString::number(_data.row));
	strData.insert("phi",		QString::number(_data.phi));
	strData.insert("lenth1",	QString::number(_data.lenth1));
	strData.insert("lenth2",	QString::number(_data.lenth2));

	return 0;
}

void EllipseItem::SetRotatedRect(MRotatedRect& MRRect)
{
	RectangleRItem::SetRotatedRect(MRRect);
	UpdateLinePath();
}

bool EllipseItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}	
	if (QPPath.contains(pt)) {
		dir = INRECT;
		return true;
	}

	return false;
}

void EllipseItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);
	UpdateLinePath();
	update();
}

int EllipseItem::SetParam(QString key, QString value)
{
	MRotatedRect _data;
	GetRotatedRect(_data);
	if (key == cstItemCenterX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemCenterY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemLenth1)
	{
		_data.lenth1 = value.toDouble();
	}
	else if (key == cstItemLenth2)
	{
		_data.lenth2 = value.toDouble();
	}
	else if (key == cstItemPhi)
	{
		_data.phi = value.toDouble();
	}
	SetRotatedRect(_data);
	return 0;
}

int EllipseItem::GetParam(QString key, QString&value)
{
	MRotatedRect _data;
	GetRotatedRect(_data);
	if (key == cstItemCenterX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemCenterY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemLenth1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth1));
	}
	else if (key == cstItemLenth2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth2));
	}
	else if (key == cstItemPhi)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.phi));
	}
	return 0;
}

int EllipseItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	MRotatedRect _data;
	GetRotatedRect(_data);
	 QVector<QPair<QString, QString>>   mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemCenterX,	ParamToValue(
		BaseItem::tr(cstItemCenterX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col ))));
	mapItem.push_back(QPair<QString, QString>(cstItemCenterY,	ParamToValue(
		BaseItem::tr(cstItemCenterY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row))));
	mapItem.push_back(QPair<QString, QString>(cstItemLenth1,	ParamToValue(
		BaseItem::tr(cstItemLenth1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth1))));
	mapItem.push_back(QPair<QString, QString>(cstItemLenth2,	ParamToValue(
		BaseItem::tr(cstItemLenth2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.lenth2))));
	mapItem.push_back(QPair<QString, QString>(cstItemPhi,		ParamToValue(
		BaseItem::tr(cstItemPhi.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.phi))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF EllipseItem::boundingRect() const
{
	return QRectF(0, 0, 10000, 10000);
}

void EllipseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QPen selected_pen(QColor(147, 147, 147));
	selected_pen.setWidth(LineWidth / 2.0);
	selected_pen.setStyle(Qt::DashLine);
	painter->setPen(selected_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(QPPath);

	BaseItem::paint(painter, option, widget);

	painter->drawLine(center, Pa2);
	painter->save();
	painter->translate(center);
	painter->rotate(-angle * 180 / _PI);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	painter->drawEllipse(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	painter->restore();
}

void EllipseItem::UpdateLinePath()
{
	QPPath = QPainterPath();

	SetLinePath(Pa1, Pa3, lenth1, QPPath);
	//QPainterPath _QPPath = QPainterPath();

	////QPPath.addEllipse(QRectF(center.x() - lenth1, center.y() - lenth2, center.x() + lenth1 * 2, center.y() + lenth2 * 2));
	//_QPPath.addEllipse(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	//_QPPath.setFillRule(Qt::WindingFill);
	//QTransform transform;
	//transform.reset();
	//transform = transform.translate(center.x(), center.y()).rotate(angle).translate(-center.x(),-center.y());// .translate(center.x(), center.y());
	//QPPath = transform.map(_QPPath);
}
#pragma endregion

#pragma region 圆
CircleItem::CircleItem(qreal x, qreal y, qreal R) :BaseItem(QPointF(x, y), ItemCircle)
{
	Radius = R;
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(center + QPointF(R, 0));
	//ControlList << new ControlItem(this, center, 0);
	//ControlList << new ControlItem(this, center + QPointF(R, 0), 1);
}

int CircleItem::SetData(QJsonObject & strData)
{
	MCircle _data;
	if (strData.contains("col"))	_data.col		= strData["col"].toString().toDouble();
	if (strData.contains("row"))	_data.row		= strData["row"].toString().toDouble();
	if (strData.contains("radius"))	_data.radius	= strData["radius"].toString().toDouble();

	SetCircle(_data);
	return 0;
}

int CircleItem::GetData(QJsonObject & strData)
{	
	MCircle _data;
	GetCircle(_data);
	strData.insert("col",		QString::number(_data.col));
	strData.insert("row",		QString::number(_data.row));
	strData.insert("radius",	QString::number(_data.radius));

	return 0;
}

bool CircleItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	double dDistance = sqrt((pt.x() - m_vecPoint[0].x()) * (pt.x() - m_vecPoint[0].x()) + (pt.y() - m_vecPoint[0].y()) * (pt.y() - m_vecPoint[0].y()));
	if (dDistance <= Radius){
		dir = INRECT;
		return true;
	}
	return false;
}

void CircleItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth); //UpdateLinePath();
	update();
}

int CircleItem::SetParam(QString key, QString value)
{
	MCircle _data;
	GetCircle(_data);
	if (key == cstItemPosX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemPosY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemRadius)
	{
		_data.radius	= value.toDouble();
		Radius			= value.toDouble();
	}
	SetCircle(_data);
	return 0;
}

int CircleItem::GetParam(QString key, QString&value)
{
	MCircle _data;
	GetCircle(_data);
	if (key == cstItemPosX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemPosY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemRadius)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.radius));
	}
	return 0;
}

int CircleItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	MCircle _data;
	GetCircle(_data);
	
	QVector<QPair<QString, QString>>   mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosX,			ParamToValue(
		BaseItem::tr(cstItemPosX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY,			ParamToValue(
		BaseItem::tr(cstItemPosY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row))));
	mapItem.push_back(QPair<QString, QString>(cstItemRadius,		ParamToValue(
		BaseItem::tr(cstItemRadius.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.radius))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF CircleItem::boundingRect() const
{
	//return QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2);
	return QRectF(0, 0, 10000, 10000);
}

bool CircleItem::updatePos(int index, QPointF pt)
{
	m_vecPoint[index] = pt;
	QPointF Pf = m_vecPoint[index];
	QPointF tmp = Pf - center;
	Radius = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	return true;
}

void CircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	painter->drawEllipse(QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2));
}

CircleItem::~CircleItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

void CircleItem::GetCircle(MCircle& Cir)
{
	QPointF GetRec = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	Cir.col = GetRec.x();
	Cir.row = GetRec.y();
	Cir.radius = Radius;
}

void CircleItem::SetCircle(MCircle & Cir)
{
	QPointF ptPoit ;
	ptPoit.setX(Cir.col);
	ptPoit.setY(Cir.row);
	ptPoit = this->mapFromScene(ptPoit);
	m_vecPoint[0] = ptPoit;
	//ControlList[0]->SetPoint(ptPoit);
	center.setX(ptPoit.x());
	center.setY(ptPoit.y());
	Radius = Cir.radius;

	ptPoit.setX(center.x() + Radius);
	m_vecPoint[1] = ptPoit;
}
#pragma endregion

#pragma region 同心圆
ConcentricCircleItem::ConcentricCircleItem(qreal x, qreal y, qreal radiusMin, qreal radiusMax) :BaseItem(QPointF(x, y), ItemConcentric_Circle)
{
	RadiusMax = radiusMax;
	RadiusMin = radiusMin > radiusMax ? radiusMax : radiusMin;
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(center + QPointF(RadiusMin, 0));
	m_vecPoint.push_back(center + QPointF(RadiusMax, 0));
	//ControlList << new ControlItem(this, center, 0);
	//ControlList << new ControlItem(this, center + QPointF(RadiusMin, 0), 1);
	//ControlList << new ControlItem(this, center + QPointF(RadiusMax, 0), 2);
	UpdateLinePath();
}

bool ConcentricCircleItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {	return true;	}
	if (QPPath.contains(pt)) {
		dir = INRECT;	
		return true;
	}

	return false;
}

void ConcentricCircleItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);
	UpdateLinePath();
	update();
}

int ConcentricCircleItem::SetData(QJsonObject & strData)
{
	CCircle _data;
	if (strData.contains("col"))			_data.col			= strData["col"].toString().toDouble();
	if (strData.contains("row"))			_data.row			= strData["row"].toString().toDouble();
	if (strData.contains("big_radius"))		_data.big_radius	= strData["big_radius"].toString().toDouble();
	if (strData.contains("small_radius"))	_data.small_radius	= strData["small_radius"].toString().toDouble();

	SetConcentricCircle(_data);
	return 0;
}

int ConcentricCircleItem::GetData(QJsonObject & strData)
{
	CCircle _data;
	GetConcentricCircle(_data);
	strData.insert("col",			QString::number(_data.col));
	strData.insert("row",			QString::number(_data.row));
	strData.insert("big_radius",	QString::number(_data.big_radius));
	strData.insert("small_radius",	QString::number(_data.small_radius));
	return 0;
}

int ConcentricCircleItem::SetParam(QString key, QString value)
{
	CCircle _data;
	GetConcentricCircle(_data);
	if (key == cstItemPosX)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemPosY)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemSmallRadius)
	{
		_data.small_radius = value.toDouble();
	}
	else if (key == cstItemBigRadius)
	{
		_data.big_radius = value.toDouble();
	}
	SetConcentricCircle(_data);
	return 0;
}

int ConcentricCircleItem::GetParam(QString key, QString&value)
{
	CCircle _data;
	GetConcentricCircle(_data);
	if (key == cstItemPosX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemPosY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemSmallRadius)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.small_radius ));
	}
	else if (key == cstItemBigRadius)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.big_radius));
	}
	return 0;
}

int ConcentricCircleItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	CCircle _data;
	GetConcentricCircle(_data);
	
	QVector<QPair<QString, QString>>   mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosX,				ParamToValue(
		BaseItem::tr(cstItemPosX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col ))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY,				ParamToValue(
		BaseItem::tr(cstItemPosY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row ))));
	mapItem.push_back(QPair<QString, QString>(cstItemSmallRadius,		ParamToValue(
		BaseItem::tr(cstItemSmallRadius.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.small_radius))));
	mapItem.push_back(QPair<QString, QString>(cstItemBigRadius,		ParamToValue(
		BaseItem::tr(cstItemBigRadius.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.big_radius))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF ConcentricCircleItem::boundingRect() const
{
	//return QRectF(center.x() - RadiusMax, center.y() - RadiusMax, RadiusMax * 2, RadiusMax * 2);
	return QRectF(0, 0, 10000, 10000);
}

bool ConcentricCircleItem::updatePos(int index, QPointF pt)
{
	QPointF Pf = m_vecPoint[index];
	QPointF tmp = Pf - center;
	qreal R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	if (index == 1){
		m_vecPoint[index] = pt;
		tmp = pt - center;
		R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());

		//if (R > RadiusMax)	return false;
		RadiusMin = R;
	}
	else if (index == 2){
		m_vecPoint[index] = pt;
		tmp = pt - center;
		R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
		//if (R < RadiusMin)	return false;
		RadiusMax = R;
	}
	UpdateLinePath();
	return true;
}

void ConcentricCircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	QPainterPath pth;
	pth.addEllipse(QRectF(center.x() - RadiusMin, center.y() - RadiusMin, RadiusMin * 2, RadiusMin * 2));
	pth.addEllipse(QRectF(center.x() - RadiusMax, center.y() - RadiusMax, RadiusMax * 2, RadiusMax * 2));
	if(m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	painter->drawPath(pth);
	//绘制分割线	
	painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
	QLine line;
	double angle = 0;
	ring_small_points.clear();
	ring_big_points.clear();
	for (int i = 0; i <= segment_line_num; i++){
		QPointF pf_min, pf_max;
		if (0 <= angle && angle < 90){
			line = QLine(QPoint(center.x() + cos(angle * _PI / 180) * RadiusMin, center.y() - sin(angle * _PI / 180) * RadiusMin), QPoint(center.x() + cos(angle * _PI / 180) * RadiusMax, center.y() - sin(angle * _PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() + cos(angle * _PI / 180) * RadiusMin, center.y() - sin(angle * _PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() + cos(angle * _PI / 180) * RadiusMax, center.y() - sin(angle * _PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (90 <= angle && angle < 180)		{
			line = QLine(QPoint(center.x() - sin((angle - 90) * _PI / 180) * RadiusMin, center.y() - cos((angle - 90) * _PI / 180) * RadiusMin), QPoint(center.x() - sin((angle - 90) * _PI / 180) * RadiusMax, center.y() - cos((angle - 90) * _PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() - sin((angle - 90) * _PI / 180) * RadiusMin, center.y() - cos((angle - 90) * _PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() - sin((angle - 90) * _PI / 180) * RadiusMax, center.y() - cos((angle - 90) * _PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (180 <= angle && angle < 270)		{
			line = QLine(QPoint(center.x() - cos((angle - 180) * _PI / 180) * RadiusMin, center.y() + sin((angle - 180) * _PI / 180) * RadiusMin), QPoint(center.x() - cos((angle - 180) * _PI / 180) * RadiusMax, center.y() + sin((angle - 180) * _PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() - cos((angle - 180) * _PI / 180) * RadiusMin, center.y() + sin((angle - 180) * _PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() - cos((angle - 180) * _PI / 180) * RadiusMax, center.y() + sin((angle - 180) * _PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (270 <= angle && angle < 360)	{
			line = QLine(QPoint(center.x() + sin((angle - 270) * _PI / 180) * RadiusMin, center.y() + cos((angle - 270) * _PI / 180) * RadiusMin), QPoint(center.x() + sin((angle - 270) * _PI / 180) * RadiusMax, center.y() + cos((angle - 270) * _PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() + sin((angle - 270) * _PI / 180) * RadiusMin, center.y() + cos((angle - 270) * _PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() + sin((angle - 270) * _PI / 180) * RadiusMax, center.y() + cos((angle - 270) * _PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		painter->drawLine(line);
		angle = angle + (360.0 / (double)segment_line_num);
	}
}

void ConcentricCircleItem::UpdateLinePath()
{
	QPPath = QPainterPath();
	QPPath.addEllipse(QRectF(m_vecPoint[0].x() - RadiusMax, m_vecPoint[0].y() - RadiusMax, RadiusMax * 2, RadiusMax * 2));
	QPPath.addEllipse(QRectF(m_vecPoint[0].x() - RadiusMin, m_vecPoint[0].y() - RadiusMin, RadiusMin * 2, RadiusMin * 2));
	QPPath.setFillRule(Qt::OddEvenFill);
}

ConcentricCircleItem::~ConcentricCircleItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

void ConcentricCircleItem::GetConcentricCircle(CCircle& CCir)
{
	QPointF GetCCircle = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	CCir.col = GetCCircle.x();
	CCir.row = GetCCircle.y();
	CCir.small_radius = RadiusMin;
	CCir.big_radius = RadiusMax;
}

void ConcentricCircleItem::SetConcentricCircle(CCircle & CCir)
{
	QPointF ptPoit;
	ptPoit.setX(CCir.col);
	ptPoit.setY(CCir.row);
	ptPoit = this->mapFromScene(ptPoit);
	m_vecPoint[0] = ptPoit;
	//ControlList[0]->SetPoint(ptPoit);
	center.setX(ptPoit.x());
	center.setY(ptPoit.y());
	RadiusMin = CCir.small_radius;
	RadiusMax = CCir.big_radius;

	ptPoit.setX(center.x() + RadiusMin);
	//ControlList[1]->SetPoint(ptPoit);
	m_vecPoint[1] = ptPoit;

	ptPoit.setX(center.x() + RadiusMax);
	//ControlList[2]->SetPoint(ptPoit);
	m_vecPoint[2] = ptPoit;
	UpdateLinePath();

}
#pragma endregion

#pragma region 直线
LineItem::LineItem(qreal x1, qreal y1, qreal x2, qreal y2) :BaseItem(QPointF((x1 + x2) / 2, (y1 + y2) / 2), ItemLineObj)
{
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(QPointF(x1, y1));
	m_vecPoint.push_back(QPointF(x2, y2));

	P1 = QPointF(x1, y1);
	P2 = QPointF(x2, y2);

	UpdateLinePath();
}

LineItem::~LineItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

int LineItem::SetData(QJsonObject & strData)
{
	LineData _data;
	if (strData.contains("col"))			_data.col = strData["col"].toString().toDouble();
	if (strData.contains("row"))			_data.row = strData["row"].toString().toDouble();
	if (strData.contains("row1"))			_data.row1 = strData["row1"].toString().toDouble();
	if (strData.contains("col1"))			_data.col1 = strData["col1"].toString().toDouble();

	SetLineData(_data);
	return 0;
}

int LineItem::GetData(QJsonObject & strData)
{	
	LineData _data;
	GetLineData(_data);
	strData.insert("col",			QString::number(_data.col));
	strData.insert("row",			QString::number(_data.row));
	strData.insert("row1",			QString::number(_data.row1));
	strData.insert("col1",			QString::number(_data.col1));
	return 0;
}

void LineItem::GetLineData(LineData & line)
{
	QPointF Center	= this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	QPointF _p1		= this->mapToScene(m_vecPoint[1].x(), m_vecPoint[1].y());
	QPointF _p2		= this->mapToScene(m_vecPoint[2].x(), m_vecPoint[2].y());
	line.col = _p1.x();
	line.row = _p1.y();
	line.col1 = _p2.x();
	line.row1 = _p2.y();

}

void LineItem::SetLineData(LineData & line)
{
	P1				= this->mapFromScene(line.col, line.row);
	P2				= this->mapFromScene(line.col1, line.row1);
	center			= QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);
	m_vecPoint[0]	= center;
	m_vecPoint[1]	= P1;
	m_vecPoint[2]	= P2;
	UpdateLinePath();
	this->setFocus();
}

bool LineItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	if (linePath.contains(pt)) {
		dir = INRECT;	return true;
	}
	return false;
}

int LineItem::SetParam(QString key, QString value)
{
	LineData _data;
	GetLineData(_data);
	if (key == cstItemPosX1)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemPosY1)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemPosX2)
	{
		_data.col1 = value.toDouble();
	}
	else if (key == cstItemPosY2)
	{
		_data.row1 = value.toDouble();
	}
	SetLineData(_data);
	return 0;
}

int LineItem::GetParam(QString key, QString&value)
{
	LineData _data;
	GetLineData(_data);
	if (key == cstItemPosX1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemPosY1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemPosX2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col1));
	}
	else if (key == cstItemPosY2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row1));
	}
	return 0;
}

int LineItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	LineData _data;
	GetLineData(_data);

	QVector<QPair<QString, QString>>   mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosY1,			ParamToValue(
		BaseItem::tr(cstItemPosY1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX1,			ParamToValue(
		BaseItem::tr(cstItemPosX1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY2,			ParamToValue(
		BaseItem::tr(cstItemPosY2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row1))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX2,			ParamToValue(
		BaseItem::tr(cstItemPosX2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col1))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

void LineItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);
	UpdateLinePath();
	update();
}

QRectF LineItem::boundingRect() const
{
	//return QRectF(P1, P2);
	return QRectF(0, 0, 10000, 10000);
}

bool LineItem::updatePos(int index, QPointF pt)
{
	if (index == 1)		{
		m_vecPoint[index] = pt;
		P1 = m_vecPoint[index];
	}
	else if (index == 2){
		m_vecPoint[index] = pt;
		P2 = m_vecPoint[index];
	}
	//ControlList[0]->SetPoint(QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2));
	m_vecPoint[0] = QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);

	UpdateLinePath();
	return true;
}

void LineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	thisPen.setWidthF(1);
	painter->setPen(thisPen);	
	painter->setBrush(thisPen.color());
	painter->drawPath(linePath);
}

void LineItem::UpdateLinePath()
{
	linePath = QPainterPath();
	P1 = m_vecPoint[1];
	P2 = m_vecPoint[2];
	SetLinePath(P1, P2, LineWidth, linePath);
}
#pragma endregion

#pragma region 方向直线
LineArrowItem::LineArrowItem(qreal x1, qreal y1, qreal x2, qreal y2) :BaseItem(QPointF((x1 + x2) / 2, (y1 + y2) / 2), ItemArrowLineObj)
{
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(QPointF(x1, y1));
	m_vecPoint.push_back(QPointF(x2, y2));

	P1 = QPointF(x1, y1);
	P2 = QPointF(x2, y2);

	UpdateLinePath();
}

LineArrowItem::~LineArrowItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

int LineArrowItem::SetData(QJsonObject & strData)
{
	LineData _data;
	if (strData.contains("col"))			_data.col = strData["col"].toString().toDouble();
	if (strData.contains("row"))			_data.row = strData["row"].toString().toDouble();
	if (strData.contains("row1"))			_data.row1 = strData["row1"].toString().toDouble();
	if (strData.contains("col1"))			_data.col1 = strData["col1"].toString().toDouble();

	SetLineData(_data);
	return 0;
}

int LineArrowItem::GetData(QJsonObject & strData)
{	
	LineData _data;
	GetLineData(_data);
	strData.insert("col",			QString::number(_data.col));
	strData.insert("row",			QString::number(_data.row));
	strData.insert("row1",			QString::number(_data.row1));
	strData.insert("col1",			QString::number(_data.col1));
	return 0;
}

void LineArrowItem::GetLineData(LineData & line)
{
	QPointF Center	= this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	QPointF _p1		= this->mapToScene(m_vecPoint[1].x(), m_vecPoint[1].y());
	QPointF _p2		= this->mapToScene(m_vecPoint[2].x(), m_vecPoint[2].y());
	line.col		= _p1.x();
	line.row		= _p1.y();
	line.col1		= _p2.x();
	line.row1		= _p2.y();
}

void LineArrowItem::SetLineData(LineData & line)
{
	P1 = this->mapFromScene(line.col, line.row);
	P2 = this->mapFromScene(line.col1, line.row1);
	center = QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);

	m_vecPoint[0] = center;
	m_vecPoint[1] = P1;
	m_vecPoint[2] = P2;
	this->setFocus();
	UpdateLinePath();
}

void LineArrowItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);

	UpdateLinePath();
	update();

}

bool LineArrowItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)){
		return true;
	}
	if (linePath.contains(pt) || ArrowlinePath1.contains(pt) || ArrowlinePath2.contains(pt)) {
		dir = INRECT;
		return true;
	}
	return false;
}

int LineArrowItem::SetParam(QString key, QString value)
{
	LineData _data;
	GetLineData(_data);
	if (key == cstItemPosX1)
	{
		_data.col = value.toDouble();
	}
	else if (key == cstItemPosY1)
	{
		_data.row = value.toDouble();
	}
	else if (key == cstItemPosX2)
	{
		_data.col1 = value.toDouble();
	}
	else if (key == cstItemPosY2)
	{
		_data.row1 = value.toDouble();
	}
	SetLineData(_data);
	return 0;
}

int LineArrowItem::GetParam(QString key, QString & value)
{
	LineData _data;
	GetLineData(_data);
	if (key == cstItemPosX1 )
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col));
	}
	else if (key == cstItemPosY1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row));
	}
	else if (key == cstItemPosX2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col1));
	}
	else if (key == cstItemPosY2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row1));
	}
	return 0;
}

int LineArrowItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	LineData _data;
	GetLineData(_data);

	QVector<QPair<QString, QString>>   mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosY1, ParamToValue(
		BaseItem::tr(cstItemPosY1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX1, ParamToValue(
		BaseItem::tr(cstItemPosX1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY2, ParamToValue(
		BaseItem::tr(cstItemPosY2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.row1))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX2, ParamToValue(
		BaseItem::tr(cstItemPosX2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.col1))));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));

	return 0;
}

QRectF LineArrowItem::boundingRect() const
{
	//QRectF rect = QRectF(P1, P2);
	//rect.setHeight(fmax(rect.height(), 40));
	//rect.setWidth(fmax(rect.width() , 40));
	return QRectF(0, 0, 10000, 10000);
}

bool LineArrowItem::updatePos(int index, QPointF pt)
{
	if (index == 1)
	{
		m_vecPoint[index] = pt;
		P1 = m_vecPoint[index];
	}
	else if (index == 2)
	{
		m_vecPoint[index] = pt;
		P2 = m_vecPoint[index];
	}
	m_vecPoint[0] = QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);
	UpdateLinePath();

	return true;
}

void LineArrowItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	thisPen.setWidthF(1);
	painter->setPen(thisPen);
	painter->setBrush(thisPen.color());
	painter->drawPath(linePath);
	painter->drawPath(ArrowlinePath1);
	painter->drawPath(ArrowlinePath2);
}

void LineArrowItem::UpdateLinePath()
{
	linePath		= QPainterPath();
	ArrowlinePath1	= QPainterPath();
	ArrowlinePath2	= QPainterPath();

	P1 = m_vecPoint[1];
	P2 = m_vecPoint[2];
	SetLinePath(P1, P2, LineWidth, linePath);

	//绘制方向箭头
	float l = sqrt((P2.y() - P1.y()) * (P2.y() - P1.y()) + (P2.x() - P1.x()) * (P2.x() - P1.x())) / 6.0;//箭头长度
	l = qMin<int>(l,100);
	double atn1 = atan2((P2.y() - P1.y()), (P2.x() - P1.x()));
	double atn2 = atan2((P2.x() - P1.x()), (P2.y() - P1.y()));
	QPointF Arrow1(P2.x() - l * cos(atn1 - 0.5), P2.y() - l * sin(atn1 - 0.5));
	QPointF Arrow2(P2.x() - l * sin(atn2 - 0.5), P2.y() - l * cos(atn2 - 0.5));
	//ArrowlinePath1.moveTo(P2);
	//ArrowlinePath1.lineTo(Arrow1);
	//ArrowlinePath2.moveTo(P2);
	//ArrowlinePath2.lineTo(Arrow2);
	SetLinePath(P2, Arrow1, 0.5, ArrowlinePath1);
	SetLinePath(P2, Arrow2, 0.5, ArrowlinePath2);
}
#pragma endregion

#pragma region 自由直线
BezierLineItem::BezierLineItem() :BaseItem(QPointF(0,0), ItemBezierLineObj)
{
	m_bSelected			= true;
	m_vecPoint.clear();
	m_vecPoint.push_back(center);
	bDrawFinished		= false;
	bDrawNormalLine		= true;
	fTwoLineLenth		= 5;
	fNormalLineHeight	= 20;
	fNormalLineWidth	= 5;
	UpdateLinePath();
	m_dSigma			= 1;
	m_dThreshold		= 0.02;
	m_strTransition		= "all";
	m_strSelect			= "all";
	m_iSelectedIndex	= -1;
	bNeedCheckDistance = false;
}

BezierLineItem::~BezierLineItem()
{

}

int BezierLineItem::SetData(QJsonObject & strData)
{
	if (strData.contains("m_dSigma"))			m_dSigma			= strData["m_dSigma"].toString().toDouble();
	if (strData.contains("m_dThreshold"))		m_dThreshold		= strData["m_dThreshold"].toString().toDouble();
	if (strData.contains("m_strTransition"))	m_strTransition		= strData["m_strTransition"].toString();
	if (strData.contains("m_strSelect"))		m_strSelect			= strData["m_strSelect"].toString();
	if (strData.contains("m_iSelectedIndex"))	m_iSelectedIndex	= strData["m_iSelectedIndex"].toString().toDouble();
	if (strData.contains("fTwoLineLenth"))		fTwoLineLenth		= strData["fTwoLineLenth"].toString().toDouble();
	if (strData.contains("fNormalLineHeight"))	fNormalLineHeight	= strData["fNormalLineHeight"].toString().toDouble();
	if (strData.contains("fNormalLineWidth"))	fNormalLineWidth	= strData["fNormalLineWidth"].toString().toDouble();
	if (strData.contains(cstItemCheckDistance))	bNeedCheckDistance	= strData[cstItemCheckDistance].toString().toDouble();

	m_Measurelines.clear();
	QJsonArray _VecArray					= strData.find("VecArray").value().toArray();
	for (int i = 0; i < _VecArray.count(); i++)	{
		QJsonArray		_Object				= _VecArray.at(i).toArray();
		MeasureRect2	_rect;
		QPointF			_PtTemp;
		int _iItemIndex = 0;
		_PtTemp.setX(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_PtTemp.setY(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		auto ptPoit = this->mapFromScene(_PtTemp.x(), _PtTemp.y());
		_rect.line.setP1(ptPoit);

		_PtTemp.setX(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_PtTemp.setY(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;

		ptPoit = this->mapFromScene(_PtTemp.x(), _PtTemp.y());
		_rect.line.setP2(	ptPoit	);

		_rect.dLastLineWidth				= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dNormalLineWidth				= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dSigma						= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dThreshold					= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.strTransition					= _Object.at(_iItemIndex).toString();
		_iItemIndex++;
		_rect.strSelect						= _Object.at(_iItemIndex).toString();
		_iItemIndex++;
		_rect.bNeedCheckDistance			= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		m_Measurelines.push_back(_rect);
	}
	m_vecPoint.clear();
	QJsonArray _VecPosArray = strData.find("_VecPosArray").value().toArray();
	for (int i = 0; i < _VecPosArray.count(); i++) {
		QJsonArray		_Object				= _VecPosArray.at(i).toArray();
		QPointF			_PtTemp;
		_PtTemp.setX(_Object.at(0).toString().toDouble());
		_PtTemp.setY(_Object.at(1).toString().toDouble());
		auto ptPoit = this->mapFromScene(_PtTemp.x(), _PtTemp.y());
		m_vecPoint.push_back(ptPoit);
	}
	SetDrawEnd();
	return 0;
}

int BezierLineItem::GetData(QJsonObject & strData)
{
	//m_Measurelines = GetDrawLines();
	QVector<MeasureRect2> _vecData = m_Measurelines;
	QJsonArray _VecArray;
	for (int i = 0; i < _vecData.size(); i++)
	{
		QJsonArray _Array;
		QPointF _Rec = this->mapToScene(_vecData[i].line.x1(),_vecData[i].line.y1());
		_Array.append(QString::number(	_Rec.x()	));
		_Array.append(QString::number(	_Rec.y()	));
		QPointF _Rec1 = this->mapToScene(_vecData[i].line.x2(), _vecData[i].line.y2());
		_Array.append(QString::number(	_Rec1.x()	));
		_Array.append(QString::number(	_Rec1.y()	));
		_Array.append(QString::number(_vecData[i].dLastLineWidth));
		_Array.append(QString::number(_vecData[i].dNormalLineWidth));
		_Array.append(QString::number(_vecData[i].dSigma));
		_Array.append(QString::number(_vecData[i].dThreshold));
		_Array.append(QString(_vecData[i].strTransition));
		_Array.append(QString(_vecData[i].strSelect));
		_Array.append(QString::number(_vecData[i].bNeedCheckDistance));
		_VecArray.append(_Array);
	}
	strData.insert("VecArray",			_VecArray);

	QJsonArray _VecPosArray;
	for (int i = 0; i < m_vecPoint.size(); i++)	{
		QJsonArray _Array;
		QPointF _Rec = this->mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y());
		_Array.append(QString::number(_Rec.x()));
		_Array.append(QString::number(_Rec.y()));

		_VecPosArray.append(_Array);
	}
	strData.insert("_VecPosArray", _VecPosArray);

	strData.insert("m_dSigma",				QString::number(m_dSigma));
	strData.insert("m_dThreshold",			QString::number(m_dThreshold));
	strData.insert("m_strTransition",		QString(m_strTransition));
	strData.insert("m_strSelect",			QString(m_strSelect));
	strData.insert("m_iSelectedIndex",		QString::number(m_iSelectedIndex));
	strData.insert("fTwoLineLenth",			QString::number(fTwoLineLenth));
	strData.insert("fNormalLineHeight",		QString::number(fNormalLineHeight));
	strData.insert("fNormalLineWidth",		QString::number(fNormalLineWidth));
	strData.insert(cstItemCheckDistance,	QString::number(bNeedCheckDistance));

	return 0;
}

int BezierLineItem::ChangeParamTovecMeasure(QJsonObject & vecData, QVector<MeasureRect2>& vecMeasure)
{
	QVector<MeasureRect2> _vecMeasure;
	_vecMeasure.clear();
	QJsonArray _VecArray = vecData.find("VecArray").value().toArray();
	for (int i = 0; i < _VecArray.count(); i++)	{
		QJsonArray _Object		= _VecArray.at(i).toArray();

		MeasureRect2	_rect;
		QPointF			_PtTemp;
		int _iItemIndex = 0;
		_PtTemp.setX(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_PtTemp.setY(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_rect.line.setP1(_PtTemp);
		_PtTemp.setX(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_PtTemp.setY(	_Object.at(_iItemIndex).toString().toDouble()	);
		_iItemIndex++;
		_rect.line.setP2(_PtTemp);
		_rect.dLastLineWidth		= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dNormalLineWidth		= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dSigma				= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.dThreshold			= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;
		_rect.strTransition			= _Object.at(_iItemIndex).toString();
		_iItemIndex++;
		_rect.strSelect				= _Object.at(_iItemIndex).toString();
		_iItemIndex++;
		_rect.bNeedCheckDistance	= _Object.at(_iItemIndex).toString().toDouble();
		_iItemIndex++;

		_vecMeasure.push_back(_rect);
	}
	vecMeasure = (_vecMeasure);

	return 0;
}

void BezierLineItem::PushPos(QPointF pos)
{
	if (!bDrawFinished) {
		if (m_vecPoint.size() > 1)
		{
			if ((pos.x()	>= (m_vecPoint[1].x() - 2 * LineWidth))
				&& (pos.x() <= (m_vecPoint[1].x() + 2 * LineWidth))
				&& (pos.y() <= (m_vecPoint[1].y() + 2 * LineWidth))
				&& (pos.y() >= (m_vecPoint[1].y() - 2 * LineWidth)))
			{
				bDrawFinished = true;
			}
			else {
				m_vecPoint.push_back(pos);
			}
		}
		else {
			m_vecPoint.push_back(pos);
		}

		center = getCentroid(m_vecPoint);
		m_vecPoint[0] = center;
		this->update();
	}
	UpdateLinePath();
}

QPointF BezierLineItem::getCentroid(QList<QPointF> list)
{
	qreal x = 0;
	qreal y = 0;
	for (auto& temp : list) {
		x += temp.x();
		y += temp.y();
	}
	x = x / list.size();
	y = y / list.size();
	return QPointF(x, y);
}

QPointF BezierLineItem::getCentroid(QVector<QPointF>& list)
{
	qreal x = 0;
	qreal y = 0;
	for (int i = 0; i < list.size(); i++) {
		x += list[i].x();
		y += list[i].y();
	}
	x = x / list.size();
	y = y / list.size();
	return QPointF(x, y);
}

void BezierLineItem::GetPolygon(MPolygon & mpolygon)
{
	QList<QPointF> list_p;
	list_p.reserve(200);
	list_p.clear();
	for (int i = 1; i < m_vecPoint.size(); i++)
	{
		QPointF GetRec = this->mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y());
		list_p.append(GetRec);
	}
	mpolygon.list_p = list_p;
	mpolygon.points = list_p;
}

void BezierLineItem::SetPolygon(MPolygon & mpolygon)
{
	QPointF _ptCenter = getCentroid(mpolygon.points);
	m_vecPoint.clear();
	m_vecPoint.push_back(_ptCenter);
	for (int i = 0; i < mpolygon.points.size(); i++) {
		auto ptPoit = this->mapFromScene(mpolygon.points[i].x(), mpolygon.points[i].y());
		m_vecPoint.push_back(ptPoit);
	}

	QList<QPointF> list;
	for (int i = 1; i < m_vecPoint.size(); i++) {
		list << m_vecPoint[i];
	}
	center = getCentroid(list);
	m_vecPoint[0] = center;
}

bool BezierLineItem::IsMouseInItem(QPointF & pt, DIRECTION & dir, int & index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) {
		return true;
	}
	if (QPPath.contains(pt)) {
		dir = INRECT;
		return true;
	}
	return false;
}

void BezierLineItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth); 
	UpdateLinePath();
	update();
}

void BezierLineItem::SetDrawEnd()
{
	bDrawFinished = true;
}

bool BezierLineItem::IsDrawEnd()
{
	return bDrawFinished;
}

int BezierLineItem::SetParam(QString key, QString value)
{
	if (key == cstItemNormalSigma) {
		m_dSigma = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].dSigma = m_dSigma;
		}
	}
	else if (key == cstItemNormalThreshold) {
		m_dThreshold = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].dThreshold = m_dThreshold;
		}
	}
	else if (key == cstItemNormalTransition) {
		QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
		m_strTransition = strlst[ value.toInt()];
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].strTransition = m_strTransition;
		}
	}
	else if (key == cstItemNormalSelection) {
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		m_strSelect = strlst[value.toInt()];
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].strSelect = m_strSelect;
		}
	}
	else if (key == cstItemNormalSelectedIndex) {
		m_iSelectedIndex = value.toDouble();
	}
	else if (key == cstItemTwoLineLenth) {		//卡尺之间的距离
		fTwoLineLenth = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++)	{
			if(i == 0)
				m_Measurelines[i].dLastLineWidth = fTwoLineLenth / 2.0;
			else	{
				m_Measurelines[i].dLastLineWidth = fTwoLineLenth ;
			}
		}
	}
	else if (key == cstItemCheckDistance) {
		bNeedCheckDistance = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].bNeedCheckDistance = bNeedCheckDistance;
		}
	}
	else if (key == cstItemNormalLineHeight) {
		fNormalLineHeight = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].dNormalLineHeight = fNormalLineHeight;
		}
	}
	else if (key == cstItemNormalLineWidth) {
		fNormalLineWidth = value.toDouble();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			m_Measurelines[i].dNormalLineWidth = fNormalLineWidth;
		}
	}
	else	{

		for (auto& mapIter = m_mapParam.begin(); mapIter != m_mapParam.end(); mapIter++) {
			if (key == mapIter.key())	{
				mapIter.value() = value;
			}
		}

		QVector<MeasureRect2> _vecRect2 = GetDrawLines();
		for (int i = 0; i <		m_Measurelines.size(); i++) {
			auto& _rect			= m_Measurelines[i];
			if (key == (cstItemLine + QString::number(i) + cstItemNormalSigma))						{
				_rect.dSigma	= value.toDouble();
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalThreshold))			{
				_rect.dThreshold = value.toDouble();
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemTwoLineLenth))				{
				_rect.dLastLineWidth = value.toDouble();
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalLineWidth))			{
				_rect.dNormalLineWidth = value.toDouble();
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalTransition))			{
				QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
				_rect.strTransition = strlst[ value.toInt()];
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalSelection))			{
				QStringList strlst = QStringList{ "all" ,"first" ,"last" };
				_rect.strSelect = strlst[value.toInt()];
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemCheckDistance)) {
				_rect.bNeedCheckDistance = value.toDouble();
			}
			else	{
				for (auto& mapIter = _rect.mapParam.begin(); mapIter != _rect.mapParam.end(); mapIter++) {
					if (key == (	cstItemLine + QString::number(i) + mapIter.key()	)) {
						mapIter.value() = value;
					}
				}
			}
		}
	}
	UpdateLinePath();

	return 0;
}

int BezierLineItem::GetParam(QString key, QString&value)
{
	if (key == cstItemNormalSigma)																	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(m_dSigma), QStringList(), QString::number(0)	);
	}
	else if (key == cstItemNormalThreshold)															{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(m_dThreshold), QStringList(), QString::number(0)	);
	}
	else if (key == cstItemNormalTransition)														{
		int	_iIndex = 0;
		{
			QStringList strlst						= QStringList{ "all" ,"positive" ,"negative" };
			QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
			for (int i = 0; i < strlst.size(); i++) {	if (strlst[i] == m_strTransition) { _iIndex = i;	break; } }
			value = ParamToValue(
				BaseItem::tr(key.toStdString().c_str()), 1,			ParamType_Enum,			QString::number(_iIndex),		strlst);
		}
	}
	else if (key == cstItemNormalSelection)															{
		int	_iIndex = 0;
		{
			QStringList strlst						= QStringList{ "all" ,"first" ,"last" };
			QStringList strLanglst					= QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
			for (int i = 0; i < strlst.size(); i++) {	if (strlst[i] == m_strSelect) { _iIndex = i;	break; } }
			value = ParamToValue(
				BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum,		QString::number(_iIndex), strlst);
		}
	}
	else if (key == cstItemNormalSelectedIndex)														{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Int,		QString::number(m_iSelectedIndex), QStringList(), QString::number(-1) );
	}
	else if (key == cstItemTwoLineLenth)															{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(fTwoLineLenth), QStringList(), QString::number(0)	);
	}
	else if (key == cstItemNormalLineHeight)														{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(fNormalLineHeight), QStringList(), QString::number(0));
	}
	else if (key == cstItemNormalLineWidth)															{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(fNormalLineWidth), QStringList(), QString::number(0)	);
	}
	else if (key == cstItemCheckDistance) {
		QStringList strlst = QStringList{ "false" ,"true" };
		QStringList strLanglst = QStringList{ BaseItem::tr("false") ,BaseItem::tr("true") };
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum, QString::number(bNeedCheckDistance ? 1 : 0), strLanglst);
	}
	else {
		for (auto& mapIter = m_mapParam.begin(); mapIter != m_mapParam.end(); mapIter++) {
			if (key == mapIter.key()) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_String, QString(mapIter.value()));
			}
		}
		UpdateLinePath();
		for (int i = 0; i < m_Measurelines.size(); i++) {
			auto& _rect = m_Measurelines[i];
			if (key == (cstItemLine + QString::number(i) + cstItemPosX1)) {
				QPointF _Line1 = this->mapToScene(_rect.line.x1(), _rect.line.y1());
				QString _strValue = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 0, ParamType_Double,QString("%1").arg(QString::number(_Line1.x())));
				value = _strValue;
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemPosY1)) {
				QPointF _Line1 = this->mapToScene(_rect.line.x1(), _rect.line.y1());
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 0, ParamType_Double, QString::number(_Line1.y()));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemPosX2)) {
				QPointF _Line1 = this->mapToScene(_rect.line.x2(), _rect.line.y2());
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 0, ParamType_Double, QString::number(_Line1.x()));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemPosY2)) {
				QPointF _Line1 = this->mapToScene(_rect.line.x2(), _rect.line.y2());
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 0, ParamType_Double, QString::number(_Line1.y()));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalSigma)) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_rect.dSigma), QStringList(), QString::number(0));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalThreshold)) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_rect.dThreshold), QStringList(), QString::number(0));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemTwoLineLenth)) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_rect.dLastLineWidth), QStringList(), QString::number(0));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalLineWidth)) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_rect.dNormalLineWidth), QStringList(), QString::number(0));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalLineHeight)) {
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_rect.dNormalLineHeight), QStringList(), QString::number(0));
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalTransition)) {
				int	_iIndex = 0;
				{
					QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
					QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
					for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _rect.strTransition) { _iIndex = i;	break; } }
					value = ParamToValue(
						BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum, QString::number(_iIndex), strLanglst);
				}
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemNormalSelection)) {
				int	_iIndex = 0;
				{
					QStringList strlst = QStringList{ "all" ,"first" ,"last" };
					QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
					for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _rect.strTransition) { _iIndex = i;	break; } }
					value = ParamToValue(
						BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum, QString::number(_iIndex), strLanglst);
				}
			}
			else if (key == (cstItemLine + QString::number(i) + cstItemCheckDistance) ) {
				QStringList strlst = QStringList{ "false" ,"true" };
				QStringList strLanglst = QStringList{ BaseItem::tr("false") ,BaseItem::tr("true") };
				value = ParamToValue(
					BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Enum, QString::number(_rect.bNeedCheckDistance ? 1 : 0), strLanglst);
			}
			else {
				for (auto& mapIter = _rect.mapParam.begin(); mapIter != _rect.mapParam.end(); mapIter++) {
					if (key == (cstItemLine + QString::number(i) + mapIter.key())) {
						value = ParamToValue(
							BaseItem::tr(key.toStdString().c_str()), 1, ParamType_String, QString(mapIter.value()));
					}
				}
			}
		}
	}
	return 0;
}

int BezierLineItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	QVector<QPair<QString, QString>>				mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemNormalSigma,													ParamToValue(
		BaseItem::tr(cstItemNormalSigma.toStdString().c_str()),
		1, ParamType_Double,	QString::number(m_dSigma),			QStringList(), QString::number(0) )));
	mapItem.push_back(QPair<QString, QString>(cstItemNormalThreshold,												ParamToValue(
		BaseItem::tr(cstItemNormalThreshold.toStdString().c_str())
		, 1, ParamType_Double,	QString::number(m_dThreshold),		QStringList(), QString::number(0))));
	int	_iIndex = 0; 
	{	_iIndex = 0;
		QStringList strlst																= QStringList{ "all" ,"positive" ,"negative" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == m_strTransition)		{ _iIndex = i;	break; } }
		mapItem.push_back(QPair<QString, QString>(cstItemNormalTransition,											ParamToValue(
			BaseItem::tr(cstItemNormalTransition.toStdString().c_str()),
			1, ParamType_Enum,		QString::number(_iIndex), strLanglst)));
	}	{	_iIndex = 0;
		QStringList strlst																= QStringList{ "all" ,"first" ,"last" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == m_strSelect)			{ _iIndex = i;	break;	} }
		mapItem.push_back(QPair<QString, QString>(cstItemNormalSelection,											ParamToValue(
			BaseItem::tr(cstItemNormalSelection.toStdString().c_str()),
			1, ParamType_Enum,		QString::number(_iIndex), strLanglst)));
	}	{
		QStringList strlst																= QStringList{ "false" ,"true" };
		QStringList strLanglst = QStringList{ BaseItem::tr("false") ,BaseItem::tr("true") };
		mapItem.push_back(QPair<QString, QString>(cstItemCheckDistance,											ParamToValue(BaseItem::tr(cstItemCheckDistance.toStdString().c_str()),
			1, ParamType_Enum, QString::number(bNeedCheckDistance ? 1 : 0), strLanglst)));
	}
	mapItem.push_back(QPair<QString, QString>(cstItemNormalSelectedIndex,											ParamToValue(BaseItem::tr(cstItemNormalSelectedIndex.toStdString().c_str()),1, ParamType_Int,		QString::number(m_iSelectedIndex),	QStringList(), QString::number(-1), QString::number( m_Measurelines.size() - 1 ))));
	mapItem.push_back(QPair<QString, QString>(cstItemTwoLineLenth,													ParamToValue(BaseItem::tr(cstItemTwoLineLenth.toStdString().c_str()),1, ParamType_Double,	QString::number(fTwoLineLenth),		QStringList(), QString::number(0))));
	mapItem.push_back(QPair<QString, QString>(cstItemNormalLineHeight,												ParamToValue(BaseItem::tr(cstItemNormalLineHeight.toStdString().c_str()),1, ParamType_Double,	QString::number(fNormalLineHeight), QStringList(), QString::number(0)	)));
	mapItem.push_back(QPair<QString, QString>(cstItemNormalLineWidth,												ParamToValue(BaseItem::tr(cstItemNormalLineWidth.toStdString().c_str()), 1, ParamType_Double,	QString::number(fNormalLineWidth),	QStringList(), QString::number(0)	)));
	
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	UpdateLinePath();

	for (int i = 0; i < m_Measurelines.size(); i++)	{
		auto& _rect								= m_Measurelines[i];
		mapItem.clear();
		QPointF _Line1 = this->mapToScene(	_rect.line.x1()	, _rect.line.y1()	);
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemPosX1,					ParamToValue(
			BaseItem::tr(cstItemPosX1.toStdString().c_str())
			, 0, ParamType_Double,	QString::number(_Line1.x()	))));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemPosY1,					ParamToValue(
			BaseItem::tr(cstItemPosY1.toStdString().c_str())
			, 0, ParamType_Double,	QString::number(_Line1.y()	))));

		_Line1 = this->mapToScene(	_rect.line.x2()	, _rect.line.y2()	);
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemPosX2,					ParamToValue(
			BaseItem::tr(cstItemPosX2.toStdString().c_str())
			, 0, ParamType_Double,	QString::number(_Line1.x()	))));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemPosY2,					ParamToValue(
			BaseItem::tr(cstItemPosY2.toStdString().c_str())
			, 0, ParamType_Double,	QString::number(_Line1.y()	))));

		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalSigma,			ParamToValue(
			BaseItem::tr(cstItemNormalSigma.toStdString().c_str())
			,1, ParamType_Double,	QString::number(_rect.dSigma),				QStringList(), QString::number(0)	)));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalThreshold,		ParamToValue(
			BaseItem::tr(cstItemNormalThreshold.toStdString().c_str())
			,1, ParamType_Double,	QString::number(_rect.dThreshold),			QStringList(), QString::number(0)	)));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemTwoLineLenth,			ParamToValue(
			BaseItem::tr(cstItemTwoLineLenth.toStdString().c_str())
			,1, ParamType_Double,	QString::number(_rect.dLastLineWidth),		QStringList(), QString::number(0)	)));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalLineHeight,		ParamToValue(
			BaseItem::tr(cstItemNormalLineHeight.toStdString().c_str())
			,1, ParamType_Double,	QString::number(_rect.dNormalLineHeight),	QStringList(), QString::number(0))));
		mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalLineWidth,		ParamToValue(
			BaseItem::tr(cstItemNormalLineWidth.toStdString().c_str())
			, 1, ParamType_Double,	QString::number(_rect.dNormalLineWidth),	QStringList(), QString::number(0)	)));

		{	_iIndex = 0;
			QStringList strlst		= QStringList{ "all" ,"positive" ,"negative" };
			QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
			for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _rect.strTransition)	{ _iIndex = i;	break; } }
			mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalTransition,	ParamToValue(
				BaseItem::tr(cstItemNormalTransition.toStdString().c_str())
				, 1, ParamType_Enum,		QString::number(_iIndex), strLanglst)));
		}	{	_iIndex = 0;
			QStringList strlst		= QStringList{ "all" ,"first" ,"last" };
			QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
			for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _rect.strSelect)	{ _iIndex = i;	break; } }
			mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemNormalSelection,	ParamToValue(
				BaseItem::tr(cstItemNormalSelection.toStdString().c_str())
				, 1, ParamType_Enum,		QString::number(_iIndex),	strLanglst)));
		}	{
			QStringList strlst		= QStringList{ "false" ,"true" };
			QStringList strLanglst = QStringList{ BaseItem::tr("false") ,BaseItem::tr("true") };
			mapItem.push_back(QPair<QString, QString>(cstItemLine + QString::number(i) + cstItemCheckDistance,		ParamToValue(
				BaseItem::tr(cstItemCheckDistance.toStdString().c_str())
				, 1, ParamType_Enum, QString::number(_rect.bNeedCheckDistance ? 1 : 0), strLanglst)));
		}
		//mapValue.insert(cstItemLine + QString::number(i), mapItem);
		mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItemLine + QString::number(i), mapItem));

	}
	return 0;
}

QList<QLineF> BezierLineItem::GetDrawLines(QList<QPointF>& points)
{
	QList<QPointF> _vecSrcPoint;
	QList<QLineF> lstLine;
	for (int i = 1; i < m_vecPoint.size(); i++) {
		_vecSrcPoint.push_back(mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y()));
		//_vecSrcPoint.push_back(QPointF(m_vecPoint[i].x(), m_vecPoint[i].y()));
	}
	lstLine = get_beziernormalline_curves(_vecSrcPoint, points, 0.5, fTwoLineLenth, fNormalLineHeight);
	return lstLine;
}

QVector<MeasureRect2> BezierLineItem::GetDrawLines()
{
	QVector<MeasureRect2>	_vecMeasurelines;

	QList<QPointF> _vecSrcPoint;
	QList<QPointF> _vecDstPoint;
	QList<QPointF> _vecPoint;
	for (int i = 1; i < m_vecPoint.size(); i++) {
		_vecSrcPoint.push_back(mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y()));
	}
	auto _lines = get_beziernormalline_curves(_vecSrcPoint, _vecDstPoint, 0.5, fTwoLineLenth, fNormalLineHeight);

	double _dLineLenth = 0;
	QVector<double> _vecLenth;
	for (int i = 0; i < m_Measurelines.size(); i++) {
		_dLineLenth = _dLineLenth + m_Measurelines[i].dLastLineWidth;
		_vecLenth.push_back(_dLineLenth);
	}
	QVector<double> _vecCenterLenth;
	double			_dLineCenterLenth = 0;
	for (int i = 0; i < _vecLenth.size(); i++) {
		if (i == 0) {
			_dLineCenterLenth = m_Measurelines[i].dLastLineWidth;
		}
		else {
			_dLineCenterLenth = _dLineCenterLenth + m_Measurelines[i].dLastLineWidth;
		}
		_vecCenterLenth.push_back(_dLineCenterLenth);
	}
	QPointF _temp, _temp1;
	QPointF _tempRetn, _temp1Retn;
	double _dCurvesLenth = 0;
	int		_iCurrentRow = 0;

	for (int i = 1; i < _vecDstPoint.size(); i++) {
		_dCurvesLenth = _dCurvesLenth + sqrt((_vecDstPoint[i].x() - _vecDstPoint[i - 1].x())	* (_vecDstPoint[i].x() - _vecDstPoint[i - 1].x())
			+ (_vecDstPoint[i].y() - _vecDstPoint[i - 1].y()) * (_vecDstPoint[i].y() - _vecDstPoint[i - 1].y()));
		_temp = _vecDstPoint[i];
		_temp1 = _vecDstPoint[i - 1];

		if (_iCurrentRow < _vecCenterLenth.size()) {
			if (_dCurvesLenth > _vecCenterLenth[_iCurrentRow]) {

				if (_iCurrentRow < (_vecCenterLenth.size() - 1) && _iCurrentRow < _vecMeasurelines.size()) {
					GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
					_vecMeasurelines[_iCurrentRow].line.setP1(_tempRetn);
					_vecMeasurelines[_iCurrentRow].line.setP2(_temp1Retn);
					if (_iCurrentRow == 0)
					{
						QPointF ResP1, ResP2;
						GetLineArrowPoint(_vecMeasurelines[_iCurrentRow].line.p2(), _vecMeasurelines[_iCurrentRow].line.p1(), fNormalLineHeight / 3.0, 150, ResP1, ResP2);
						QPArrowPath.moveTo(_vecMeasurelines[_iCurrentRow].line.p1());
						QPArrowPath.lineTo(ResP1);
						QPArrowPath.moveTo(_vecMeasurelines[_iCurrentRow].line.p1());
						QPArrowPath.lineTo(ResP2);
					}
				}
				else {
					//_vecLenth.push_back(_vecLenth[_vecLenth.size() - 1] + fTwoLineLenth);
					if (_vecCenterLenth.size() == 0) {
						_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth / 2.0);
					}
					else if (_iCurrentRow < _vecCenterLenth.size()) {
						_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth);
					}
					GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
					MeasureRect2				rect;
					rect.line.setP1(_tempRetn);
					rect.line.setP2(_temp1Retn);
					rect.dSigma					= m_dSigma;
					rect.dThreshold				= m_dThreshold;
					rect.strTransition			= m_strTransition;
					rect.strSelect				= m_strSelect;
					rect.dLastLineWidth			= m_Measurelines.size() > 0 ? fTwoLineLenth : fTwoLineLenth / 2.0;
					rect.dNormalLineWidth		= fNormalLineWidth;
					_vecMeasurelines.push_back(rect);

					_vecMeasurelines[_iCurrentRow].line.setP1(_tempRetn);
					_vecMeasurelines[_iCurrentRow].line.setP2(_temp1Retn);
				}

				//计算法线
				_iCurrentRow++;
			}
		}
		else {
			if (_vecCenterLenth.size() == 0) {
				_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth / 2.0);
			}
			else if (_iCurrentRow < _vecCenterLenth.size()) {
				_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth);
			}
			if (_dCurvesLenth > _vecCenterLenth[_iCurrentRow]) {
				GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
				MeasureRect2				rect;
				rect.line.setP1(_tempRetn);
				rect.line.setP2(_temp1Retn);
				rect.dSigma						= m_dSigma;
				rect.dThreshold					= m_dThreshold;
				rect.strTransition				= m_strTransition;
				rect.strSelect					= m_strSelect;
				rect.dLastLineWidth				= m_Measurelines.size() > 0 ? fTwoLineLenth : fTwoLineLenth / 2.0;
				rect.dNormalLineWidth			= fNormalLineWidth;
				_vecMeasurelines.push_back(rect);

				_vecMeasurelines[_iCurrentRow].line.setP1(_tempRetn);
				_vecMeasurelines[_iCurrentRow].line.setP2(_temp1Retn);
				_iCurrentRow++;
			}

		}

	}

	//删除
	if (_iCurrentRow < _vecMeasurelines.size() && _vecMeasurelines.size() > 0) {
		for (int i = _vecMeasurelines.size() - 1; i >= _iCurrentRow; i--) {
			_vecMeasurelines.removeAt(i);
		}
	}

	for (int i = 0; i < m_Measurelines.size(); i++)	{
		if (i < _vecMeasurelines.size())	{
			_vecMeasurelines[i].dLastLineWidth		= m_Measurelines[i].dLastLineWidth;
			_vecMeasurelines[i].dNormalLineWidth	= m_Measurelines[i].dNormalLineWidth;
			_vecMeasurelines[i].dSigma				= m_Measurelines[i].dSigma;
			_vecMeasurelines[i].dThreshold			= m_Measurelines[i].dThreshold;
			
			_vecMeasurelines[i].strTransition		= m_Measurelines[i].strTransition;
			_vecMeasurelines[i].strSelect			= m_Measurelines[i].strSelect;
		}
	}

	return _vecMeasurelines;
}

QRectF BezierLineItem::boundingRect() const
{
	return QRectF(0, 0, 10000, 10000);
}

bool BezierLineItem::updatePos(int index, QPointF pt)
{
	Q_UNUSED(index);
	QList<QPointF> list;
	list.reserve(200);
	list.clear();
	m_vecPoint[index] = QPointF(pt.x(), pt.y());
	for (int i = 1; i < m_vecPoint.size(); i++) {
		list << m_vecPoint[i];
	}
	center = getCentroid(list);
	m_vecPoint[0] = center;
	UpdateLinePath();
	//ControlList[0]->SetPoint(center);
	return true;
}

void BezierLineItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QThread::msleep(1);
	BaseItem::paint(painter, option, widget);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	thisPen.setWidthF(LineWidth);
	painter->setPen(thisPen);
	painter->setBrush(QBrush(fillColor));
	painter->drawPath(QPPath);

	if (bDrawNormalLine)	{
		thisPen.setWidthF(1 / (*scale));
		painter->setPen(thisPen);
		//painter->drawPath(QNormalLinePath);

		//painter->setBrush(QBrush(Qt::blue));
		thisPen.setWidthF(1 / (*scale));
		thisPen.setColor(noSelectedPen.color());
		painter->setPen(thisPen);
		painter->drawPath(QPArrowPath);
		bool _bSelected = false;
		for (int i = 0; i < m_Measurelines.size(); i++) {
			if (_bSelected)		{
				thisPen.setWidthF(1 / (*scale));
				thisPen.setColor(noSelectedPen.color());
				painter->setPen(thisPen);
				_bSelected = false;
			}
			QPointF ResP[4];
			if (i == 0) {	painter->drawLine(m_Measurelines[i].line.p1(), m_Measurelines[i].line.p2());		}

			GetStartEndNormalLine(m_Measurelines[i].line.p1(), m_Measurelines[i].line.p2(), m_Measurelines[i].dNormalLineWidth / 2.0, ResP[0], ResP[1]);
			GetStartEndNormalLine(m_Measurelines[i].line.p2(), m_Measurelines[i].line.p1(), m_Measurelines[i].dNormalLineWidth / 2.0, ResP[2], ResP[3]);

			if (m_iSelectedIndex == i)		{
				thisPen.setWidthF(2 / (*scale));
				thisPen.setColor(SelectedPen.color());
				painter->setPen(thisPen);
				_bSelected = true;
			}
			painter->drawLine(ResP[0], ResP[1]);
			painter->drawLine(ResP[1], ResP[2]);
			painter->drawLine(ResP[2], ResP[3]);
			painter->drawLine(ResP[3], ResP[0]);
		}
	}
}

void BezierLineItem::UpdateLinePath()
{
	QList<QPointF> _vecSrcPoint;
	QList<QPointF> _vecDstPoint;
	QList<QPointF> _vecPoint;
	for (int i = 1; i < m_vecPoint.size(); i++) {
		//_vecSrcPoint.push_back(mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y()));
		_vecSrcPoint.push_back(QPointF(m_vecPoint[i].x(), m_vecPoint[i].y()));
	}
	_vecPoint = draw_bezierPoint_curves(_vecSrcPoint, _vecDstPoint, 0.5);
	QPPath = QPainterPath();
	for (int i = 0; i < _vecDstPoint.size() - 1; i++) {
		QPPath.moveTo(_vecDstPoint[i]);
		QPPath.lineTo(_vecDstPoint[i + 1]);
		//SetLinePath(_vecDstPoint[i], _vecDstPoint[i + 1], LineWidth / 2.0, QPPath);
	}

	//计算贝塞尔曲线的法线
	QNormalLinePath = QPainterPath();
	QPArrowPath		= QPainterPath();
	//auto _lines = get_beziernormalline_curves(_vecSrcPoint, _vecDstPoint, 0.5, fTwoLineLenth, fNormalLineHeight);
	//for (int i = 0; i < _lines.size(); i++){
	//	if (i == 0)	{
	//		QPointF ResP1, ResP2;
	//		GetLineArrowPoint(_lines[i].p2(), _lines[i].p1(), fNormalLineHeight / 3.0, 150, ResP1, ResP2);
	//		QPArrowPath.moveTo(_lines[i].p1());
	//		QPArrowPath.lineTo(ResP1);
	//		QPArrowPath.moveTo(_lines[i].p1());
	//		QPArrowPath.lineTo(ResP2);
	//	}
	//	QNormalLinePath.moveTo(_lines[i].p1());
	//	QNormalLinePath.lineTo(_lines[i].p2());
	//}

	UpdateMeasurePath();

}

void BezierLineItem::UpdateMeasurePath(bool	bToScene)
{
	QList<QPointF> _vecSrcPoint;
	QList<QPointF> _vecDstPoint;
	QList<QPointF> _vecPoint;
	for (int i = 1; i < m_vecPoint.size(); i++) {
		if (bToScene)	{
			_vecSrcPoint.push_back(mapToScene(m_vecPoint[i].x(), m_vecPoint[i].y()));
		}
		else	{
			_vecSrcPoint.push_back(QPointF(m_vecPoint[i].x(), m_vecPoint[i].y()));
		}
	}
	auto _lines = get_beziernormalline_curves(_vecSrcPoint, _vecDstPoint, 0.5, fTwoLineLenth, fNormalLineHeight);

	double _dLineLenth = 0;
	QVector<double> _vecLenth;
	for (int i = 0; i < m_Measurelines.size(); i++) {
		_dLineLenth = _dLineLenth + m_Measurelines[i].dLastLineWidth;
		_vecLenth.push_back(_dLineLenth);
	}
	QVector<double> _vecCenterLenth;
	double			_dLineCenterLenth = 0;
	for (int i = 0; i < _vecLenth.size(); i++) {
		if (i == 0)		{
			_dLineCenterLenth = m_Measurelines[i].dLastLineWidth;
		}
		else	{
			_dLineCenterLenth = _dLineCenterLenth + m_Measurelines[i].dLastLineWidth;
		}
		_vecCenterLenth.push_back(_dLineCenterLenth);
	}
	QPointF _temp,		_temp1;
	QPointF _tempRetn,	_temp1Retn;
	double _dCurvesLenth = 0;
	int		_iCurrentRow = 0;

	for (int i = 1; i < _vecDstPoint.size(); i++) {
		_dCurvesLenth = _dCurvesLenth + sqrt((_vecDstPoint[i].x() - _vecDstPoint[i - 1].x())	* (_vecDstPoint[i].x() - _vecDstPoint[i - 1].x())
			+ (_vecDstPoint[i].y() - _vecDstPoint[i - 1].y()) * (_vecDstPoint[i].y() - _vecDstPoint[i - 1].y()));
		_temp	= _vecDstPoint[i];
		_temp1	= _vecDstPoint[i - 1];

		if (_iCurrentRow < _vecCenterLenth.size())	{
			if (_dCurvesLenth > _vecCenterLenth[_iCurrentRow]) {
				if (_iCurrentRow < (_vecCenterLenth.size() - 1)) {
					GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
					m_Measurelines[_iCurrentRow].line.setP1(_tempRetn);
					m_Measurelines[_iCurrentRow].line.setP2(_temp1Retn);
					if (_iCurrentRow == 0)
					{
						QPointF ResP1, ResP2;
						GetLineArrowPoint(m_Measurelines[_iCurrentRow].line.p2(), m_Measurelines[_iCurrentRow].line.p1(), fNormalLineHeight / 3.0, 150, ResP1, ResP2);
						QPArrowPath.moveTo(m_Measurelines[_iCurrentRow].line.p1());
						QPArrowPath.lineTo(ResP1);
						QPArrowPath.moveTo(m_Measurelines[_iCurrentRow].line.p1());
						QPArrowPath.lineTo(ResP2);
					}
				}
				else {
					//_vecLenth.push_back(_vecLenth[_vecLenth.size() - 1] + fTwoLineLenth);
					if (_vecCenterLenth.size() == 0)	{
						_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth / 2.0 );
					}
					else if (_iCurrentRow < _vecCenterLenth.size() )  {
						_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth);
					}
					GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
					MeasureRect2				rect;
					rect.line.setP1(_tempRetn);
					rect.line.setP2(_temp1Retn);
					rect.dSigma					= m_dSigma;
					rect.dThreshold				= m_dThreshold;
					rect.strTransition			= m_strTransition;
					rect.strSelect				= m_strSelect;
					rect.dLastLineWidth			= m_Measurelines.size() > 0 ? fTwoLineLenth : fTwoLineLenth / 2.0;
					rect.dNormalLineWidth		= fNormalLineWidth;
					m_Measurelines.push_back(rect);

					m_Measurelines[_iCurrentRow].line.setP1(_tempRetn);
					m_Measurelines[_iCurrentRow].line.setP2(_temp1Retn);
				}

				//计算法线
				_iCurrentRow++;
			}
		}
		else	{
			if (_vecCenterLenth.size() == 0) {
				_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth / 2.0);
			}
			else if (_iCurrentRow < _vecCenterLenth.size() ) {
				_vecCenterLenth.push_back(_vecCenterLenth[_vecCenterLenth.size() - 1] + fTwoLineLenth);
			}
			if (_dCurvesLenth > _vecCenterLenth[_iCurrentRow]) {
				GetNormalLine(_temp, _temp1, fNormalLineHeight, _tempRetn, _temp1Retn);
				MeasureRect2				rect;
				rect.line.setP1(_tempRetn);
				rect.line.setP2(_temp1Retn);
				rect.dSigma					= m_dSigma;
				rect.dThreshold				= m_dThreshold;
				rect.strTransition			= m_strTransition;
				rect.strSelect				= m_strSelect;
				rect.dLastLineWidth			= m_Measurelines.size() > 0 ? fTwoLineLenth : fTwoLineLenth / 2.0;
				rect.dNormalLineWidth		= fNormalLineWidth;
				m_Measurelines.push_back(rect);

				m_Measurelines[_iCurrentRow].line.setP1(_tempRetn);
				m_Measurelines[_iCurrentRow].line.setP2(_temp1Retn);
				_iCurrentRow++;
			}

		}
	}

	//删除
	if (_iCurrentRow < m_Measurelines.size() && m_Measurelines.size() > 0)	{
		for (int i = m_Measurelines.size() - 1; i >= _iCurrentRow; i--)		{
			m_Measurelines.removeAt( i );
		}
	}

}

#pragma endregion

#pragma region 卡尺
int Caliper::SetData(QJsonObject & strData)
{
	return 0;
}

int Caliper::GetData(QJsonObject & strData)
{
	return 0;
}

Caliper::Caliper(qreal x1, qreal y1, qreal x2, qreal y2, qreal height) :BaseItem(QPointF((x1 + x2) / 2, (y1 + y2) / 2), ItemLineObj)
{	
	P1 = QPointF(x1, y1);
	P2 = QPointF(x2, y2);
	Height = height;
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(QPointF(x1, y1));
	m_vecPoint.push_back(QPointF(x2, y2));

	//ControlList << new ControlItem(this, center, 0);
	//ControlList << new ControlItem(this, QPointF(x1, y1), 1);
	//ControlList << new ControlItem(this, QPointF(x2, y2), 2);
	qreal dx = P1.x() - P2.x();
	qreal dy = P1.y() - P2.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = _PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = _PI * 2 - atan2(dy, dx);
	}
	Lenth = sqrt(dx * dx + dy * dy);
	qreal s = sin(angle);
	qreal c = cos(angle);
	m_vecPoint.push_back(center + QPointF(s * Height / 2, c * Height / 2));
	//ControlList << new ControlItem(this, center + QPointF(s * Height / 2, c * Height / 2), 3);			
}

bool Caliper::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index	)) {
		return true;
	}
	return false;
}

int Caliper::SetParam(QString key, QString value)
{
	return 0;
}

int Caliper::GetParam(QString key, QString&value)
{
	return 0;
}

int Caliper::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	return 0;
}

QRectF Caliper::boundingRect() const
{
	//int min = Lenth < Height ? Lenth : Height;
	//int size = min < 5 ? 5 : min;
	//QPointF c = (P1 + P2) / 2;
	//return QRectF(c.x() - size, c.y() - size, size, size);
	return QRectF(0, 0, 10000, 10000);
}

bool Caliper::updatePos(int index, QPointF pt)
{
	if (index == 1)
	{
		m_vecPoint[1] = pt;
		P1 = m_vecPoint[1];
	}
	else if (index == 2)
	{
		m_vecPoint[2] = pt;
		P2 = m_vecPoint[2];
	}
	else if (index == 3)
	{
		m_vecPoint[3] = pt;
		QPointF Pf = m_vecPoint[3];
		qreal dx = Pf.x() - center.x();
		qreal dy = Pf.y() - center.y();
		Height = sqrt(dx * dx + dy * dy) * 2;
	}
	center = QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);
	m_vecPoint[0] = center;
	//ControlList[0]->SetPoint(center);
	qreal dx = P1.x() - P2.x();
	qreal dy = P1.y() - P2.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = _PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = _PI * 2 - atan2(dy, dx);
	}
	qreal s = sin(angle);
	qreal c = cos(angle);
	m_vecPoint[3] = center + QPointF(s * Height / 2, c * Height / 2);

	//ControlList[3]->SetPoint(center + QPointF(s * Height / 2, c * Height / 2));
	Lenth = sqrt(dx * dx + dy * dy);
	return true;
}

void Caliper::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);	
	//绘制分割线	
	QLine line;
	painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
	double angle_buf = 180 * angle / _PI;
	line_small_points.clear();
	line_big_points.clear();
	for (int i = 0; i <= segment_line_num; i++)
	{
		QPointF pf_min, pf_max;
		if (0 <= angle_buf && angle_buf < 90)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * sin(angle), P1.y() - 0.5 * (double)Height * cos(angle));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * sin(angle), P2.y() - 0.5 * (double)Height * cos(angle));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * sin(angle), RecPS.y() + ((double)Height / (double)segment_line_num) * i * cos(angle));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * sin(angle), RecPE.y() + ((double)Height / (double)segment_line_num) * i * cos(angle));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (90 <= angle_buf && angle_buf < 180)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() + 0.5 * (double)Height * cos((angle_buf - 90) * _PI / 180), P1.y() - 0.5 * (double)Height * sin((angle_buf - 90) * _PI / 180));
			RecPE = QPoint(P2.x() + 0.5 * (double)Height * cos((angle_buf - 90) * _PI / 180), P2.y() - 0.5 * (double)Height * sin((angle_buf - 90) * _PI / 180));
			Ps = QPoint(RecPS.x() - ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 90) * _PI / 180), RecPS.y() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 90) * _PI / 180));
			Pe = QPoint(RecPE.x() - ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 90) * _PI / 180), RecPE.y() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 90) * _PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (180 <= angle_buf && angle_buf < 270)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * sin((angle_buf - 180) * _PI / 180), P1.y() - 0.5 * (double)Height * cos((angle_buf - 180) * _PI / 180));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * sin((angle_buf - 180) * _PI / 180), P2.y() - 0.5 * (double)Height * cos((angle_buf - 180) * _PI / 180));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 180) * _PI / 180), RecPS.y() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 180) * _PI / 180));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 180) * _PI / 180), RecPE.y() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 180) * _PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (270 <= angle_buf && angle_buf < 360)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * cos((angle_buf - 270) * _PI / 180), P1.y() + 0.5 * (double)Height * sin((angle_buf - 270) * _PI / 180));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * cos((angle_buf - 270) * _PI / 180), P2.y() + 0.5 * (double)Height * sin((angle_buf - 270) * _PI / 180));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 270) * _PI / 180), RecPS.y() - ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 270) * _PI / 180));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 270) * _PI / 180), RecPE.y() - ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 270) * _PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		painter->drawLine(line);
	}
	painter->setPen(QPen(QColor(0, 0, 255, 255), 3));
	painter->drawLine(P1, P2);	
	//绘制方向箭头
	float l = 22.0;//箭头长度
	double atn1 = atan2((P2.y() - P1.y()), (P2.x() - P1.x()));
	double atn2 = atan2((P2.x() - P1.x()), (P2.y() - P1.y()));
	QPointF Arrow1(P2.x() - l * cos(atn1 - 0.5), P2.y() - l * sin(atn1 - 0.5));
	QPointF Arrow2(P2.x() - l * sin(atn2 - 0.5), P2.y() - l * cos(atn2 - 0.5));
	painter->drawLine(P2, Arrow1);
	painter->drawLine(P2, Arrow2);
	painter->save();
	painter->translate(center);
	painter->setBrush(QBrush(fillColor));
	painter->rotate(-angle * 180 / _PI);
	QPen tp = painter->pen();
	tp.setWidthF(0);
	tp.setColor(fillColor);
	painter->setPen(tp);
	thisPen.setWidthF(0);
	painter->drawRect(-Lenth / 2, -abs(Height / 2), Lenth, abs(Height));
	painter->restore();
}

void Caliper::GetCaliper(CaliperP& CP)
{			
	QPointF GetCenter = this->mapToScene(center.x(), center.y());
	//CP.col = GetCenter.x();
	//CP.row = GetCenter.y();
	//CP.len1 = Lenth;
	//CP.len2 = Height;
	//CP.angle = angle;
	//CP.x1 = P1.x();
	//CP.y1 = P1.y();
	//CP.x2 = P2.x();
	//CP.y2 = P2.y();
	CP.height = Height;
	CP.pp1 = this->mapToScene(P1.x(), P1.y());
	CP.pp2 = this->mapToScene(P2.x(), P2.y());
}

Caliper::~Caliper()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
}

void Caliper::SetCaliper(QPointF p1, QPointF p2)
{	
	P1 = p1;
	P2 = p2;
	m_vecPoint[1] = P1;
	m_vecPoint[2] = P2;
	//ControlList[1] = new ControlItem(this, P1, 1);
	//ControlList[2] = new ControlItem(this, P2, 2);
	//updatePos(0);
	//updatePos(1);
	//updatePos(2);
	//updatePos(3);	
}
#pragma endregion

//直线卡尺
#pragma region 直线卡尺
LineCaliperItem::LineCaliperItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi)
	:BaseItem(QPointF(x, y), LineCaliperObj)
{
	angle = Pi;
	lenth1 = Lenth1 / 2;
	lenth2 = Lenth2 / 2;
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);
	m_vecPoint.clear();
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(Pa1);
	m_vecPoint.push_back(Pa2);
	m_vecPoint.push_back(Pa3);
	m_vecPoint.push_back(Pa4);
}

LineCaliperItem::LineCaliperItem()
	:BaseItem(QPointF(0, 0), LineCaliperObj)
{
	m_vecPoint.clear();
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(Pa1);
	m_vecPoint.push_back(Pa2);
	m_vecPoint.push_back(Pa3);
	m_vecPoint.push_back(Pa4);
}

LineCaliperItem::LineCaliperItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, qreal height)
	:BaseItem(QPointF((x1 + x2) / 2.0, (y1 + y2) / 2.0), LineCaliperObj)
{
	qreal dx = x2 - center.x();
	qreal dy = y2 - center.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = M_PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = M_PI * 2 - atan2(dy, dx);
	}
	lenth1 = sqrt(dx * dx + dy * dy);
	lenth2 = height;
	segment_line_width = width;

	//lenth1 = Lenth1 / 2;
	//lenth2 = Lenth2 / 2;
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);
	m_vecPoint.clear();
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(Pa1);
	m_vecPoint.push_back(Pa2);
	m_vecPoint.push_back(Pa3);
	m_vecPoint.push_back(Pa4);
	////中心
	//ControlList << new ControlItem(this, center, 0);
	////中心线控制点
	//ControlList << new ControlItem(this, Pa1, 1);
	//ControlList << new ControlItem(this, Pa2, 2);
	//ControlList << new ControlItem(this, Pa3, 3);
	//ControlList << new ControlItem(this, Pa4, 4);
}

LineCaliperItem::~LineCaliperItem()
{
	//for (auto iter : ControlList)
	//{
	//	if (iter != nullptr)	delete iter;	iter = nullptr;
	//}
	//ControlList.clear();
	m_vecPoint.clear();
}

int LineCaliperItem::SetData(QJsonObject & strData)
{
	LineCaliperP _data;
	if (strData.contains("y1"))				_data.y1 = strData["y1"].toString().toDouble();
	if (strData.contains("x1"))				_data.x1 = strData["x1"].toString().toDouble();
	if (strData.contains("y2"))				_data.y2 = strData["y2"].toString().toDouble();
	if (strData.contains("x2"))				_data.x2 = strData["x2"].toString().toDouble();
	if (strData.contains("num"))			_data.num = strData["num"].toString().toDouble();
	if (strData.contains("height"))			_data.height = strData["height"].toString().toDouble();
	if (strData.contains("width"))			_data.width = strData["width"].toString().toDouble();
	if (strData.contains("sSigma"))			_data.sSigma = strData["sSigma"].toString().toDouble();
	if (strData.contains("sThreshold"))		_data.sThreshold = strData["sThreshold"].toString().toDouble();
	if (strData.contains("sTransition"))	_data.sTransition = strData["sTransition"].toString();
	if (strData.contains("sSelect"))		_data.sSelect = strData["sSelect"].toString();
	if (strData.contains("sActiveNum"))		_data.sActiveNum = strData["sActiveNum"].toString().toDouble();

	SetLineData(_data);
	return 0;
}

int LineCaliperItem::GetData(QJsonObject & strData)
{
	LineCaliperP _data;
	GetLineData(_data);
	strData.insert("y1",			QString::number(_data.y1));
	strData.insert("x1",			QString::number(_data.x1));
	strData.insert("y2",			QString::number(_data.y2));
	strData.insert("x2",			QString::number(_data.x2));
	strData.insert("num",			QString::number(_data.num));
	strData.insert("height",		QString::number(_data.height));
	strData.insert("width",			QString::number(_data.width));
	strData.insert("sSigma",		QString::number(_data.sSigma));
	strData.insert("sThreshold",	QString::number(_data.sThreshold));
	strData.insert("sSelect",		QString(_data.sSelect));
	strData.insert("sTransition",	QString(_data.sTransition));
	strData.insert("sActiveNum",	QString::number(_data.sActiveNum));
	return 0;
}

void LineCaliperItem::GetLineData(LineCaliperP & MRRect)
{
	QPointF GetRRect = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	QPointF _Pt1 = this->mapToScene(m_vecPoint[2].x(), m_vecPoint[2].y());
	QPointF _Pt2 = this->mapToScene(m_vecPoint[4].x(), m_vecPoint[4].y());

	MRRect.x1 = _Pt1.x();
	MRRect.y1 = _Pt1.y();
	MRRect.x2 = _Pt2.x();
	MRRect.y2 = _Pt2.y();

	MRRect.height = lenth2;
	MRRect.width = segment_line_width;
	MRRect.num = segment_line_num;

	MRRect.sSigma		= m_dSigma;
	MRRect.sThreshold	= m_dThreshold;
	MRRect.sTransition	= m_strTransition;
	MRRect.sSelect		= m_strSelect;
	MRRect.sActiveNum	= m_ActiveNum;

}

void LineCaliperItem::SetLineData(LineCaliperP & MRRect)
{
	center = QPointF((MRRect.x1 + MRRect.x2) / 2.0, (MRRect.y1 + MRRect.y2) / 2.0);
	qreal dx = MRRect.x1 - center.x();
	qreal dy = MRRect.y1 - center.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = M_PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = M_PI * 2 - atan2(dy, dx);
	}
	lenth1 = sqrt(dx * dx + dy * dy);
	lenth2 = MRRect.height;
	segment_line_width = MRRect.width;
	segment_line_num = MRRect.num;

	qreal s = sin(-angle);
	qreal c = cos(-angle);
	QPointF _Center = this->mapFromScene((MRRect.x1 + MRRect.x2) / 2.0, (MRRect.y1 + MRRect.y2) / 2.0);

	QPointF _Pa1 = _Center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	QPointF _Pa2 = _Center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	QPointF _Pa3 = _Center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	QPointF _Pa4 = _Center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	QPointF _PArrow = _Center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);

	m_vecPoint[0] = _Center;
	m_vecPoint[1] = _Pa1;
	m_vecPoint[2] = _Pa2;
	m_vecPoint[3] = _Pa3;
	m_vecPoint[4] = _Pa4;
	//ControlList[0]->SetPoint(_Center);
	//ControlList[1]->SetPoint(_Pa1);
	//ControlList[2]->SetPoint(_Pa2);
	//ControlList[3]->SetPoint(_Pa3);
	//ControlList[4]->SetPoint(_Pa4);

	center	= _Center;
	Pa1		= _Pa1;
	Pa2		= _Pa2;
	Pa3		= _Pa3;
	Pa4		= _Pa4;
	PArrow	= _PArrow;

	m_dSigma			= MRRect.sSigma		;
	m_dThreshold		= MRRect.sThreshold	;
	m_strTransition		= MRRect.sTransition;
	m_strSelect			= MRRect.sSelect	;
	m_ActiveNum			= MRRect.sActiveNum	;

}

int LineCaliperItem::SetParam(QString key, QString value)
{
	LineCaliperP _data;
	GetLineData(_data);
	if (key == cstItemPosX1)
	{
		_data.x1 = value.toDouble();
	}
	else if (key == cstItemPosY1)
	{
		_data.y1 = value.toDouble();
	}
	else if (key == cstItemPosX2)
	{
		_data.x2 = value.toDouble();
	}
	else if (key == cstItemPosY2)
	{
		_data.y2 = value.toDouble();
	}
	else if (key == cstItemNumber)
	{
		_data.num = value.toDouble();
		if (_data.sActiveNum > _data.num)
		{
			_data.sActiveNum = _data.num - 2;
		}
		_data.sActiveNum = qMax<int>(_data.sActiveNum,0);
	}
	else if (key == cstItemWidth)
	{
		_data.width = value.toDouble();
	}
	else if (key == cstItemHeight)
	{
		_data.height = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSigma])
	{
		_data.sSigma = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sThreshold])
	{
		_data.sThreshold = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sActiveNum])
	{
		_data.sActiveNum = value.toDouble();
		if (_data.sActiveNum > _data.num)
		{
			_data.sActiveNum = _data.num - 2;
		}
		_data.sActiveNum = qMax<int>(_data.sActiveNum, 0);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSelect])
	{
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		_data.sSelect = strlst[value.toDouble()];
	}
	else if (key == mapKeyString[EnumBaseItemStr_sTransition])
	{
		QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
		_data.sTransition = strlst[value.toDouble()];
	}
	SetLineData(_data);
	return 0;
}

int LineCaliperItem::GetParam(QString key, QString & value)
{
	LineCaliperP _data;
	GetLineData(_data);

	int	_iIndex = 0;
	if (key == cstItemPosX1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x1));
	}
	else if (key == cstItemPosY1)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y1));
	}
	else if (key == cstItemPosX2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x2));
	}
	else if (key == cstItemPosY2)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y2));
	}
	else if (key == cstItemNumber)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.num));
	}
	else if (key == cstItemWidth)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width), QStringList(), "0");
	}
	else if (key == cstItemHeight)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.height), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSigma])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sSigma].toStdString().c_str()), 
			1, ParamType_Double, QString::number(_data.sSigma), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sThreshold])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sThreshold].toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.sThreshold), QStringList(), "0","255");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sActiveNum])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sActiveNum].toStdString().c_str()), 1, ParamType_Double, QString::number(_data.sActiveNum), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSelect])
	{
		_iIndex = 0;
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sSelect) { _iIndex = i;	break; } }
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sSelect].toStdString().c_str()),
			1, ParamType_Enum, QString::number(_iIndex), strLanglst);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sTransition])
	{
		_iIndex = 0;
		QStringList strlst		= QStringList{ "all" ,"positive" ,"negative" };
		QStringList strLanglst	= QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sTransition) { _iIndex = i;	break; } }
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sTransition].toStdString().c_str()),
			1, ParamType_Enum, QString::number(_iIndex), strLanglst);
	}
	return 0;
}

int LineCaliperItem::GetAllParam(QVector<QPair<QString, QVector<QPair<QString, QString>>>>&mapValue)
{
	LineCaliperP _data;
	GetLineData(_data);
	QVector<QPair<QString, QString>>			mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosY1,	ParamToValue(
		BaseItem::tr(cstItemPosY1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y1))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX1,	ParamToValue(
		BaseItem::tr(cstItemPosX1.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x1))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY2,	ParamToValue(
		BaseItem::tr(cstItemPosY2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y2))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosX2,	ParamToValue(
		BaseItem::tr(cstItemPosX2.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x2))));
	mapItem.push_back(QPair<QString, QString>(cstItemNumber,	ParamToValue(
		BaseItem::tr(cstItemNumber.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.num),QStringList(), "0")));
	mapItem.push_back(QPair<QString, QString>(cstItemWidth,	ParamToValue(
		BaseItem::tr(cstItemWidth.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width),QStringList(), "0")));
	mapItem.push_back(QPair<QString, QString>(cstItemHeight,	ParamToValue(
		BaseItem::tr(cstItemHeight.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.height),QStringList(), "0")));
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sSigma],		ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sSigma].toStdString().c_str()),
		1, ParamType_Double, QString::number(_data.sSigma),QStringList(),"0","255")));
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sThreshold],	ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sThreshold].toStdString().c_str()),
		1, ParamType_Int, QString::number(_data.sThreshold),QStringList(),"0","255")));
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sActiveNum],	ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sActiveNum].toStdString().c_str()),
		1, ParamType_Int, QString::number(_data.sActiveNum),QStringList(),"0", QString::number(_data.num))));

	int	_iIndex = 0;
	{	_iIndex = 0;
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sSelect) { _iIndex = i;	break; } }
		mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sSelect],ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sSelect].toStdString().c_str()),
				1, ParamType_Enum, QString::number(_iIndex), strLanglst)));
	}

	{	_iIndex = 0;
		QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sTransition) { _iIndex = i;	break; } }
		mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sTransition], 
			ParamToValue(BaseItem::tr(mapKeyString[EnumBaseItemStr_sTransition].toStdString().c_str()),
				1, ParamType_Enum, QString::number(_iIndex), strLanglst)));
	}
	//mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sTransition],	ParamToValue(1, ParamType_String, QString(_data.sTransition)));
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF LineCaliperItem::boundingRect() const
{
	return QRectF(0, 0, 100000, 100000);
}

bool LineCaliperItem::updatePos(int index, QPointF pt)
{
	m_vecPoint[index] = pt;
	UpDate(index);
	return false;
}

bool LineCaliperItem::UpDate(int index)
{
	QPointF Pf = m_vecPoint[index];// ControlList[index]->GetPoint();
	if (index == 2 || index == 4)
	{
		center = QPointF((m_vecPoint[2].x() + m_vecPoint[4].x()) / 2.0,
			(m_vecPoint[2].y() + m_vecPoint[4].y()) / 2.0);
	}
	qreal dx = Pf.x() - center.x();
	qreal dy = Pf.y() - center.y();
	if (index == 2 || index == 4)
	{
		if (dx >= 0 && dy < 0)
		{
			angle = atan2((-1) * (dy), dx);
		}
		else if (dx < 0 && dy < 0)
		{
			angle = atan2((-1) * dy, dx);
		}
		else if (dx < 0 && dy >= 0)
		{
			angle = M_PI * 2 + atan2((-1) * dy, dx);
		}
		else if (dx >= 0 && dy >= 0)
		{
			angle = M_PI * 2 - atan2(dy, dx);
		}
		//角度补偿 角度方向逆时针 控制点排列方向 顺时针
		switch (index)
		{
		case 1:
			angle += M_PI / 2;
			break;
		case 3:
			angle += M_PI * 3 / 2;
			break;
		case 4:
			angle += M_PI;
			break;
		}
	}
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	if (index == 2 || index == 4)
	{
		lenth1 = sqrt(dx * dx + dy * dy);
	}
	else if (index == 1 || index == 3)
	{
		lenth2 = sqrt(dx * dx + dy * dy);
	}
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);
	m_vecPoint[0] = center;
	m_vecPoint[1] = Pa1;
	m_vecPoint[2] = Pa2;
	m_vecPoint[3] = Pa3;
	m_vecPoint[4] = Pa4;
	return true;
}

void LineCaliperItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QThread::msleep(1);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	switch (m_eRegionType) {
	case RegionType_Add:
		painter->setBrush(QBrush(fillColor));
		break;
	case RegionType_Differe:
		painter->setBrush(Qt::NoBrush);
		break;
	default:	break;
	}
	painter->setPen(QPen(QColor(0, 0, 255, 255), 3));
	{	//绘制旋转箭头
		float l = 30;//箭头长度
		double atn1 = atan2((Pa3.y() - Pa1.y()), (Pa3.x() - Pa1.x()));
		double atn2 = atan2((Pa3.x() - Pa1.x()), (Pa3.y() - Pa1.y()));
		QPointF Arrow1(Pa3.x() - l * cos(atn1 - 0.5), Pa3.y() - l * sin(atn1 - 0.5));
		QPointF Arrow2(Pa3.x() - l * sin(atn2 - 0.5), Pa3.y() - l * cos(atn2 - 0.5));
		painter->drawLine(Pa3, Arrow1);
		painter->drawLine(Pa3, Arrow2);
	}
	//画角度方向线
	painter->drawLine(Pa2, Pa4);
	{	//绘制旋转箭头
		float l = 30;//箭头长度
		double atn1 = atan2((Pa2.y() - Pa4.y()), (Pa2.x() - Pa4.x()));
		double atn2 = atan2((Pa2.x() - Pa4.x()), (Pa2.y() - Pa4.y()));
		QPointF Arrow1(Pa2.x() - l * cos(atn1 - 0.5), Pa2.y() - l * sin(atn1 - 0.5));
		QPointF Arrow2(Pa2.x() - l * sin(atn2 - 0.5), Pa2.y() - l * cos(atn2 - 0.5));
		painter->drawLine(Pa2, Arrow1);
		painter->drawLine(Pa2, Arrow2);
	}
	//绘制旋转矩形
	painter->save();
	painter->translate(center);
	painter->rotate(-angle * 180 / M_PI);
	{
		painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
		if (segment_line_num > 1) {
			qreal _EveryDx = lenth1 * 2 / (segment_line_num - 1);
			for (int i = 0; i < segment_line_num; i++) {
				QPointF _ptPoint1 = QPointF(-lenth1 + _EveryDx * i, -lenth2);
				QPointF _ptPoint2 = QPointF(-lenth1 + _EveryDx * i, lenth2);
				painter->drawRect(QRectF(-lenth1 + _EveryDx * i - segment_line_width / 2.0, -lenth2, segment_line_width, lenth2 * 2));
			}
		}
	}
	painter->setPen(QPen(QColor(0, 0, 255, 255), 1, Qt::DotLine));
	painter->drawRect(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	painter->setPen(QPen(QColor(0, 0, 255, 255), 1));
	painter->drawLine(0, -lenth2,0, lenth2);
	painter->restore();
}

void LineCaliperItem::UpdateLinePath()
{
	UpDate(0);
	linePath = QPainterPath();
	SetLinePath(Pa1, Pa3, lenth1, linePath);
	linePath.setFillRule(Qt::OddEvenFill);
}

#pragma endregion

#pragma region 圆弧卡尺
ArcCaliperItem::ArcCaliperItem(qreal x, qreal y,qreal Radius) :BaseItem(QPointF(x, y), ArcCaliperObj)
{
	m_vecPoint.clear();
	m_Radius = Radius;
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(center + QPointF(Radius, 0));

	m_vecPoint[1] = GetAnglePos(Radius, (m_dStartAngle + m_dEndAngle) / 2.0);
	m_vecPoint[2] = GetAnglePos(m_Radius + m_Height, (m_dStartAngle + m_dEndAngle) / 2.0);
	m_vecPoint[3] = GetAnglePos(m_Radius , m_dStartAngle  / 2.0);
	m_vecPoint[4] = GetAnglePos(m_Radius , m_dEndAngle / 2.0);

	UpdateLinePath();
	UpDateCenter();
}

ArcCaliperItem::ArcCaliperItem() :BaseItem(QPointF(0, 0),ArcCaliperObj)
{
	m_vecPoint.clear();
	m_vecPoint.push_back(center);
	m_vecPoint.push_back(center + QPointF(m_Radius, 0));

	m_vecPoint[1] = GetAnglePos(m_Radius, (m_dStartAngle + m_dEndAngle) / 2.0);
	m_vecPoint[2] = GetAnglePos(m_Radius + m_Height, (m_dStartAngle + m_dEndAngle) / 2.0);
	m_vecPoint[3] = GetAnglePos(m_Radius, m_dStartAngle / 2.0);
	m_vecPoint[4] = GetAnglePos(m_Radius, m_dEndAngle / 2.0);

	UpdateLinePath();
	UpDateCenter();
}

ArcCaliperItem::~ArcCaliperItem()
{

}

bool ArcCaliperItem::IsMouseInItem(QPointF& pt, DIRECTION & dir, int& index)
{
	if (BaseItem::IsMouseInItem(pt, dir, index)) { return true; }
	if (QPPath.contains(pt)) {
		dir = INRECT;
		return true;
	}

	return false;
}

void ArcCaliperItem::SetDrawPenWidth(qreal lenth)
{
	BaseItem::SetDrawPenWidth(lenth);
	UpdateLinePath();
	update();
}

int ArcCaliperItem::SetData(QJsonObject & strData)
{
	ArcCaliperP _data;
	if (strData.contains("col"))			_data.x				= strData["col"].toString().toDouble();
	if (strData.contains("row"))			_data.y				= strData["row"].toString().toDouble();
	if (strData.contains("Radius"))			_data.Radius		= strData["Radius"].toString().toDouble();
	if (strData.contains("dStartAngle"))	_data.dStartAngle	= strData["dStartAngle"].toString().toDouble();
	if (strData.contains("dEndAngle"))		_data.dEndAngle		= strData["dEndAngle"].toString().toDouble();
	if (strData.contains("num"))			_data.num			= strData["num"].toString().toDouble();
	if (strData.contains("height"))			_data.height		= strData["height"].toString().toDouble();
	if (strData.contains("width"))			_data.width			= strData["width"].toString().toDouble();
	if (strData.contains("sSigma"))			_data.sSigma		= strData["sSigma"].toString().toDouble();
	if (strData.contains("sThreshold"))		_data.sThreshold	= strData["sThreshold"].toString().toDouble();
	if (strData.contains("sActiveNum"))		_data.sActiveNum	= strData["sActiveNum"].toString().toDouble();
	if (strData.contains("sSelect"))		_data.sSelect		= strData["sSelect"].toString();
	if (strData.contains("sTransition"))	_data.sTransition	= strData["sTransition"].toString();

	SetArcCaliper(_data);
	return 0;
}

int ArcCaliperItem::GetData(QJsonObject & strData)
{
	ArcCaliperP _data;
	GetArcCaliper(_data);
	strData.insert("col",			QString::number(_data.x));
	strData.insert("row",			QString::number(_data.y));
	strData.insert("Radius",		QString::number(_data.Radius));
	strData.insert("dStartAngle",	QString::number(_data.dStartAngle));
	strData.insert("dEndAngle",		QString::number(_data.dEndAngle));
	strData.insert("num",			QString::number(_data.num));
	strData.insert("height",		QString::number(_data.height));
	strData.insert("width",			QString::number(_data.width));
	strData.insert("sSigma",		QString::number(_data.sSigma));
	strData.insert("sThreshold",	QString::number(_data.sThreshold));
	strData.insert("sActiveNum",	QString::number(_data.sActiveNum));
	strData.insert("sSelect",		QString(_data.sSelect));
	strData.insert("sTransition",	QString(_data.sTransition));

	
	return 0;
}

int ArcCaliperItem::SetParam(QString key, QString value)
{
	ArcCaliperP _data;
	GetArcCaliper(_data);
	if (key == cstItemPosX)
	{
		_data.x = value.toDouble();
	}
	else if (key == cstItemPosY)
	{
		_data.y = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sRadius])
	{
		_data.Radius = value.toDouble();
	}
	else if (key == cstItemNumber)
	{
		_data.num = value.toDouble();
		if (_data.sActiveNum > _data.num)
		{
			_data.sActiveNum = _data.num - 2;
		}
		_data.sActiveNum = qMax<int>(_data.sActiveNum, 0);
	}
	else if (key == cstItemWidth)
	{
		_data.width = value.toDouble();
	}
	else if (key == cstItemHeight)
	{
		_data.height = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSigma])
	{
		_data.sSigma = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sThreshold])
	{
		_data.sThreshold = value.toDouble();
	}
	else if (key == mapKeyString[EnumBaseItemStr_sEndAngle])
	{
		_data.dEndAngle = value.toDouble();
		m_vecPoint[1] = GetAnglePos(m_Radius, (m_dStartAngle + m_dEndAngle) / 2.0);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sStartAngle])
	{
		_data.dStartAngle = value.toDouble();
		m_vecPoint[1] = GetAnglePos(m_Radius, (m_dStartAngle + m_dEndAngle) / 2.0);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sActiveNum])
	{
		_data.sActiveNum = value.toDouble();
		if (_data.sActiveNum > _data.num)
		{
			_data.sActiveNum = _data.num - 2;
		}
		_data.sActiveNum = qMax<int>(_data.sActiveNum, 0);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSelect])
	{
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		_data.sSelect = strlst[value.toDouble()];
	}
	else if (key == mapKeyString[EnumBaseItemStr_sTransition])
	{
		QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
		_data.sTransition = strlst[value.toDouble()];
	}
	SetArcCaliper(_data);
	return 0;
}

int ArcCaliperItem::GetParam(QString key, QString&value)
{
	int	_iIndex = 0;
	ArcCaliperP _data;
	GetArcCaliper(_data);
	if (key == cstItemPosX)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x));
	}
	else if (key == cstItemPosY)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y));
	}
	else if (key == mapKeyString[EnumBaseItemStr_sRadius])
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.Radius));
	}
	else if (key == cstItemNumber)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.num));
	}
	else if (key == cstItemWidth)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width), QStringList(), "0");
	}
	else if (key == cstItemHeight)
	{
		value = ParamToValue(
			BaseItem::tr(key.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.height), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSigma])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sSigma].toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.sSigma), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sThreshold])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sThreshold].toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.sThreshold), QStringList(), "0", "255");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sStartAngle])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sStartAngle].toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.sThreshold), QStringList(), "-360", "360");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sEndAngle])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sEndAngle].toStdString().c_str()),
			1, ParamType_Double, QString::number(_data.sThreshold), QStringList(), "-360", "360");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sActiveNum])
	{
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sActiveNum].toStdString().c_str()), 1, ParamType_Double, QString::number(_data.sActiveNum), QStringList(), "0");
	}
	else if (key == mapKeyString[EnumBaseItemStr_sSelect])
	{
		_iIndex = 0;
		QStringList strlst = QStringList{ "all" ,"first" ,"last" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sSelect) { _iIndex = i;	break; } }
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sSelect].toStdString().c_str()),
			1, ParamType_Enum, QString::number(_iIndex), strLanglst);
	}
	else if (key == mapKeyString[EnumBaseItemStr_sTransition])
	{
		_iIndex = 0;
		QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
		QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
		for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sTransition) { _iIndex = i;	break; } }
		value = ParamToValue(
			BaseItem::tr(mapKeyString[EnumBaseItemStr_sTransition].toStdString().c_str()),
			1, ParamType_Enum, QString::number(_iIndex), strLanglst);
	}
	return 0;
}

int ArcCaliperItem::GetAllParam(QVector<QPair<QString,QVector<QPair<QString, QString>>>>&mapValue)
{
	ArcCaliperP _data;
	GetArcCaliper(_data);

	QVector<QPair<QString, QString>>				mapItem;
	mapItem.push_back(QPair<QString, QString>(cstItemPosX, ParamToValue(
		BaseItem::tr(cstItemPosX.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.x))));
	mapItem.push_back(QPair<QString, QString>(cstItemPosY, ParamToValue(
		BaseItem::tr(cstItemPosY.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.y))));
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sRadius], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sRadius].toStdString().c_str()),
		1, ParamType_Double, QString::number(_data.Radius))));

	mapItem.push_back(QPair<QString, QString>(cstItemNumber, ParamToValue(
		BaseItem::tr(cstItemNumber.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.num), QStringList(), "0")));
	mapItem.push_back(QPair<QString, QString>(cstItemWidth, ParamToValue(
		BaseItem::tr(cstItemWidth.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.width), QStringList(), "0")));
	mapItem.push_back(QPair<QString, QString>(cstItemHeight, ParamToValue(
		BaseItem::tr(cstItemHeight.toStdString().c_str()), 1, ParamType_Double, QString::number(_data.height), QStringList(), "0")));

	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sStartAngle], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sStartAngle].toStdString().c_str()),
		1, ParamType_Double, QString::number(_data.dStartAngle), QStringList(), "-360", "360")));

	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sEndAngle], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sEndAngle].toStdString().c_str()),
		1, ParamType_Double, QString::number(_data.dEndAngle), QStringList(), "-360", "360")));

	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sSigma], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sSigma].toStdString().c_str()),
		1, ParamType_Double, QString::number(_data.sSigma), QStringList(), "0", "255")));

	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sThreshold], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sThreshold].toStdString().c_str()),
		1, ParamType_Int, QString::number(_data.sThreshold), QStringList(), "0", "255")));
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sActiveNum], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sActiveNum].toStdString().c_str()),
		1, ParamType_Int, QString::number(_data.sActiveNum), QStringList(), "0", QString::number(_data.num))));

	int	_iIndex = 0;
	{	_iIndex = 0;
	QStringList strlst = QStringList{ "all" ,"first" ,"last" };
	QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("first") ,BaseItem::tr("last") };
	for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sSelect) { _iIndex = i;	break; } }
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sSelect], ParamToValue(
		BaseItem::tr(mapKeyString[EnumBaseItemStr_sSelect].toStdString().c_str()),
		1, ParamType_Enum, QString::number(_iIndex), strLanglst)));
	}

	{	_iIndex = 0;
	QStringList strlst = QStringList{ "all" ,"positive" ,"negative" };
	QStringList strLanglst = QStringList{ BaseItem::tr("all") ,BaseItem::tr("positive") ,BaseItem::tr("negative") };
	for (int i = 0; i < strlst.size(); i++) { if (strlst[i] == _data.sTransition) { _iIndex = i;	break; } }
	mapItem.push_back(QPair<QString, QString>(mapKeyString[EnumBaseItemStr_sTransition],
		ParamToValue(BaseItem::tr(mapKeyString[EnumBaseItemStr_sTransition].toStdString().c_str()),
			1, ParamType_Enum, QString::number(_iIndex), strLanglst)));
	}
	mapValue.push_back(QPair<QString, QVector<QPair<QString, QString>>>(cstItem, mapItem));
	return 0;
}

QRectF ArcCaliperItem::boundingRect() const
{
	//return QRectF(center.x() - RadiusMax, center.y() - RadiusMax, RadiusMax * 2, RadiusMax * 2);
	return QRectF(0, 0, 10000, 10000);
}

bool ArcCaliperItem::updatePos(int index, QPointF pt)
{
	QPointF Pf = m_vecPoint[index];
	QPointF tmp = Pf - center;
	qreal R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	if (index == 1) {
		tmp = pt - center;
		R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
		m_Radius = R;
		m_vecPoint[index] = GetAnglePos(R,(m_dStartAngle + m_dEndAngle) / 2.0);
		UpDateCenter();
	}
	UpdateLinePath();
	return true;
}

void ArcCaliperItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	double angle = m_dStartAngle;
	double	dAngleRang = (m_dEndAngle - m_dStartAngle);
	if (dAngleRang > 360) {
		dAngleRang = (m_dEndAngle - m_dStartAngle) - ((int)(m_dEndAngle - m_dStartAngle) / 360) * 360;
	}

	QPointF dStartPos;
	QPointF dEndPos;
	{
		dStartPos = GetAnglePos(m_Radius, angle);
	}
	if (m_eRegionType == RegionType_Add)	painter->setBrush(QBrush(fillColor));
	painter->drawArc(QRectF(center.x() - m_Radius, center.y() - m_Radius, m_Radius * 2, m_Radius * 2), m_dStartAngle * 16, dAngleRang * 16);
	//绘制分割线	
	QLineF line;
	QPointF _temp, _temp1;
	QPointF _tempRetn, _temp1Retn, _temp2Retn, _temp3Retn;
	if (dAngleRang >= 360)	{
		double dAngleStep = dAngleRang / (double)(segment_line_num);
		for (int i = 0; i < segment_line_num; i++) {
			angle = m_dStartAngle + (dAngleStep * i);

			dStartPos	= GetAnglePos(m_Radius - m_Height, angle);
			dEndPos		= GetAnglePos(m_Radius + m_Height, angle);
			line.setP1(dStartPos);
			line.setP2(dEndPos);

			painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
			painter->drawLine(line);
			GetStartEndNormalLine(dStartPos, dEndPos, m_Width, _tempRetn, _temp1Retn);
			GetStartEndNormalLine(dEndPos, dStartPos, m_Width, _temp2Retn, _temp3Retn);

			painter->setPen(QPen(QBrush(QColor(0, 0, 255)), 1));
			painter->drawLine(_tempRetn, _temp1Retn);
			painter->drawLine(_temp1Retn, _temp2Retn);
			painter->drawLine(_temp2Retn, _temp3Retn);
			painter->drawLine(_temp3Retn, _tempRetn);
		}
	}
	else	{
		double dAngleStep = dAngleRang / (double)(segment_line_num - 1);
		for (int i = 0; i < segment_line_num; i++) {
			angle = m_dStartAngle + (dAngleStep * i);
			dStartPos	= GetAnglePos(m_Radius - m_Height, angle);
			dEndPos		= GetAnglePos(m_Radius + m_Height, angle);
			line.setP1(dStartPos);
			line.setP2(dEndPos);

			painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
			painter->drawLine(line);

			painter->setPen(QPen(QBrush(QColor(0, 0, 255)), 1));
			GetStartEndNormalLine(dStartPos, dEndPos, m_Width, _tempRetn, _temp1Retn);
			GetStartEndNormalLine(dEndPos, dStartPos, m_Width, _temp2Retn, _temp3Retn);
			painter->drawLine(_tempRetn, _temp1Retn);
			painter->drawLine(_temp1Retn, _temp2Retn);
			painter->drawLine(_temp2Retn, _temp3Retn);
			painter->drawLine(_temp3Retn, _tempRetn);
		}
	}

}

QPointF ArcCaliperItem::GetAnglePos(qreal Radius, qreal angle)
{
	QPointF dStartPos;
	if (0 <= angle && angle < 90) {
		dStartPos = QPointF(center.x() + cos(angle * _PI / 180) * Radius, center.y() - sin(angle * _PI / 180) * Radius);
	}
	else if (90 <= angle && angle < 180) {
		dStartPos = QPointF(center.x() - sin((angle - 90) * _PI / 180) * Radius, center.y() - cos((angle - 90) * _PI / 180) * Radius);
	}
	else if (180 <= angle && angle < 270) {
		dStartPos = QPointF(center.x() - cos((angle - 180) * _PI / 180) * Radius, center.y() + sin((angle - 180) * _PI / 180) * Radius);
	}
	else if (270 <= angle && angle < 360) {
		dStartPos = QPointF(center.x() + sin((angle - 270) * _PI / 180) * Radius, center.y() + cos((angle - 270) * _PI / 180) * Radius);
	}
	else {
		dStartPos = QPointF(center.x() + cos(angle * _PI / 180) * Radius, center.y() - sin(angle * _PI / 180) * Radius);
	}
	return dStartPos;
}

qreal ArcCaliperItem::CalculateAngle(const QPointF & p1, const QPointF & p2, const QPointF & p3)
{
	double theta = atan2(p1.x() - p2.x(), p1.y() - p2.y()) - atan2(p3.x() - p2.x(), p3.y() - p2.y());
	if (theta > M_PI)
		theta -= 2 * M_PI;
	if (theta < -M_PI)
		theta += 2 * M_PI;
	theta = /*abs*/-(theta * 180.0 / M_PI);
	return theta;
}

void ArcCaliperItem::UpdateLinePath()
{
	double m_RadiusMin = m_Radius - m_Height;
	double m_RadiusMax = m_Radius + m_Height;
	QPPath = QPainterPath();
	double angle = m_dStartAngle;
	double	dAngleRang = (m_dEndAngle - m_dStartAngle);
	if (dAngleRang > 360) {
		dAngleRang = (m_dEndAngle - m_dStartAngle) - ((int)(m_dEndAngle - m_dStartAngle) / 360) * 360;
	}
	QPointF dStartPos;
	{
		dStartPos =	GetAnglePos(m_RadiusMin, angle);
		QPPath.moveTo(dStartPos);
	}	{
		dStartPos = GetAnglePos(m_RadiusMax, angle);
		QPPath.moveTo(dStartPos);
	}
	QPPath.arcTo(QRectF(center.x() - m_RadiusMin, center.y() - m_RadiusMin, m_RadiusMin * 2, m_RadiusMin * 2), m_dStartAngle, dAngleRang);
	angle = m_dEndAngle;
	{
		dStartPos = GetAnglePos(m_RadiusMax, angle);
		QPPath.lineTo(dStartPos);
	}
	QPPath.arcTo(QRectF(center.x() - m_RadiusMax, center.y() - m_RadiusMax, m_RadiusMax * 2, m_RadiusMax * 2), m_dEndAngle, -dAngleRang);
	QPPath.setFillRule(Qt::OddEvenFill);

}

void ArcCaliperItem::GetArcCaliper(ArcCaliperP& CCir)
{
	QPointF GetCCircle = this->mapToScene(m_vecPoint[0].x(), m_vecPoint[0].y());
	CCir.x				= GetCCircle.x();
	CCir.y				= GetCCircle.y();
	CCir.Radius			= m_Radius;

	CCir.dStartAngle	= m_dStartAngle	;
	CCir.dEndAngle		= m_dEndAngle	;	
	CCir.sSigma			= m_dSigma		;
	CCir.sThreshold		= m_dThreshold	;
	CCir.sTransition	= m_strTransition; 
	CCir.sSelect		= m_strSelect	;	
	CCir.sActiveNum		= m_ActiveNum	;	
	CCir.num			= segment_line_num;
	CCir.width			= m_Width		;
	CCir.height			= m_Height		;
}

void ArcCaliperItem::SetArcCaliper(ArcCaliperP & CCir)
{
	QPointF ptPoit;
	ptPoit.setX(CCir.x);
	ptPoit.setY(CCir.y);
	ptPoit = this->mapFromScene(ptPoit);
	m_vecPoint[0] = ptPoit;
	center.setX(ptPoit.x());
	center.setY(ptPoit.y());
	m_Radius = CCir.Radius;

	m_vecPoint[1]		= GetAnglePos(m_Radius, (m_dStartAngle + m_dEndAngle) / 2.0);

	m_dStartAngle		= CCir.dStartAngle;
	m_dEndAngle			= CCir.dEndAngle;
	m_dSigma			= CCir.sSigma;
	m_dThreshold		= CCir.sThreshold;
	m_strTransition		= CCir.sTransition;
	m_strSelect			= CCir.sSelect;
	m_ActiveNum			= CCir.sActiveNum;
	segment_line_num	= CCir.num;
	m_Height			= CCir.height;
	m_Width				= CCir.width;

	UpdateLinePath();
	UpDateCenter();
}

void ArcCaliperItem::UpDateCenter()
{
	double m_RadiusMin = m_Radius - m_Height;
	double m_RadiusMax = m_Radius + m_Height;

	if (m_vecPoint.size() > 3) {	//起点
		if ((m_dEndAngle - m_dStartAngle) > 360)
		{
			if (m_dStartAngle > 0)
			{
				m_vecPoint[3] = GetAnglePos((m_RadiusMin + m_RadiusMax) / 2.0, m_dStartAngle);
			}
			else
			{
				m_vecPoint[3] = GetAnglePos((m_RadiusMin + m_RadiusMax) / 2.0, m_dStartAngle + 360);
			}
		}
		else
		{
			m_vecPoint[3] = GetAnglePos((m_RadiusMin + m_RadiusMax) / 2.0, m_dStartAngle);
		}
	}
	if (m_vecPoint.size() > 4) {	//结束点
		m_vecPoint[4] = GetAnglePos((m_RadiusMin + m_RadiusMax) / 2.0, m_dEndAngle);
	}
}
#pragma endregion