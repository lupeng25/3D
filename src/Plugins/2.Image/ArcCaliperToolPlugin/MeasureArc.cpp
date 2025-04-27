#include "MeasureArc.h"
#include <qdebug.h>
#include "frmMeasureArc.h"
#include "databaseVar.h"

MeasureArc::MeasureArc() :FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));

	ClearModel();
}

MeasureArc::MeasureArc(QPointF pos) : FlowchartTool()
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));

	ClearModel();
}

MeasureArc::~MeasureArc()
{
	ClearModel();
	Clearport();
}

void MeasureArc::pts_to_best_line(HObject *ho_Line, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
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

void MeasureArc::gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
	HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth)
{
	// Local iconic variables
	HObject  ho_TempArrow;

	// Local control variables
	HTuple  hv_Length, hv_ZeroLengthIndices, hv_DR;
	HTuple  hv_DC, hv_HalfHeadWidth, hv_RowP1, hv_ColP1, hv_RowP2;
	HTuple  hv_ColP2, hv_Index;

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

void MeasureArc::rake(HObject ho_Image, HObject *ho_Regions, HTuple hv_Elements, HTuple hv_DetectHeight,
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
			HalconCpp::GenMeasureRectangle2(hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight / 2, hv_DetectWidth / 2,
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
			HalconCpp::MeasurePos(ho_Image, hv_MsrHandle_Measure, hv_Sigma, hv_Threshold, hv_Transition,
				hv_Select, &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);
			//清除测量对象句柄
			HalconCpp::CloseMeasure(hv_MsrHandle_Measure);

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

void MeasureArc::Spoke(HObject ho_Image, HObject * ho_Regions, HTuple hv_Elements,HTuple hv_DetectHeight,
	HTuple hv_DetectWidth, HTuple hv_Sigma, HTuple hv_Threshold,HTuple hv_Transition,
	HTuple hv_Select, HTuple hv_CenterRow, HTuple hv_CenterColumn, 
	HTuple hv_Radius, HTuple hv_StartAngle, HTuple hv_EndAngle, HTuple * hv_ResultRow, HTuple * hv_ResultColumn)
{	// Local iconic variables
	
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
	double angle = hv_StartAngle.D();
	double	dAngleRang = (hv_EndAngle.D() - hv_StartAngle.D());
	if (dAngleRang > 360) {
		dAngleRang = (hv_EndAngle.D() - hv_StartAngle.D()) - ((int)(hv_EndAngle.D() - hv_StartAngle.D()) / 360) * 360;
	}
	QPointF dStartPos;
	QPointF dEndPos;
	QPointF Center;
	QPointF MeasureCenter;
	Center.setX(hv_CenterColumn.D());
	Center.setY(hv_CenterRow.D());
	if (dAngleRang >= 360) {
		double dAngleStep = dAngleRang / (double)(hv_Elements.D());
		for (int i = 0; i < (int)hv_Elements.D(); i++)		{
			angle		= hv_StartAngle.D() + (dAngleStep * i);
			dStartPos	= GetAnglePos(Center,hv_Radius - hv_DetectHeight, angle);
			dEndPos		= GetAnglePos(Center,hv_Radius + hv_DetectHeight, angle);
			MeasureCenter.setX((dEndPos.x() + dStartPos.x()) / 2.0);
			MeasureCenter.setY((dEndPos.y() + dStartPos.y()) / 2.0);
			hv_RowC = (dEndPos.y() + dStartPos.y()) / 2.0;
			hv_ColC = (dEndPos.x() + dStartPos.x()) / 2.0;
			//计算直线与x轴的夹角，逆时针方向为正向。
			HalconCpp::AngleLx(dStartPos.y(), dStartPos.x(), dEndPos.y(), dEndPos.x(), &hv_ATan);
			HalconCpp::GenMeasureRectangle2(hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight, hv_DetectWidth,
				hv_Width, hv_Height, "nearest_neighbor", &hv_MsrHandle_Measure);
			//设置极性
			if (0 != (int(hv_Transition == HTuple("negative"))))			{
				hv_Transition = "negative";
			}
			else			{
				if (0 != (int(hv_Transition == HTuple("positive"))))				{
					hv_Transition = "positive";
				}
				else				{
					hv_Transition = "all";
				}
			}
			//设置边缘位置。最强点是从所有边缘中选择幅度绝对值最大点，需要设置为'all'
			if (0 != (int(hv_Select == HTuple("first"))))			{
				hv_Select = "first";
			}
			else			{
				if (0 != (int(hv_Select == HTuple("last"))))				{
					hv_Select = "last";
				}
				else				{
					hv_Select = "all";
				}
			}
			//检测边缘
			HalconCpp::MeasurePos(ho_Image, hv_MsrHandle_Measure, hv_Sigma, hv_Threshold, hv_Transition,
				hv_Select, &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);
			//清除测量对象句柄
			HalconCpp::CloseMeasure(hv_MsrHandle_Measure);

			GenRectangle2ContourXld(&ho_Rectangle,hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight, hv_DetectWidth);
			ConcatObj((*ho_Regions), ho_Rectangle, &(*ho_Regions));

			//临时变量初始化
			//tRow，tCol保存找到指定边缘的坐标
			hv_tRow = 0;
			hv_tCol = 0;
			//t保存边缘的幅度绝对值
			hv_t = 0;
			//找到的边缘必须至少为1个
			TupleLength(hv_RowEdge, &hv_Number);
			if (0 != (int(hv_Number < 1)))	continue;
			//有多个边缘时，选择幅度绝对值最大的边缘
			{
				HTuple end_val100 = hv_Number - 1;
				HTuple step_val100 = 1;
				for (hv_j = 0; hv_j.Continue(end_val100, step_val100); hv_j += step_val100)			{
					if (0 != (int((HTuple(hv_Amplitude[hv_j]).TupleAbs()) > hv_t)))					{
						hv_tRow = HTuple(hv_RowEdge[hv_j]);
						hv_tCol = HTuple(hv_ColEdge[hv_j]);
						hv_t = HTuple(hv_Amplitude[hv_j]).TupleAbs();
					}
				}
			}
			//把找到的边缘保存在输出数组
			if (0 != (int(hv_t > 0)))			{
				(*hv_ResultRow) = (*hv_ResultRow).TupleConcat(hv_tRow);
				(*hv_ResultColumn) = (*hv_ResultColumn).TupleConcat(hv_tCol);
			}
		}
	}
	else	{
		double dAngleStep = dAngleRang / (double)(hv_Elements.D() - 1);
		for (int i = 0; i < (int)hv_Elements.D(); i++) {
			angle		= hv_StartAngle.D() + (dAngleStep * i);
			dStartPos	= GetAnglePos(Center, hv_Radius - hv_DetectHeight, angle);
			dEndPos		= GetAnglePos(Center, hv_Radius + hv_DetectHeight, angle);
			MeasureCenter.setX((dEndPos.x() + dStartPos.x()) / 2.0);
			MeasureCenter.setY((dEndPos.y() + dStartPos.y()) / 2.0);
			hv_RowC = (dEndPos.y() + dStartPos.y()) / 2.0;
			hv_ColC = (dEndPos.x() + dStartPos.x()) / 2.0;
			//计算直线与x轴的夹角，逆时针方向为正向。
			HalconCpp::AngleLx(dStartPos.y(), dStartPos.x(), dEndPos.y(), dEndPos.x(), &hv_ATan);
			HalconCpp::GenMeasureRectangle2(hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight, hv_DetectWidth,
				hv_Width, hv_Height, "nearest_neighbor", &hv_MsrHandle_Measure);
			//设置极性
			if (0 != (int(hv_Transition == HTuple("negative")))) {
				hv_Transition = "negative";
			}
			else {
				if (0 != (int(hv_Transition == HTuple("positive")))) {
					hv_Transition = "positive";
				}
				else {
					hv_Transition = "all";
				}
			}
			//设置边缘位置。最强点是从所有边缘中选择幅度绝对值最大点，需要设置为'all'
			if (0 != (int(hv_Select == HTuple("first")))) {
				hv_Select = "first";
			}
			else {
				if (0 != (int(hv_Select == HTuple("last")))) {
					hv_Select = "last";
				}
				else {
					hv_Select = "all";
				}
			}
			//检测边缘
			HalconCpp::MeasurePos(ho_Image, hv_MsrHandle_Measure, hv_Sigma, hv_Threshold, hv_Transition,
				hv_Select, &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);
			//清除测量对象句柄
			HalconCpp::CloseMeasure(hv_MsrHandle_Measure);

			GenRectangle2ContourXld(&ho_Rectangle, hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight, hv_DetectWidth);
			ConcatObj((*ho_Regions), ho_Rectangle, &(*ho_Regions));

			//临时变量初始化
			//tRow，tCol保存找到指定边缘的坐标
			hv_tRow = 0;
			hv_tCol = 0;
			//t保存边缘的幅度绝对值
			hv_t = 0;
			//找到的边缘必须至少为1个
			TupleLength(hv_RowEdge, &hv_Number);
			if (0 != (int(hv_Number < 1)))	continue;
			//有多个边缘时，选择幅度绝对值最大的边缘
			{
				HTuple end_val100 = hv_Number - 1;
				HTuple step_val100 = 1;
				for (hv_j = 0; hv_j.Continue(end_val100, step_val100); hv_j += step_val100) {
					if (0 != (int((HTuple(hv_Amplitude[hv_j]).TupleAbs()) > hv_t))) {
						hv_tRow = HTuple(hv_RowEdge[hv_j]);
						hv_tCol = HTuple(hv_ColEdge[hv_j]);
						hv_t = HTuple(hv_Amplitude[hv_j]).TupleAbs();
					}
				}
			}
			//把找到的边缘保存在输出数组
			if (0 != (int(hv_t > 0))) {
				(*hv_ResultRow) = (*hv_ResultRow).TupleConcat(hv_tRow);
				(*hv_ResultColumn) = (*hv_ResultColumn).TupleConcat(hv_tCol);
			}
		}
	}
}

void MeasureArc::pts_to_best_circle(HObject *ho_Circle, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
	HTuple hv_ArcType, HTuple *hv_RowCenter, HTuple *hv_ColCenter, HTuple *hv_Radius,
	HTuple *hv_StartPhi, HTuple *hv_EndPhi, HTuple *hv_PointOrder, HTuple *hv_ArcAngle)
{
	// Local iconic variables
	HObject  ho_Contour;

	// Local control variables
	HTuple  hv_Length, hv_Length1, hv_CircleLength;

	//初始化
	(*hv_RowCenter) = 0;
	(*hv_ColCenter) = 0;
	(*hv_Radius) = 0;
	//产生一个空的直线对象，用于保存拟合后的圆
	GenEmptyObj(&(*ho_Circle));
	//计算边缘数量
	TupleLength(hv_Cols, &hv_Length);
	//当边缘数量不小于有效点数时进行拟合
	if (0 != (HTuple(int(hv_Length >= hv_ActiveNum)).TupleAnd(int(hv_ActiveNum>2))))
	{
		//halcon的拟合是基于xld的，需要把边缘连接成xld
		if (0 != (int(hv_ArcType == HTuple("circle"))))
		{
			//如果是闭合的圆，轮廓需要首尾相连
			GenContourPolygonXld(&ho_Contour, hv_Rows.TupleConcat(HTuple(hv_Rows[0])),
				hv_Cols.TupleConcat(HTuple(hv_Cols[0])));
		}
		else
		{
			GenContourPolygonXld(&ho_Contour, hv_Rows, hv_Cols);
		}
		//拟合圆。使用的算法是''geotukey''，其他算法请参考fit_circle_contour_xld的描述部分。
		FitCircleContourXld(ho_Contour, "geotukey", -1, 0, 0, 3, 2, &(*hv_RowCenter),
			&(*hv_ColCenter), &(*hv_Radius), &(*hv_StartPhi), &(*hv_EndPhi), &(*hv_PointOrder));
		//判断拟合结果是否有效：如果拟合成功，数组中元素的数量大于0
		TupleLength((*hv_StartPhi), &hv_Length1);
		if (0 != (int(hv_Length1<1)))
		{
			return;
		}
		//根据拟合结果，产生直线xld
		if (0 != (int(hv_ArcType == HTuple("arc"))))
		{
			GenCircleContourXld(&(*ho_Circle), (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
				(*hv_StartPhi), (*hv_EndPhi), (*hv_PointOrder), 1);

			LengthXld((*ho_Circle), &hv_CircleLength);
			(*hv_ArcAngle) = (*hv_EndPhi) - (*hv_StartPhi);
			if (0 != (int(hv_CircleLength>((HTuple(180).TupleRad())*(*hv_Radius)))))
			{
				if (0 != (int(((*hv_ArcAngle).TupleAbs())<(HTuple(180).TupleRad()))))
				{
					if (0 != (int((*hv_ArcAngle)>0)))
					{
						(*hv_ArcAngle) = (HTuple(360).TupleRad()) - (*hv_ArcAngle);
					}
					else
					{

						(*hv_ArcAngle) = (HTuple(360).TupleRad()) + (*hv_ArcAngle);
					}
				}
			}
			else
			{
				if (0 != (int(hv_CircleLength<((HTuple(180).TupleRad())*(*hv_Radius)))))
				{
					if (0 != (int(((*hv_ArcAngle).TupleAbs())>(HTuple(180).TupleRad()))))
					{
						if (0 != (int((*hv_ArcAngle)>0)))
						{
							(*hv_ArcAngle) = (*hv_ArcAngle) - (HTuple(360).TupleRad());

						}
						else
						{
							(*hv_ArcAngle) = (HTuple(360).TupleRad()) + (*hv_ArcAngle);
						}
					}
				}

			}

		}
		else		{
			(*hv_StartPhi) = 0;
			(*hv_EndPhi) = HTuple(360).TupleRad();
			(*hv_ArcAngle) = HTuple(360).TupleRad();
			GenCircleContourXld(&(*ho_Circle), (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
				(*hv_StartPhi), (*hv_EndPhi), (*hv_PointOrder), 1);
		}
	}

	return;
}

QPointF MeasureArc::GetAnglePos(QPointF center, qreal Radius, qreal angle)
{
	QPointF dStartPos;
	if (0 <= angle && angle < 90) {
		dStartPos = QPointF(center.x() + cos(angle * M_PI / 180) * Radius, center.y() - sin(angle * M_PI / 180) * Radius);
	}
	else if (90 <= angle && angle < 180) {
		dStartPos = QPointF(center.x() - sin((angle - 90) * M_PI / 180) * Radius, center.y() - cos((angle - 90) * M_PI / 180) * Radius);
	}
	else if (180 <= angle && angle < 270) {
		dStartPos = QPointF(center.x() - cos((angle - 180) * M_PI / 180) * Radius, center.y() + sin((angle - 180) * M_PI / 180) * Radius);
	}
	else if (270 <= angle && angle < 360) {
		dStartPos = QPointF(center.x() + sin((angle - 270) * M_PI / 180) * Radius, center.y() + cos((angle - 270) * M_PI / 180) * Radius);
	}
	else {
		dStartPos = QPointF(center.x() + cos(angle * M_PI / 180) * Radius, center.y() - sin(angle * M_PI / 180) * Radius);
	}
	return dStartPos;
}

void MeasureArc::ClearModel()
{
	//try {	(m_handleModel); }
	//catch (...) {}
}

int MeasureArc::SetData(QJsonObject & strData)
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

int MeasureArc::GetData(QJsonObject & strData)
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

void MeasureArc::UpdateParam()
{
	if (m_mapParam.contains("txtLinkFollow"))				m_strLinkName			= m_mapParam["txtLinkFollow"];
	if (m_mapParam.contains("checkUseFollow"))				m_bEnableCom			= m_mapParam["checkUseFollow"].toInt();

	if (m_mapShowParam.contains(QString(showregion)))		m_bEnableShowRegion		= m_mapShowParam[showregion].toInt();
	if (m_mapShowParam.contains(QString(showResult)))		m_bEnableShowResult		= m_mapShowParam[showResult].toInt();
	if (m_mapShowParam.contains(QString(showDrawResult)))	m_bEnableshowDrawResult = m_mapShowParam[showDrawResult].toInt();
	if (m_mapShowParam.contains(QString(FontSize)))			m_iFontSize				= m_mapShowParam[FontSize].toInt();

	m_vecResultLines.clear();
	m_vecResultLines = frmMeasureArc::QJsonToShapeData(m_vecROIData);

}

void MeasureArc::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
	frmMeasureArc * frm = new frmMeasureArc(databaseVar::Instance().m_pWindow);
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;
	UpdateParam();
	//设置识别区域
	delete frm;
}

EnumNodeResult MeasureArc::InitBeforeRun(MiddleParam& param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult MeasureArc::PrExecute(MiddleParam & param, QString& strError)
{
	return NodeResult_OK;
}

EnumNodeResult MeasureArc::Execute(MiddleParam& param, QString& strError)
{	
	if (param.MapPImgVoid.count(m_strImg) > 0)
	{
		HObject& hImg = *((HObject*)param.MapPImgVoid.at(m_strImg));
		if (!hImg.IsInitialized()) {
			SetError(QString("Image %1 Isn't IsInitialized").arg(m_strInImg));
			return NodeResult_ParamError;
		}
		QString _strResultName = GetItemId() + ".Out.";
		QVector<ArcCaliperP>	_vecResultLines = m_vecResultLines;

		std::vector<QPointF>	_VecPt1D;
		std::vector<sDrawText>	_VecText1D;
		std::vector<sDrawCross> _PointLst;
		std::vector<sColorLine> _LineLst;

		if (m_bEnableCom) {
			HTuple _Metrix;
			if (CheckMetrixType(m_strLinkName, _Metrix) == 0) {
				_vecResultLines.clear();
				for (auto& _Lines : m_vecResultLines) {
					ArcCaliperP _line = _Lines;
					HTuple	_Rows, _Columns;
					AffineTransPixel(_Metrix, _Lines.y, _Lines.x, &_Rows, &_Columns);
					_line.y = _Rows.D();
					_line.x = _Columns.D();
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
		HObject ho_Circle;
		HTuple  hv_ArcType, hv_RowCenter, hv_ColCenter, hv_Radius;
		HTuple  hv_StartPhi, hv_EndPhi, hv_PointOrder, hv_ArcAngle;
		for (auto& _Lines : _vecResultLines) {
			MeasureArc::Spoke(hImg, &ho_Regions, _Lines.num, _Lines.height, _Lines.width,
				_Lines.sSigma, _Lines.sThreshold, _Lines.sTransition.toStdString().c_str(),
				_Lines.sSelect.toStdString().c_str(), _Lines.y, _Lines.x, _Lines.Radius,
				_Lines.dStartAngle, _Lines.dEndAngle, &hv_ResultRow, &hv_ResultColumn);
			if (m_bEnableShowResult)	{
				for (int i = 0; i < hv_ResultRow.TupleLength().I(); i++) {
					sDrawCross _Cross;
					_Cross.Center.setX(hv_ResultColumn[i].D());
					_Cross.Center.setY(hv_ResultRow[i].D());
					_Cross.DrawColor = QColor(0, 0, 255);
					if (m_mapShowParam.count(FontSize) > 0)	_Cross.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_PointLst.push_back(_Cross);
				}
			}
			MeasureArc::pts_to_best_circle(&ho_Circle, hv_ResultRow, hv_ResultColumn, _Lines.sActiveNum, "circle",
				&hv_RowCenter, &hv_ColCenter, &hv_Radius, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder,
				&hv_ArcAngle);
			if (hv_RowCenter.Length() > 0) {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(0, 255, 0);
				_strText.strText = GetItemContent() + " " + tr("Measure OK ");
				_VecText1D.push_back(_strText);

				if (m_bEnableShowRegion) {
					QPointF dStartPos;
					QPointF dEndPos;
					QPointF Center;
					QPointF MeasureCenter;
					double angle = _Lines.dStartAngle;
					double	dAngleRang = (_Lines.dEndAngle - _Lines.dStartAngle);
					if (dAngleRang > 360) {
						dAngleRang = (_Lines.dEndAngle - _Lines.dStartAngle) - ((int)(_Lines.dEndAngle - _Lines.dStartAngle) / 360) * 360;
					}
					Center.setX(_Lines.x);
					Center.setY(_Lines.y);
					if (dAngleRang >= 360) {
						double dAngleStep = dAngleRang / (double)_Lines.num;
						for (int i = 0; i < _Lines.num; i++) {
							angle = _Lines.dStartAngle + (dAngleStep * i);
							dStartPos = MeasureArc::GetAnglePos(Center, _Lines.Radius - _Lines.height, angle);
							dEndPos = MeasureArc::GetAnglePos(Center, _Lines.Radius + _Lines.height, angle);
							sColorLineRect _LineRect;
							_LineRect.Dline.setP1(dStartPos);
							_LineRect.Dline.setP2(dEndPos);
							_LineRect.DWidth = _Lines.width * 2;
							_LineRect.DrawQColor = QColor(0, 0, 255);;
							_LineRect.LineWidth = -1;
							param.VecShowColorLineRect.push_back(_LineRect);
						}
					}
					else {
						double dAngleStep = dAngleRang / (_Lines.num - 1);
						for (int i = 0; i < _Lines.num; i++) {
							angle = _Lines.dStartAngle + (dAngleStep * i);
							dStartPos = MeasureArc::GetAnglePos(Center, _Lines.Radius - _Lines.height, angle);
							dEndPos = MeasureArc::GetAnglePos(Center, _Lines.Radius + _Lines.height, angle);
							sColorLineRect _LineRect;
							_LineRect.Dline.setP1(dStartPos);
							_LineRect.Dline.setP2(dEndPos);
							_LineRect.DWidth = _Lines.width * 2;
							_LineRect.DrawQColor = QColor(0, 0, 255);;
							_LineRect.LineWidth = -1;
							param.VecShowColorLineRect.push_back(_LineRect);
						}
					}

					sDrawCross _Cross;
					_Cross.Center.setX(hv_ColCenter.D());
					_Cross.Center.setY(hv_RowCenter.D());
					_Cross.DrawColor = QColor(0, 255, 0);
					if (m_mapShowParam.count(FontSize) > 0)	_Cross.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_PointLst.push_back(_Cross);

					sColorCircle _circle;
					_circle.CenterPoints.setX(hv_ColCenter.D());
					_circle.CenterPoints.setY(hv_RowCenter.D());
					_circle.qRadius = hv_Radius.D();
					_circle.DrawQColor = QColor(255, 0, 0);
					_circle.LineWidth = -1;
					param.VecShowColorCircle.push_back(_circle);
				}
				if (m_bEnableShowResult) {
					sDrawText _strText;
					_strText.bControl = true;
					if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
					_strText.DrawColor = QColor(0, 255, 0);
					_strText.strText = "  " + tr("Measure Result ") +
						QString("X:%1,Y:%2,R:%3")
						.arg(QString::number(hv_ColCenter.D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(hv_RowCenter.D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(hv_Radius.D(), 'f',	databaseVar::Instance().form_System_Precision))
						;
					_VecText1D.push_back(_strText);
				}
			}
			else {
				sDrawText _strText;
				_strText.bControl = true;
				if (m_mapShowParam.count(FontSize) > 0)	_strText.iDrawLenth = m_mapShowParam[FontSize].toInt();
				_strText.DrawColor = QColor(255, 0, 0);
				_strText.strText = GetItemContent() + " " + tr("Measure Line NG");
				_VecText1D.push_back(_strText);
			}
		}
		for (auto iter : _VecPt1D)		param.VecShowQPointFs.push_back(iter);
		for (auto iter : _VecText1D)	param.VecDrawText.push_back(iter);
		for (int i = 0; i < _PointLst.size(); i++)
			param.MapDrawCross.insert(std::pair<QString, sDrawCross>(_strResultName + QString::number(i), _PointLst[i]));
		for (auto iter : _LineLst)	param.VecShowColorLine.push_back(iter);
	}
	else
	{
		SetError(QString("Image %1 didn't Exist").arg(m_strImg));
		return NodeResult_ParamError;
	}
	return NodeResult_OK;
}

QPair<QString, QString> MeasureArc::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> MeasureArc::GetToolName()
{
	return QPair<QString, QString>("ArcCaliper", tr("ArcCaliper"));
}
