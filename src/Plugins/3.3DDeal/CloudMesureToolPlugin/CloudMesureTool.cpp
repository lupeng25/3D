#include "CloudMesureTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmCloudMesureTool.h"
#include "databaseVar.h"

CloudMesureTool::CloudMesureTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	//m_vecCommonData2D.clear();
}

CloudMesureTool::~CloudMesureTool()
{

}

int CloudMesureTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_MesureParam = strData.find("vecMesureParam").value().toArray();
	m_vecMesureRegion.clear();
	for (size_t i = 0; i < array_MesureParam.count(); i++) {
		QJsonObject array_value		= array_MesureParam.at(i).toObject();
		tagCloudMesure _Mesure;
		{
			QJsonArray _JLinkValue			= array_value.find("LinkValue").value().toArray();
			_Mesure.bEnableLink				= _JLinkValue.at(0).toString().toInt();
			_Mesure.eAlargrim				= (EnumALargrim)_JLinkValue.at(1).toString().toInt();
			_Mesure.dHighValue				= _JLinkValue.at(2).toString().toDouble();
			_Mesure.dLowValue				= _JLinkValue.at(3).toString().toDouble();
			_Mesure.strLinkName				= _JLinkValue.at(4).toString();
		}	{
			_Mesure.vecRoi.clear();
			QJsonArray _JLinkValue			= array_value.find("vecROI").value().toArray();
			for (size_t k = 0;	 k	<		_JLinkValue.count(); k++)	{
				QJsonArray _DataValue		= _JLinkValue.at(k).toArray();
				CommonData _CommonData;
				_CommonData.type			= (ShapeItemType)_DataValue.at(0).toString().toInt();
				_CommonData.eRegionType		= (EnumItemRegionType)_DataValue.at(1).toString().toInt();
				_CommonData.strData			= _DataValue.at(2).toString();
				_Mesure.vecRoi.push_back(_CommonData);
			}
		}
		m_vecMesureRegion.push_back(_Mesure);
	}

	if (m_vecMesureRegion.size() <= 1)	m_vecMesureRegion.resize(2);


	UpdateParam();

	return 0;
}

int CloudMesureTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);
	QJsonArray array_MesureRegion;
	for (int i = 0; i < m_vecMesureRegion.size(); i++){
		QJsonObject _CommonParam;
		QJsonArray _MesureRegion = {
			QString::number(m_vecMesureRegion[i].bEnableLink),
			QString::number(m_vecMesureRegion[i].eAlargrim),
			QString::number(m_vecMesureRegion[i].dHighValue),
			QString::number(m_vecMesureRegion[i].dLowValue),
			QString(m_vecMesureRegion[i].strLinkName),
		};
		_CommonParam.insert("LinkValue", _MesureRegion);
		QJsonArray _MesureROI;
		for (int K = 0; K < m_vecMesureRegion[i].vecRoi.size(); K++){
			QJsonArray _Object = {
				QString::number(m_vecMesureRegion[i].vecRoi[K].type),
				QString::number(m_vecMesureRegion[i].vecRoi[K].eRegionType),
				m_vecMesureRegion[i].vecRoi[K].strData,
			};
			_MesureROI.append(_Object);
		}
		_CommonParam.insert("vecROI", _MesureROI);
		array_MesureRegion.append(_CommonParam);
	}
	strData.insert("vecMesureParam", array_MesureRegion);

	return 0;
}

void CloudMesureTool::UpdateParam()
{
	if (m_mapParam.contains("spinMinValue"))	m_fLowLimit = m_mapParam["spinMinValue"].toDouble();
	if (m_mapParam.contains("spinMaxValue"))	m_fHighLimit = m_mapParam["spinMaxValue"].toDouble();

	if (m_mapShowParam.contains(QString("showregion"))) m_bEnableShowRegion = m_mapShowParam["showregion"].toInt();

}

//双击后弹出控件的QT窗口
void CloudMesureTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmCloudMesureTool * frm = new frmCloudMesureTool(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;

	UpdateParam();

	delete frm;
}

void CloudMesureTool::StopExecute()
{
}

EnumNodeResult CloudMesureTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult CloudMesureTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult CloudMesureTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)
	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())	{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		std::vector<sDrawText>						_vecDrawText;
		QString _strResultName						= GetItemId() + ".Out.";
		int _iCheckMode								= databaseVar::Instance().m_iCheckMode;
		double _dFlatNess							= 0;
		HObject _MetrixRegion						= m_ROIModelRegion;
		for (int i = 0; i < m_vecMesureRegion.size(); i++){
			auto& _model							= m_vecMesureRegion[i];
			HObject _ModelROIRegion;
			CloudMesureTool::ChangeParamToRegion(m_vecMesureRegion[i].vecRoi, _ModelROIRegion);
			HObject _MetrixRegion					= _ModelROIRegion;
			//链接
			if (_model.bEnableLink) {
				HTuple _Metrix;
				HTuple	Rows, Columns, _Lenth;
				if (CloudMesureTool::CheckMetrixType(_model.strLinkName, _Metrix) == 0) {
					AffineTransRegion(_ModelROIRegion, &_MetrixRegion, _Metrix, "nearest_neighbor");

					if (m_bEnableShowRegion)
						frmCloudMesureTool::AffineShapeTrans(m_vecMesureRegion[i].vecRoi, _Metrix, param, true);
				}
				else {	//跟随
					sDrawText			_strText;
					_strText.bControl = true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor = QColor(255, 0, 0);
					_strText.strText = GetItemContent() + tr(" Link Metrix Error!");
					param.VecDrawText.push_back(_strText);

					if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
					else			return NodeResult_NG;
				}
			}
			else {
				if (m_bEnableShowRegion) {
					frmCloudMesureTool::AffineShapeTrans(m_vecMesureRegion[i].vecRoi, HTuple(), param);
				}
			}
			try {
				//算法
				switch (_model.eAlargrim) {
				case ALargrim_Medium: {
					HTuple Mean, Deviation;
					HalconCpp::Intensity(_MetrixRegion, hImg, &Mean, &Deviation);
					_model.dMeasureValue		= Mean.D();
					sDrawText					_strText;
					_strText.bControl			= true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor			= QColor(0, 255, 0);
					_strText.strText			= QString("  %1.%2:%3").arg(QString::number(i)).arg(tr("Medium Value")).arg(QString::number(Mean.D(), 'f', databaseVar::Instance().form_System_Precision));
					_vecDrawText.push_back(_strText);
				} break;
				case ALargrim_Max: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, hImg, 0, &Min, &Max, &Range);
					_model.dMeasureValue		= Max.D();
					sDrawText					_strText;
					_strText.bControl			= true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor			= QColor(0, 255, 0);
					_strText.strText			= QString("  %1.%2:%3").arg(QString::number(i)).arg(tr("Max Value")).arg(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_vecDrawText.push_back(_strText);

				} break;
				case ALargrim_Mean: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, hImg, 50, &Min, &Max, &Range);
					_model.dMeasureValue		= Max.D();
					sDrawText					_strText;
					_strText.bControl			= true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor			= QColor(0, 255, 0);
					_strText.strText			= QString("  %1.%2:%3").arg(QString::number(i)).arg(tr("Mean Value")).arg(QString::number(Max.D(), 'f', databaseVar::Instance().form_System_Precision));
					_vecDrawText.push_back(_strText);

				} break;
				case ALargrim_Min: {
					HTuple Percent, Min, Max, Range;
					HalconCpp::MinMaxGray(_MetrixRegion, hImg, 0, &Min, &Max, &Range);
					_model.dMeasureValue		= Min.D();
					sDrawText					_strText;
					_strText.bControl			= true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor			= QColor(0, 255, 0);
					_strText.strText			= QString("  %1.%2:%3").arg(QString::number(i)).arg(tr("Min Value")).arg(QString::number(Min.D(), 'f', databaseVar::Instance().form_System_Precision));
					_vecDrawText.push_back(_strText);

				} break;
				case ALargrim_GrayHisto: {
					HTuple Percent, Min, Max, Range, AbsoluteHisto, RelativeHisto, Area, Row, Column;
					HalconCpp::GrayHisto(_MetrixRegion, hImg, &AbsoluteHisto, &RelativeHisto);
					HalconCpp::MinMaxGray(_MetrixRegion, hImg, 0, &Min, &Max, &Range);
					HalconCpp::AreaCenter(_MetrixRegion, &Area, &Row, &Column);

					double	_dMinValue			= Min.D();
					double	_dMaxValue			= Max.D();
					int		_iAllNumber			= Area.L();
					long	_lLowNumber			= Area.L() * m_vecMesureRegion[i].dLowValue;
					long	_HighNumber			= Area.L() * m_vecMesureRegion[i].dHighValue;
					int		_iLenth				= RelativeHisto.Length();
					long	_lCurrentValue		= 0;
					long	_lFirstTrrigerLTime = 0;	//触发次数
					long	_lFirstTrrigerHTime = 0;	//触发次数
					double	_dCurrentAllValue	= 0;
					double	_dCurrentValue		= 0;
					_lCurrentValue = Area.L();
					for (int i = _iLenth - 1; i >= 0; i--) {
						int		_iNumber = AbsoluteHisto[i].I();
						double	_dHistoValue = _dMaxValue - abs(_dMaxValue - _dMinValue) / (_iLenth - 1);
						_lCurrentValue -= _iNumber;
						if (_lFirstTrrigerLTime > 0 && _lFirstTrrigerHTime <= 0) {
							if (_lCurrentValue > _lLowNumber) {
								_dCurrentAllValue += _iNumber * _dHistoValue;
							}
						}
						if (_lFirstTrrigerLTime <= 0) {
							if (_lCurrentValue <= _HighNumber) {
								_dCurrentAllValue += abs(_lCurrentValue - _HighNumber) * _dHistoValue;
								_lFirstTrrigerLTime++;
							}
						}
						if (_lFirstTrrigerHTime <= 0) {
							if (_lCurrentValue <= _lLowNumber) {
								_dCurrentAllValue += abs(_lCurrentValue + _iNumber - _lLowNumber) * _dHistoValue;
								_lFirstTrrigerHTime++;
							}
						}
					}
					_dCurrentValue = _dCurrentAllValue / (_iAllNumber		*	(m_vecMesureRegion[i].dHighValue - m_vecMesureRegion[i].dLowValue));
					_model.dMeasureValue = _dCurrentValue;

					sDrawText			_strText;
					_strText.bControl	= true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor	= QColor(0, 255, 0);
					_strText.strText	= QString("  %1.%2:%3").arg(QString::number(i)).arg(tr("Histogram")).arg(QString::number(_dCurrentValue, 'f', databaseVar::Instance().form_System_Precision));
					_vecDrawText.push_back(_strText);
				} break;
				default: {
				}break;
				}
			}
			catch (const HException& except) {
				QString strError = except.ErrorText();
				QStringList strErrorArray = strError.split(':');
				if (strErrorArray.size() > 1) {
					if (!strErrorArray[1].isEmpty()) {
						strError.clear();
						for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
						sDrawText			_strText;
						_strText.bControl	= true;
						_strText.DrawColor	= QColor(255, 0, 0);
						if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
						_strText.strText	= GetItemContent() + strError;
						_vecDrawText.push_back(_strText);
					}
				}
			}
			catch (...) {
				sDrawText			_strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = GetItemContent() + QObject::tr("Unknow Error");
				_vecDrawText.push_back(_strText);
			}
		}
		double _dMeasureValue = 0;
		if (m_vecMesureRegion.size() > 1) {
			_dMeasureValue = m_vecMesureRegion[1].dMeasureValue - m_vecMesureRegion[0].dMeasureValue;
		}
		if (_dMeasureValue		>=	m_fLowLimit
			&& _dMeasureValue	<=	m_fHighLimit) {
			sDrawText			_strText;
			_strText.bControl	= true;
			if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
			_strText.DrawColor	= QColor(0, 255, 0);
			_strText.strText	= GetItemContent() + QString(" %1:%3").arg(tr("Check OK")).arg(QString::number(_dMeasureValue, 'f', databaseVar::Instance().form_System_Precision));
			_vecDrawText.insert(_vecDrawText.begin(), _strText);
		}
		else
		{
			sDrawText			_strText;
			_strText.bControl	= true;
			if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
			_strText.DrawColor	= QColor(255, 0, 0);
			_strText.strText	= GetItemContent() + QString(" %1:%3").arg(tr("Check Fail")).arg(QString::number(_dMeasureValue, 'f', databaseVar::Instance().form_System_Precision));
			_vecDrawText.insert(_vecDrawText.begin(), _strText);
		}
		for(auto iter : _vecDrawText)	param.VecDrawText.push_back(iter);
	}
	else {
		SetError(QString("Image %1 is Empty").arg(m_strInImg));
		sDrawText			_strText;
		_strText.bControl = true;
		if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
		_strText.DrawColor = QColor(255, 0, 0);
		_strText.strText = GetItemContent() + tr(" Didn't Contain Image!");
		param.VecDrawText.push_back(_strText);

		if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
		else			return NodeResult_NG;
	}
	return NodeResult_OK;
}

int CloudMesureTool::ExcutePlaneFit(HObject & hImg, HObject &region, int iCheckMode,QMap<QString, QString>& mapParam,double & dValue)
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

QPair<QString, QString> CloudMesureTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> CloudMesureTool::GetToolName()
{
	return 	QPair<QString, QString>("CloudMesure", tr("CloudMesure"));
}
