#include "MeasureLine.h"
#include <qdebug.h>
#include "frmMeasureLine.h"
#include "databaseVar.h"

MeasureLine::MeasureLine() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));

	ClearModel();
}

MeasureLine::MeasureLine(QPointF pos) : FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));

	ClearModel();
}

MeasureLine::~MeasureLine()
{
	ClearModel();
	Clearport();
}

void MeasureLine::pts_to_best_line(HObject *ho_Line, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
	HTuple *hv_Row1, HTuple *hv_Column1, HTuple *hv_Row2, HTuple *hv_Column2)
{

	// Local iconic variables
	HObject  ho_Contour;

	// Local control variables
	HTuple  hv_Length, hv_Nr, hv_Nc, hv_Dist, hv_Length1;

	//初始化
	(*hv_Row1) = 0;
	(*hv_Column1) = 0;
	(*hv_Row2) = 0;
	(*hv_Column2) = 0;
	//产生一个空的直线对象，用于保存拟合后的直线
	GenEmptyObj(&(*ho_Line));
	//计算边缘数量
	TupleLength(hv_Cols, &hv_Length);
	//当边缘数量不小于有效点数时进行拟合
	if (0 != (HTuple(int(hv_Length >= hv_ActiveNum)).TupleAnd(int(hv_ActiveNum > 1))))
	{
		//halcon的拟合是基于xld的，需要把边缘连接成xld
		GenContourPolygonXld(&ho_Contour, hv_Rows, hv_Cols);
		//拟合直线。使用的算法是'tukey'，其他算法请参考fit_line_contour_xld的描述部分。
		FitLineContourXld(ho_Contour, "tukey", -1, 0, 5, 2, &(*hv_Row1), &(*hv_Column1),
			&(*hv_Row2), &(*hv_Column2), &hv_Nr, &hv_Nc, &hv_Dist);
		//判断拟合结果是否有效：如果拟合成功，数组中元素的数量大于0
		TupleLength(hv_Dist, &hv_Length1);
		if (0 != (int(hv_Length1 < 1)))
		{
			return;
		}
		//根据拟合结果，产生直线xld
		GenContourPolygonXld(&(*ho_Line), (*hv_Row1).TupleConcat((*hv_Row2)), (*hv_Column1).TupleConcat((*hv_Column2)));
	}

	return;
}

void MeasureLine::gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
	HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth)
{

	// Local iconic variables
	HObject  ho_TempArrow;

	// Local control variables
	HTuple  hv_Length, hv_ZeroLengthIndices, hv_DR;
	HTuple  hv_DC, hv_HalfHeadWidth, hv_RowP1, hv_ColP1, hv_RowP2;
	HTuple  hv_ColP2, hv_Index;

	//This procedure generates arrow shaped XLD contours,
	//pointing from (Row1, Column1) to (Row2, Column2).
	//If starting and end point are identical, a contour consisting
	//of a single point is returned.
	//
	//input parameters:
	//Row1, Column1: Coordinates of the arrows' starting points
	//Row2, Column2: Coordinates of the arrows' end points
	//HeadLength, HeadWidth: Size of the arrow heads in pixels
	//
	//output parameter:
	//Arrow: The resulting XLD contour
	//
	//The input tuples Row1, Column1, Row2, and Column2 have to be of
	//the same length.
	//HeadLength and HeadWidth either have to be of the same length as
	//Row1, Column1, Row2, and Column2 or have to be a single element.
	//If one of the above restrictions is violated, an error will occur.
	//
	//
	//Initialization.
	GenEmptyObj(&(*ho_Arrow));
	//
	//Calculate the arrow length
	DistancePp(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_Length);
	//
	//Mark arrows with identical start and end point
	//(set Length to -1 to avoid division-by-zero exception)
	hv_ZeroLengthIndices = hv_Length.TupleFind(0);
	if (0 != (int(hv_ZeroLengthIndices != -1)))
	{
		hv_Length[hv_ZeroLengthIndices] = -1;
	}
	//
	//Calculate auxiliary variables.
	hv_DR = (1.0*(hv_Row2 - hv_Row1)) / hv_Length;
	hv_DC = (1.0*(hv_Column2 - hv_Column1)) / hv_Length;
	hv_HalfHeadWidth = hv_HeadWidth / 2.0;
	//
	//Calculate end points of the arrow head.
	hv_RowP1 = (hv_Row1 + ((hv_Length - hv_HeadLength)*hv_DR)) + (hv_HalfHeadWidth*hv_DC);
	hv_ColP1 = (hv_Column1 + ((hv_Length - hv_HeadLength)*hv_DC)) - (hv_HalfHeadWidth*hv_DR);
	hv_RowP2 = (hv_Row1 + ((hv_Length - hv_HeadLength)*hv_DR)) - (hv_HalfHeadWidth*hv_DC);
	hv_ColP2 = (hv_Column1 + ((hv_Length - hv_HeadLength)*hv_DC)) + (hv_HalfHeadWidth*hv_DR);
	//
	//Finally create output XLD contour for each input point pair
	{
		HTuple end_val45 = (hv_Length.TupleLength()) - 1;
		HTuple step_val45 = 1;
		for (hv_Index = 0; hv_Index.Continue(end_val45, step_val45); hv_Index += step_val45)
		{
			if (0 != (int(HTuple(hv_Length[hv_Index]) == -1)))
			{
				//Create_ single points for arrows with identical start and end point
				GenContourPolygonXld(&ho_TempArrow, HTuple(hv_Row1[hv_Index]), HTuple(hv_Column1[hv_Index]));
			}
			else
			{
				//Create arrow contour
				GenContourPolygonXld(&ho_TempArrow, ((((HTuple(hv_Row1[hv_Index]).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP1[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP2[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index])),
					((((HTuple(hv_Column1[hv_Index]).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP1[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP2[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index])));
			}
			ConcatObj((*ho_Arrow), ho_TempArrow, &(*ho_Arrow));
		}
	}
	return;
}

void MeasureLine::rake(HObject ho_Image, HObject *ho_Regions, HTuple hv_Elements, HTuple hv_DetectHeight,
	HTuple hv_DetectWidth, HTuple hv_Sigma, HTuple hv_Threshold, HTuple hv_Transition,
	HTuple hv_Select, HTuple hv_Row1, HTuple hv_Column1, HTuple hv_Row2, HTuple hv_Column2,
	HTuple *hv_ResultRow, HTuple *hv_ResultColumn)
{

	// Local iconic variables
	HObject  ho_RegionLines, ho_Rectangle, ho_Arrow1;

	// Local control variables
	HTuple  hv_Width, hv_Height, hv_ATan, hv_i, hv_RowC;
	HTuple  hv_ColC, hv_Distance, hv_RowL2, hv_RowL1, hv_ColL2;
	HTuple  hv_ColL1, hv_MsrHandle_Measure, hv_RowEdge, hv_ColEdge;
	HTuple  hv_Amplitude, hv_tRow, hv_tCol, hv_t, hv_Number;
	HTuple  hv_j;

	//获取图像尺寸
	GetImageSize(ho_Image, &hv_Width, &hv_Height);
	//产生一个空显示对象，用于显示
	GenEmptyObj(&(*ho_Regions));
	//初始化边缘坐标数组
	(*hv_ResultRow) = HTuple();
	(*hv_ResultColumn) = HTuple();
	//产生直线xld
	GenContourPolygonXld(&ho_RegionLines, hv_Row1.TupleConcat(hv_Row2), hv_Column1.TupleConcat(hv_Column2));
	//存储到显示对象
	ConcatObj((*ho_Regions), ho_RegionLines, &(*ho_Regions));
	//计算直线与x轴的夹角，逆时针方向为正向。
	AngleLx(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_ATan);

	//边缘检测方向垂直于检测直线：直线方向正向旋转90°为边缘检测方向
	hv_ATan += HTuple(90).TupleRad();

	//根据检测直线按顺序产生测量区域矩形，并存储到显示对象
	{
		HTuple end_val18 = hv_Elements;
		HTuple step_val18 = 1;
		for (hv_i = 1; hv_i.Continue(end_val18, step_val18); hv_i += step_val18)
		{
			//RowC := Row1+(((Row2-Row1)*i)/(Elements+1))
			//ColC := Column1+(Column2-Column1)*i/(Elements+1)
			//if (RowC>Height-1 or RowC<0 or ColC>Width-1 or ColC<0)
			  //continue
			//endif
			//如果只有一个测量矩形，作为卡尺工具，宽度为检测直线的长度
			if (0 != (int(hv_Elements == 1)))
			{
				hv_RowC = (hv_Row1 + hv_Row2)*0.5;
				hv_ColC = (hv_Column1 + hv_Column2)*0.5;
				//判断是否超出图像,超出不检测边缘
				if (0 != (HTuple(HTuple(HTuple(int(hv_RowC > (hv_Height - 1))).TupleOr(int(hv_RowC < 0))).TupleOr(int(hv_ColC > (hv_Width - 1)))).TupleOr(int(hv_ColC < 0))))
				{
					continue;
				}
				DistancePp(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_Distance);
				hv_DetectWidth = hv_Distance;
				GenRectangle2ContourXld(&ho_Rectangle, hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight / 2,
					hv_Distance / 2);
			}
			else
			{
				//如果有多个测量矩形，产生该测量矩形xld
				hv_RowC = hv_Row1 + (((hv_Row2 - hv_Row1)*(hv_i - 1)) / (hv_Elements - 1));
				hv_ColC = hv_Column1 + (((hv_Column2 - hv_Column1)*(hv_i - 1)) / (hv_Elements - 1));
				//判断是否超出图像,超出不检测边缘
				if (0 != (HTuple(HTuple(HTuple(int(hv_RowC > (hv_Height - 1))).TupleOr(int(hv_RowC < 0))).TupleOr(int(hv_ColC > (hv_Width - 1)))).TupleOr(int(hv_ColC < 0))))
				{
					continue;
				}
				GenRectangle2ContourXld(&ho_Rectangle, hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight / 2,
					hv_DetectWidth / 2);
			}

			//把测量矩形xld存储到显示对象
			ConcatObj((*ho_Regions), ho_Rectangle, &(*ho_Regions));
			if (0 != (int(hv_i == 1)))
			{
				//在第一个测量矩形绘制一个箭头xld，用于只是边缘检测方向
				hv_RowL2 = hv_RowC + ((hv_DetectHeight / 2)*((-hv_ATan).TupleSin()));
				hv_RowL1 = hv_RowC - ((hv_DetectHeight / 2)*((-hv_ATan).TupleSin()));
				hv_ColL2 = hv_ColC + ((hv_DetectHeight / 2)*((-hv_ATan).TupleCos()));
				hv_ColL1 = hv_ColC - ((hv_DetectHeight / 2)*((-hv_ATan).TupleCos()));
				gen_arrow_contour_xld(&ho_Arrow1, hv_RowL1, hv_ColL1, hv_RowL2, hv_ColL2, 25,
					25);
				//把xld存储到显示对象
				ConcatObj((*ho_Regions), ho_Arrow1, &(*ho_Regions));
			}
			//产生测量对象句柄
			GenMeasureRectangle2(hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight / 2, hv_DetectWidth / 2,
				hv_Width, hv_Height, "nearest_neighbor", &hv_MsrHandle_Measure);

			//设置极性
			if (0 != (int(hv_Transition == HTuple("negative"))))
			{
				hv_Transition = "negative";
			}
			else
			{
				if (0 != (int(hv_Transition == HTuple("positive"))))
				{

					hv_Transition = "positive";
				}
				else
				{
					hv_Transition = "all";
				}
			}
			//设置边缘位置。最强点是从所有边缘中选择幅度绝对值最大点，需要设置为'all'
			if (0 != (int(hv_Select == HTuple("first"))))
			{
				hv_Select = "first";
			}
			else
			{
				if (0 != (int(hv_Select == HTuple("last"))))
				{

					hv_Select = "last";
				}
				else
				{
					hv_Select = "all";
				}
			}
			//检测边缘
			MeasurePos(ho_Image, hv_MsrHandle_Measure, hv_Sigma, hv_Threshold, hv_Transition,
				hv_Select, &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);
			//清除测量对象句柄
			CloseMeasure(hv_MsrHandle_Measure);

			//临时变量初始化
			//tRow，tCol保存找到指定边缘的坐标
			hv_tRow = 0;
			hv_tCol = 0;
			//t保存边缘的幅度绝对值
			hv_t = 0;
			//找到的边缘必须至少为1个
			TupleLength(hv_RowEdge, &hv_Number);
			if (0 != (int(hv_Number < 1)))
			{
				continue;
			}
			//有多个边缘时，选择幅度绝对值最大的边缘
			{
				HTuple end_val100 = hv_Number - 1;
				HTuple step_val100 = 1;
				for (hv_j = 0; hv_j.Continue(end_val100, step_val100); hv_j += step_val100)
				{
					if (0 != (int((HTuple(hv_Amplitude[hv_j]).TupleAbs()) > hv_t)))
					{

						hv_tRow = HTuple(hv_RowEdge[hv_j]);
						hv_tCol = HTuple(hv_ColEdge[hv_j]);
						hv_t = HTuple(hv_Amplitude[hv_j]).TupleAbs();
					}
				}
			}
			//把找到的边缘保存在输出数组
			if (0 != (int(hv_t > 0)))
			{
				(*hv_ResultRow) = (*hv_ResultRow).TupleConcat(hv_tRow);
				(*hv_ResultColumn) = (*hv_ResultColumn).TupleConcat(hv_tCol);
			}
		}
	}

	return;
}

void MeasureLine::ClearModel()
{
	//try {	(m_handleModel); }
	//catch (...) {}
}

int MeasureLine::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);
	QJsonArray array_object = strData.find("LineConFig").value().toArray();
	if (!array_object.empty())
	{
		m_strImg			= array_object.at(0).toString();
		m_strSendContent	= array_object.at(1).toString();
	}

	QJsonArray array_Roiobject = strData.find("ROIConFig").value().toArray();
	m_vecROIData = array_Roiobject;

	UpdateParam();
	return 0;
}

int MeasureLine::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	QJsonArray ConFig = {
		m_strImg,
		m_strSendContent,
	};
	strData.insert("LineConFig", ConFig);

	strData.insert("ROIConFig", m_vecROIData);

	UpdateParam();
	return 0;
}

void MeasureLine::UpdateParam()
{
	if (m_mapParam.contains("txtLinkFollow"))				m_strLinkName			= m_mapParam["txtLinkFollow"];
	if (m_mapParam.contains("checkUseFollow"))				m_bEnableCom			= m_mapParam["checkUseFollow"].toInt();

	if (m_mapShowParam.contains(QString(showregion)))		m_bEnableShowRegion		= m_mapShowParam[showregion].toInt();
	if (m_mapShowParam.contains(QString(showResult)))		m_bEnableShowResult		= m_mapShowParam[showResult].toInt();
	if (m_mapShowParam.contains(QString(showDrawResult)))	m_bEnableshowDrawResult = m_mapShowParam[showDrawResult].toInt();
	if (m_mapShowParam.contains(QString(FontSize)))			m_iFontSize				= m_mapShowParam[FontSize].toInt();

	m_vecResultLines.clear();
	m_vecResultLines = frmMeasureLine::QJsonToShapeData(m_vecROIData);

}

void MeasureLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmMeasureLine * frm = new frmMeasureLine(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;
	UpdateParam();
	//设置识别区域
	delete frm;
}

EnumNodeResult MeasureLine::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult MeasureLine::PrExecute(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult MeasureLine::Execute(MiddleParam& param, QString& strError)
{	
	if (param.MapPImgVoid.count(m_strImg) > 0)
	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strImg));
		if (!hImg.IsInitialized()) {
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		QString _strResultName = GetItemId() + ".Out.";
		QVector<LineCaliperP>	_vecResultLines = m_vecResultLines;

		std::vector<QPointF>	_VecPt1D;
		std::vector<sDrawText>	_VecText1D;
		std::vector<sDrawCross> _PointLst;
		std::vector<sColorLine> _LineLst;

		if (m_bEnableCom) {
			HTuple _Metrix;
			if (CheckMetrixType(m_strLinkName, _Metrix) == 0) {
				_vecResultLines.clear();
				for (auto& _Lines : m_vecResultLines) {
					LineCaliperP _line = _Lines;
					HTuple	_Rows, _Columns;
					AffineTransPixel(_Metrix, _Lines.y1, _Lines.x1, &_Rows, &_Columns);
					_line.y1 = _Rows.D();
					_line.x1 = _Columns.D();
					AffineTransPixel(_Metrix, _Lines.y2, _Lines.x2, &_Rows, &_Columns);
					_line.y2 = _Rows.D();
					_line.x2 = _Columns.D();
					_vecResultLines.push_back(_line);
				}
			}
			else {
				SetError(QString("%1 Find Metrix Isn't IsInitialized").arg(m_strInImg));
				sDrawText			_strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = GetItemContent() + tr(" The Link Matrix Failed!");
				param.VecDrawText.push_back(_strText);
				if (databaseVar::Instance().m_iCheckNGMotion > 0)		return NodeResult_ParamError;
				else			return NodeResult_NG;
			}
		}

		HObject ho_Regions, ho_Line;
		HTuple hv_ResultRow, hv_ResultColumn;
		HTuple hv_Line1Row1, hv_Line1Col1, hv_Line1Row2, hv_Line1Col2;
		for (auto& _Lines : _vecResultLines) {
			MeasureLine::rake(hImg, &ho_Regions, _Lines.num, _Lines.height, _Lines.width,
				_Lines.sSigma, _Lines.sThreshold, _Lines.sTransition.toStdString().c_str(),
				_Lines.sSelect.toStdString().c_str(), _Lines.y1, _Lines.x1, _Lines.y2, _Lines.x2,
				&hv_ResultRow, &hv_ResultColumn);
			for (int i = 0; i < hv_ResultRow.TupleLength().I(); i++) {
				sDrawCross _Cross;
				_Cross.Center.setX(hv_ResultColumn[i].D());
				_Cross.Center.setY(hv_ResultRow[i].D());
				_Cross.DrawColor = QColor(0, 0, 255);
				if (m_mapShowParam.count(FontSize) > 0)	_Cross.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_PointLst.push_back(_Cross);
			}
			MeasureLine::pts_to_best_line(&ho_Line, hv_ResultRow, hv_ResultColumn, _Lines.sActiveNum,
				&hv_Line1Row1, &hv_Line1Col1, &hv_Line1Row2, &hv_Line1Col2);
			if (hv_Line1Row2.Length() > 0) {
				if (m_mapShowParam.count(showDrawResult) > 0) {
					if (m_mapShowParam[showDrawResult].toInt() > 0) {
						sColorLine _line;
						_line.Dline.setLine(hv_Line1Col1.D(), hv_Line1Row1.D(), hv_Line1Col2.D(), hv_Line1Row2.D());
						_line.DrawQColor = QColor(255, 0, 0);
						_line.LineWidth = 2;
						_LineLst.push_back(_line);
					}
				}
			}
			else {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = tr("Measure Line NG");
				_VecText1D.push_back(_strText);
			}
		}
		for (auto iter : _VecPt1D)		param.VecShowQPointFs.push_back(iter);
		for (auto iter : _VecText1D)	param.VecDrawText.push_back(iter);
		for (int i = 0; i < _PointLst.size(); i++)
			param.MapDrawCross.insert(std::pair<QString, sDrawCross>(_strResultName + QString::number(i), _PointLst[i]));
		for (auto iter : _LineLst)	param.VecShowColorLine.push_back(iter);

		if (m_bEnableShowRegion) {
			for (int i = 0; i < _vecResultLines.size(); i++) {
				qreal qdx = (_vecResultLines[i].x1 - _vecResultLines[i].x2) / (_vecResultLines[i].num - 1);
				qreal qdy = (_vecResultLines[i].y1 - _vecResultLines[i].y2) / (_vecResultLines[i].num - 1);
				QPointF	_ptCenter;
				qreal qdistance = sqrt(	(_vecResultLines[i].x1 - _vecResultLines[i].x2) * (_vecResultLines[i].x1 - _vecResultLines[i].x2)
					+ (_vecResultLines[i].y1 - _vecResultLines[i].y2) * (_vecResultLines[i].y1 - _vecResultLines[i].y2)	);
				qreal _dx = (_vecResultLines[i].x1 - _vecResultLines[i].x2) / qdistance;
				qreal _dy = (_vecResultLines[i].y1 - _vecResultLines[i].y2) / qdistance;

				for (int J = 0; J < _vecResultLines[i].num; J++)	{
					_ptCenter.setX(_vecResultLines[i].x2 + qdx * J);
					_ptCenter.setY(_vecResultLines[i].y2 + qdy * J);
					sColorLineRect _LineRect;
					qreal _dWidth = _vecResultLines[i].width / 2.0;
					_LineRect.Dline.setLine(
						_ptCenter.x() - _dx * _dWidth,
						_ptCenter.y() - _dy * _dWidth,
						_ptCenter.x() + _dx * _dWidth,
						_ptCenter.y() + _dy * _dWidth
					);
					_LineRect.DWidth = _vecResultLines[i].height * 2.0;
					_LineRect.DrawQColor = QColor(0, 0, 255);;
					_LineRect.LineWidth = -1;
					param.VecShowColorLineRect.push_back(_LineRect);
				}

			}
		}
	}
	else
	{
		SetError(QString("Image %1 didn't Exist").arg(m_strImg));
		return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

QPair<QString, QString> MeasureLine::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> MeasureLine::GetToolName()
{
	return QPair<QString, QString>("LineCaliper", tr("LineCaliper"));
}
