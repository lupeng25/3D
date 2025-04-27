#include "GlueMeasureTool.h"
#include <qdebug.h>
#include <QThread>
#include <QElapsedTimer>
#include "frmGlueMeasureTool.h"
#include "databaseVar.h"

GlueMeasureTool::GlueMeasureTool() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	m_vecMeasurelines2D.clear();
}

GlueMeasureTool::~GlueMeasureTool()
{

}

int GlueMeasureTool::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_Roiobject = strData.find("ROIConFig").value().toArray();
	m_vecROIData	= array_Roiobject;

	UpdateParam();

	return 0;
}

int GlueMeasureTool::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	strData.insert("ROIConFig", m_vecROIData);

	return 0;
}

void GlueMeasureTool::UpdateParam()
{
	if (m_mapParam.contains("txtLinkFollow"))				m_strLinkName				= m_mapParam["txtLinkFollow"];
	if (m_mapParam.contains("checkUseFollow"))				m_bEnableCom				= m_mapParam["checkUseFollow"].toInt();

	if (m_mapParam.contains("spinMinValue"))				m_fLowLimit					= m_mapParam["spinMinValue"].toDouble();
	if (m_mapParam.contains("spinMaxValue"))				m_fHighLimit				= m_mapParam["spinMaxValue"].toDouble();

	if (m_mapShowParam.contains(QString(showregion)))		m_bEnableShowRegion			= m_mapShowParam[showregion].toInt();
	if (m_mapShowParam.contains(QString(showResult)))		m_bEnableShowResult			= m_mapShowParam[showResult].toInt();
	if (m_mapShowParam.contains(QString(showDrawResult)))	m_bEnableshowDrawResult		= m_mapShowParam[showDrawResult].toInt();
	if (m_mapShowParam.contains(QString(FontSize)))			m_iFontSize					= m_mapShowParam[FontSize].toInt();

	if (m_mapParam.contains("SliderLowPercent"))			m_dLowPercent				= m_mapParam["SliderLowPercent"].toDouble();
	if (m_mapParam.contains("SliderHighPercent"))			m_dHighPercent				= m_mapParam["SliderHighPercent"].toDouble();
	if (m_mapParam.contains("cbx_ALargrimCheckType"))		m_eEnumCheckType			= (EnumCheckType)m_mapParam["cbx_ALargrimCheckType"].toInt();

	//将坐标转换
	m_vecMeasurelines2D.clear();
	ChangeParamTovecMeasure(m_vecROIData,	m_vecMeasurelines2D );
}

//双击后弹出控件的QT窗口
void GlueMeasureTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmGlueMeasureTool * frm = new frmGlueMeasureTool(/*databaseVar::Instance().m_pWindow*/);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;
	UpdateParam();
	delete frm;
}

void GlueMeasureTool::StopExecute()
{
}

EnumNodeResult GlueMeasureTool::InitBeforeRun(MiddleParam& param, QString& strError)
{
	m_iInvalidValue = -9000;
	if (databaseVar::Instance().m_mapParam.contains("spinInvalidateValue")){
		m_iInvalidValue = databaseVar::Instance().m_mapParam["spinInvalidateValue"].toDouble();
	}
	return NodeResult_OK;
}

EnumNodeResult GlueMeasureTool::PrExecute(MiddleParam & param, QString & strError)
{
	return NodeResult_OK;
}

EnumNodeResult GlueMeasureTool::Execute(MiddleParam& param, QString& strError)
{
	if (param.MapPImgVoid.count(m_strInImg) > 0)	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strInImg));
		if (!hImg.IsInitialized())	{
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		QString _strResultName	= GetItemId() + ".Out.";
		int _iCheckMode			= databaseVar::Instance().m_iCheckMode;
		QVector<QVector<MeasureRect2>> _vecResultLines = m_vecMeasurelines2D;

		QVector<QPointF>	_VecPt1D;
		QVector<sDrawText>	_VecText1D;
		QVector<sDrawText>	_Vec1Text1D;
		QVector<sDrawText>	_Vec2Text1D;
		QVector<sDrawCross> _PointLst;

		if (m_bEnableCom) {
			HTuple _Metrix;
			if (CheckMetrixType(m_strLinkName, _Metrix) == 0) {
				_vecResultLines.clear();
				for (auto& _Lines : m_vecMeasurelines2D) {
					QVector<MeasureRect2> _lines;
					for (auto& _oneLine : _Lines) {
						MeasureRect2 _line = _oneLine;
						HTuple	_Rows, _Columns;
						AffineTransPixel(_Metrix, _oneLine.line.p1().y(), _oneLine.line.p1().x(), &_Rows, &_Columns);
						_line.line.setP1(QPointF(_Columns.D(), _Rows.D()));
						AffineTransPixel(_Metrix, _oneLine.line.p2().y(), _oneLine.line.p2().x(), &_Rows, &_Columns);
						_line.line.setP2(QPointF(_Columns.D(), _Rows.D()));
						_lines.push_back(_line);
					}
					_vecResultLines.push_back(_lines);
				}
			} 
			else {
				SetError(QString("%1 Find Metrix Isn't IsInitialized").arg(m_strInImg));
				sDrawText			_strText;
				_strText.bControl	= true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor	= QColor(255, 0, 0);
				_strText.strText	= GetItemContent() + tr(" The Link Matrix Failed!");
				param.VecDrawText.push_back(_strText);

				if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
				else			return NodeResult_NG;
			}
		}
		switch (_iCheckMode)	{
		case 0: {	//2D检测
			HTuple  hv_Width, hv_Height, hv_Min, hv_Max, _Range;
			GetImageSize(hImg, &hv_Width, &hv_Height);
			HTuple	hv_FirstRow, hv_FirstCol, hv_SecondRow, hv_SecondCol;
			HObject ho_FirstContour, ho_SecondContour, ho_ContourOut, ho_Rectangle;
			HTuple  hv_Distance, hv_Gray, m_hGrayval;
			HTuple  hv_DistanceMin, hv_DistanceMax;
			HTuple	hv_ResultRow, hv_ResultCol, hv_ResultZ;

			hv_FirstRow		= HTuple();
			hv_FirstCol		= HTuple();
			hv_SecondRow	= HTuple();
			hv_SecondCol	= HTuple();

			hv_ResultRow	= HTuple();
			hv_ResultCol	= HTuple();
			hv_ResultZ		= HTuple();
			for (auto& _Lines : _vecResultLines) {
				for (int i = 0; i < _Lines.size(); i++) {
					HTuple RowEdgeFirst, ColumnEdgeFirst, AmplitudeFirst, RowEdgeSecond, ColumnEdgeSecond, AmplitudeSecond, IntraDistance, InterDistance;
					{
						HTuple Row, Column, Phi, Length1, Length2, hMeasureHandle, Distance, hAngle;
						DistancePp(_Lines[i].line.p1().y(), _Lines[i].line.p1().x(), _Lines[i].line.p2().y(), _Lines[i].line.p2().x(), &Distance);
						AngleLx(_Lines[i].line.p2().y(), _Lines[i].line.p2().x(), _Lines[i].line.p1().y(), _Lines[i].line.p1().x(), &hAngle);
						HalconCpp::GenMeasureRectangle2(_Lines[i].line.center().y(), _Lines[i].line.center().x(), hAngle, Distance / 2.0,
							_Lines[i].dNormalLineWidth / 2.0, hv_Width, hv_Height, "nearest_neighbor", &hMeasureHandle);
						//显示
						HTuple hv_AmplitudeFirst, hv_AmplitudeSecond, hv_IntraDistance, hv_InterDistance;
						if (_Lines[i].bNeedCheckDistance)	{
							MeasurePairs(hImg, hMeasureHandle, _Lines[i].dSigma, _Lines[i].dThreshold,
								_Lines[i].strTransition.toLower().toStdString().c_str(), _Lines[i].strSelect.toLower().toStdString().c_str(),
								&RowEdgeFirst, &ColumnEdgeFirst, &hv_AmplitudeFirst, &RowEdgeSecond, &ColumnEdgeSecond,
								&hv_AmplitudeSecond, &hv_IntraDistance, &hv_InterDistance);
						}
						//测量出灰度
						MeasureProjection(hImg, hMeasureHandle,&m_hGrayval);
						CloseMeasure(hMeasureHandle);
					}
					QPointF ptMeasureCenter;
					HTuple hv_line1Row = _Lines[i].line.p1().y();
					HTuple hv_line1Col = _Lines[i].line.p1().x();
					HTuple hv_line2Row = _Lines[i].line.p2().y();
					HTuple hv_line2Col = _Lines[i].line.p2().x();
					HTuple hv_lineCenterRow = (hv_line1Row + hv_line2Row) / 2.0;
					HTuple hv_lineCenterCol = (hv_line1Col + hv_line2Col) / 2.0;
					ptMeasureCenter.setX(hv_lineCenterCol.D());
					ptMeasureCenter.setY(hv_lineCenterRow.D());

					if (_Lines[i].bNeedCheckDistance) {
						HTuple hFirstNumber, hSecondNumber, hGray;
						TupleLength(RowEdgeFirst, &hFirstNumber);
						TupleLength(RowEdgeSecond, &hSecondNumber);
						if (hFirstNumber.I() > 0 && hSecondNumber.I() > 0) {
							hv_FirstRow		= hv_FirstRow.TupleConcat(RowEdgeFirst[0]);
							hv_FirstCol		= hv_FirstCol.TupleConcat(ColumnEdgeFirst[0]);
							hv_SecondRow	= hv_SecondRow.TupleConcat(RowEdgeSecond[0]);
							hv_SecondCol	= hv_SecondCol.TupleConcat(ColumnEdgeSecond[0]);
							if (m_bEnableShowResult) {
								{
									sDrawCross _Cross;
									_Cross.Center.setX(ColumnEdgeFirst[0].D());
									_Cross.Center.setY(RowEdgeFirst[0].D());
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}	{
									sDrawCross _Cross;
									_Cross.Center.setX(ColumnEdgeSecond[0].D());
									_Cross.Center.setY(RowEdgeSecond[0].D());
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}
						}
						else	{
							if (m_bEnableShowResult) {
								if(hFirstNumber.I() > 0)	{
									sDrawCross _Cross;
									_Cross.Center.setX(ColumnEdgeFirst[0].D());
									_Cross.Center.setY(RowEdgeFirst[0].D());
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
								if (hSecondNumber.I() > 0) {
									sDrawCross _Cross;
									_Cross.Center.setX(ColumnEdgeSecond[0].D());
									_Cross.Center.setY(RowEdgeSecond[0].D());
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}
						}

						if (hFirstNumber.I() > 0 && hSecondNumber.I() > 0) {
							double hv_line1Row			= RowEdgeFirst[0].D();
							double hv_line1Col			= ColumnEdgeFirst[0].D();
							double hv_line2Row			= RowEdgeSecond[0].D();
							double hv_line2Col			= ColumnEdgeSecond[0].D();
							double hv_lineCenterRow		= (hv_line1Row + hv_line2Row) / 2.0;
							double hv_lineCenterCol		= (hv_line1Col + hv_line2Col) / 2.0;
							int		iLenth = m_hGrayval.TupleLength().I();
							double	_dRow = (_Lines[i].line.p1().y() - _Lines[i].line.p2().y()) / iLenth;
							double	_dCol = (_Lines[i].line.p1().x() - _Lines[i].line.p2().x()) / iLenth;

							int		_iFirstIndex	= ((RowEdgeFirst[0].D()		- _Lines[i].line.p2().y()) / _dRow);
							int		_iEndIndex		= ((RowEdgeSecond[0].D()	- _Lines[i].line.p2().y()) / _dRow);

							QPointF	ptMeasureCenter;
							ptMeasureCenter.setX((ColumnEdgeSecond[0].D() + ColumnEdgeFirst[0].D()) / 2.0);
							ptMeasureCenter.setY((RowEdgeSecond[0].D() + RowEdgeFirst[0].D()) / 2.0);

							TupleSelectRange(m_hGrayval, abs(_iFirstIndex), abs(_iEndIndex), &hv_Gray);

							HTuple hv_Sorted, hv_SortedIndex;
							TupleSort(hv_Gray, &hv_Sorted);
							TupleSortIndex(hv_Gray, &hv_SortedIndex);
							int _iLenth = hv_Gray.TupleLength().I();
							double	_dZValue = 0;
							int		_iIndex = 0;
							double	_dPosY = 0;

							switch (m_eEnumCheckType) {
							case 0: {	//百分比
								int	_iCurrentIndex = -1;
								double	_dValue = 0;
								int	_iLowerIndex = m_dLowPercent / 100.0 * iLenth;
								int	_iUpperIndex = m_dHighPercent / 100.0 * iLenth;
								int	_iUseIndex = 0;
								int _iTotalIndex = 0;
								for (int K = _iLowerIndex; K < _iUpperIndex; K++) {
									int _iTempIndex = hv_SortedIndex[K].I();
									_dValue = _dValue + hv_Sorted[K].D();
									_iUseIndex++;
									_iTotalIndex = _iTotalIndex + _iTempIndex;
								}
								_dZValue = _dValue / _iUseIndex;
								_iIndex = _iTotalIndex / _iUseIndex;
								_dPosY = hv_Gray[_iIndex];

								hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 1: {	//最大值				
								int	_iCurrentIndex = hv_SortedIndex[iLenth - 1];
								_dZValue	= hv_Sorted[iLenth - 1].D();
								_iIndex		= _iCurrentIndex;
								_dPosY		= _dZValue;
								hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
								hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 2: {	//最小值
								int	_iCurrentIndex = hv_SortedIndex[0];
								_dZValue = hv_Sorted[0].D();
								//需要过滤无效值
								for (int i = 0; i < hv_Sorted.Length(); i++) {
									if (hv_Sorted[i] > m_iInvalidValue) {
										_iCurrentIndex = hv_SortedIndex[i];
										_dZValue = hv_Sorted[i].D();
									}
								}
								_iIndex = _iCurrentIndex;
								_dPosY = _dZValue;

								hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
								hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 3: {	//中值
								int	_iCurrentIndex = hv_SortedIndex[iLenth / 2.0];

								_dZValue		= hv_Sorted[iLenth / 2.0].D();
								_iIndex			= _iCurrentIndex;
								_dPosY			= hv_Gray[abs(_iIndex)].D();

								hv_ResultRow	= hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
								hv_ResultCol	= hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
								hv_ResultZ		= hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							case 4: {	//均值
								int	_iCurrentIndex = -1;
								HTuple	hv_Median;
								TupleMedian(hv_Gray, &hv_Median);
								for (int i = 0; i < iLenth; i++) {
									if (hv_SortedIndex[i] > hv_Median.D()) {
										_iCurrentIndex = i;	break;
									}
								}
								_dZValue = hv_Median.D();
								if (_iCurrentIndex > -1) {
									hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
									hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
									hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
									if (m_bEnableShowResult) {
										sDrawCross _Cross;
										QPointF ptQPoint = _Lines[i].line.p2();
										_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
										_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
										_Cross.fAngle = 0;
										_Cross.iDrawLenth = m_iFontSize;
										_Cross.DrawColor = QColor(0, 255, 0);
										_PointLst.push_back(_Cross);
									}
								}
							}	break;
							case 5: {	//中线
								_dZValue	= hv_Gray[iLenth / 2.0].D();
								_iIndex		= iLenth / 2.0;
								_dPosY		= hv_Gray[abs(_iIndex)].D();

								hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
								hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}	break;
							default: {		}	break;
							}
						}
					}
					else	{
						hv_Gray = m_hGrayval;
						HTuple	hv_Sorted, hv_SortedIndex;
						TupleSort(hv_Gray, &hv_Sorted);
						TupleSortIndex(hv_Gray, &hv_SortedIndex);
						int iLenth			= hv_Gray.TupleLength().I();
						double	_dRow		= (_Lines[i].line.p1().y() - _Lines[i].line.p2().y()) / iLenth;
						double	_dCol		= (_Lines[i].line.p1().x() - _Lines[i].line.p2().x()) / iLenth;
						double	_dZValue	= 0;
						int		_iIndex		= 0;
						double	_dPosY		= 0;

						switch (m_eEnumCheckType) {
						case 0: {	//百分比
							int	_iCurrentIndex = -1;
							double	_dValue = 0;
							int	_iLowerIndex = m_dLowPercent / 100.0 * iLenth;
							int	_iUpperIndex = m_dHighPercent / 100.0 * iLenth;
							int	_iUseIndex = 0;
							int _iTotalIndex = 0;
							for (int K = _iLowerIndex; K < _iUpperIndex; K++) {
								int _iTempIndex = hv_SortedIndex[K].I();
								_dValue = _dValue + hv_Sorted[K].D();
								_iUseIndex++;
								_iTotalIndex = _iTotalIndex + _iTempIndex;
							}
							_dZValue = _dValue / _iUseIndex;
							_iIndex = _iTotalIndex / _iUseIndex;
							_dPosY = hv_Gray[_iIndex];

							hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + (_iIndex)	*	_dCol);
							hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + (_iIndex)	*	_dRow);
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
							if (m_bEnableShowResult) {
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								_Cross.fAngle = 0;
								_Cross.iDrawLenth = m_iFontSize;
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						case 1: {	//最大值				
							int	_iCurrentIndex = hv_SortedIndex[iLenth - 1];
							_dZValue		= hv_Sorted[iLenth - 1].D();
							_iIndex			= _iCurrentIndex;
							_dPosY			= _dZValue;
							hv_ResultRow	= hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
							hv_ResultCol	= hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
							hv_ResultZ		= hv_ResultZ.TupleConcat(_dZValue);
							if (m_bEnableShowResult) {
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								_Cross.fAngle = 0;
								_Cross.iDrawLenth = m_iFontSize;
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}

						}	break;
						case 2: {	//最小值
							int	_iCurrentIndex = hv_SortedIndex[0];
							_dZValue = hv_Sorted[0].D();
							//需要过滤无效值
							for (int i = 0; i < hv_Sorted.Length(); i++)	{
								if (hv_Sorted[i] >= m_iInvalidValue)			{
									_iCurrentIndex = hv_SortedIndex[i];
									_dZValue = hv_Sorted[i].D();
									break;
								}
							}

							_iIndex = _iCurrentIndex;
							_dPosY = _dZValue;

							hv_ResultRow	= hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
							hv_ResultCol	= hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
							hv_ResultZ		= hv_ResultZ.TupleConcat(_dZValue);
							if (m_bEnableShowResult) {
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								_Cross.fAngle = 0;
								_Cross.iDrawLenth = m_iFontSize;
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						case 3: {	//中值
							int	_iCurrentIndex = hv_SortedIndex[iLenth / 2.0];

							_dZValue = hv_Sorted[iLenth / 2.0].D();
							_iIndex = _iCurrentIndex;
							_dPosY = hv_Gray[abs(_iIndex)].D();

							hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
							hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
							if (m_bEnableShowResult) {
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								_Cross.fAngle = 0;
								_Cross.iDrawLenth = m_iFontSize;
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						case 4: {	//均值
							int	_iCurrentIndex = -1;
							HTuple	hv_Median;
							TupleMedian(hv_Gray, &hv_Median);
							for (int i = 0; i < iLenth; i++) {
								if (hv_SortedIndex[i] > hv_Median.D()) {
									_iCurrentIndex = i;	break;
								}
							}
							_dZValue = hv_Median.D();
							if (_iCurrentIndex > -1) {

								hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
								hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
								hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
								if (m_bEnableShowResult) {
									sDrawCross _Cross;
									QPointF ptQPoint = _Lines[i].line.p2();
									_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
									_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
									_Cross.fAngle = 0;
									_Cross.iDrawLenth = m_iFontSize;
									_Cross.DrawColor = QColor(0, 255, 0);
									_PointLst.push_back(_Cross);
								}
							}
						}	break;
						case 5: {	//中线
							_dZValue = hv_Gray[iLenth / 2.0].D();
							_iIndex = iLenth / 2.0;
							_dPosY = hv_Gray[abs(_iIndex)].D();

							hv_ResultRow = hv_ResultRow.TupleConcat(_Lines[i].line.p2().x() + _iIndex	*	_dCol);
							hv_ResultCol = hv_ResultCol.TupleConcat(_Lines[i].line.p2().y() + _iIndex	*	_dRow);
							hv_ResultZ = hv_ResultZ.TupleConcat(_dZValue);
							if (m_bEnableShowResult) {
								sDrawCross _Cross;
								QPointF ptQPoint = _Lines[i].line.p2();
								_Cross.Center.setX(_Lines[i].line.p2().x() + (_iIndex)*	_dCol);
								_Cross.Center.setY(_Lines[i].line.p2().y() + (_iIndex)*	_dRow);
								_Cross.fAngle = 0;
								_Cross.iDrawLenth = m_iFontSize;
								_Cross.DrawColor = QColor(0, 255, 0);
								_PointLst.push_back(_Cross);
							}
						}	break;
						default: {		}	break;
						}
					}
				}
			}
		}	break;
		case 1: {	//2D检测

		}	break;
		default:
			break;
		}
		for (auto iter : _VecPt1D)		param.VecShowQPointFs.push_back(iter);
		for (auto iter : _VecText1D)	param.VecDrawText.push_back(iter);
		for (int i = 0; i < _PointLst.size(); i++ ) {
			param.MapDrawCross.insert(std::pair<QString, sDrawCross>(_strResultName + QString::number(i), _PointLst[i]	));
		}
		if (m_bEnableShowRegion)	{
			for (int i = 0; i < _vecResultLines.size(); i++) {
				for (int j = 0; j < _vecResultLines[i].size(); j++) {
					sColorLineRect _LineRect;
					_LineRect.Dline			= _vecResultLines[i][j].line;
					_LineRect.DWidth		= _vecResultLines[i][j].dNormalLineWidth;
					_LineRect.DrawQColor	= QColor(0, 0, 255);;
					_LineRect.LineWidth		= -1;
					param.VecShowColorLineRect.push_back(_LineRect);
				}
			}
		}
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

int GlueMeasureTool::ExcutePlaneFit(HObject & hImg, HObject &region, int iCheckMode,QMap<QString, QString>& mapParam,double & dValue)
{
	switch (iCheckMode)	{
	case 0:	{	//2.5D
		HObject _ImageSurface, _ImgSub;
		HTuple Alpha, Beta, Gamma, hType;
		HTuple Width, Height;
		HalconCpp::GetImageType(hImg, &hType);
		HalconCpp::GetImageSize(hImg, &Width, &Height);

		////平面度矫正
		//HalconCpp::FitSurfaceFirstOrder(region, hImg, HTuple(m_strAlgorithm.toStdString().c_str()), HTuple(m_iIterations), HTuple(m_iClippingFactor), &Alpha, &Beta, &Gamma);
		//HalconCpp::GenImageSurfaceFirstOrder(&_ImageSurface, hType, Alpha, Beta, Gamma, 0, 0, Width, Height);
		//HalconCpp::SubImage(hImg, _ImageSurface, &_ImgSub, 1, 0);
		//HTuple Min, Max, Range;

		//求平面度的方法有以下几种
		//1.求灰度值直方图
		//HalconCpp::MinMaxGray(region, _ImgSub, 10, &Min, &Max, &Range);
		//dValue = Range.D();
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

QPair<QString, QString> GlueMeasureTool::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> GlueMeasureTool::GetToolName()
{
	return 	QPair<QString, QString>("GlueMeasure", tr("GlueMeasure"));
}
