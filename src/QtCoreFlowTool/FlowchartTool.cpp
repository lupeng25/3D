#include "FlowchartTool.h"
//#include "databaseVar.h"
#include "QGraphicsViews.h"

FlowchartTool::FlowchartTool() :FlowchartNode()
{
	m_mapParam.clear();
	m_mapShowParam.clear();
}

FlowchartTool::~FlowchartTool()
{

}

//将ROI参数转化为区域
int FlowchartTool::ChangeParamToRegion(QVector<CommonData>& vecData, HObject & region)
{
	region.GenEmptyObj();
	float fData[5] = { 0 };
	for (auto iter : vecData)	{
		QStringList dArray = iter.strData.split("_");
		if (dArray.size() > 0)	fData[0] = dArray[0].toDouble();
		if (dArray.size() > 1)	fData[1] = dArray[1].toDouble();
		if (dArray.size() > 2)	fData[2] = dArray[2].toDouble();
		if (dArray.size() > 3)	fData[3] = dArray[3].toDouble();
		if (dArray.size() > 4)	fData[4] = dArray[4].toDouble();
		HObject _Object;
		_Object.GenEmptyObj();
		switch (iter.type)	{
		case ItemPoint: {			// 圆
			switch ((ShapeItemType)(int)fData[3]) {
			default:
			case ItemPoint: {	GenCircle(&_Object, fData[0], fData[1], fData[2]);	}	break;
			case ItemRectangle:
			case ItemRectangleR:
			case ItemSquare: {	GenRectangle1(&_Object, fData[0] - fData[2], fData[1] - fData[2], fData[0] + fData[2], fData[1] + fData[2]); }	break;
			}
		}break;
		case ItemCircle: {			// 圆
			GenCircle(&_Object, fData[0], fData[1], fData[2]);
		}break;
		case ItemEllipse: {			// 椭圆
			GenEllipse(&_Object, fData[0], fData[1], fData[2], fData[3], fData[4]);
		}break;
		case ItemConcentric_Circle: {	// 同心圆	
			HObject _Object1;
			GenCircle(&_Object1, fData[0], fData[1], fData[2]);
			GenCircle(&_Object, fData[0], fData[1], fData[3]);
			if (fData[2] > fData[3])		{
				Difference(_Object1, _Object, &_Object);
			}
			else		{
				Difference(_Object, _Object1, &_Object);
			}
		}break;
		case ItemRectangle: {		// 矩形
			GenRectangle1(&_Object, fData[0] - fData[3] / 2.0, fData[1] - fData[2] / 2.0, fData[0] + fData[3] / 2.0, fData[1] + fData[2] / 2.0);
		}break;
		case ItemRectangleR: {		// 旋转矩形
			GenRectangle2(&_Object, fData[0], fData[1], HTuple(fData[2]).TupleRad(), fData[3], fData[4]);
		}break;
		case ItemPolygon: {		// 多边形	
			HTuple Rows, Cols;
			auto _vecData = iter.strData.split("|");
			for (int i = 0; i < (_vecData.size() - 1); i++) {
				auto _vecPos = _vecData[i].split(",");
				QPointF _PtPos;
				if (_vecPos.size() > 0)	_PtPos.setX(_vecPos[0].toDouble());
				if (_vecPos.size() > 1)	_PtPos.setY(_vecPos[1].toDouble());
				Rows.TupleConcat(_PtPos.y());
				Cols.TupleConcat(_PtPos.x());
			}
			HalconCpp::GenRegionPolygon(&_Object, Rows, Cols);
		}break;
		case ItemLineObj: {			// 直线
			GenRegionLine(&_Object, fData[0], fData[1], fData[2], fData[3]);
		}break;
		default:
			break;
		}
		switch (iter.eRegionType)
		{
		case RegionType_Add:
		{
			Union2(_Object, region, &region);
		}	break;
		case RegionType_Differe:
		{
			Difference(region, _Object, &region);
		}	break;
		default:
			break;
		}
	}
	return 0;
}

int FlowchartTool::ChangeROIDataToRegion(QJsonArray & vecData, HObject & region)
{
	if (!vecData.empty()) {
		for (int i = 0; i < vecData.count(); i++) {
			QJsonArray _ArrayObject = vecData.at(i).toArray();
			ShapeItemType _type = (ShapeItemType)vecData.at(0).toString().toInt();
			QJsonObject _DataObject = vecData.at(1).toObject();
			switch (_type) {
			case ItemPoint: {

			}	break;
			case ItemCircle: {

			}	break;
			case ItemEllipse: {

			}	break;
			case ItemConcentric_Circle: {

			}	break;
			case ItemRectangle: {

			}	break;
			case ItemRectangleR: {

			}	break;
			case ItemSquare: {

			}	break;
			case ItemPolygon: {

			}	break;
			case ItemLineObj: {

			}	break;
			case ItemArrowLineObj: {

			}	break;
			case ItemBezierLineObj: {

			}	break;
			case ItemAutoItem:
			default: {
			}	break;
			}
		}
	}
	return 0;
}

int FlowchartTool::ChangeParamTovecMeasure(QJsonArray & vecData, QVector<QVector<MeasureRect2>>& vecMeasure)
{
	vecMeasure.clear();
	if (!vecData.empty()) {
		for (int i = 0; i < vecData.count(); i++) {
			QJsonObject _ArrayObject = vecData.at(i).toObject();
			ShapeItemType _type = ItemBezierLineObj;
			if (_ArrayObject.contains("ShapeItemType"))		_type = (ShapeItemType)_ArrayObject["ShapeItemType"].toString().toInt();
			else return -1;
			QJsonObject _DataObject = _ArrayObject.find("ROIConFig").value().toObject();
			QVector<MeasureRect2> _vecMeasure;
			BezierLineItem::ChangeParamTovecMeasure(_DataObject, _vecMeasure);
			vecMeasure.push_back(_vecMeasure);
		}
	}

	return 0;
}

int FlowchartTool::CheckMetrixType(QString strCom, HTuple& Metrix)
{
	MiddleParam param;
	QStringList _ComLst = strCom.split(":");
	if (_ComLst.size() > 2) {
		bool _bFind = false;
		if (_ComLst[0] == "GlobalVar") {
			param = (*MiddleParam::Instance());
			_bFind = true;
		}
		for (auto& iter : (*MiddleParam::Instance()).MapMiddle()) {
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

int FlowchartTool::SetData(QJsonObject & strData)
{
	FlowchartNode::SetData(strData);
	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_strInImg = array_object.at(0).toString();
	}
	//显示
	{
		m_mapParam.clear();
		QJsonObject JShowParam = strData.find("mapParam").value().toObject();
		for (auto iter = JShowParam.begin(); iter != JShowParam.end(); iter++)	m_mapParam[iter.key()] = iter.value().toString();
	}

	//显示
	{
		m_mapShowParam.clear();
		QJsonObject JShowParam = strData.find("mapShowParam").value().toObject();
		for (auto iter = JShowParam.begin(); iter != JShowParam.end(); iter++)	m_mapShowParam[iter.key()] = iter.value().toString();

	}


	return 0;
}

int FlowchartTool::GetData(QJsonObject & strData)
{
	FlowchartNode::GetData(strData);
	QJsonArray ConFig = {
		m_strInImg,
	};
	strData.insert("ConFig", ConFig);

	//公共参数
	QJsonObject _CommonParam;
	for (auto iter = m_mapParam.begin(); iter != m_mapParam.end(); iter++)
		_CommonParam.insert(iter.key(), iter.value());
	strData.insert("mapParam", _CommonParam);

	//显示参数保存
	QJsonObject _ShowParam;
	for (auto iter = m_mapShowParam.begin(); iter != m_mapShowParam.end(); iter++)
		_ShowParam.insert(iter.key(), iter.value());
	strData.insert("mapShowParam", _ShowParam);

	return 0;
}
