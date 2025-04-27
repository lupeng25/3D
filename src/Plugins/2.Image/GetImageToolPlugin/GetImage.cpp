#include "GetImage.h"
#include <qdebug.h>
#include <QDir>
//#include "frmImageSource.h"
#include "frmImageGetSource.h"
#include "databaseVar.h"
#include "PluginsManager.h"
#include "CameraManager.h"

GetImage::GetImage() :FlowchartTool()
	, m_PtrCamera(nullptr)
	, m_eOutPutImg(eImgType_HeightAndGray)
	, m_eRetn(eImgType_None)
{
	AddNodePort(new Port(0, "",	Port::InStream,		Port::Stream));
	AddNodePort(new Port(0, "", Port::OutStream,	Port::Stream));
	m_hTrrigerEvent.RstEvent();
}

GetImage::~GetImage()
{
	DeleteNodePort(0, Port::InStream);
	DeleteNodePort(0, Port::OutStream);
	DeleteNodePort(1, Port::Output);
	Clearport();
}

int GetImage::SetData(QJsonObject & strData)
{
	FlowchartTool::SetData(strData);

	QJsonArray array_object = strData.find("ConFig").value().toArray();
	if (!array_object.empty())
	{
		m_iPathType		= array_object.at(0).toString().toInt();
		m_strCamera		= array_object.at(1).toString();
		m_strImgPath	= array_object.at(2).toString();
		m_strFolderPath = array_object.at(3).toString();
		m_iTimeOut		= array_object.at(4).toString().toInt();
		m_eOutPutImg	= (eImgType)array_object.at(5).toString().toInt();
	}

	return 0;
}

int GetImage::GetData(QJsonObject & strData)
{
	FlowchartTool::GetData(strData);

	//scale
	QJsonArray ConFig = {
		QString::number(m_iPathType),
		m_strCamera,
		m_strImgPath,
		m_strFolderPath,
		QString::number(m_iTimeOut),
		QString::number(m_eOutPutImg),

	};
	strData.insert("ConFig", ConFig);

	return 0;
}

//将图像转化为灰度和高度	//返回-1为正常 -2为不支持
eImgType GetImage::SplitImg(HObject & Image, HObject & hGray, HObject & hHeight)
{
	hGray.Clear();
	hHeight.Clear();
	HTuple	hType, hChannel;
	HalconCpp::GetImageType(Image, &hType);
	HalconCpp::CountChannels(Image, &hChannel);
	if (hType.S() == ImgType_Byte)
	{
		switch (hChannel.I())
		{
		default:
		case 1:
		case 2:
		{	
			hGray.GenEmptyObj();
			hGray = Image;
			return eImgType_Gray;
		} break;
		}
	}
	else if (hType.S() == ImgType_Int1)
	{
		switch (hChannel.I())
		{
		case 1:
		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			return eImgType_Height;
		} break;
		case 2:
		{			
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg,	1);
			AccessChannel(Image, &hGrayimg,		2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg,	&hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hHeightimg;
				hHeight.GenEmptyObj();
				hHeight = hGrayimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else {	return eImgType_NotOpen;	}
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if(hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight	= false;
				bool _bGray		= false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized())	{	Union2(hHeight, hImg2, &hHeight);	}
					else {	hHeight = Image;	}
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); 	}
					else { hGray = Image; 	}
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image;	}
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); 	}
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight)	{	return eImgType_HeightAndGray;	}
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); 	}
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else if (hType.S() == ImgType_Int2)
	{
		switch (hChannel.I()){
		case 1:		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			return eImgType_Height;
		} break;
		case 2:
		{
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg, 1);
			AccessChannel(Image, &hGrayimg, 2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg, &hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hHeightimg;
				hHeight.GenEmptyObj();
				hHeight = hGrayimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else if (hType.S() == ImgType_Int4)
	{
		switch (hChannel.I())
		{
		case 1:
		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			return eImgType_Height;
		} break;
		case 2:
		{
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg, 1);
			AccessChannel(Image, &hGrayimg, 2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg, &hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hHeightimg;
				hHeight.GenEmptyObj();
				hHeight = hGrayimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else if (hType.S() == ImgType_Int8)
	{
		switch (hChannel.I())
		{
		case 1:
		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			return eImgType_Height;
		} break;
		case 2:
		{
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg, 1);
			AccessChannel(Image, &hGrayimg, 2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg, &hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hHeightimg;
				hHeight.GenEmptyObj();
				hHeight = hGrayimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else if (hType.S() == ImgType_uInt2)
	{
		switch (hChannel.I())
		{
		case 1:
		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			return eImgType_Height;
		} break;
		case 2:
		{
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg, 1);
			AccessChannel(Image, &hGrayimg, 2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg, &hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else if (hType.S() == ImgType_Real)
	{
		switch (hChannel.I())
		{
		case 1:
		{
			hHeight.GenEmptyObj();
			hHeight = Image;
			//HalconCpp::CopyImage(Image, &hHeight);
			//HObject _Img;
			//scale_image_range(hHeight, &_Img);
			//WriteImage(_Img,"tiff",0,"d:\\12312");
			return eImgType_Height;
		} break;
		case 2:
		{
			HTuple hHeightType;
			HTuple hGrayType;
			HObject	hHeightimg;
			HObject	hGrayimg;
			AccessChannel(Image, &hHeightimg, 1);
			AccessChannel(Image, &hGrayimg, 2);
			HalconCpp::GetImageType(hHeightimg, &hHeightType);
			HalconCpp::GetImageType(hGrayimg, &hGrayType);
			if (hHeightType.S() == hType.S() && hGrayType.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hHeightType.S() == ImgType_Byte && hGrayType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hHeightimg;
				hHeight.GenEmptyObj();
				hHeight = hGrayimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == hType.S())
			{
				hGray.GenEmptyObj();
				hGray = hGrayimg;
				hHeight.GenEmptyObj();
				hHeight = hHeightimg;
				return eImgType_HeightAndGray;
			}
			else if (hGrayType.S() == ImgType_Byte && hHeightType.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 3:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					_bGray = true;
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		case 4:
		{
			HTuple hImgType1;
			HTuple hImgType2;
			HTuple hImgType3;
			HTuple hImgType4;
			HObject	hImg1;
			HObject	hImg2;
			HObject	hImg3;
			HObject	hImg4;
			AccessChannel(Image, &hImg1, 1);
			AccessChannel(Image, &hImg2, 2);
			AccessChannel(Image, &hImg3, 3);
			AccessChannel(Image, &hImg4, 4);
			HalconCpp::GetImageType(hImg1, &hImgType1);
			HalconCpp::GetImageType(hImg2, &hImgType2);
			HalconCpp::GetImageType(hImg3, &hImgType3);
			HalconCpp::GetImageType(hImg4, &hImgType4);
			if (hImgType1.S() == hType.S() && hImgType2.S() == hType.S() && hImgType3.S() == hType.S() && hImgType4.S() == hType.S())
			{
				hHeight.GenEmptyObj();
				hHeight = Image;
				return eImgType_Height;
			}
			else if (hImgType1.S() == ImgType_Byte && hImgType2.S() == ImgType_Byte && hImgType3.S() == ImgType_Byte && hImgType4.S() == ImgType_Byte)
			{
				hGray.GenEmptyObj();
				hGray = Image;
				return eImgType_Gray;
			}
			else if (hImgType1.S() == hType.S() || hImgType2.S() == hType.S() || hImgType3.S() == hType.S() || hImgType4.S() == hType.S())
			{
				bool _bHeight = false;
				bool _bGray = false;
				if (hImgType1.S() == hType.S())
				{
					hHeight.GenEmptyObj();
					hHeight = Image;
					_bHeight = true;
				}
				else
				{
					hGray.GenEmptyObj();
					hGray = Image;
					_bGray = true;
				}
				if (hImgType2.S() == hType.S())
				{
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
					_bHeight = true;
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType3.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (hImgType4.S() == hType.S())
				{
					_bHeight = true;
					if (hHeight.IsInitialized()) { Union2(hHeight, hImg2, &hHeight); }
					else { hHeight = Image; }
				}
				else
				{
					if (hGray.IsInitialized()) { Union2(hGray, hImg2, &hGray); }
					else { hGray = Image; }
					_bGray = true;
				}
				if (_bGray && _bHeight) { return eImgType_HeightAndGray; }
				if (_bGray) { return eImgType_Gray; }
				if (_bHeight) { return eImgType_Height; }
				return eImgType_Error;
			}
			else { return eImgType_NotOpen; }
		} break;
		}
	}
	else return eImgType_Error;
	return eImgType_Gray;
}

void GetImage::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{	
	frmImageGetSource * frm = new frmImageGetSource();
	frm->m_pTool = this;
	frm->Load();
	databaseVar::Instance().CenterMainWindow(frm);
	frm->exec();
	frm->m_pTool = nullptr;
	delete frm;
}

int GetImage::RecieveBuffer(QString strCamName,unsigned char* gray, int* IntensityData, int iwidth, int iheight, int ibit, float fscale, QString graytype, int icount)
{
	HTuple Type, Width, Height, PixelPointer;
	//m_hGrayImage.GenEmptyObj();
	//m_hHeightImage.GenEmptyObj();

	if (graytype == ImgType_Byte)
	{
		Type		= (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize	= iwidth * iheight;
			BYTE* BImg		= new BYTE[imgSize];
			BYTE* GImg		= new BYTE[imgSize];
			BYTE* RImg		= new BYTE[imgSize];
			size_t	Pixed	= 0;
			for (size_t i	= 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Real.toLocal8Bit().data(),iwidth,iheight,Hlong(IntensityData));

			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (graytype == ImgType_Int1)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			BYTE* BImg = new BYTE[imgSize];
			BYTE* GImg = new BYTE[imgSize];
			BYTE* RImg = new BYTE[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int1.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}

	}
	else if (graytype == ImgType_Int2)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			BYTE* BImg = new BYTE[imgSize];
			BYTE* GImg = new BYTE[imgSize];
			BYTE* RImg = new BYTE[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int2.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (graytype == ImgType_Int4)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			BYTE* BImg = new BYTE[imgSize];
			BYTE* GImg = new BYTE[imgSize];
			BYTE* RImg = new BYTE[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int4.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (graytype == ImgType_Int8)
	{
		Type = (HTuple)ImgType_Int8.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			BYTE* BImg = new BYTE[imgSize];
			BYTE* GImg = new BYTE[imgSize];
			BYTE* RImg = new BYTE[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int8.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (graytype == ImgType_uInt2)   //基恩士相机用数据
	{
		Type = (HTuple)ImgType_uInt2.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			WORD* BImg = new WORD[imgSize];
			WORD* GImg = new WORD[imgSize];
			WORD* RImg = new WORD[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_uInt2.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (graytype == ImgType_Real)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(gray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			BYTE* BImg = new BYTE[imgSize];
			BYTE* GImg = new BYTE[imgSize];
			BYTE* RImg = new BYTE[imgSize];
			size_t	Pixed = 0;
			for (size_t i = 0; i < iheight; i++)
			{
				size_t _Lenth = i * iwidth;
				for (size_t j = 0; j < iwidth; j++)
				{
					Pixed = j * 3 + _Lenth * 3;
					BImg[j + _Lenth] = gray[Pixed + 0];
					GImg[j + _Lenth] = gray[Pixed + 1];
					RImg[j + _Lenth] = gray[Pixed + 2];
				}
			}
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)RImg, (Hlong)GImg, (Hlong)BImg);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Real.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else
	{
		m_eRetn = eImgType_Error;
		m_hTrrigerEvent.SetEvent();
		return -1;
	}
	m_hTrrigerEvent.SetEvent();
	return 0;
}

int GetImage::RecieveRGBBuffer(QString strCamName, unsigned char * Rgray, unsigned char * Ggray, unsigned char * Bgray, int * IntensityData, int iwidth, int iheight, int ibit, float fscale, QString imgtype, int icount)
{
	HTuple Type, Width, Height, PixelPointer;
	//m_hGrayImage.GenEmptyObj();
	m_hHeightImage.Clear();

	if (imgtype == ImgType_Byte)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Real.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (imgtype == ImgType_Int1)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int1.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (imgtype == ImgType_Int2)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int2.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (imgtype == ImgType_Int4)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int4.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (imgtype == ImgType_Int8)
	{
		Type = (HTuple)ImgType_Byte.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Int8.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else if (imgtype == ImgType_Real)
	{
		Type = (HTuple)ImgType_Real.toLocal8Bit().data();
		if (ibit == 8)
		{
			HalconCpp::GenImage1(&m_hGrayImage, Type, iwidth, iheight, Hlong(Rgray));
		}
		else if (ibit == 24)
		{
			long imgSize = iwidth * iheight;
			GenImage3(&m_hGrayImage, Type, iwidth, iheight, (Hlong)Rgray, (Hlong)Ggray, (Hlong)Bgray);
		}
		if (IntensityData != nullptr)
		{
			m_hHeightImage.GenEmptyObj();
			HalconCpp::GenImage1(&m_hHeightImage, (HTuple)ImgType_Real.toLocal8Bit().data(), iwidth, iheight, Hlong(IntensityData));
			m_eRetn = eImgType_HeightAndGray;
		}
		else
		{
			m_eRetn = eImgType_Gray;
		}
	}
	else
	{
		m_eRetn = eImgType_Error;
		m_hTrrigerEvent.SetEvent();
		return -1;
	}
	m_hTrrigerEvent.SetEvent();
	return 0;
}

EnumNodeResult GetImage::InitBeforeRun(MiddleParam& param, QString& strError)
{
	m_eRetn = eImgType_Gray;
	QString _strImgPath;
	switch (m_iPathType){
	case 0: {	//本地图片
		m_hGrayImage.Clear();
		m_hHeightImage.Clear();
		if (m_strImgPath.isEmpty()) return NodeResult_Error;
		_strImgPath		= m_strImgPath;

		HObject _HObject;
		HalconCpp::ReadImage(&_HObject, _strImgPath.toLocal8Bit().toStdString().data());
		m_eRetn			= SplitImg(_HObject, m_hGrayImage, m_hHeightImage);
	}	break;
	case 1: {		//本地路径图片
		QDir dir(m_strFolderPath);
		if (!dir.exists()) { return NodeResult_Error; }
		dir.setFilter(QDir::Files);
		dir.setSorting(QDir::Name);
		dir.setNameFilters(QString("*.jpg;*.png;*.jpeg;*.bmp;*.tif;*.tiff").split(";"));
		m_strImgPaths = dir.entryList();
		if (m_strImgPaths.size() > 0){
			if (m_iSelectedPathIndex >= m_strImgPaths.size())	m_iSelectedPathIndex = 0;
			_strImgPath = m_strImgPaths[m_iSelectedPathIndex].toLocal8Bit();
		}
	}	break;
	case 2: {	//相机取图
		if(m_strCamera.isEmpty()) { 
			return NodeResult_Error; 
		}
		Camerainterface*_ptrCamera	= CameraManager::Instance().FindCamera(m_strCamera);
		if (_ptrCamera == nullptr)	if (!PluginsManager::Instance().FindCamera(m_strCamera))	{	return NodeResult_Error; }
		if (_ptrCamera == nullptr)	{ 
			_ptrCamera = PluginsManager::Instance().InitCamera(m_strCamera); 
			if (_ptrCamera != nullptr) _ptrCamera->SetStream(this); 
		}
		if (_ptrCamera == nullptr)	{ return NodeResult_Error; }
		if (!_ptrCamera->IsCamConnect()) {
			_ptrCamera->Init();
			_ptrCamera->Start();
			_ptrCamera->StartVedioLive();
		}
		if (!_ptrCamera->IsCamConnect()) { 
			return NodeResult_Error;
		}
		//设置相机参数
		for (auto iter = m_mapParam.begin(); iter != m_mapParam.end(); iter++)
			_ptrCamera->SetCameraParam(iter.key(), iter.value());
		m_PtrCamera = _ptrCamera;
	}	break;
	default: {
	}	break;
	}

	switch (m_eRetn)
	{
	case eImgType_Gray: {	//灰度图
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Gray",	&m_hGrayImage));
	} break;
	case eImgType_Height: {	//高度图
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Height", &m_hHeightImage));
	} break;
	case eImgType_HeightAndGray: {	//高度图 + 灰度图
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Gray",	&m_hGrayImage));
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Height", &m_hHeightImage));
	} break;
	default:	{
		SetError(QString("Error !"));
		return NodeResult_Error;
	}break;
	}
	return NodeResult_OK;
}

EnumNodeResult GetImage::PrExecute(MiddleParam & param, QString& strError)
{
	QString _strImgPath;
	switch (m_iPathType)
	{
	case 0: {	//本地图片
		if (m_strImgPath.isEmpty()) {
			m_hTrrigerEvent.RstEvent(); 
			return NodeResult_Error;
		} 
	}	break;
	case 1: {	//本地路径图片
		m_eRetn = eImgType_Gray;
		//m_hGrayImage.GenEmptyObj();
		//m_hHeightImage.GenEmptyObj();
	}	break;
	case 2:		//相机取图
	{
		m_eRetn = eImgType_Gray;
		//m_hGrayImage.GenEmptyObj();
		//m_hHeightImage.GenEmptyObj();
		if (m_strCamera.isEmpty()) {
			m_hTrrigerEvent.RstEvent();
			return NodeResult_Error;
		}
	}	break;
	default:
		break;
	}
	m_hTrrigerEvent.RstEvent();
	return NodeResult_OK;
}

EnumNodeResult GetImage::Execute(MiddleParam& param, QString& strError)
{
	QString _strImgPath;
	switch (m_iPathType){
	case 0:{
		_strImgPath = m_strImgPath;
		if (_strImgPath.isEmpty())	{
			SetError(QString("File %1 is Empty").arg(m_strImgPath));
			return NodeResult_Error;
		}
	}	break;
	case 1: {
		m_hGrayImage.Clear();
		m_hHeightImage.Clear();
		if (m_strImgPaths.size() > 0)	{
			if (m_iSelectedPathIndex >= m_strImgPaths.size())
				m_iSelectedPathIndex = 0;
			_strImgPath = m_strFolderPath + "/" + m_strImgPaths[m_iSelectedPathIndex];
			HObject _HObject;
			HalconCpp::ReadImage(&_HObject, _strImgPath.toLocal8Bit().toStdString().data());
			m_eRetn = SplitImg(_HObject, m_hGrayImage, m_hHeightImage);
			m_iSelectedPathIndex++;
		}
		else { SetError(QString("Folder %1 is Empty").arg(m_strFolderPath)); return NodeResult_Error; }
	}	break;
	case 2:{		
		m_hGrayImage.Clear();
		m_hHeightImage.Clear();
		if (m_PtrCamera == nullptr) {
			Camerainterface*_ptrCamera = CameraManager::Instance().FindCamera(m_strCamera);
			if (_ptrCamera == nullptr) { _ptrCamera = PluginsManager::Instance().InitCamera(m_strCamera); if (_ptrCamera != nullptr) 	_ptrCamera->SetStream(this);	}
			m_PtrCamera = _ptrCamera;
		}
		if (m_PtrCamera == nullptr) { SetError("Didn't Find Camera " + m_strCamera); return NodeResult_Error; }
		if (!m_PtrCamera->IsCamConnect()) {
			m_PtrCamera->Init();
			m_PtrCamera->Start();
			m_PtrCamera->StartVedioLive();
		}
		if (!m_PtrCamera->IsCamConnect()) { SetError(QString("Camera %1 Didn't Connected").arg(m_strCamera)); return NodeResult_Error; }

		//设置相机参数
		for (auto iter = m_mapParam.begin(); iter != m_mapParam.end(); iter++)
			m_PtrCamera->SetCameraParam(iter.key(),	iter.value());

		m_PtrCamera->Trigger();
		if(m_hTrrigerEvent.WaitEvent(m_iTimeOut) == m_hTrrigerEvent.EventRetnTimeOut)	{ SetError(QString("Camera %1 Trriger TimeOut").arg(m_strCamera)); return NodeResult_TimeOut; }
		if (FlowchartGraphicsItem::m_bVisionThreadNeedStop) {	return NodeResult_Return;	}
		//将BYTE转化为Halcon变量
	}	break;
	default:
		break;
	}
	switch (m_eRetn){
	case eImgType_Gray:{	param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + ".Gray",	&m_hGrayImage));} break;
	case eImgType_Height:{	param.MapPImgVoid.insert(std::pair<QString, void*>(GetItemId() + ".Height", &m_hHeightImage));} break;
	case eImgType_HeightAndGray:{
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Gray", &m_hGrayImage));
		param.MapPImgVoid.insert(std::pair<QString, void*>( GetItemId() + ".Height", &m_hHeightImage));
	} break;
	default: {	SetError(QString("Error !"));	return NodeResult_Error;	}break;
	}

	switch (m_eOutPutImg){
	case eImgType_NotOpen:	break;
	case eImgType_Error:	break;
	case eImgType_Gray:			{	param.VecShowPImg.push_back( GetItemId() + ".Gray");}	break;
	case eImgType_Height:		{	param.VecShowPImg.push_back( GetItemId() + ".Height");}	break;
	case eImgType_HeightAndGray:{
		param.VecShowPImg.push_back( GetItemId() + ".Gray");
		param.VecShowPImg.push_back( GetItemId() + ".Height");
	}	break;
	default:
		break;
	}

	return NodeResult_OK;
}

void GetImage::StopExecute()
{
	switch (m_iPathType)
	{
	case 2:
	{
		m_hTrrigerEvent.SetEvent();
	}	break;
	}
}

QPair<QString, QString> GetImage::GetNodeTypeName()
{
	return GetToolName();
}

QPair<QString, QString> GetImage::GetToolName()
{
	return QPair<QString, QString>("GetImage",tr("GetImage"));
	//return QString::fromLocal8Bit("GetImage");
}

EnumNodeResult GetImage::InitCamera(QString came, Camerainterface *& ptrcam)
{
	if (came.isEmpty()) { return NodeResult_Error; }
	if (!PluginsManager::Instance().FindCamera(came)) { return NodeResult_Error; }
	Camerainterface*_ptrCamera = CameraManager::Instance().FindCamera(came);
	if (_ptrCamera == nullptr) { _ptrCamera = PluginsManager::Instance().InitCamera(came); if (_ptrCamera != nullptr) _ptrCamera->SetStream(this); }
	if (_ptrCamera == nullptr) { return NodeResult_Error; }
	if (!_ptrCamera->IsCamConnect()) {
		_ptrCamera->SetCameraParam(CameraName,came);
		_ptrCamera->Init();
		_ptrCamera->Start();
		_ptrCamera->StartVedioLive();
	}
	if (!_ptrCamera->IsCamConnect()) { return NodeResult_Error; }
	ptrcam = _ptrCamera;
	for (auto iter = m_mapParam.begin(); iter != m_mapParam.end(); iter++)
		_ptrCamera->SetCameraParam(iter.key(), iter.value());
	return NodeResult_OK;
}

