#include "PlaneFitTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmPlaneFitTool.h"
#include "databaseVar.h"

PlaneFitTool::PlaneFitTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
}

PlaneFitTool::~PlaneFitTool()
{

}

int PlaneFitTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_Roiobject = strData.find("ROIConFig").value().toArray();
	m_vecCommonData.clear();
	if (!array_Roiobject.empty())
	{
		for (size_t i = 0; i < array_Roiobject.count(); i++)
		{
			CommonData _CommonData;
			QJsonArray _Object			= array_Roiobject.at(i).toArray();
			_CommonData.type			= (ShapeItemType)_Object.at(0).toString().toInt();
			_CommonData.eRegionType		= (EnumItemRegionType)_Object.at(1).toString().toInt();
			_CommonData.strData			= _Object.at(2).toString();
			m_vecCommonData.push_back(_CommonData);
		}
	}
	ChangeParamToRegion(m_vecCommonData, m_ROISearchRegion);

	UpdateParam();

	return 0;
}

int PlaneFitTool::GetData(QJsonObject & strData)
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

	return 0;
}

void PlaneFitTool::UpdateParam()
{
	if (m_mapParam.contains("txtLinkFollow"))	m_strLinkName = m_mapParam["txtLinkFollow"];
	if (m_mapParam.contains("checkUseFollow"))	m_bEnableCom = m_mapParam["checkUseFollow"].toInt();

	if (m_mapParam.contains("spinMinValue"))	m_fLowLimit = m_mapParam["spinMinValue"].toDouble();
	if (m_mapParam.contains("spinMaxValue"))	m_fHighLimit = m_mapParam["spinMaxValue"].toDouble();

	if (m_mapShowParam.contains(QString("showregion"))) m_bEnableShowRegion = m_mapShowParam["showregion"].toInt();
	if (m_mapShowParam.contains(QString(FontSize)))		m_iFontSize = m_mapShowParam[FontSize].toInt();

}

//双击后弹出控件的QT窗口
void PlaneFitTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmPlaneFitTool * frm = new frmPlaneFitTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	ChangeParamToRegion(m_vecCommonData, m_ROISearchRegion);
	frm->m_pTool = nullptr;

	UpdateParam();

	delete frm;
}

void PlaneFitTool::StopExecute()
{
}

EnumNodeResult PlaneFitTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult PlaneFitTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult PlaneFitTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())	{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		QString _strResultName	= GetItemId() + ".Out.";
		int _iCheckMode			= databaseVar::Instance().m_iCheckMode;
		double _dFlatNess = 0;

		HObject _MetrixRegion = m_ROISearchRegion;
		if (m_bEnableCom) {
			HTuple _Metrix;
			if (CheckMetrixType(m_strLinkName, _Metrix) == 0) {
				AffineTransRegion(m_ROISearchRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");
				if (m_bEnableShowRegion)
					frmPlaneFitTool::AffineShapeTrans(m_vecCommonData, _Metrix, param, true);
			}
			else {
				SetError(QString("%1 Find Metrix Isn't IsInitialized").arg(m_strInImg));
				sDrawText			_strText;
				_strText.bControl	= true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor	= QColor(255, 0, 0);
				_strText.strText	= GetItemContent() + tr(" Link Metrix Error!");
				param.VecDrawText.push_back(_strText);

				if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
				else			return NodeResult_NG;
			}
		}
		else {
			if (m_bEnableShowRegion)
				frmPlaneFitTool::AffineShapeTrans(m_vecCommonData, HTuple(), param);
		}
		//计算平面度
		ExcutePlaneFit(hImg, _MetrixRegion, _iCheckMode, m_mapParam, _dFlatNess);
		param.MapDou[_strResultName]	= _dFlatNess;
		if (_dFlatNess >= m_fLowLimit && _dFlatNess <= m_fHighLimit)	{
			sDrawText _strText;
			_strText.bControl		= true;
			_strText.iDrawLenth		= m_iFontSize;
			_strText.DrawColor		= QColor(0, 255, 0);
			_strText.strText		= GetItemContent() + tr(" Check OK Planeness") + QString("%1 %2 %3 -- %4")
				.arg(QString::number(_dFlatNess,	'f', databaseVar::Instance().form_System_Precision))
				.arg(tr("In Range:"))
				.arg(QString::number(m_fLowLimit,	'f', databaseVar::Instance().form_System_Precision))
				.arg(QString::number(m_fHighLimit,	'f', databaseVar::Instance().form_System_Precision));
			param.VecDrawText.push_back(_strText);
		}
		else {
			sDrawText _strText;
			_strText.bControl = true;
			_strText.iDrawLenth = m_iFontSize;
			_strText.DrawColor = QColor(255, 0, 0);
			_strText.strText = GetItemContent() + tr("  Check NG Planeness") + QString("%1 %2 %3 -- %4")
				.arg(QString::number(_dFlatNess,	'f', databaseVar::Instance().form_System_Precision))
				.arg(tr("In Range:"))
				.arg(QString::number(m_fLowLimit,	'f', databaseVar::Instance().form_System_Precision))
				.arg(QString::number(m_fHighLimit,	'f', databaseVar::Instance().form_System_Precision));
			param.VecDrawText.push_back(_strText);
		}
		if (m_bEnableShowRegion)	{
			HTuple	Rows, Columns, _Lenth;
			GetRegionContour(_MetrixRegion, &Rows, &Columns);
			HalconCpp::TupleLength(Rows, &_Lenth);
			sColorPoint _ptColors;
			_ptColors.DrawQColor = QColor(0, 0, 255);
			for (int iPtNum = 0; iPtNum < _Lenth.I(); iPtNum++) {
				_ptColors.vecPoints.push_back(QPointF(Columns[iPtNum], Rows[iPtNum]));
			}
			param.VecShowColorPoints.push_back(_ptColors);
		}
	}
	else  {	
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		sDrawText			_strText;
		_strText.bControl	= true;
		if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
		_strText.DrawColor	= QColor(255, 0, 0);
		_strText.strText	= GetItemContent() + tr(" Didn't Contain Image!");
		param.VecDrawText.push_back(_strText);

		if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
		else			return NodeResult_NG;
	}
	return NodeResult_OK;
}

int PlaneFitTool::ExcutePlaneFit(HObject & hImg, HObject &region, int iCheckMode,QMap<QString, QString>& mapParam,double & dValue)
{
	switch (iCheckMode)	{
	case 0:	{	//2.5D
		HObject _ImageSurface, _ImgSub;
		HTuple Alpha, Beta, Gamma, hType;
		HTuple Width, Height;
		HalconCpp::GetImageType(hImg, &hType);
		HalconCpp::GetImageSize(hImg, &Width, &Height);

		//平面度矫正
		HalconCpp::FitSurfaceFirstOrder(region, hImg, HTuple(m_strAlgorithm.toStdString().c_str()), HTuple(m_iIterations), HTuple(m_iClippingFactor), &Alpha, &Beta, &Gamma);
		HalconCpp::GenImageSurfaceFirstOrder(&_ImageSurface, hType, Alpha, Beta, Gamma, 0, 0, Width, Height);
		HalconCpp::SubImage(hImg, _ImageSurface, &_ImgSub, 1, 0);
		HTuple Min, Max, Range;

		//求平面度的方法有以下几种
		//1.求灰度值直方图
		HalconCpp::MinMaxGray(region, _ImgSub, 10, &Min, &Max, &Range);
		dValue = Range.D();
	}	break;
	case 1:	{	//3D点云
		HObject _ImgXMap, _ImgYMap, _ImgReduce;
		HTuple hType, hv_ReduceObjectModel3D, hv_FitObjectModel3DOut1;
		HTuple Width, Height;
		HTuple  hv_paraName, hv_paraVal;
		HTuple	hv_pointsDis, hv_flatness;
		HalconCpp::GetImageType(hImg, &hType);
		HalconCpp::GetImageSize(hImg, &Width, &Height);
		float fScale = 0.05;

		//计算出平面校准
		HalconCpp::GenImageSurfaceFirstOrder(&_ImgXMap, hType, 0, fScale, 0, 0, 0, Width, Height);
		HalconCpp::GenImageSurfaceFirstOrder(&_ImgYMap, hType, fScale, 0, 0, 0, 0, Width, Height);

		ReduceDomain(hImg, region, &_ImgReduce);
		XyzToObjectModel3d(_ImgXMap, _ImgYMap, _ImgReduce, &hv_ReduceObjectModel3D);

		hv_paraName.Clear();
		hv_paraName[0] = "primitive_type";
		hv_paraName[1] = "fitting_algorithm";
		hv_paraVal.Clear();
		hv_paraVal[0] = "plane";
		hv_paraVal[1] = "least_squares_tukey";
		HalconCpp::FitPrimitivesObjectModel3d(hv_ReduceObjectModel3D, hv_paraName, hv_paraVal, &hv_FitObjectModel3DOut1);
		//计算方式有以下几种
		//1.计算ROI区域内点云距离拟合平面的距离
		HalconCpp::DistanceObjectModel3d(hv_ReduceObjectModel3D, hv_FitObjectModel3DOut1, HTuple(), 0, HTuple(), HTuple());	//计算点云每个点的距离比较耗时
		HalconCpp::GetObjectModel3dParams(hv_ReduceObjectModel3D, "&distance", &hv_pointsDis);
		//点云距离拟合平面的最大值和最小值之间的波动范围就是平面度
		hv_flatness = ((hv_pointsDis.TupleMax()) - (hv_pointsDis.TupleMin())).TupleAbs();

		HalconCpp::ClearObjectModel3d(hv_ReduceObjectModel3D);
		HalconCpp::ClearObjectModel3d(hv_FitObjectModel3DOut1);
		dValue = hv_flatness.D();
	}	break;
	default:	{

	}	break;
	}
	return 0;
}

QPair<QString, QString> PlaneFitTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> PlaneFitTool::GetToolName()
{
	return 	QPair<QString, QString>("PlaneNess", tr("PlaneNess"));
}
