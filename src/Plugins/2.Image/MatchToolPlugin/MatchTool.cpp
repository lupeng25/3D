#include "MatchTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmMatchTool.h"
#include "databaseVar.h"
#include <QCryptographicHash>

MatchTool::MatchTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	m_VecCreateCross.clear();
	m_EMatchtype	= EnumModeType_Shape;
}

MatchTool::~MatchTool()
{
	m_ROISearchRegion.Clear();
	ClearMatchShapeModel();
	ClearMatchNCCModel();
	m_VecCreateCross.clear();
}

void MatchTool::ClearMatchShapeModel()
{
	try {	ClearShapeModel(m_hShapeModel);	}catch (...) {	}
}

void MatchTool::ClearMatchNCCModel()
{
	try {	ClearNccModel(m_hNCCModel);	}	catch (...) {}
}

int MatchTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_Roiobject = strData.find("ROIConFig").value().toArray();
	m_vecCommonData.clear();
	if (!array_Roiobject.empty())	{
		for (size_t i = 0; i < array_Roiobject.count(); i++)	{
			CommonData _CommonData;
			QJsonArray _Object		= array_Roiobject.at(i).toArray();
			_CommonData.type		= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType = (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData		= _Object.at(2).toString();
			m_vecCommonData.push_back(_CommonData);
		}
	}
	QJsonArray array_Searchobject = strData.find("SearchROIConFig").value().toArray();
	m_vecSearchData.clear();
	if (!array_Searchobject.empty())	{
		for (size_t i = 0; i < array_Searchobject.count(); i++)		{
			CommonData _CommonData;
			QJsonArray _Object			= array_Searchobject.at(i).toArray();
			_CommonData.type			= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType		= (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData = _Object.at(2).toString();
			m_vecSearchData.push_back(_CommonData);
		}
	}
	ChangeParamToRegion(m_vecSearchData, m_ROISearchRegion);	//当前表示模板区域

	m_VecCreateCross.clear();
	QJsonArray CreateCrossArray = strData.find("CreateCrossArray").value().toArray();
	for (size_t i = 0; i < CreateCrossArray.count(); i++)	{
		QJsonArray _ConFig = CreateCrossArray.at(i).toArray();
		sDrawCross _data;
		_data.Center.setX(_ConFig.at(0).toString().toDouble());
		_data.Center.setY(_ConFig.at(1).toString().toDouble());
		_data.fAngle = (_ConFig.at(2).toString().toDouble());
		m_VecCreateCross.push_back(_data);
	}

	m_fStartAngle		= m_mapParam["spBx_FindAngleStart"].toDouble();
	m_fEndAngle			= m_mapParam["spBx_FindAngleRangle"].toDouble();
	m_fMinScore			= m_mapParam["spBx_FindMinScore"].toDouble();
	m_iMatchNumber		= m_mapParam["spBx_FindMinNumber"].toDouble();
	m_fMatchGreedy		= m_mapParam["spBx_FindGreedy"].toDouble();
	m_fMatchOverLap		= m_mapParam["spBx_FindMinOverLap"].toDouble();
	m_iSubPixType		= m_mapParam["cbx_SubPixType"].toDouble();
	m_EMatchtype		= (EnumModeType)m_mapParam["cbx_MatchType"].toInt();

	ChangeParamToRegion(m_vecSearchData, m_ROISearchRegion);	//当前表示模板区域

	switch (m_EMatchtype)
	{
	case EnumModeType_Shape: {
		ClearMatchShapeModel();
		try {
			//将反序列化转化为模板
			HTuple _ShapeModel;
			QString _str		= strData.find("ShapeModel").value().toString();
			QByteArray	_data	= _str.toLatin1();
			QByteArray _Buf		= QByteArray::fromBase64(_data);
			Hlong hSize			= _Buf.length();
			BYTE*_Data			= new BYTE[hSize];
			memset(_Data, 0, hSize);
			memcpy_s(_Data, hSize, _Buf.data(), hSize);
			HSerializedItem item(_Data, hSize, "true");
			DeserializeShapeModel(item, &_ShapeModel);
			m_hShapeModel		= _ShapeModel;
			delete[]_Data;
		}
		catch (...) { qCritical() << tr("Flow") + QString::number(m_iFlowIndex) + tr("Read Shape Model Data Fail!"); }
	}	break;
	case EnumModeType_NCC: {

		ClearMatchNCCModel();
		try {
			//将反序列化转化为模板
			HTuple _ShapeModel;
			QString _str		= strData.find("NCCModel").value().toString();
			QByteArray	_data	= _str.toLatin1();
			QByteArray _Buf		= QByteArray::fromBase64(_data);

			Hlong hSize			= _Buf.length();
			BYTE*_Data			= new BYTE[hSize];
			memset(_Data, 0, hSize);
			memcpy_s(_Data, hSize, _Buf.data(), hSize);
			HSerializedItem item(_Data, hSize, "true");
			DeserializeNccModel(item, &_ShapeModel);
			m_hNCCModel = _ShapeModel;
			delete[]_Data;
		}
		catch (...) { qCritical() << tr("Flow") + QString::number(m_iFlowIndex) + tr("Read NCC Model Data Fail!"); }

	}	break;
	default:
		break;
	}

	return 0;
}

int MatchTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ROIConFig;
	for (auto& iter : m_vecCommonData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		ROIConFig.append(_Object);
	}
	strData.insert("ROIConFig", ROIConFig);

	QJsonArray SearchROIConFig;
	for (auto& iter : m_vecSearchData)
	{
		QJsonArray _Object = {
			QString::number(iter.type),
			QString::number(iter.eRegionType),
			iter.strData,
		};
		SearchROIConFig.append(_Object);
	}
	strData.insert("SearchROIConFig", SearchROIConFig);

	QJsonArray CreateCrossArray;
	for (int i = 0; i < m_VecCreateCross.size(); i++)
	{
		QJsonArray _ROI = {
			QString::number(m_VecCreateCross[i].Center.x()),
			QString::number(m_VecCreateCross[i].Center.y()),
			QString::number(m_VecCreateCross[i].fAngle),
		};
		CreateCrossArray.append(_ROI);
	}
	strData.insert("CreateCrossArray", CreateCrossArray);

	QString _str;
	switch (m_EMatchtype)
	{
	case EnumModeType_Shape: {
		try {
			//将模板序列化
			HTuple SerializedItemHandle;
			SerializeShapeModel(m_hShapeModel, &SerializedItemHandle);
			//HSerializedItem item(SerializedItemHandle.H());	//高版本的Halcon里面有相关的东西
			HSerializedItem item(SerializedItemHandle);
			Hlong hSize;
			void *_pStart = item.GetSerializedItemPtr(&hSize);
			BYTE*_Data = new BYTE[hSize];
			memset(_Data, 0, hSize);
			memcpy_s(_Data, hSize, _pStart, hSize);

			//将数据转化为QString
			QByteArray byteArray = QByteArray((char*)_Data, hSize);
			_str = QString::fromLatin1(byteArray.toBase64());

			strData.insert("ShapeModel", _str);//
			delete[]_Data;
		}
		catch (...) {
			strData.insert("ShapeModel", _str);//
			qCritical() << tr("Flow") + QString::number(m_iFlowIndex) + tr( "Set Shape Model Data Error!");
		}
	}	break;
	case EnumModeType_NCC:	{
		_str.clear();
		try		{
			//将模板序列化
			HTuple SerializedItemHandle;
			SerializeNccModel(m_hNCCModel, &SerializedItemHandle);
			//HSerializedItem item(SerializedItemHandle.H());	//高版本的Halcon里面有相关的东西
			HSerializedItem item(SerializedItemHandle);
			Hlong hSize;
			void *_pStart = item.GetSerializedItemPtr(&hSize);
			BYTE*_Data = new BYTE[hSize];
			memset(_Data, 0, hSize);
			memcpy_s(_Data, hSize, _pStart, hSize);
			//将数据转化为QString
			QByteArray byteArray = QByteArray((char*)_Data, hSize);
			_str = QString::fromLatin1(byteArray.toBase64());
			strData.insert("NCCModel", _str);//
			delete[]_Data;
		}
		catch (...) {
			strData.insert("NCCModel", _str);//
			qCritical() << tr("Flow") + QString::number(m_iFlowIndex) + tr("Set NCC Model Data Error!");
		}
	}break;
	default: {

	}	break;
	}

	return 0;
}

//双击后弹出控件的QT窗口
void MatchTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmMatchTool * frm = new frmMatchTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	//frm->Save();
	frm->m_pTool = nullptr;

	m_fStartAngle	= m_mapParam["spBx_FindAngleStart"].toDouble();
	m_fEndAngle		= m_mapParam["spBx_FindAngleRangle"].toDouble();
	m_fMinScore		= m_mapParam["spBx_FindMinScore"].toDouble();
	m_iMatchNumber	= m_mapParam["spBx_FindMinNumber"].toDouble();
	m_fMatchGreedy	= m_mapParam["spBx_FindGreedy"].toDouble();
	m_fMatchOverLap = m_mapParam["spBx_FindMinOverLap"].toDouble();
	m_iSubPixType	= m_mapParam["cbx_SubPixType"].toDouble();
	ChangeParamToRegion(m_vecSearchData, m_ROISearchRegion);	//当前表示模板区域

	//WriteRegion(m_ROISearchRegion,"d:\\m_ROISearchRegion.hobj");
	delete frm;
}

void MatchTool::StopExecute()
{
}

EnumNodeResult MatchTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult MatchTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult MatchTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0){
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())	{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		HTuple hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_Greedy, hv_SubPixel;
		HTuple Row, Column, Angle, Scale, Score, hv_HomMat;
		hv_AngleStart	= HTuple(m_fStartAngle).TupleRad();
		hv_AngleExtent	= HTuple(m_fEndAngle).TupleRad();
		hv_MinScore		= m_fMinScore ;
		hv_NumMatches	= m_iMatchNumber;
		hv_Greedy		= m_fMatchGreedy;
		hv_MaxOverlap	= m_fMatchOverLap;

		QString _strResultName = GetItemId() + ".Out.";
		try	{
			HObject	_ReduceImg, _ROIRegion;
			ReduceDomain(hImg, m_ROISearchRegion, &_ReduceImg);
			switch (m_EMatchtype)
			{
			case EnumModeType_Shape: {
				switch (m_iSubPixType) {
				case 0: {	hv_SubPixel = "none";						}break;
				case 1: {	hv_SubPixel = "interpolation";				}break;
				case 2: {	hv_SubPixel = "least_squares";				}break;
				case 3: {	hv_SubPixel = "least_squares_high";			}break;
				case 4: {	hv_SubPixel = "least_squares_very_high";	}break;
				default:	break;				}
				FindShapeModel(_ReduceImg, m_hShapeModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, hv_Greedy, &Row, &Column, &Angle, &Score);
			}	break;
			case EnumModeType_NCC: {
				switch (m_iSubPixType) {
				case 0: {	hv_SubPixel = "false";			}break;
				case 1: {	hv_SubPixel = "true";			}break;
				default:	break;
				}
				FindNccModel(_ReduceImg, m_hNCCModel, hv_AngleStart, hv_AngleExtent, hv_MinScore, hv_NumMatches, hv_MaxOverlap, hv_SubPixel, 0, &Row, &Column, &Angle, &Score);
			}	break;
			default:
				break;
			}

			int	_iScoreIndex = Score.TupleLength();
			if (_iScoreIndex <= 0)	{
				sDrawText			_strText;
				_strText.bControl	= true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor	= QColor(255,0,0);
				_strText.strText	= GetItemContent() + tr(" Match NG");
				param.VecDrawText.push_back( _strText);
				param.MapBool.insert(std::pair<QString, bool>(_strResultName + "Result", false));
				param.MapInt.insert(std::pair<QString, int>(_strResultName + "Number", _iScoreIndex));
			}
			else{
				sDrawText _strText;
				_strText.bControl		= true;				
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();

				_strText.DrawColor		= QColor(0, 255, 0);
				_strText.strText		= GetItemContent() + tr(" Match OK");
				param.VecDrawText.push_back(_strText);
				param.MapBool.insert(std::pair<QString, bool>(_strResultName + "Result", true));
				param.MapInt.insert(std::pair<QString, int>(_strResultName + "Number", _iScoreIndex));
			}
			for (int hv_MatchingObjIdx = 0; hv_MatchingObjIdx < _iScoreIndex; hv_MatchingObjIdx++){
				param.MapCrossPointF.insert(std::pair<QString, sResultCross>(_strResultName + QString::number(hv_MatchingObjIdx), sResultCross(Column[hv_MatchingObjIdx], Row[hv_MatchingObjIdx], Angle[hv_MatchingObjIdx])));
				//结果

				sDrawCross _Cross;
				_Cross.Center		= QPointF(Column[hv_MatchingObjIdx], Row[hv_MatchingObjIdx]);
				_Cross.DrawColor	= QColor(0, 255, 0);
				if (m_mapShowParam.count(FontSize) > 0)	_Cross.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_Cross.fAngle		= Angle[hv_MatchingObjIdx].D();
				param.MapDrawCross.insert(std::pair<QString, sDrawCross>(_strResultName + QString::number(hv_MatchingObjIdx), _Cross));

				sDrawText _strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(0, 255, 0);
				_strText.strText = QString("  %1.r:%2,c:%3,a:%4,s:%5")
					.arg(QString::number(hv_MatchingObjIdx))
					.arg(QString::number(Row[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(Column[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number((Angle[hv_MatchingObjIdx]).ToTuple().TupleDeg().D(), 'f', databaseVar::Instance().form_System_Precision))
					.arg(QString::number(Score[hv_MatchingObjIdx].D(), 'f', databaseVar::Instance().form_System_Precision));

				param.VecDrawText.push_back(_strText);
				//param.MapDrawText.insert(std::pair<QString, sDrawText>(_strResultName		+ QString::number(hv_MatchingObjIdx),	_strText));
				param.MapDou.insert( std::pair<QString, double>(_strResultName + "x"		+ QString::number(hv_MatchingObjIdx),	Column[hv_MatchingObjIdx].D()	));
				param.MapDou.insert( std::pair<QString, double>(_strResultName + "y"		+ QString::number(hv_MatchingObjIdx),	Row[hv_MatchingObjIdx].D()		));
				param.MapDou.insert( std::pair<QString, double>(_strResultName + "angle"	+ QString::number(hv_MatchingObjIdx),	Angle[hv_MatchingObjIdx].D()	));
				param.MapDou.insert( std::pair<QString, double>(_strResultName + "score"	+ QString::number(hv_MatchingObjIdx),	Score[hv_MatchingObjIdx].D()	));

				//计算跟随矩阵
				if (m_VecCreateCross.size() > hv_MatchingObjIdx){
					VectorAngleToRigid(HTuple(m_VecCreateCross[hv_MatchingObjIdx].Center.y()),
						HTuple(m_VecCreateCross[hv_MatchingObjIdx].Center.x()),
						HTuple(m_VecCreateCross[hv_MatchingObjIdx].fAngle),
						Row[hv_MatchingObjIdx], 
						Column[hv_MatchingObjIdx], 
						Angle[hv_MatchingObjIdx],
						&m_hHomMat2D);
					param.MapPtrMetrix.insert(std::pair<QString, void*>(_strResultName + QString::number(hv_MatchingObjIdx), &m_hHomMat2D));
				}
			}
			if (m_mapShowParam.count(showregion) > 0) {
				if (m_mapShowParam[showregion].toInt() > 0) {
					frmMatchTool::AffineShapeTrans(m_vecSearchData, HTuple(), param);
				}
			}
		}
		catch (...)	{
			sDrawText _strText;
			_strText.bControl	= true;
			if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
			_strText.DrawColor	= QColor(255, 0, 0);
			_strText.strText	= GetItemContent() + tr(" Match Error");
			param.VecDrawText.push_back(_strText);
			param.MapBool.insert(std::pair<QString,		bool>(_strResultName + "Result", false));
			param.MapInt.insert(std::pair<QString,		int>(_strResultName + "Number", 0));
		}
	}
	else
	{
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

QPair<QString, QString> MatchTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> MatchTool::GetToolName()
{
	return QPair<QString, QString>("Match",tr("Match"));
}
