#include "ImgGraphicsViews.h"
#include "MyEvent.h"
//#include "data/datavar.h"
#include <QMetaType>
#include <QElapsedTimer>
#include <QtWidgets/QApplication>
#include "ImageShow/ImageItem.h"
#include "databaseVar.h"

ImgGraphicsViews::ImgGraphicsViews(QWidget* parent)
	:QGraphicsViews(parent)
{
	qRegisterMetaType<MiddleParam>("MiddleParam");
	connect(this, &QGraphicsViews::sig_MouseMoveInImg, this, &ImgGraphicsViews::slot_MouseMoveInImg);
}

ImgGraphicsViews::~ImgGraphicsViews()
{

}

// Short Description: Scale the gray values of an image from the interval [Min,Max] to [0,255] 
void ImgGraphicsViews::scale_image_range(HObject ho_Image, HObject *ho_ImageScaled)
{
	HObject ho_Rectangle, ho_ImageScale;
	HTuple  hv_Width, hv_Height, hv_Min, hv_Max, _Range;
	GetImageSize(ho_Image, &hv_Width, &hv_Height);
	GenRectangle1(&ho_Rectangle, 0, 0, hv_Height, hv_Width);
	MinMaxGray(ho_Rectangle, ho_Image, 0, &hv_Min, &hv_Max, &_Range);

	// Local iconic variables
	HObject  ho_ImageSelected, ho_SelectedChannel;
	HObject  ho_LowerRegion, ho_UpperRegion, ho_ImageSelectedScaled;

	// Local control variables
	HTuple  hv_LowerLimit, hv_UpperLimit, hv_Mult;
	HTuple  hv_Add, hv_NumImages, hv_ImageIndex, hv_Channels;
	HTuple  hv_ChannelIndex, hv_MinGray, hv_MaxGray, hv_Range;

	if (0 != ((hv_Min.TupleLength()) == 2))	{
		hv_LowerLimit	= ((const HTuple&)hv_Min)[1];
		hv_Min			= ((const HTuple&)hv_Min)[0];
	}
	else	{
		hv_LowerLimit = 0.0;
	}
	if (0 != ((hv_Max.TupleLength()) == 2))		{
		hv_UpperLimit = ((const HTuple&)hv_Max)[1];
		hv_Max = ((const HTuple&)hv_Max)[0];
	}
	else	{
		hv_UpperLimit = 255.0;
	}
	//
	//Calculate scaling parameters.
	hv_Mult = ((hv_UpperLimit - hv_LowerLimit).TupleReal()) / (hv_Max - hv_Min);
	hv_Add = ((-hv_Mult)*hv_Min) + hv_LowerLimit;
	//
	//Scale image.
	ScaleImage(ho_Image, &ho_Image, hv_Mult, hv_Add);
	//
	//Clip gray values if necessary.
	//This must be done for each image and channel separately.
	//GenEmptyObj(&ho_ImageScale);
	ho_ImageScale.GenEmptyObj();
	CountObj(ho_Image, &hv_NumImages);
	{
		HTuple end_val49 = hv_NumImages;
		HTuple step_val49 = 1;
		for (hv_ImageIndex = 1; hv_ImageIndex.Continue(end_val49, step_val49); hv_ImageIndex += step_val49)
		{
			SelectObj(ho_Image, &ho_ImageSelected, hv_ImageIndex);
			CountChannels(ho_ImageSelected, &hv_Channels);
			{
				HTuple end_val52 = hv_Channels;
				HTuple step_val52 = 1;
				for (hv_ChannelIndex = 1; hv_ChannelIndex.Continue(end_val52, step_val52); hv_ChannelIndex += step_val52)
				{
					AccessChannel(ho_ImageSelected, &ho_SelectedChannel, hv_ChannelIndex);
					MinMaxGray(ho_SelectedChannel, ho_SelectedChannel, 0, &hv_MinGray, &hv_MaxGray,
						&hv_Range);
					Threshold(ho_SelectedChannel, &ho_LowerRegion, (hv_MinGray.TupleConcat(hv_LowerLimit)).TupleMin(),
						hv_LowerLimit);
					Threshold(ho_SelectedChannel, &ho_UpperRegion, hv_UpperLimit, (hv_UpperLimit.TupleConcat(hv_MaxGray)).TupleMax());
					PaintRegion(ho_LowerRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_LowerLimit,
						"fill");
					PaintRegion(ho_UpperRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_UpperLimit,
						"fill");
					if (0 != (hv_ChannelIndex == 1))
					{
						CopyObj(ho_SelectedChannel, &ho_ImageSelectedScaled, 1, 1);
					}
					else
					{
						AppendChannel(ho_ImageSelectedScaled, ho_SelectedChannel, &ho_ImageSelectedScaled
						);
					}
				}
			}
			ConcatObj(ho_ImageScale, ho_ImageSelectedScaled, &ho_ImageScale);
		}
	}
	HalconCpp::ConvertImageType(ho_ImageScale, &(*ho_ImageScaled), "byte");
	return;
}

QImage ImgGraphicsViews::HObject2QImage(const HObject & hObject)
{
	HTuple Type, Channels;
	GetImageType(hObject, &Type);
	CountChannels(hObject, &Channels);

	Hlong  width = 0, height = 0;
	HString	htype;
	if (Type.S() == HString(HString("byte")))
	{
		switch (Channels.I())
		{
		case 1:
		{
			HImage from(hObject);
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			QImage img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
			return img;
		}	break;
		case 3:
		{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		case 4:
		{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		default:
		{

		}	break;
		}
	}
	else if (Type.S() == HString(HString("real")) || Type.S() == HString(HString("uint2")) || Type.S() == HString(HString("int4")))
	{
		int	iChannel = Channels.I();
		switch (Channels.I())
		{
		case 1:
		{

			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			//HImage from(hObject);
			//from = from.ScaleImageMax();
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			QImage img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
			return img;
		}	break;
		case 3:
		{
			//HImage from(hObject);
			//from = from.ScaleImageMax();
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		case 4:
		{
			//HImage from(hObject);
			//from = from.ScaleImageMax();
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);
			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			QImage	to = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
			return to;
		}	break;
		default:
		{

		}	break;
		}
	}
	return QImage();
}

void ImgGraphicsViews::HObject2ToQImage(const HalconCpp::HObject &hobj, QImage& image) //Halcon中的HObject类型转QImage类型
{
	if (	!hobj.IsInitialized()	){	image = QImage();return ;	}

	HTuple channel, Type;
	HalconCpp::CountChannels(hobj,	&channel);
	HalconCpp::GetImageType(hobj,	&Type);
	int m_iChanal = 0;

	if (Type.S() == HString(HString("byte"))){
		switch (channel.I()) {	//将Halcon转换为QImage
		
		case 1:{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);
			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)
				image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++){
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}
		}	break;
		case 3:{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				hSelectChannal = hobj;
			else{
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
				return;
			}
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_RGB32);
			for (int row = 0; row < height; row++){
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
			return ;
		}	break;
		case 4:{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				hSelectChannal = hobj;
			else{
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
				return;
			}
			HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_ARGB32);
			for (int row = 0; row < height; row++){
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
			return;
		}	break;
		default:{		}	break;
		}
	}
	else if (Type.S() == HString(HString("real")) || Type.S() == HString(HString("uint2")) || Type.S() == HString(HString("int4")))		//用来显示相关的参数
	{
		switch (channel.I()){
		case 1:
		{
			HObject	_Object;
			scale_image_range(hobj, &_Object);
			HImage	himg(_Object);

			//HImage	himg(hobj);
			Hlong	width;
			Hlong	height;
			HString type;
			
			//himg	= himg.ScaleImageMax();
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);

			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)	image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++)
			{
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}
			//return image;
		}	break;
		case 2:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);
			HObject	_Object;
			scale_image_range(hSelectChannal, &_Object);
			HImage	himg(_Object);

			//HImage	himg(hSelectChannal);
			Hlong	width;
			Hlong	height;
			HString type;
			//himg = himg.ScaleImageMax();
			void*	ptr = himg.GetImagePointer1(&type, &width, &height);

			image = QImage(width, height, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)
				image.setColor(i, qRgb(i, i, i));
			uchar* pSrc = static_cast<UCHAR*>(ptr);
			for (int row = 0; row < height; row++)
			{
				uchar* pDest = image.scanLine(row);
				memcpy(pDest, pSrc, width);
				pSrc += width;
			}

		}	break;
		default:
		{
			HObject	hSelectChannal;
			if (m_iChanal == 0)
				AccessChannel(hobj, &hSelectChannal, m_iChanal + 1);
			else
				AccessChannel(hobj, &hSelectChannal, m_iChanal);

			HObject	_Object;
			scale_image_range(hSelectChannal, &_Object);
			HImage	himg(_Object);

			//HImage	himg(hSelectChannal);

			Hlong	width;
			Hlong	height;
			HString type;
			uchar*	PointerRed;
			uchar*	PointerGreen;
			uchar*	PointerBlue;
			//himg = himg.ScaleImageMax();
			himg.GetImagePointer3(reinterpret_cast<void**>(&PointerRed), reinterpret_cast<void**>(&PointerGreen), reinterpret_cast<void**>(&PointerBlue), &type, &width, &height);
			image = QImage(width, height, QImage::Format_RGB32);
			for (int row = 0; row < height; row++)
			{
				QRgb*line = reinterpret_cast<QRgb*>(image.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*PointerRed++, *PointerGreen++, *PointerBlue++);
			}
		}	break;
		}
	}
}

bool ImgGraphicsViews::event(QEvent * ev)
{
	if (ev->type() == MsgEvent::EventType)	{
		MsgEvent* MyEvent = (MsgEvent*)ev;
		if (MyEvent != nullptr)	{
			UpdateImg();
		}
	}
	return QGraphicsViews::event(ev);
}

void ImgGraphicsViews::HObjectToQImage(const HObject & hObject, QImage & img)
{
	HTuple Type, Channels;
	GetImageType(hObject, &Type);
	CountChannels(hObject, &Channels);

	Hlong  width = 0, height = 0;
	HString	htype;
	if (Type.S() == HString(HString("byte")))	{
		switch (Channels.I())	{
		case 1:	{
			HImage from(hObject);
			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			int bytePerLine = calcBytesPerLine(width, 8);	//图像每行字节对齐
			img = QImage(ptr, width, height, bytePerLine, QImage::Format_Grayscale8);//不知道是否已自动4字节对齐
			BYTE *data8 = img.bits();
			int pix = 0;

			for (int i = 0; i < height; i++)	{
				for (int k = 0; k < width; k++)	{
					pix = bytePerLine * i + k;
					data8[pix + 0] = ptr[width * i + k];
				}
			}
		}	break;
		case 3:	{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			int bytePerLine = (width * 24 + 31) / 8;//图像每行字节对齐
			img = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)	{
				QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
		}	break;
		case 4:	{
			uchar *R, *G, *B;
			HImage from(hObject);
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			img = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)	{
				QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
		}	break;
		default: {

		}	break;
		}
	}
	else if (Type.S() == HString(HString("real")))	{
		int	iChannel = Channels.I();
		switch (Channels.I())	{
		case 1:	{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);

			HTuple hv_pointer;
			uchar *ptr = (uchar*)from.GetImagePointer1(&htype, &width, &height);
			int bytePerLine = (width * 24 + 31) / 8;//图像每行字节对齐
			img = QImage(ptr, width, height, width, QImage::Format_Indexed8);//不知道是否已自动4字节对齐
		}	break;
		case 3:	{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);

			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);

			img = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)	{
				QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
		}	break;
		case 4:	{
			HObject	_Object;
			scale_image_range(hObject, &_Object);
			HImage	from(_Object);

			uchar *R, *G, *B;
			from.GetImagePointer3(reinterpret_cast<void **>(&R), reinterpret_cast<void **>(&G),
				reinterpret_cast<void **>(&B), &htype, &width, &height);
			img = QImage(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB32);
			for (int row = 0; row < height; row++)	{
				QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(row));
				for (int col = 0; col < width; col++)
					line[col] = qRgb(*R++, *G++, *B++);
			}
		}	break;
		default:	{

		}	break;
		}
	}
}

void ImgGraphicsViews::ClearShowAllImage()
{
	ClearImgShow();
	ClearAllDrawItem();
	QApplication::postEvent(this, new MsgEvent(0, 0, ""));
}

void ImgGraphicsViews::CloneParams(MiddleParam & params)
{
	m_iShowIndex	= params.m_iIndex;
	QElapsedTimer _dDurtime;
	_dDurtime.start();
	ClearAllDrawItem();
	//m_ShowLock.lock();
	QVector<sDrawCross> _PointLst;
	for (auto& iter : params.MapDrawCross) {_PointLst.push_back(iter.second);}
	DispCrossPoint(_PointLst);
	{	QVector<QPointF> _showPointLst;
		for (auto& iter : params.VecShowQPointFs) {_showPointLst.push_back(iter);}
		DispPoint(_showPointLst);
	}
	{	QVector<sColorPoint> _showPointLst;
		for (auto& iter : params.VecShowColorPoints) {_showPointLst.push_back(iter);}
		DispColorPoints(_showPointLst);
	}
	QVector<sDrawText>	_VecText1D;
	int iIndex = 0;
	for (auto& iter : params.VecDrawText){_VecText1D.push_back(iter);}
	DispTextList(_VecText1D);
	{
		QVector<sColorLineRect> _list;
		for (auto& iter : params.VecShowColorLineRect) { _list.push_back(iter); }
		DispLineRectPoint(_list);
	}	{
		QVector<sColorRRectangle> _list;
		for (auto& iter : params.VecShowColorRRect) { _list.push_back(iter); }
		DispRRectanglePoint(_list);
	} {
		QVector<sColorRRectangle> _list;
		for (auto& iter : params.VecShowColorEllipse) { _list.push_back(iter); }
		DispREllipsePoint(_list);
	} {
		QVector<sColorLine> _list;
		for (auto& iter : params.VecShowColorLine) { _list.push_back(iter); }
		DispLinePoint(_list);
	} {
		QVector<sColorCircle> _list;
		for (auto& iter : params.VecShowColorCircle) { _list.push_back(iter); }
		DispColorCircle(_list);
	}
	ClearImgShow();
	bool _bShow = false;
	for (auto& iter : params.VecShowPImg)	{
		if (params.MapPImgVoid.count(iter))	{
			HObject* _ptrImg = ((HObject*)params.MapPImgVoid[iter]);
			if (_ptrImg == nullptr)			continue;
			if (!_ptrImg->IsInitialized())	continue;
			m_strShowImg	= iter;
			_bShow = true;
			QImage _img;
			HObject2ToQImage(*_ptrImg, _img);
			DispImage(_img, false);				//这东西这么耗时不能用界面刷啊要不然很卡啊,4相机就顶不住了
			QApplication::postEvent(this, new MsgEvent(0, 0, ""));
		}
	}
	if (params.VecShowPImg.size() <= 0)	{QApplication::postEvent(this, new MsgEvent(0, 0, ""));}
	if(!_bShow) { QApplication::postEvent(this, new MsgEvent(0, 0, "")); }
	//m_ShowLock.unlock();
}

void ImgGraphicsViews::slot_MouseMoveInImg(QPoint pos, QString & imgtype, QString & strInfo)
{
	try
	{
		if (MiddleParam::MapMiddle().size() > m_iShowIndex) {
			if (MiddleParam::MapMiddle()[m_iShowIndex].MapPImgVoid.count(m_strShowImg) > 0) {
				HObject _dstImage = *((HObject*)MiddleParam::MapMiddle()[m_iShowIndex].MapPImgVoid[m_strShowImg]);
				if (_dstImage.IsInitialized()) {
					HImage from(_dstImage);
					HTuple gray;
					if (pos.y() < from.Height() && pos.x() < from.Width()){
						GetGrayval(_dstImage, pos.y(), pos.x(), &gray);
					}
					int iChannal = from.CountChannels().I();
					if (iChannal == 1 && gray.Length() > 0 )
						strInfo = QString("Gray:%1")
						.arg(QString::number(gray.D(), 'f', databaseVar::Instance().form_System_Precision));
					else if (iChannal == 2 && gray.Length() > 1)
						strInfo = QString("Gray:%1,%2")
						.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision));
					else if (iChannal == 3 && gray.Length() > 2)
						strInfo = QString("R:%1,G:%2,B:%3")
						.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(gray[2].D(), 'f', databaseVar::Instance().form_System_Precision));
					else if (iChannal == 4 && gray.Length() > 3)
						strInfo = QString("R:%1,G:%2,B:%3")
						.arg(QString::number(gray[0].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(gray[1].D(), 'f', databaseVar::Instance().form_System_Precision))
						.arg(QString::number(gray[2].D(), 'f', databaseVar::Instance().form_System_Precision));
				}
			}
		}
	}
	catch (const std::exception& ex) {
		qCritical() << ex.what();
	}
	catch (const HException& except) {
		QString strError = except.ErrorText();
		QStringList strErrorArray = strError.split(':');
		if (strErrorArray.size() > 1) {
			if (!strErrorArray[1].isEmpty()) {
				strError.clear();
				for (int i = 1; i < strErrorArray.size(); i++)	strError.append(strErrorArray[i]);
				qCritical() << QObject::tr("软件发生异常") << QObject::tr("图形显示") << strError;
			}
		}
	}
	catch (...) {
		qCritical() << QObject::tr("软件异常");
	}

}
