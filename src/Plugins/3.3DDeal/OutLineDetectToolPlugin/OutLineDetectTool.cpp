#include "OutLineDetectTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmOutLineDetectTool.h"
#include "databaseVar.h"

#pragma region 检查基类和结果
CheckBase::CheckBase(QString name, EnumCheckType type, CheckManager* pManager)
{
	strName			= name;
	eType			= type;
	pCheckManager	= pManager;
	strResult.clear();
}

CheckBase::~CheckBase()
{
}

QString CheckBase::GetTypeName()
{
	return QString();
}

int CheckBase::ChangeParamToRegion(CommonData & vecData, HObject & region)
{
	region.GenEmptyObj();
	float fData[5] = { 0 };
	QStringList dArray = vecData.strData.split("_");
	if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
	if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
	if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
	if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
	if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();
	HObject _Object;
	_Object.GenEmptyObj();
	switch (vecData.type)	{
	case ItemCircle: {	HalconCpp::GenCircle(&_Object, fData[0], fData[1], fData[2]);						} break;	// 圆
	case ItemEllipse: {	HalconCpp::GenEllipse(&_Object, fData[0], fData[1], fData[2], fData[3], fData[4]);	} break;	// 椭圆
	case ItemConcentric_Circle: {	// 同心圆	
		HObject _Object1;
		HalconCpp::GenCircle(&_Object1, fData[0], fData[1], fData[2]);
		HalconCpp::GenCircle(&_Object, fData[0], fData[1], fData[3]);
		if (fData[2] > fData[3])		{
			HalconCpp::Difference(_Object1, _Object, &_Object);
		}
		else		{
			HalconCpp::Difference(_Object, _Object1, &_Object);
		}
	}break;
	case ItemRectangle: {		// 矩形
		HalconCpp::GenRectangle1(&_Object, fData[0], fData[1], fData[0] + fData[3], fData[1] + fData[2]);
	}break;
	case ItemRectangleR: {		// 旋转矩形
		HalconCpp::GenRectangle2(&_Object, fData[0], fData[1], fData[2], fData[3], fData[4]);
	}break;
	case ItemPolygon: {					// 多边形
		HTuple Rows, Cols;
		HalconCpp::GenRegionPolygon(&_Object, Rows, Cols);
	}break;
	case ItemLineObj:					// 直线
	{
		HalconCpp::GenRegionLine(&_Object, fData[0], fData[1], fData[2], fData[3]);
	}break;
	case ItemArrowLineObj:				// 直线
	{
		HalconCpp::GenRegionLine(&_Object, fData[0], fData[1], fData[2], fData[3]);
	}break;
	default: {	}	break;
	}
	region	= _Object.Clone();
	return 0;
}

QString CheckBase::GetCheckName()
{
	return strName;
}

void CheckBase::SetCheckName(QString name)
{
	strName = name;
}

EnumCheckType CheckBase::GetCheckType()
{
	return eType;
}

void CheckBase::SetCheckType(EnumCheckType type)
{
	eType = type;
}

QString CheckBase::GetCheckData()
{
	GetCheckDataText(strVecParam, iIsLimit, iRegionType, strVecLimit, strVecJudgeValue,strVecJudgeCondition, strData);
	return strData;
}

void CheckBase::SetCheckData(QString Data)
{
	strData = Data;
	SetCheckDataText(Data, strVecParam, iIsLimit, iRegionType, strVecLimit, strVecJudgeValue, strVecJudgeCondition);
}

QString CheckBase::GetCheckResult()
{
	return strResult;
}

int CheckBase::GetCheckDataText(QStringList& strParamArray, bool & iIsLimit, int & iRegionType, QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams,QString & strText)
{
	QString	_strData1 = "P:";
	for (size_t i = 0; i < strParamArray.size(); i++)	{
		if (i == (strParamArray.size() - 1))
			_strData1.append(QString(strParamArray[i]));
		else
			_strData1.append(QString(strParamArray[i])).append("|");
	}
	_strData1.append(QString("&R%1%2:").arg(QString::number(iIsLimit ? 1 : 0)).arg(QString::number(iRegionType)));
	for (size_t i = 0; i < strLimitArray.size(); i++)	{
		if (i == (strLimitArray.size() - 1))
			_strData1.append(QString(strLimitArray[i]));
		else
			_strData1.append(QString(strLimitArray[i])).append("|");
	}
	_strData1.append("@@");
	for (size_t i = 0; i < strLimitParams.size(); i++)	{
		if (i == (strLimitParams.size() - 1))
			_strData1.append(strLimitParams[i]);
		else
			_strData1.append(strLimitParams[i]).append("|");
	}	
	_strData1.append("&&");
	for (size_t i = 0; i < strLimitValues.size(); i++)	{
		if (i == (strLimitValues.size() - 1))
			_strData1.append(strLimitValues[i]);
		else
			_strData1.append(strLimitValues[i]).append("|");
	}
	strText = _strData1;
	return 0;
}

int CheckBase::SetCheckDataText(QString & strText, QStringList & strParamArray, bool & iIsLimit, int & iRegionType, QStringList & strLimitArray, QStringList&strLimitValues, QStringList & strLimitParams)
{
	strParamArray.clear();
	strLimitParams.clear();
	int _iLastIndex				= strText.indexOf("&R");
	int _iFirstIndex			= 2;
	QString _strParamData		= strText.mid(_iFirstIndex, _iLastIndex - _iFirstIndex);
	strParamArray				= _strParamData.split("|");
	QStringList _strArray		= strText.split("@@");
	_strParamData				= strText.mid(_iLastIndex + 5, _strArray[0].length() - (_iLastIndex + 5));
	strLimitArray				= _strParamData.split("|");
	iIsLimit					= strText.mid(_iLastIndex + 2, 1).toInt();
	iRegionType					= strText.mid(_iLastIndex + 3, 1).toInt();
	_strArray					= strText.split("&&");
	if (_strArray.size()		>	1)	{
		if (!_strArray[1].isEmpty())
			strLimitValues = _strArray[1].split("|");
	}
	QStringList _ParamsArray	= _strArray[0].split("@@");
	if (_ParamsArray.size() > 1)	{
		if (!_ParamsArray[1].isEmpty())
			strLimitParams			= _ParamsArray[1].split("|");
	}
	return 0;
}

int CheckBase::CheckImgContourPoint(HTuple & Grayval, int iCheckType, double xValue, double xRange, QPointF& pt)
{
	int _iMin		= MAX(xValue, 0);
	int	_iLenth		= Grayval.Length() - 1;
	int _iMax		= MIN(xRange + xValue, Grayval.Length() - 1);
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	switch (iCheckType)	{
	case 0: {	//最大值
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)		{
			int _iIndex = _hIndices.Length() - 1;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			pt.setX(_hIndices[_iIndex] + (int)xValue);
			pt.setY(_hValue.D());
			return 0;
		}
	}	break;
	case 1: {	//最小值

		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)		{
			int _iIndex = 0;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			pt.setX(_hIndices[_iIndex] + (int)xValue);
			pt.setY(_hValue.D());
			return 0;
		}
	}	break;
	case 2:	//中值点
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		TupleSortIndex(_hSelected, &_hIndices);
		if (_hIndices.Length() > 0)
		{
			int _iIndex = _hIndices.Length() / 2.0;
			TupleSelect(_hSelected, _hIndices[_iIndex], &_hValue);
			pt.setX(_hIndices[_iIndex] + (int)xValue);
			pt.setY(_hValue.D());
			return 0;
		}
	}	break;
	case 3:	//均值点
	{
		HTuple _hSelected, _hIndices, _hValue;
		TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		TupleMedian(_hSelected, &_hValue);		//绘制直线
		if (_hSelected.Length() > 0)
		{
			pt.setX((_iMin + _iMax) / 2.0);
			pt.setY(_hValue.D());
			return 0;
		}
	}	break;
	case 4:	//拐点
	{

	}	break;
	case 5:	//关联点
	{

	}	break;
	default:
		break;
	}
	return -1;
}

int CheckBase::CheckImgContourLine(HTuple&Grayval, int iCheckType, double xValue, double xRange, QLineF & line)
{
	int _iMin = MAX(xValue, 0);
	int _iMax = MIN(xRange + xValue, Grayval.Length() - 1);
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	try
	{
		HTuple _hSelected, _hValueXIndex, _hValue;
		HObject _hContour;
		HTuple RowBegin, ColBegin, RowEnd, ColEnd, Nr, Nc, Dist;
		HalconCpp::TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		for (int i = _iMin; i <= _iMax; i++)	_hValueXIndex.Append(i);
		std::string	_strAlgorithm;
		switch (iCheckType)
		{
		case 0:	//最大值
		{
			_strAlgorithm = "tukey";
		}	break;
		case 1:	//最小值
		{
			_strAlgorithm = "tukey";
		}	break;
		case 2:	//中值点
		{
			_strAlgorithm = "drop";
		}	break;
		default:
			break;
		}
		HalconCpp::GenContourPolygonXld(&_hContour, _hSelected, _hValueXIndex);
		HalconCpp::FitLineContourXld(_hContour, _strAlgorithm.c_str(), -1, 0, 5, 2, &RowBegin, &ColBegin, &RowEnd, &ColEnd, &Nr, &Nc, &Dist);

		line.setP1(QPointF(ColBegin.D(), RowBegin.D()));
		line.setP2(QPointF(ColEnd.D(), RowEnd.D()));
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

int CheckBase::CheckImgContourCircle(HTuple&Grayval, int iCheckType, double xValue, double xRange, MCircle & circle)
{
	int _iMin = MAX(xValue, 0);
	int _iMax = MIN(xRange + xValue, Grayval.Length() - 1);
	_iMin = MIN(_iMin, _iMax);
	_iMax = MAX(_iMin, _iMax);

	try
	{
		HTuple	_hSelected, _hValueXIndex, _hValue;
		HObject _hContour;
		HTuple  hv_Nr, hv_Nc, hv_Dist, hv_Row, hv_Column, hv_Radius;
		HTuple  hv_StartPhi, hv_EndPhi, hv_PointOrder;
		HalconCpp::TupleSelectRange(Grayval, _iMin, _iMax, &_hSelected);
		for (int i = _iMin; i <= _iMax; i++)	_hValueXIndex.Append(i);
		std::string	_strAlgorithm;
		switch (iCheckType)
		{
		case 0:	//最大值
		{
			_strAlgorithm = "algebraic";
		}	break;
		case 1:	//最小值
		{
			_strAlgorithm = "ahuber";
		}	break;
		case 2:	//中值点
		{
			_strAlgorithm = "atukey";
		}	break;
		case 3:	//中值点
		{
			_strAlgorithm = "geometric";
		}	break;
		case 4:	//中值点
		{
			_strAlgorithm = "geohuber";
		}	break;
		case 5:	//中值点
		{
			_strAlgorithm = "geotukey";
		}	break;
		default:
			break;
		}
		HalconCpp::GenContourPolygonXld(&_hContour, _hSelected, _hValueXIndex);
		HalconCpp::FitCircleContourXld(_hContour, _strAlgorithm.c_str(), -1, 0, 0, 5, 2, 
			&hv_Row, &hv_Column, &hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder);
		circle.row = hv_Row.D();
		circle.col = hv_Column.D();
		circle.radius = hv_Radius.D();
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

EnumCheckResult CheckBase::CheckMesureResult(double dvalue, QStringList & strLimitParams, QStringList&strVecJudgeValue)
{
	double dOldValue;
	if (strVecJudgeValue.size() > 0)
	{
		dOldValue = strVecJudgeValue[0].toDouble();
	}
	if (strLimitParams.size() > 0)
		if (dvalue > (dOldValue + strLimitParams[0].toDouble()))
		{
			return CheckResult_OverUp;
		}
	if (strLimitParams.size() > 1)
		if (dvalue < (dOldValue + strLimitParams[1].toDouble()))
		{
			return CheckResult_OverDown;
		}
	if (strLimitParams.size() > 1)
		if (strLimitParams[0].toDouble() < strLimitParams[1].toDouble())
		{
			return CheckResult_ParamError;
		}
	return CheckResult_OK;
}

EnumErrorRetn CheckBase::Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult& result, QString& strError)
{
	return ErrorRetn_OK;
}

int CheckBase::PrExcute()
{
	strErrorMsg.clear();
	strResult.clear();
	return 0;
}

CheckResult::CheckResult()
{
	ClearALL();
}

CheckResult::~CheckResult()
{
	ClearALL();
}

void CheckResult::ClearALL()
{
	mapResultPos.clear();
	mapResultLineF.clear();
	mapResultMCircle.clear();
	mapResultValue.clear();
}
#pragma endregion

#pragma region 创建图型
CheckPoint::CheckPoint(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckPoint::~CheckPoint()
{
}

EnumErrorRetn CheckPoint::Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError)
{
	int		_iCheckType		= strVecParam[0].toInt();
	double	_dCheckvalue	= strVecLimit[0].toDouble();
	double	_dCheckRange	= strVecLimit[1].toDouble();
	QPointF _ptPoint;
	CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPoint);
	result.mapResultPos[strName] = _ptPoint;
	strResult = QString("x,y(%1,%2)")
		.arg(QString::number(_ptPoint.x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_ptPoint.y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckPoint::GetTypeName()
{
	return TypeName();
}

QString CheckPoint::TypeName()
{
	return  frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint];
}

CheckPPCenter::CheckPPCenter(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckPPCenter::~CheckPPCenter()
{
}

EnumErrorRetn CheckPPCenter::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1, _ptPos2;
	{
		int		_iCheckType = strVecParam[_iParamIndex].toInt();
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[0].toDouble();
			double	_dCheckRange = strVecLimit[1].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos1);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())		{
				strError.append(tr("Didn't Contain ") + QString(" %1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)		{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	} {
		int		_iCheckType = strVecParam.size() > 1 ? strVecParam[_iParamIndex].toInt() : 0;
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[2].toDouble();
			double	_dCheckRange = strVecLimit[3].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos2);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())		{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)	{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos2 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	}
	QPointF _qPtCenter = QPointF((_ptPos1.x() + _ptPos2.x()) / 2.0, (_ptPos1.y() + _ptPos2.y()) / 2.0);
	result.mapResultPos[strName] = _qPtCenter;

	strResult = QString("x,y(%1,%2)")
		.arg(QString::number(_qPtCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qPtCenter.y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckPPCenter::GetTypeName()
{
	return TypeName();
}

QString CheckPPCenter::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointCenter]);
}

CheckLLCrossPoint::CheckLLCrossPoint(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckLLCrossPoint::~CheckLLCrossPoint()
{
}

EnumErrorRetn CheckLLCrossPoint::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	QLineF	_qline1F, _qline2F;
	{
		int		_iCheckType = strVecParam[0].toInt();
		double	_dCheckvalue = strVecLimit[0].toDouble();
		double	_dCheckRange = strVecLimit[1].toDouble();
		if (CheckImgContourLine(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _qline1F) < 0)
		{
			strError.append(QString("CheckImgContourLine NG"));

			return ErrorRetn_NG;
		}
	} {
		int		_iCheckType = strVecParam[1].toInt();
		double	_dCheckvalue = strVecLimit[2].toDouble();
		double	_dCheckRange = strVecLimit[3].toDouble();
		if (CheckImgContourLine(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _qline2F) < 0)
		{
			strError.append(QString("CheckImgContourLine NG"));
			return ErrorRetn_NG;
		}
	}
	QPointF _qPtCenter;
	QLineF::IntersectType _type = _qline1F.intersect(_qline2F, &_qPtCenter);
	switch (_type)
	{
	case QLineF::NoIntersection: {	}	break;	//无交叉点
	case QLineF::BoundedIntersection:	//边界交叉点
	{
		result.mapResultPos[strName] = _qPtCenter;
		strResult = QString("x,y(%1,%2)")
			.arg(QString::number(_qPtCenter.x(), 'f', databaseVar::Instance().form_System_Precision))
			.arg(QString::number(_qPtCenter.y(), 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	case QLineF::UnboundedIntersection:	//交点
	{
		return ErrorRetn_NG;
	}	break;
	default:
		break;
	}
	return ErrorRetn_OK;
}

QString CheckLLCrossPoint::GetTypeName()
{
	return TypeName();
}

QString CheckLLCrossPoint::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineCross]);
}

CheckLine::CheckLine(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckLine::~CheckLine()
{
}

EnumErrorRetn CheckLine::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	QLineF	_qline1F, _qline2F;

	int		_iCheckType = strVecParam[0].toInt();
	double	_dCheckvalue = strVecLimit[0].toDouble();
	double	_dCheckRange = strVecLimit[1].toDouble();
	if (CheckImgContourLine(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _qline1F) < 0)
	{
		strError.append(QString("CheckImgContourLine NG"));
		return ErrorRetn_NG;
	}
	result.mapResultLineF[strName] = _qline1F;

	strResult = QString("p1(%1,%2)--p2(%3,%4)")
		.arg(QString::number(_qline1F.p1().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p1().y(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p2().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p2().y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckLine::GetTypeName()
{
	return TypeName();
}

QString CheckLine::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine]);
}

CheckHLine::CheckHLine(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckHLine::~CheckHLine()
{
}

EnumErrorRetn CheckHLine::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	QLineF	_qline1F;
	_qline1F.setP1(QPointF(0, strVecParam[0].toDouble()));
	_qline1F.setP2(QPointF(Grayval.TupleLength().I(), strVecParam[0].toDouble()));
	result.mapResultLineF[strName] = _qline1F;

	strResult = QString("p1(%1,%2)--p2(%3,%4)")
		.arg(QString::number(_qline1F.p1().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p1().y(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p2().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_qline1F.p2().y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckHLine::GetTypeName()
{
	return TypeName();
}

QString CheckHLine::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeHorizontalLine]);
}

CheckCircle::CheckCircle(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckCircle::~CheckCircle()
{
}

EnumErrorRetn CheckCircle::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	MCircle _Circle;
	int		_iCheckType = strVecParam[0].toInt();
	double	_dCheckvalue = strVecLimit[0].toDouble();
	double	_dCheckRange = strVecLimit[1].toDouble();
	if (CheckImgContourCircle(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _Circle) < 0)
	{
		strError.append(QString("CheckImgContourCircle NG"));
		return ErrorRetn_NG;
	}
	result.mapResultMCircle[strName] = _Circle;

	strResult = QString("x,y,r(%1,%2,%3)")
		.arg(QString::number(_Circle.col, 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_Circle.row, 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_Circle.radius, 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckCircle::GetTypeName()
{
	return TypeName();
}

QString CheckCircle::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeCircle]);
}

CheckPPLine::CheckPPLine(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckPPLine::~CheckPPLine()
{
}

EnumErrorRetn CheckPPLine::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1, _ptPos2;
	QLineF _line;
	{
		int		_iCheckType = strVecParam[_iParamIndex].toInt();
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[0].toDouble();
			double	_dCheckRange = strVecLimit[1].toDouble();
			if (CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos1) < 0)	{
				strError.append(QString("CheckImgContourPoint NG"));
				return ErrorRetn_NG;
			}
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())		{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)	{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	} {
		int		_iCheckType = strVecParam.size() > 1 ? strVecParam[_iParamIndex].toInt() : 0;
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[2].toDouble();
			double	_dCheckRange = strVecLimit[3].toDouble();
			if (CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos2) < 0)	{
				strError.append(QString("CheckImgContourPoint NG"));
				return ErrorRetn_NG;
			}
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())		{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)		{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos2 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	}
	_line.setP1(_ptPos1);
	_line.setP2(_ptPos2);
	result.mapResultLineF[strName] = _line;
	strResult = QString("p1(%1,%2)--p2(%3,%4)")
		.arg(QString::number(_line.p1().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_line.p1().y(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_line.p2().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_line.p2().y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckPPLine::GetTypeName()
{
	return TypeName();
}

QString CheckPPLine::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointLine]);
}

CheckChuiLine::CheckChuiLine(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckChuiLine::~CheckChuiLine()
{
}

EnumErrorRetn CheckChuiLine::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QLineF _Line, _OutLine;
	QPointF _qPt;
	QPointF _StokePt;
	{
		int	_iCheckType = strVecParam[_iParamIndex].toInt();
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_qPt = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultLineF.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_Line = result.mapResultLineF[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{	//获取垂点
		_StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
	}
	if ((abs(_qPt.x() - _StokePt.x()) <= 0.001) && (abs(_qPt.y() - _StokePt.y()) <= 0.001))
	{
		double k1 = (_Line.p1().y() - _Line.p2().y()) / (_Line.p1().x() - _Line.p2().x());
		// 计算过垂点的直线的斜率（垂直线的斜率是原斜率的负倒数）
		double k2 = -1.0 / k1;

		// 使用点斜式计算过垂点的直线上的一点
		double new_y = _StokePt.y() + k2 * (_StokePt.x() - _Line.p1().x());

		_qPt.setX(_Line.p1().x());
		_qPt.setY(new_y);

		_OutLine.setP1(_qPt);
		_OutLine.setP2(_StokePt);
	}
	else
	{
		_OutLine.setP1(_qPt);
		_OutLine.setP2(_StokePt);
	}
	result.mapResultLineF[strName] = _OutLine;
	strResult = QString("p1(%1,%2)--p2(%3,%4)")
		.arg(QString::number(_OutLine.p1().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p1().y(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p2().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p2().y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckChuiLine::GetTypeName()
{
	return TypeName();
}

QString CheckChuiLine::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeverticalLine]);
}

CheckChuiPoint::CheckChuiPoint(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckChuiPoint::~CheckChuiPoint()
{
}

EnumErrorRetn CheckChuiPoint::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QLineF _Line, _OutLine;
	QPointF _qPt;
	QPointF _StokePt;
	{
		int	_iCheckType = strVecParam[_iParamIndex].toInt();
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_qPt = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultLineF.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_Line = result.mapResultLineF[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}

	{	//获取垂点
		_StokePt = OutLineDetectTool::FindFoot(_Line, _qPt);
	}
	if ((abs(_qPt.x() - _StokePt.x()) <= 0.001) && (abs(_qPt.y() - _StokePt.y()) <= 0.001))
	{
		double k1 = (_Line.p1().y() - _Line.p2().y()) / (_Line.p1().x() - _Line.p2().x());
		// 计算过垂点的直线的斜率（垂直线的斜率是原斜率的负倒数）
		double k2 = -1.0 / k1;

		// 使用点斜式计算过垂点的直线上的一点
		double new_y = _StokePt.y() + k2 * (_StokePt.x() - _Line.p1().x());

		_qPt.setX(_Line.p1().x());
		_qPt.setY(new_y);

		_OutLine.setP1(_qPt);
		_OutLine.setP2(_StokePt);
	}
	else
	{
		_OutLine.setP1(_qPt);
		_OutLine.setP2(_StokePt);
	}
	result.mapResultLineF[strName] = _OutLine;
	strResult = QString("p1(%1,%2)--p2(%3,%4)")
		.arg(QString::number(_OutLine.p1().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p1().y(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p2().x(), 'f', databaseVar::Instance().form_System_Precision))
		.arg(QString::number(_OutLine.p2().y(), 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckChuiPoint::GetTypeName()
{
	return TypeName();
}

QString CheckChuiPoint::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypepointStroke]);

}
#pragma endregion

#pragma region 测量
CheckHeight::CheckHeight(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckHeight::~CheckHeight()
{
}

EnumErrorRetn CheckHeight::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1, _ptPos2;
	{
		int		_iCheckType = strVecParam[_iParamIndex].toInt();
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[0].toDouble();
			double	_dCheckRange = strVecLimit[1].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos1);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	}	{
		int		_iCheckType = strVecParam.size() > 1 ? strVecParam[_iParamIndex].toInt() : 0;
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[2].toDouble();
			double	_dCheckRange = strVecLimit[3].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos2);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos2 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	}

	double _dHeight = (_ptPos1.y() - _ptPos2.y());

	eCheckResult = CheckMesureResult(_dHeight, strVecJudgeCondition, strVecJudgeValue);

	result.mapResultValue[strName] = _dHeight;	//高度差
	strResult = QString("%1(%2)")
		.arg(tr("height"))
		.arg(QString::number(_dHeight, 'f', databaseVar::Instance().form_System_Precision));

	return ErrorRetn_OK;
}

QString CheckHeight::GetTypeName()
{
	return TypeName();
}

QString CheckHeight::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeHeightDifference]);
}

CheckWidth::CheckWidth(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckWidth::~CheckWidth()
{
}

EnumErrorRetn CheckWidth::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1, _ptPos2;
	{
		int		_iCheckType = strVecParam[_iParamIndex].toInt();
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[0].toDouble();
			double	_dCheckRange = strVecLimit[1].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos1);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	} {
		int		_iCheckType = strVecParam.size() > 1 ? strVecParam[_iParamIndex].toInt() : 0;
		if (_iCheckType != 5)
		{
			double	_dCheckvalue = strVecLimit[2].toDouble();
			double	_dCheckRange = strVecLimit[3].toDouble();
			CheckImgContourPoint(Grayval, _iCheckType, _dCheckvalue, _dCheckRange, _ptPos2);
		}
		else  //关联
		{
			_iParamIndex++;
			QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
			if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
			{
				strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
				return ErrorRetn_NotContain;
			}
			_ptPos2 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		}
		_iParamIndex++;
	}
	double _dWidth = (_ptPos1.x() - _ptPos2.x());

	eCheckResult = CheckMesureResult(_dWidth, strVecJudgeCondition, strVecJudgeValue);

	result.mapResultValue[strName] = _dWidth;	//高度差
	strResult = QString("%1(%2)")
		.arg(tr("Width"))
		.arg(QString::number(_dWidth, 'f', databaseVar::Instance().form_System_Precision));

	return ErrorRetn_OK;
}

QString CheckWidth::GetTypeName()
{
	return TypeName();
}

QString CheckWidth::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeWidth]);
}

CheckPPLenth::CheckPPLenth(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckPPLenth::~CheckPPLenth()
{
}

EnumErrorRetn CheckPPLenth::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1, _ptPos2;
	{
		int	_iCheckType = strVecParam[_iParamIndex].toInt();
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_ptPos2 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	int	_iCheckType = strVecParam[2].toInt();

	switch (_iCheckType)
	{
	case 0: {
		double _dDistance = OutLineDetectTool::GetP2PDistance(_ptPos1, _ptPos2);

		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult = QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	case 1: {
		double _dDistance = abs(_ptPos1.x() - _ptPos2.x());

		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult =	QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	case 2: {
		double _dDistance = abs(_ptPos1.y() - _ptPos2.y());

		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult = QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	default:
		break;
	}

	return ErrorRetn_OK;
}

QString CheckPPLenth::GetTypeName()
{
	return TypeName();
}

QString CheckPPLenth::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointDistance]);
}

CheckPLineLenth::CheckPLineLenth(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckPLineLenth::~CheckPLineLenth()
{
}

EnumErrorRetn CheckPLineLenth::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QPointF _ptPos1;
	QLineF _Line;
	{
		int	_iCheckType = strVecParam[_iParamIndex].toInt();
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultPos.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_ptPos1 = result.mapResultPos[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultLineF.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_Line = result.mapResultLineF[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	QPointF _StokePt = OutLineDetectTool::FindFoot(_Line, _ptPos1);
	int	_iCheckType = strVecParam[_iParamIndex].toInt();
	switch (_iCheckType)
	{
	case 0: {
		double _dDistance = OutLineDetectTool::GetP2PDistance(_ptPos1, _StokePt);
		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult = QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	case 1: {
		double _dDistance = abs(_ptPos1.x() - _StokePt.x());
		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult = QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	case 2: {
		double _dDistance = abs(_ptPos1.y() - _StokePt.y());
		eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

		result.mapResultValue[strName] = _dDistance;	//高度差
		strResult = QString("%1(%2)")
			.arg(tr("Distance"))
			.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	}	break;
	default:
		break;
	}
	return ErrorRetn_OK;
}

QString CheckPLineLenth::GetTypeName()
{
	return TypeName();
}

QString CheckPLineLenth::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointLineDistance]);
}

CheckLineLineAngle::CheckLineLineAngle(QString name, EnumCheckType type, CheckManager* pManager)
	:CheckBase(name, type, pManager)
{
}

CheckLineLineAngle::~CheckLineLineAngle()
{
}

EnumErrorRetn CheckLineLineAngle::Excute(int Index, HTuple & row, HTuple & col, HTuple & Grayval, CheckResult & result, QString& strError)
{
	int _iParamIndex = 0;
	QLineF	_Line, _Line2;
	{
		int	_iCheckType = strVecParam[_iParamIndex].toInt();
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultLineF.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_Line = result.mapResultLineF[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}
	{
		QStringList enumNames = pCheckManager->GetCheckBaseLst(Index, frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine], pCheckManager->m_iCurrentRow);
		if (enumNames.size() <= strVecParam[_iParamIndex].toInt())
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		if (result.mapResultLineF.count(enumNames[strVecParam[_iParamIndex].toInt()]) <= 0)
		{
			strError.append(tr("Didn't Contain ") + QString("%1").arg(strVecParam[_iParamIndex]));
			return ErrorRetn_NotContain;
		}
		_Line2 = result.mapResultLineF[enumNames[strVecParam[_iParamIndex].toInt()]];
		_iParamIndex++;
	}

	HTuple _hdAngle;
	AngleLl(_Line.p1().y(), _Line.p1().x(), _Line.p2().y(), _Line.p2().x(), _Line2.p1().y(), _Line2.p1().x(), _Line2.p2().y(), _Line2.p2().x(), &_hdAngle);
	double _dDistance = _hdAngle.TupleDeg().D();

	eCheckResult = CheckMesureResult(_dDistance, strVecJudgeCondition, strVecJudgeValue);

	result.mapResultValue[strName] = _dDistance;
	strResult = QString("%1(%2)")
		.arg(tr("Distance"))
		.arg(QString::number(_dDistance, 'f', databaseVar::Instance().form_System_Precision));
	return ErrorRetn_OK;
}

QString CheckLineLineAngle::GetTypeName()
{
	return TypeName();
}

QString CheckLineLineAngle::TypeName()
{
	return QString(frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineLineAngle]);
}
#pragma endregion

#pragma region 检测管理
CheckManager::CheckManager()
{
	m_vecCheckOutLine.clear();
	m_vecRegion.clear();
	m_iCurrentRow = -1;
	m_result.ClearALL();
	m_vecCheck2D.clear();
	m_mapMapTools.clear();
}
CheckManager::CheckManager(const CheckManager & manager)
{
	m_vecCheckOutLine	= manager.m_vecCheckOutLine;
	m_vecRegion			= manager.m_vecRegion;
	m_iCurrentRow		= manager.m_iCurrentRow;
	m_result			= manager.m_result;
	m_vecCheck2D		= manager.m_vecCheck2D;
	m_mapMapTools		= manager.m_mapMapTools;
}

CheckManager CheckManager::Instance()
{
	static CheckManager ins;
	return ins;
}

void CheckManager::ClearAll(int Index)
{
	if (Index < 0)												{
		for (size_t i = 0; i < m_vecCheck2D.size(); i++)		{
			for (size_t j = 0; j < m_vecCheck2D[i].size(); j++)	{
				if (m_vecCheck2D[i][j] != nullptr)				{
					delete m_vecCheck2D[i][j];
					m_vecCheck2D[i][j] = nullptr;
				}
			}
		}
		m_vecRegion.clear();
		m_vecCheck2D.clear();
		m_mapMapTools.clear();
		m_vecCheckOutLine.clear();
	}
	else if (Index >= 0 && Index <	m_vecCheck2D.size())				{
		for (size_t j = 0; j <		m_vecCheck2D[Index].size(); j++)	{
			if (m_vecCheck2D[Index][j] != nullptr)						{
				delete m_vecCheck2D[Index][j];
				m_vecCheck2D[Index][j] = nullptr;
			}
		}
		m_vecCheck2D.erase(m_vecCheck2D.begin() + Index);
		m_vecRegion.erase(m_vecRegion.begin() + Index);
		m_mapMapTools.erase(m_mapMapTools.begin() + Index);
		m_vecCheckOutLine.erase(m_vecCheckOutLine.begin() + Index);
	}
}

int CheckManager::AddCheckTool(int Index,EnumCheckType type, QString data)
{
	QVector<int> vecLst = m_mapMapTools[Index][type];
	std::sort(vecLst.begin(), vecLst.end());						//去重前需要排序
	int _iIndex = m_mapMapTools[Index][type].size() + 1;
	for (size_t i = 0; i < m_mapMapTools[Index][type].size(); i++)	{
		if (vecLst[i] != (i + 1))	{
			_iIndex = (i + 1);
			break;
		}
	}
	m_mapMapTools[Index][type].push_back(_iIndex);

	CheckBase*	_CheckBase = nullptr;
	QString		_strName;
	switch (type)
	{
	case ECheckType_Point:								{
		_strName	= CheckPoint::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckPoint(_strName, type, this);
	}	break;
	case ECheckType_PPCenter:							{
		_strName	= CheckPPCenter::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckPPCenter(_strName, type, this);
	}	break;
	case ECheckType_LLCrossPoint:						{
		_strName	= CheckLLCrossPoint::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckLLCrossPoint(_strName, type, this);
	}	break;
	case ECheckType_Line:								{
		_strName	= CheckLine::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckLine(_strName, type, this);
	}	break;
	case ECheckType_HLine:								{
		_strName	= CheckHLine::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckHLine(_strName, type, this);
	}	break;
	case ECheckType_Circle:								{
		_strName	= CheckCircle::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckCircle(_strName, type, this);
	}	break;
	case ECheckType_PPLine:								{
		_strName	= CheckPPLine::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckPPLine(_strName, type, this);
	}	break;
	case ECheckType_ChuiLine:							{
		_strName	= CheckChuiLine::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckChuiLine(_strName, type, this);
	}	break;
	case ECheckType_ChuiPoint:							{
		_strName	= CheckChuiPoint::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckChuiPoint(_strName, type, this);
	}	break;
	case ECheckType_Height:								{
		_strName	= CheckHeight::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckHeight(_strName, type, this);
	}	break;
	case ECheckType_Width:								{
		_strName	= CheckWidth::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckWidth(_strName, type, this);
	}	break;
	case ECheckType_PPLenth:							{
		_strName	= CheckPPLenth::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckPPLenth(_strName, type, this);
	}	break;
	case ECheckType_PLineLenth:							{
		_strName	= CheckPLineLenth::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckPLineLenth(_strName, type, this);
	}	break;
	case ECheckType_LineLineAngle:						{
		_strName	= CheckLineLineAngle::TypeName().append("_").append(QString::number(_iIndex));
		_CheckBase	= new CheckLineLineAngle(_strName, type, this);
	}	break;
	default:											{
	}	break;
	}
	if (_CheckBase != nullptr)							{
		_CheckBase->SetCheckData(data);
		if (m_vecCheck2D.size() > Index)				{
			m_vecCheck2D[Index].push_back(_CheckBase);
			return 0;
		}
		else if (m_vecCheck2D.size() == Index)			{
			m_vecCheck2D.push_back(QVector<CheckBase*>());
			m_vecCheck2D[Index].push_back(_CheckBase);
			return 0;
		}
		else	{
			delete _CheckBase;
		}
	}
	return -1;
}

int CheckManager::AddCheckTool(int Index,QString type, QString data)
{
	if (type == CheckPoint::TypeName())					{
		AddCheckTool(Index,ECheckType_Point, data);
	}
	else if (type == CheckPPCenter::TypeName())			{
		AddCheckTool(Index,ECheckType_PPCenter, data);
	}
	else if (type == CheckLLCrossPoint::TypeName())		{
		AddCheckTool(Index,ECheckType_LLCrossPoint, data);
	}
	else if (type == CheckLine::TypeName())				{
		AddCheckTool(Index,ECheckType_Line, data);
	}
	else if (type == CheckHLine::TypeName())			{
		AddCheckTool(Index,ECheckType_HLine, data);
	}
	else if (type == CheckCircle::TypeName())			{
		AddCheckTool(Index,ECheckType_Circle, data);
	}
	else if (type == CheckPPLine::TypeName())			{
		AddCheckTool(Index,ECheckType_PPLine, data);
	}
	else if (type == CheckChuiLine::TypeName())			{
		AddCheckTool(Index,ECheckType_ChuiLine, data);
	}
	else if (type == CheckChuiPoint::TypeName())		{
		AddCheckTool(Index,ECheckType_ChuiPoint, data);
	}
	else if (type == CheckHeight::TypeName())			{
		AddCheckTool(Index,ECheckType_Height, data);
	}
	else if (type == CheckWidth::TypeName())			{
		AddCheckTool(Index,ECheckType_Width, data);
	}
	else if (type == CheckPPLenth::TypeName())			{
		AddCheckTool(Index,ECheckType_PPLenth, data);
	}
	else if (type == CheckPLineLenth::TypeName())		{
		AddCheckTool(Index,ECheckType_PLineLenth, data);
	}
	else if (type == CheckLineLineAngle::TypeName())	{
		AddCheckTool(Index,ECheckType_LineLineAngle, data);
	}
	return 0;
}

int CheckManager::DeleteCheckTool(QString name, int Index)
{
	if (Index < 0)														{
		for (size_t i = 0; i < m_vecCheck2D.size(); i++)				{
			for (size_t j = 0; j < m_vecCheck2D[i].size(); j++)			{
				if (m_vecCheck2D[i][j] != nullptr)
					if (m_vecCheck2D[i][j]->GetCheckName() == name)		{
						delete m_vecCheck2D[i][j];
						m_vecCheck2D[i][j] = nullptr;
						m_vecCheck2D[i].erase(m_vecCheck2D[i].begin() + j);
						if (m_vecCheck2D[i].size() <= 0) {
							m_vecCheck2D.erase(m_vecCheck2D.begin() + i);
						}
						return  0;
					}
			}
		}
	}
	else if(Index < m_vecCheck2D.size())								{
		for (size_t j = 0; j < m_vecCheck2D[Index].size(); j++)			{
			if (m_vecCheck2D[Index][j] != nullptr)
				if (m_vecCheck2D[Index][j]->GetCheckName() == name)		{
					delete m_vecCheck2D[Index][j];
					m_vecCheck2D[Index][j] = nullptr;
					m_vecCheck2D[Index].erase(m_vecCheck2D[Index].begin() + j);
					if (m_vecCheck2D[Index].size() <= 0) {
						m_vecCheck2D.erase(m_vecCheck2D.begin() + Index);
					}
					return  0;
				}
		}
	}
	return  -1;
}

CheckBase* CheckManager::GetCurrentTool(QString name, int Index)
{
	if (Index < 0)													{
		for (size_t i = 0; i < m_vecCheck2D.size(); i++)			{
			for (size_t j = 0; j < m_vecCheck2D[i].size(); j++)		{
				if (m_vecCheck2D[i][j] != nullptr)
					if (m_vecCheck2D[i][j]->GetCheckName() == name)	{	
						return m_vecCheck2D[i][j];	
					}
			}
		}
	}
	else if (Index < m_vecCheck2D.size())							{
		for (size_t j = 0; j < m_vecCheck2D[Index].size(); j++)		{
			if (m_vecCheck2D[Index][j] != nullptr)
				if (m_vecCheck2D[Index][j]->GetCheckName() == name)	{
					return m_vecCheck2D[Index][j];
				}
		}
	}
	return nullptr;
}

int CheckManager::CheckMetrixType(QString strCom, HTuple & Metrix)
{
	MiddleParam param;
	QStringList _ComLst = strCom.split(":");
	if (_ComLst.size() > 2) {
		bool _bFind = false;
		if (_ComLst[0] == "GlobalVar") {
			param = (*MiddleParam::Instance());
			_bFind = true;
		}
		for (auto iter : (*MiddleParam::Instance()).MapMiddle()) {
			if (_ComLst[0] == QString::number(iter.m_iIndex)) {
				_bFind = true;
				param = iter; break;
			}
		}
		if (_bFind) {
			if (_ComLst[1] == "Metrix") {
				if (param.MapPtrMetrix.count(_ComLst[2]) > 0) {
					Metrix = ((HTuple*)param.MapPtrMetrix[_ComLst[2]])->Clone();
					return 0;
				}
			}
		}
	}
	return -1;
}

int CheckManager::LoadData(int Index,QVector<CheckString>&mapData)
{
	ClearAll(Index);
	for (auto iter = mapData.begin(); iter != mapData.end(); iter++)	{
		QStringList _strArray = iter->strName.split("_");
		AddCheckTool(Index,_strArray[0], iter->strData);
	}
	return 0;
}

int CheckManager::LoadData(QVector<CheckOutLineData>&vecData)
{
	ClearAll();
	for (size_t i = 0; i < vecData.size(); i++)	{
		CommonData	_comdata;
		QStringList _strNamelst		= vecData[i].strData.split("@");
		QStringList _strFollowlst	= _strNamelst[1].split("|");
		_comdata.type				= ItemArrowLineObj;
		_comdata.strData			= _strNamelst[0];
		auto _Pointlst = _comdata.strData.split('_');
		CheckOutLine _Checkline;
		if (_Pointlst.size() > 1)	_Checkline.line.setP1(QPointF(_Pointlst[1].toDouble(), _Pointlst[0].toDouble()));
		if (_Pointlst.size() > 3)	_Checkline.line.setP2(QPointF(_Pointlst[3].toDouble(), _Pointlst[2].toDouble()));
		_Checkline.bIsFollow		= _strFollowlst[0].toInt();
		_Checkline.txtLinkFollow	= _strFollowlst[1];
		m_vecCheckOutLine.push_back(_Checkline);

		CheckBase::ChangeParamToRegion(_comdata, m_vecCheckOutLine[i].hROILine);
		m_vecRegion.push_back(m_vecCheckOutLine[i].hROILine);

		for (int j = 0; j < vecData[i].vecData.size(); j++)		{
			QStringList _strArray = vecData[i].vecData[j].strName.split("_");
			AddCheckTool(i, _strArray[0], vecData[i].vecData[j].strData);
		}
	}
	return 0;
}

QStringList CheckManager::GetCheckBaseLst(int Index,QString strFilterName, int Currentrow)
{
	QStringList _strNames;
	if (Index < 0)	{
		for (size_t iRow = 0; iRow < m_vecCheck2D.size(); iRow++)	{
			for (size_t j = 0; j < m_vecCheck2D[iRow].size(); j++)	{
				QString	_strCurrentRowName = m_vecCheck2D[iRow][j]->GetCheckName();
				QStringList _strNameArray = _strCurrentRowName.split("_");
				QString _strName = _strNameArray[0];
				bool _bFind = false;
				if (iRow >= Currentrow)	{
					break;
				}
				if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint])
				{
					if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointCenter])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineCross])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypepointStroke])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineCircleCross]))
					{
						_bFind = true;
					}
				}
				else if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine])
				{
					if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeHorizontalLine])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointLine])
						|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeverticalLine]))
					{
						_bFind = true;
					}
				}
				else if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeCircle])
				{
					if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeCircle]))
					{
						_bFind = true;
					}
				}
				if (_bFind)	{
					_strNames.append(_strCurrentRowName);
				}
			}

		}
		return _strNames;
	}
	else if(Index < m_vecCheck2D.size() )	{
		for (size_t j = 0; j < m_vecCheck2D[Index].size(); j++)	{
			QString	_strCurrentRowName = m_vecCheck2D[Index][j]->GetCheckName();
			QStringList _strNameArray = _strCurrentRowName.split("_");
			QString _strName = _strNameArray[0];
			bool _bFind = false;
			if (Index >= Currentrow)	{
				break;
			}
			if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint])
			{
				if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePoint])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointCenter])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineCross])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypepointStroke])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLineCircleCross]))
				{
					_bFind = true;
				}
			}
			else if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine])
			{
				if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeHorizontalLine])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeLine])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypePointPointLine])
					|| (_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeverticalLine]))
				{
					_bFind = true;
				}
			}
			else if (strFilterName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeCircle])
			{
				if ((_strName == frmOutLineDetectTool::PortCheckTypeMap[EnumTypeCircle]))
				{
					_bFind = true;
				}
			}
			if (_bFind)	{
				_strNames.append(_strCurrentRowName);
			}
		}

		
		return _strNames;
	}
	return QStringList();
}

EnumErrorRetn CheckManager::PrExcute(int Index)
{
	if (Index < 0)													{
		for (size_t i = 0; i < m_vecCheck2D.size(); i++)			{
			for (size_t j = 0; j < m_vecCheck2D[i].size(); j++)		{
				m_vecCheck2D[i][j]->eCheckResult = CheckResult_NoCheck;
			}
		}
		return ErrorRetn_OK;
	}
	else if (Index == m_vecCheck2D.size())						{
		for (size_t j = 0; j < m_vecCheck2D[Index].size(); j++) {
			m_vecCheck2D[Index][j]->eCheckResult = CheckResult_NoCheck;
		}
		return ErrorRetn_OK;
	}
	return ErrorRetn_NotContain;
}

EnumErrorRetn CheckManager::Excute(int Index,bool bShowRegion,MiddleParam& param,HObject&img,QString& strError)
{
	HObject hContours, hRegion;
	HTuple Row, Col, Grayval;
	try
	{
		if (m_vecCheck2D.size() <= 0)		{
			strError.append(QString(tr(" Error : didn't have Check Items ")));
			return ErrorRetn_Error;
		}
		if (Index >= m_vecCheck2D.size())	{
			strError.append(QString(tr(" Error : Index is out of ROI size ")));
			return ErrorRetn_Error;
		}
		else if ( Index < 0 )	{
			for (size_t iToolIndex = 0; iToolIndex < m_vecCheck2D.size(); iToolIndex++)	{
				hRegion					= m_vecRegion[iToolIndex];
				HObject _MetrixRegion	= hRegion;

				double dPos[4] = { 0 };
				dPos[0] = m_vecCheckOutLine[iToolIndex].line.p1().y();
				dPos[1] = m_vecCheckOutLine[iToolIndex].line.p1().x();
				dPos[2] = m_vecCheckOutLine[iToolIndex].line.p2().y();
				dPos[3] = m_vecCheckOutLine[iToolIndex].line.p2().x();

				if (m_vecCheckOutLine[iToolIndex].bIsFollow)	{
					HTuple _Metrix;
					if (CheckMetrixType(m_vecCheckOutLine[iToolIndex].txtLinkFollow, _Metrix) == 0) {
						AffineTransRegion(hRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");

						HTuple _Row, _Col;
						//AffineTransPixel(_Metrix, dPos[0], dPos[1], &_Row, &_Col);
						AffineTransPixel(_Metrix, m_vecCheckOutLine[iToolIndex].line.p1().y(), m_vecCheckOutLine[iToolIndex].line.p1().x(), &_Row, &_Col);
						dPos[0] = _Row.D();
						dPos[1] = _Col.D();
						AffineTransPixel(_Metrix, m_vecCheckOutLine[iToolIndex].line.p2().y(), m_vecCheckOutLine[iToolIndex].line.p2().x(), &_Row, &_Col);
						dPos[2] = _Row.D();
						dPos[3] = _Col.D();
					}
					else {
						strError.append(QString(tr(" Error : Metrix Isn't IsInitialized")));
						return ErrorRetn_Continue;
					}
				}
				if (bShowRegion) {
					sColorLine	_Line;
					_Line.Dline.setP1(QPointF(dPos[1], dPos[0]));
					_Line.Dline.setP2(QPointF(dPos[3], dPos[2]));
					_Line.DrawQColor = QColor(0, 0, 255);
					param.VecShowColorLine.push_back(_Line);
				}
				QLineF _line;
				_line.setP1(QPointF(dPos[1], dPos[0]));
				_line.setP2(QPointF(dPos[3], dPos[2]));

				HTuple  hv_Width, hv_Height;
				GetImageSize(img, &hv_Width, &hv_Height);
				HTuple  Phi, Length1, Length2, hMeasureHandle, Distance, hAngle, m_hGrayval;
				DistancePp(_line.p1().y(), _line.p1().x(), _line.p2().y(), _line.p2().x(), &Distance);
				AngleLx(_line.p2().y(), _line.p2().x(), _line.p1().y(), _line.p1().x(), &hAngle);
				HalconCpp::GenMeasureRectangle2(_line.center().y(), _line.center().x(), hAngle, Distance / 2.0,
					8 / 2.0, hv_Width, hv_Height, "nearest_neighbor", &hMeasureHandle);
				//测量出灰度
				MeasureProjection(img, hMeasureHandle, &Grayval);
				//WriteTuple(Grayval,"D:\\tuple.tup");
				CloseMeasure(hMeasureHandle);
				int		iLenth = Grayval.TupleLength().I();

				m_result.ClearALL();
				for (size_t i = 0; i < m_vecCheck2D[iToolIndex].size(); i++) {
					m_iCurrentRow = i;
					EnumErrorRetn retn = m_vecCheck2D[iToolIndex][i]->Excute(iToolIndex, Row, Col, Grayval, m_result, strError);
					if (retn != ErrorRetn_OK) {
						strError.append(QString("%1 %2").arg(tr(" Error In Line")).arg(QString::number(i + 1)));
						return retn;
					}
				}

			}
		}
		else {
			hRegion						= m_vecRegion[Index];
			HObject _MetrixRegion		= hRegion;

			double dPos[4] = {0};		
			dPos[0] = m_vecCheckOutLine[Index].line.p1().y();
			dPos[1] = m_vecCheckOutLine[Index].line.p1().x();
			dPos[2] = m_vecCheckOutLine[Index].line.p2().y();
			dPos[3] = m_vecCheckOutLine[Index].line.p2().x();

			if (m_vecCheckOutLine[ Index ].bIsFollow)	{
				HTuple _Metrix;
				if (CheckMetrixType(m_vecCheckOutLine[ Index ].txtLinkFollow, _Metrix) == 0) {
					AffineTransRegion(hRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
					HTuple _Row,_Col;
					AffineTransPixel(_Metrix, m_vecCheckOutLine[Index].line.p1().y(), m_vecCheckOutLine[Index].line.p1().x(), &_Row, &_Col);
					dPos[0] = _Row.D();
					dPos[1] = _Col.D();
					AffineTransPixel(_Metrix, m_vecCheckOutLine[Index].line.p2().y(), m_vecCheckOutLine[Index].line.p2().x(),  &_Row, &_Col);
					dPos[2] = _Row.D();
					dPos[3] = _Col.D();
				}
				else {
					strError.append(QString(tr(" Error : Metrix Isn't IsInitialized")));
					return ErrorRetn_Continue;
				}
			}
			if (bShowRegion)	{
				sColorLine	_Line;
				_Line.Dline.setP1(QPointF(dPos[1], dPos[0]));
				_Line.Dline.setP2(QPointF(dPos[3], dPos[2]));
				_Line.DrawQColor	= QColor(0, 0, 255);
				param.VecShowColorLine.push_back(_Line);
			}
			QLineF _line;
			_line.setP1(QPointF(dPos[1], dPos[0]));
			_line.setP2(QPointF(dPos[3], dPos[2]));

			HTuple  hv_Width, hv_Height;
			GetImageSize(img, &hv_Width, &hv_Height);
			HTuple  Phi, Length1, Length2, hMeasureHandle, Distance, hAngle, m_hGrayval;
			DistancePp(_line.p1().y(), _line.p1().x(), _line.p2().y(), _line.p2().x(), &Distance);
			AngleLx(_line.p2().y(), _line.p2().x(), _line.p1().y(), _line.p1().x(), &hAngle);
			HalconCpp::GenMeasureRectangle2(_line.center().y(), _line.center().x(), hAngle, Distance / 2.0,
				8 / 2.0, hv_Width, hv_Height, "nearest_neighbor", &hMeasureHandle);
			//测量出灰度
			MeasureProjection(img, hMeasureHandle, &Grayval);
			CloseMeasure(hMeasureHandle);

			int		iLenth = Grayval.TupleLength().I();
			m_result.ClearALL();
			for (size_t i = 0; i < m_vecCheck2D[Index].size(); i++) {
				m_iCurrentRow = i;
				EnumErrorRetn retn = m_vecCheck2D[Index][i]->Excute(Index, Row, Col, Grayval, m_result, strError);
				if (retn != ErrorRetn_OK)							{
					strError.append(QString("%1 %2").arg(tr(" Error In Line")).arg(QString::number(i + 1)));
					return retn;
				}
			}
		}
	}
	catch (const HException& except)			{
		QString _strError = except.ErrorText();
		QStringList strErrorArray = _strError.split(':');
		if (strErrorArray.size() > 1)			{
			if (!strErrorArray[1].isEmpty())	{
				_strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	_strError.append(strErrorArray[i]);
				strError.append(QString("%1 %2").arg(tr(" Error:")).arg(_strError));
				return ErrorRetn_Error;
			}
		}
	}
	catch (...)									{
		strError.append( QString(tr(" Error "))	);
		return ErrorRetn_Error;
	}
	return ErrorRetn_OK;
}
#pragma endregion

#pragma region 检测工具
OutLineDetectTool::OutLineDetectTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "", Port::InStream, Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream, Port::Stream));
}

OutLineDetectTool::~OutLineDetectTool()
{
}

int OutLineDetectTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	//ROI参数
	QJsonArray array_Roiobject = strData.find("ROIConFig").value().toArray();
	m_vecCommonData.clear();
	if (!array_Roiobject.empty())		{
		for (size_t i = 0; i < array_Roiobject.count(); i++)	{
			CommonData _CommonData;
			QJsonArray _Object			= array_Roiobject.at(i).toArray();
			_CommonData.type			= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType		= (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData			= _Object.at(2).toString();
			m_vecCommonData.push_back(_CommonData);
		}
	}
	ChangeParamToRegion(m_vecCommonData, m_ROISearchRegion);

	m_vecCheckOutLineData.clear();
	{	QJsonArray _CheckOutLineArray = strData.find("CheckOutLine").value().toArray();
		for (size_t i = 0; i < _CheckOutLineArray.count(); i++)	{
			CheckOutLineData _Data;
			QJsonArray ROIConFig	= _CheckOutLineArray.at(i).toArray();
			_Data.strName			= ROIConFig.at(0).toString();
			_Data.strData			= ROIConFig.at(1).toString();
			QJsonArray _ROIData		= ROIConFig.at(2).toArray();
			for (size_t JK = 0;JK < _ROIData.count(); JK++)		{
				QJsonArray _ROI = _ROIData.at(JK).toArray();

				CheckString  _data;
				_data.strName = _ROI.at(0).toString();
				_data.strData = _ROI.at(1).toString();

				_Data.vecData.push_back(_data);
			}
			m_vecCheckOutLineData.push_back(_Data);
		}
	}

	UpdateParam();
	return 0;
}

int OutLineDetectTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ROIConFig;
	for (auto& iter : m_vecCommonData)	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		ROIConFig.append(_Object);
	}
	strData.insert("ROIConFig", ROIConFig);

	{	QJsonArray _CheckOutLineParam;
		for (int i = 0; i < m_vecCheckOutLineData.size(); i++)	{
			QJsonArray ROIConFig;
			ROIConFig.append(m_vecCheckOutLineData[i].strName);
			ROIConFig.append(m_vecCheckOutLineData[i].strData);

			QJsonArray _ROIData;
			for (int j = 0; j < m_vecCheckOutLineData[i].vecData.size(); j++)	{
				CheckString&  _data = m_vecCheckOutLineData[i].vecData[j];
				QJsonArray _Object = {
					_data.strName,
					_data.strData,
				};
				_ROIData.append(_Object);
			}
			ROIConFig.append(_ROIData);
			_CheckOutLineParam.append(ROIConFig);
		}
		strData.insert("CheckOutLine", _CheckOutLineParam);
	}

	return 0;
}

void OutLineDetectTool::UpdateParam()
{
	m_CheckManager.ClearAll();
	m_CheckManager.LoadData(m_vecCheckOutLineData);

	if (m_mapParam.contains(QString("checkUseFollow")))		m_bEnableLinkFollow		= m_mapParam["checkUseFollow"].toInt();
	if (m_mapParam.contains(QString("txtLinkFollow")))		m_strLinkName			= m_mapParam["txtLinkFollow"];
	if (m_mapShowParam.contains(QString("showregion")))		m_bEnableShowRegion		= m_mapShowParam["showregion"].toInt();
	if (m_mapShowParam.contains(QString("showDrawData")))	m_bEnableShowData		= m_mapShowParam["showDrawData"].toInt();
	if (m_mapShowParam.contains(QString(FontSize)))			m_iFontSize				= m_mapShowParam[FontSize].toInt();
}

//双击后弹出控件的QT窗口
void OutLineDetectTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmOutLineDetectTool * frm = new frmOutLineDetectTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	ChangeParamToRegion(m_vecCommonData, m_ROISearchRegion);
	frm->m_pTool = nullptr;
	UpdateParam();
	delete frm;
}

void OutLineDetectTool::StopExecute()
{
}

EnumNodeResult OutLineDetectTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult OutLineDetectTool::PrExecute(MiddleParam & param, QString & strError)
{
	m_CheckManager.PrExcute();
	return NodeResult_OK;
}

EnumNodeResult OutLineDetectTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())	{	SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));return NodeResult_ParamError;}
		QString	strError;
		EnumErrorRetn retn			= m_CheckManager.Excute(-1,m_bEnableShowRegion,param,hImg, strError);
		QString _strResultName		= GetItemId() + ".Out.";
		QVector<sDrawText>			_VecText1D;
		switch (retn)				{
		case ErrorRetn_NotContain:	{
			sDrawText				_strText;
			_strText.bControl		= true;
			_strText.iDrawLenth		= m_iFontSize;
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText		= GetItemContent() + tr(" Detection Parameter Error! ");
			_VecText1D.push_back(_strText);
			param.VecDrawText.push_back(_strText);
			return NodeResult_ParamError;
		}	break;
		case ErrorRetn_Error:		{
			sDrawText				_strText;
			_strText.bControl		= true;
			_strText.iDrawLenth		= m_iFontSize;
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText		= GetItemContent() + tr(" Detection Error! ");
			_VecText1D.push_back(_strText);
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}	break;
		case ErrorRetn_NG:		{
			sDrawText _strText;
			_strText.bControl		= true;
			_strText.iDrawLenth		= m_iFontSize;
			_strText.DrawColor		= QColor(255, 0, 0);
			_strText.strText		= GetItemContent() + tr(" Detection NG! ");
			_VecText1D.push_back(_strText);
			param.VecDrawText.push_back(_strText);
			return NodeResult_Error;
		}	break;
		case ErrorRetn_OK:		{
			sDrawText _strText;
			_strText.bControl		= true;
			_strText.iDrawLenth		= m_iFontSize;
			_strText.DrawColor		= QColor(0, 255, 0);
			_strText.strText		= GetItemContent() + tr(" Detection Complete! ");
			_VecText1D.push_back(_strText);
		}	break;
		case ErrorRetn_Continue:		{
			sDrawText			_strText;
			_strText.bControl = true;
			if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = GetItemContent() + tr(" The Link Matrix Failed!");
			param.VecDrawText.push_back(_strText);

			if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
			else			return NodeResult_NG;
		}	break;
		default:	{		}	break;		}

		int _iIndex = 0;
		for (int i = 0; i	<				m_CheckManager.m_vecCheck2D.size(); i++)	{
			for (size_t j = 0; j	<		m_CheckManager.m_vecCheck2D[i].size(); j++) {
				auto&Check					= m_CheckManager.m_vecCheck2D[i][j];
				sDrawText					_sDrawText;
				_sDrawText.bControl			= true;
				_sDrawText.DrawColor		= QColor(255, 0, 0);
				_sDrawText.iDrawLenth		= m_iFontSize;
				QString	_strName			= QString("  %1.%2.").arg( QString::number(i) ).arg(QString::number( j ));
				switch (Check->eCheckResult) {
				case CheckResult_Error: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection Error ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_NG: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection NG ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_OverUp: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Upper Limit of Detection ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_OverDown: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Down Limit of Detection ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_ParamError: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection Parameter Error ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_OK: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Detection OK ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_sDrawText.DrawColor	= QColor(0, 255, 0);
					_VecText1D.push_back(_sDrawText);
				}	break;
				case CheckResult_NoCheck: {
					_sDrawText.strText		= _strName + Check->GetCheckName() + tr(" Created Successfully ") + (m_bEnableShowData ? Check->GetCheckResult() : "");
					_sDrawText.DrawColor	= QColor(0, 255, 0);
					_VecText1D.push_back(_sDrawText);
				}	break;
				default:
					break;
				}
			}
		}

		//显示参数
		for (size_t i = 0; i < _VecText1D.size(); i++){	param.VecDrawText.push_back(_VecText1D[i]);	}
	}
	else {
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		sDrawText			_strText;
		_strText.bControl = true;
		if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = GetItemContent() + tr(" No Relevant Images Are Included!");
		param.VecDrawText.push_back(_strText);

		if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
		else			return NodeResult_NG;
	}
	return NodeResult_OK;
}

QPair<QString, QString>  OutLineDetectTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString>  OutLineDetectTool::GetToolName()
{
	return 	QPair<QString, QString>("OutLine", tr("OutLine"));
}

QPointF OutLineDetectTool::FindFoot(QLineF line, QPointF pA)
{
	float k = 0.0;
	QPointF pFoot;
	if (line.p1().x() == line.p2().x())
	{
		//pFoot = QPointF(pA.x(), line.p1().y());
		pFoot = QPointF(line.p1().x(), pA.y());
		return pFoot;
	}
	k = (line.p2().y() - line.p1().y()) * 1.0 / (line.p2().x() - line.p1().x());
	float A = k;
	float B = -1.0;
	float C = line.p1().y() - k * line.p1().x();

	pFoot.setX((B * B * pA.x() - A * B * pA.y() - A * C) / (A * A + B * B));
	pFoot.setY((A * A * pA.y() - A * B * pA.x() - B * C) / (A * A + B * B));
	return pFoot;
}

double OutLineDetectTool::GetP2PDistance(QPointF&ptA, QPointF&ptB)
{
	return sqrt((ptA.x() - ptB.x()) * (ptA.x() - ptB.x()) + (ptA.y() - ptB.y()) * (ptA.y() - ptB.y()));
}

double OutLineDetectTool::GetP2LDistance(QPointF & ptA, QLineF&line)
{
	QPointF ptstart = line.p1();
	QPointF ptEnd = line.p2();
	return abs((ptstart.y() - ptEnd.y()) * ptA.x() + (ptEnd.x() - ptstart.x()) * ptA.y() + ptstart.x() * ptEnd.y() - ptstart.y() * ptEnd.x())
		/ sqrt((ptstart.y() - ptEnd.y()) * (ptstart.y() - ptEnd.y()) + (ptstart.x() - ptEnd.x()) * (ptstart.x() - ptEnd.x()));
	return 0.0;
}

double OutLineDetectTool::GetAngleInLines(const QLineF& inline1, const QLineF& inline2)
{
	QPointF point1 = inline1.p1();
	QPointF point2 = inline1.p2();
	QPointF point3 = inline2.p1();
	QPointF point4 = inline2.p2();

	QVector2D vector1(point2 - point1);
	QVector2D vector2(point4 - point3);

	qreal dotProduct = QVector2D::dotProduct(vector1, vector2);
	qreal length1 = vector1.length();
	qreal length2 = vector2.length();

	// 防止除以零
	if (length1 == 0 || length2 == 0) { return 0; }
	qreal cosAngle = dotProduct / (length1 * length2);
	qreal angle = std::acos(cosAngle);// *(180 / M_PI); // 转换弧度到角度

	return angle;
}

QList<QPointF> OutLineDetectTool::calculateLineRectIntersections(const QLineF &line, const QRectF &rect)
{
	QList<QPointF> intersections;
	// 检查与矩形的左边相交
	QLineF leftLine(rect.topLeft(), rect.bottomLeft());
	QPointF intersectionLeft;
	if (line.intersect(leftLine, &intersectionLeft) == QLineF::BoundedIntersection) {
		intersections.append(intersectionLeft);
	}
	// 检查与矩形的右边相交
	QLineF rightLine(rect.topRight(), rect.bottomRight());
	QPointF intersectionRight;
	if (line.intersect(rightLine, &intersectionRight) == QLineF::BoundedIntersection) {
		intersections.append(intersectionRight);
	}
	// 检查与矩形的顶部相交
	QLineF topLine(rect.topLeft(), rect.topRight());
	QPointF intersectionTop;
	if (line.intersect(topLine, &intersectionTop) == QLineF::BoundedIntersection) {
		intersections.append(intersectionTop);
	}
	// 检查与矩形的底部相交
	QLineF bottomLine(rect.bottomLeft(), rect.bottomRight());
	QPointF intersectionBottom;
	if (line.intersect(bottomLine, &intersectionBottom) == QLineF::BoundedIntersection) {
		intersections.append(intersectionBottom);
	}
	return intersections;
}

int OutLineDetectTool::CalculateLineRectIntersections(const QLineF & Inline1, const QRectF & rect, QLineF& Outline)
{
	QList<QPointF> intersections;
	// 检查与矩形的左边相交
	QLineF leftLine(rect.topLeft(), rect.bottomLeft());
	QPointF intersectionLeft;
	QLineF::IntersectType Retn = Inline1.intersect(leftLine, &intersectionLeft);
	if (Retn == QLineF::BoundedIntersection) {
		intersections.append(intersectionLeft);
	}
	else if (IsPointInRect(intersectionLeft, rect) && (Retn == QLineF::UnboundedIntersection))
	{
		intersections.append(intersectionLeft);
	}
	// 检查与矩形的右边相交
	QLineF rightLine(rect.topRight(), rect.bottomRight());
	QPointF intersectionRight;
	Retn = Inline1.intersect(rightLine, &intersectionRight);
	if (Retn == QLineF::BoundedIntersection) {
		intersections.append(intersectionRight);
	}
	else if (IsPointInRect(intersectionRight, rect) && (Retn == QLineF::UnboundedIntersection))
	{
		intersections.append(intersectionRight);
	}
	// 检查与矩形的顶部相交
	QLineF topLine(rect.topLeft(), rect.topRight());
	QPointF intersectionTop;
	Retn = Inline1.intersect(topLine, &intersectionTop);
	if (Retn == QLineF::BoundedIntersection) {
		intersections.append(intersectionTop);
	}
	else if (IsPointInRect(intersectionTop, rect) && (Retn == QLineF::UnboundedIntersection))
	{
		intersections.append(intersectionTop);
	}
	// 检查与矩形的底部相交
	QLineF bottomLine(rect.bottomLeft(), rect.bottomRight());
	QPointF intersectionBottom;
	Retn = Inline1.intersect(bottomLine, &intersectionBottom);
	if (Retn == QLineF::BoundedIntersection) {
		intersections.append(intersectionBottom);
	}
	else if (IsPointInRect(intersectionBottom, rect) && (Retn == QLineF::UnboundedIntersection))
	{
		intersections.append(intersectionBottom);
	}

	if (intersections.size() > 1)
	{
		Outline.setP1(intersections[0]);
		Outline.setP2(intersections[1]);
	}
	else
	{
		return -1;
	}
	return 0;
}

bool OutLineDetectTool::IsPointInRect(const QPointF& pt, const QRectF &rect)
{
	if ((pt.x() - rect.right()) < 0.001 && (pt.x() - rect.left()) > -0.001
		&& (pt.y() - rect.bottom()) < 0.001 && (pt.y() - rect.top()) > -0.001)
		return true;
	return false;
}

bool OutLineDetectTool::IsInLenthLimit(qreal x1, qreal x2)
{
	return (abs(x1 - x2) <= 0.001) ? true : false;
}

#pragma endregion
