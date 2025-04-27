#pragma once
#include <QDir>
#include <QtWidgets/QApplication>
#include <QList>
#include <QtPlugin>
#include <QThreadPool>
#include <QRunnable>

#include "halconcpp\HalconCpp.h"
using namespace HalconCpp;

#ifndef _VISIONMANAGER_EXPORT_
#define _VISIONMANAGER_API_ _declspec(dllexport)
#else
#define _VISIONMANAGER_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

class _VISIONMANAGER_API_ VisionManager
{
public:
	static VisionManager& Instance();

	static QThreadPool& ThreadPool();
public:
	int Init();
	int Exit();
public:
	static	void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
		HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box);
	static	void draw_rake(HObject *ho_Regions, HTuple hv_WindowHandle, HTuple hv_Elements,
		HTuple hv_DetectHeight, HTuple hv_DetectWidth, HTuple *hv_Row1, HTuple *hv_Column1,
		HTuple *hv_Row2, HTuple *hv_Column2);
	// Chapter: XLD / Creation
	// Short Description: Create an arrow shaped XLD contour. 
	static	void gen_arrow_contour_xld(HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
		HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth);
	static	void pts_to_best_line(HObject *ho_Line, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ActiveNum,
		HTuple *hv_Row1, HTuple *hv_Column1, HTuple *hv_Row2, HTuple *hv_Column2);
	static	void rake(HObject ho_Image, HObject *ho_Regions, HTuple hv_Elements, HTuple hv_DetectHeight,
		HTuple hv_DetectWidth, HTuple hv_Sigma, HTuple hv_Threshold, HTuple hv_Transition,
		HTuple hv_Select, HTuple hv_Row1, HTuple hv_Column1, HTuple hv_Row2, HTuple hv_Column2,
		HTuple *hv_ResultRow, HTuple *hv_ResultColumn);
public:
	QThreadPool m_ThreadPool;

};


