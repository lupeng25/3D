#include "VisionClass.h"
#include <QPluginLoader>
//#include "ToolTree.h"

VisionManager & VisionManager::Instance()
{
	// TODO: 在此处插入 return 语句
	static VisionManager Ins;
	return Ins;
}

QThreadPool & VisionManager::ThreadPool()
{
	// TODO: 在此处插入 return 语句
	return VisionManager::Instance().m_ThreadPool;
}

int VisionManager::Init()
{
	return 0;
}

int VisionManager::Exit()
{
	return 0;
}

void VisionManager::disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
	HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_GenParamName, hv_GenParamValue;

	//This procedure displays text in a graphics window.
	//
	//Input parameters:
	//WindowHandle: The WindowHandle of the graphics window, where
	//   the message should be displayed.
	//String: A tuple of strings containing the text messages to be displayed.
	//CoordSystem: If set to 'window', the text position is given
	//   with respect to the window coordinate system.
	//   If set to 'image', image coordinates are used.
	//   (This may be useful in zoomed images.)
	//Row: The row coordinate of the desired text position.
	//   You can pass a single value or a tuple of values.
	//   See the explanation below.
	//   Default: 12.
	//Column: The column coordinate of the desired text position.
	//   You can pass a single value or a tuple of values.
	//   See the explanation below.
	//   Default: 12.
	//Color: defines the color of the text as string.
	//   If set to [] or '' the currently set color is used.
	//   If a tuple of strings is passed, the colors are used cyclically
	//   for every text position defined by Row and Column,
	//   or every new text line in case of |Row| == |Column| == 1.
	//Box: A tuple controlling a possible box surrounding the text.
	//   Its entries:
	//   - Box[0]: Controls the box and its color. Possible values:
	//     -- 'true' (Default): An orange box is displayed.
	//     -- 'false': No box is displayed.
	//     -- color string: A box is displayed in the given color, e.g., 'white', '#FF00CC'.
	//   - Box[1] (Optional): Controls the shadow of the box. Possible values:
	//     -- 'true' (Default): A shadow is displayed in
	//               darker orange if Box[0] is not a color and in 'white' otherwise.
	//     -- 'false': No shadow is displayed.
	//     -- color string: A shadow is displayed in the given color, e.g., 'white', '#FF00CC'.
	//
	//It is possible to display multiple text strings in a single call.
	//In this case, some restrictions apply on the
	//parameters String, Row, and Column:
	//They can only have either 1 entry or n entries.
	//Behavior in the different cases:
	//   - Multiple text positions are specified, i.e.,
	//       - |Row| == n, |Column| == n
	//       - |Row| == n, |Column| == 1
	//       - |Row| == 1, |Column| == n
	//     In this case we distinguish:
	//       - |String| == n: Each element of String is displayed
	//                        at the corresponding position.
	//       - |String| == 1: String is displayed n times
	//                        at the corresponding positions.
	//   - Exactly one text position is specified,
	//      i.e., |Row| == |Column| == 1:
	//      Each element of String is display in a new textline.
	//
	//
	//Convert the parameters for disp_text.
	if (0 != (HTuple(int(hv_Row == HTuple())).TupleOr(int(hv_Column == HTuple()))))
	{
		return;
	}
	if (0 != (int(hv_Row == -1)))
	{
		hv_Row = 12;
	}
	if (0 != (int(hv_Column == -1)))
	{
		hv_Column = 12;
	}
	//
	//Convert the parameter Box to generic parameters.
	hv_GenParamName = HTuple();
	hv_GenParamValue = HTuple();
	if (0 != (int((hv_Box.TupleLength())>0)))
	{
		if (0 != (int(HTuple(hv_Box[0]) == HTuple("false"))))
		{
			//Display no box
			hv_GenParamName = hv_GenParamName.TupleConcat("box");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (int(HTuple(hv_Box[0]) != HTuple("true"))))
		{
			//Set a color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
		}
	}
	if (0 != (int((hv_Box.TupleLength())>1)))
	{
		if (0 != (int(HTuple(hv_Box[1]) == HTuple("false"))))
		{
			//Display no shadow.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (int(HTuple(hv_Box[1]) != HTuple("true"))))
		{
			//Set a shadow color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
		}
	}
	//Restore default CoordSystem behavior.
	if (0 != (int(hv_CoordSystem != HTuple("window"))))
	{
		hv_CoordSystem = "image";
	}
	//
	if (0 != (int(hv_Color == HTuple(""))))
	{
		//disp_text does not accept an empty string for Color.
		hv_Color = HTuple();
	}
	//
	DispText(hv_WindowHandle, hv_String, hv_CoordSystem, hv_Row, hv_Column, hv_Color,
		hv_GenParamName, hv_GenParamValue);
	return;
}

void VisionManager::draw_rake(HObject *ho_Regions, HTuple hv_WindowHandle, HTuple hv_Elements,
	HTuple hv_DetectHeight, HTuple hv_DetectWidth, HTuple *hv_Row1, HTuple *hv_Column1,
	HTuple *hv_Row2, HTuple *hv_Column2)
{

	// Local iconic variables
	HObject  ho_RegionLines, ho_Rectangle, ho_Arrow1;

	// Local control variables
	HTuple  hv_ATan, hv_i, hv_RowC, hv_ColC, hv_Distance;
	HTuple  hv_RowL2, hv_RowL1, hv_ColL2, hv_ColL1;

	//提示
	disp_message(hv_WindowHandle, //'点击鼠标左键画一条直线,点击右键确认'
		HTuple("\265\343\273\367\312\363\261\352\327\363\274\374\273\255\322\273\314\365\326\261\317\337,\265\343\273\367\323\322\274\374\310\267\310\317"),
		"window", 12, 12, "red", "false");
	//产生一个空显示对象，用于显示
	GenEmptyObj(&(*ho_Regions));
	//画矢量检测直线
	DrawLine(hv_WindowHandle, &(*hv_Row1), &(*hv_Column1), &(*hv_Row2), &(*hv_Column2));
	//产生直线xld
	GenContourPolygonXld(&ho_RegionLines, (*hv_Row1).TupleConcat((*hv_Row2)), (*hv_Column1).TupleConcat((*hv_Column2)));
	//存储到显示对象
	ConcatObj((*ho_Regions), ho_RegionLines, &(*ho_Regions));
	//计算直线与x轴的夹角，逆时针方向为正向。
	AngleLx((*hv_Row1), (*hv_Column1), (*hv_Row2), (*hv_Column2), &hv_ATan);

	//边缘检测方向垂直于检测直线：直线方向正向旋转90°为边缘检测方向
	hv_ATan += HTuple(90).TupleRad();

	//根据检测直线按顺序产生测量区域矩形，并存储到显示对象
	{
		HTuple end_val17 = hv_Elements;
		HTuple step_val17 = 1;
		for (hv_i = 1; hv_i.Continue(end_val17, step_val17); hv_i += step_val17)
		{
			//如果只有一个测量矩形，作为卡尺工具，宽度为检测直线的长度
			if (0 != (int(hv_Elements == 1)))
			{
				hv_RowC = ((*hv_Row1) + (*hv_Row2))*0.5;
				hv_ColC = ((*hv_Column1) + (*hv_Column2))*0.5;
				DistancePp((*hv_Row1), (*hv_Column1), (*hv_Row2), (*hv_Column2), &hv_Distance);
				GenRectangle2ContourXld(&ho_Rectangle, hv_RowC, hv_ColC, hv_ATan, hv_DetectHeight / 2,
					hv_Distance / 2);
			}
			else
			{
				//如果有多个测量矩形，产生该测量矩形xld
				hv_RowC = (*hv_Row1) + ((((*hv_Row2) - (*hv_Row1))*(hv_i - 1)) / (hv_Elements - 1));
				hv_ColC = (*hv_Column1) + ((((*hv_Column2) - (*hv_Column1))*(hv_i - 1)) / (hv_Elements - 1));
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
		}
	}

	return;
}

void VisionManager::gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
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

void VisionManager::pts_to_best_line(HObject *ho_Line, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
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
	if (0 != (HTuple(int(hv_Length >= hv_ActiveNum)).TupleAnd(int(hv_ActiveNum>1))))
	{
		//halcon的拟合是基于xld的，需要把边缘连接成xld
		GenContourPolygonXld(&ho_Contour, hv_Rows, hv_Cols);
		//拟合直线。使用的算法是'tukey'，其他算法请参考fit_line_contour_xld的描述部分。
		FitLineContourXld(ho_Contour, "tukey", -1, 0, 5, 2, &(*hv_Row1), &(*hv_Column1),
			&(*hv_Row2), &(*hv_Column2), &hv_Nr, &hv_Nc, &hv_Dist);
		//判断拟合结果是否有效：如果拟合成功，数组中元素的数量大于0
		TupleLength(hv_Dist, &hv_Length1);
		if (0 != (int(hv_Length1<1)))
		{
			return;
		}
		//根据拟合结果，产生直线xld
		GenContourPolygonXld(&(*ho_Line), (*hv_Row1).TupleConcat((*hv_Row2)), (*hv_Column1).TupleConcat((*hv_Column2)));
	}

	return;
}

void VisionManager::rake(HObject ho_Image, HObject *ho_Regions, HTuple hv_Elements, HTuple hv_DetectHeight,
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
				if (0 != (HTuple(HTuple(HTuple(int(hv_RowC>(hv_Height - 1))).TupleOr(int(hv_RowC<0))).TupleOr(int(hv_ColC>(hv_Width - 1)))).TupleOr(int(hv_ColC<0))))
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
				if (0 != (HTuple(HTuple(HTuple(int(hv_RowC>(hv_Height - 1))).TupleOr(int(hv_RowC<0))).TupleOr(int(hv_ColC>(hv_Width - 1)))).TupleOr(int(hv_ColC<0))))
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
			if (0 != (int(hv_Number<1)))
			{
				continue;
			}
			//有多个边缘时，选择幅度绝对值最大的边缘
			{
				HTuple end_val100 = hv_Number - 1;
				HTuple step_val100 = 1;
				for (hv_j = 0; hv_j.Continue(end_val100, step_val100); hv_j += step_val100)
				{
					if (0 != (int((HTuple(hv_Amplitude[hv_j]).TupleAbs())>hv_t)))
					{

						hv_tRow = HTuple(hv_RowEdge[hv_j]);
						hv_tCol = HTuple(hv_ColEdge[hv_j]);
						hv_t = HTuple(hv_Amplitude[hv_j]).TupleAbs();
					}
				}
			}
			//把找到的边缘保存在输出数组
			if (0 != (int(hv_t>0)))
			{
				(*hv_ResultRow) = (*hv_ResultRow).TupleConcat(hv_tRow);
				(*hv_ResultColumn) = (*hv_ResultColumn).TupleConcat(hv_tCol);
			}
		}
	}

	return;
}