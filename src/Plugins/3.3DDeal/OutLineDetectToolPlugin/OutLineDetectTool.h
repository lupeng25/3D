#pragma once
#ifndef OutLineDetectTool_H
#define OutLineDetectTool_H

//#include "Common.h"
#include "FlowchartTool.h"
#include "HalconCpp.h"
using namespace HalconCpp;

class CheckBase;
class CheckResult;
class CheckManager;

struct CheckString
{
	QString strName;
	QString strData;
};

struct CheckOutLineData
{
	QString strName;
	QString strData;
	QVector<CheckString> vecData;
};

enum EnumCheckResult
{
	CheckResult_Error = -2,
	CheckResult_NG = -1,
	CheckResult_NoCheck,		//不检测
	CheckResult_OK,
	CheckResult_OverUp,			//超上限
	CheckResult_OverDown,		//超下限
	CheckResult_ParamError,		//参数错误
};
enum EnumErrorRetn
{
	ErrorRetn_NotContain = -3,
	ErrorRetn_Error = -2,
	ErrorRetn_NG = -1,
	ErrorRetn_OK,
	ErrorRetn_Continue,
};
enum EnumCheckType
{
	ECheckType_Point,		//单点
	ECheckType_PPCenter,
	ECheckType_LLCrossPoint,
	//直线
	ECheckType_Line,
	ECheckType_HLine,
	ECheckType_Circle,
	ECheckType_PPLine,

	ECheckType_ChuiLine,
	ECheckType_ChuiPoint,
	ECheckType_PCircleCrossPoint,
	ECheckType_Height,
	ECheckType_Width,

	ECheckType_PPLenth,
	ECheckType_PLineLenth,
	ECheckType_LineLineAngle,
};
class CheckResult
{
public:
	CheckResult();
	~CheckResult();
public:
	QMap<QString, QPointF>	mapResultPos;
	QMap<QString, QLineF>	mapResultLineF;
	QMap<QString, MCircle>	mapResultMCircle;
	QMap<QString, double>	mapResultValue;
public:
	void ClearALL();
};
#pragma region 测量基类
class CheckBase:public QObject
{
public:
	CheckBase(QString name, EnumCheckType type, CheckManager* pThis = nullptr);
	~CheckBase();
public:
	virtual QString	GetTypeName();
	static int ChangeParamToRegion(CommonData &vecData,HObject& region);
public:
	QString GetCheckName();
	void SetCheckName(QString name);

	EnumCheckType GetCheckType();
	void SetCheckType(EnumCheckType type);

	QString GetCheckData();
	void SetCheckData(QString data);
public:
	virtual QString GetCheckResult();
public:
	static int GetCheckDataText(QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams, QString & strText);
	static int SetCheckDataText(QString&strText, QStringList& strParamArray, bool& iIsLimit, int& iRegionType, QStringList& strLimitArray, QStringList&strLimitValues, QStringList&strLimitParams);
	static int CheckImgContourPoint(HTuple&Grayval, int iCheckType, double xValue, double xRange, QPointF& pt);
	static int CheckImgContourLine(HTuple&Grayval, int iCheckType, double xValue, double xRange, QLineF& line);
	static int CheckImgContourCircle(HTuple&Grayval, int iCheckType, double xValue, double xRange, MCircle& circle);
public:
	static EnumCheckResult CheckMesureResult(double dvalue, QStringList&strLimitParams, QStringList&strVecJudgeValue);
public:
	virtual EnumErrorRetn Excute(int Index,HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
	virtual int PrExcute();
protected:
	QString			strResult;			//检测结果数据
	//**********************************************
	QString			strName;				//检测名称
	EnumCheckType	eType;					//检测类型
	QString			strData;				//检测数据
	QStringList		strVecParam;			//参数
	QStringList		strVecLimit;			//限位
	QStringList		strVecJudgeCondition;	//判定条件
	QStringList		strVecJudgeValue;		//判定值
	bool			iIsLimit;
	int				iRegionType;
	QString			strErrorMsg;				//检测数据
	CheckManager*	pCheckManager;
public:
	EnumCheckResult	eCheckResult;			//检测结果
};
#pragma endregion

#pragma region 创建图形
class CheckPoint :public CheckBase
{
	Q_OBJECT
public:
	CheckPoint(QString name, EnumCheckType type = ECheckType_Point, CheckManager* pThis = nullptr);
	~CheckPoint();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckPPCenter :public CheckBase
{
	Q_OBJECT
public:
	CheckPPCenter(QString name, EnumCheckType type = ECheckType_PPCenter, CheckManager* pThis = nullptr);
	~CheckPPCenter();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckLLCrossPoint :public CheckBase
{
	Q_OBJECT
public:
	CheckLLCrossPoint(QString name, EnumCheckType type = ECheckType_LLCrossPoint, CheckManager* pThis = nullptr);
	~CheckLLCrossPoint();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckLine :public CheckBase
{
	Q_OBJECT
public:
	CheckLine(QString name, EnumCheckType type = ECheckType_Line, CheckManager* pThis = nullptr);
	~CheckLine();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckHLine :public CheckBase
{
	Q_OBJECT
public:
	CheckHLine(QString name, EnumCheckType type = ECheckType_HLine, CheckManager* pThis = nullptr);
	~CheckHLine();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckCircle :public CheckBase
{
	Q_OBJECT
public:
	CheckCircle(QString name, EnumCheckType type = ECheckType_Circle, CheckManager* pThis = nullptr);
	~CheckCircle();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckPPLine :public CheckBase
{
	Q_OBJECT
public:
	CheckPPLine(QString name, EnumCheckType type = ECheckType_PPLine, CheckManager* pThis = nullptr);
	~CheckPPLine();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckChuiLine :public CheckBase
{
	Q_OBJECT
public:
	CheckChuiLine(QString name, EnumCheckType type = ECheckType_ChuiLine, CheckManager* pThis = nullptr);
	~CheckChuiLine();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckChuiPoint :public CheckBase
{
	Q_OBJECT
public:
	CheckChuiPoint(QString name, EnumCheckType type = ECheckType_ChuiPoint, CheckManager* pThis = nullptr);
	~CheckChuiPoint();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
#pragma endregion

#pragma region 测量
class CheckHeight :public CheckBase
{
	Q_OBJECT
public:
	CheckHeight(QString name, EnumCheckType type = ECheckType_Height, CheckManager* pThis = nullptr);
	~CheckHeight();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckWidth :public CheckBase
{
	Q_OBJECT
public:
	CheckWidth(QString name, EnumCheckType type = ECheckType_Width, CheckManager* pThis = nullptr);
	~CheckWidth();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckPPLenth :public CheckBase
{
	Q_OBJECT
public:
	CheckPPLenth(QString name, EnumCheckType type = ECheckType_PPLenth, CheckManager* pThis = nullptr);
	~CheckPPLenth();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckPLineLenth :public CheckBase
{
	Q_OBJECT
public:
	CheckPLineLenth(QString name, EnumCheckType type = ECheckType_PLineLenth, CheckManager* pThis = nullptr);
	~CheckPLineLenth();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
class CheckLineLineAngle :public CheckBase
{
	Q_OBJECT
public:
	CheckLineLineAngle(QString name, EnumCheckType type = ECheckType_LineLineAngle, CheckManager* pThis = nullptr);
	~CheckLineLineAngle();
public:
	EnumErrorRetn Excute(int Index, HTuple&row, HTuple&col, HTuple&Grayval, CheckResult&result, QString& strError);
public:
	QString	GetTypeName();
	static QString TypeName();
};
#pragma endregion

struct CheckOutLine
{
	HObject								hROILine;
	bool								bIsFollow;
	QLineF								line;
	QString								txtLinkFollow;
	QVector<CheckBase*>					vecCheck1D;
	CheckResult							result;
	QMap<EnumCheckType, QVector<int>>	ToolCheck;
	int									iCurrentRow;
};

class CheckManager: public QObject
{
	Q_OBJECT
public:
	CheckManager();
	CheckManager(const CheckManager& manager);
public:
	static CheckManager Instance();
public:
	void ClearAll(int Index = -1);
	int	AddCheckTool(int Index,EnumCheckType type, QString data);
	int	AddCheckTool(int Index,QString type, QString data);
	int DeleteCheckTool(QString name, int Index = -1);
	CheckBase*GetCurrentTool(QString name, int Index = -1);
public:
	static int CheckMetrixType(QString strCom, HTuple&Metrix);
	int LoadData(int Index, QVector<CheckString>&mapData);
	int LoadData(QVector<CheckOutLineData>&mapData);
	QStringList	GetCheckBaseLst(int Index, QString strFilterName, int Currentrow);
public:
	EnumErrorRetn PrExcute(int Index = -1);
	EnumErrorRetn Excute(int Index,bool	bShowRegion,MiddleParam& param,HObject&img, QString& strError);
public:
	QVector<CheckOutLine>							m_vecCheckOutLine;
	QVector<HObject>								m_vecRegion;
	int												m_iCurrentRow;
	CheckResult										m_result;
	QVector<QVector<CheckBase*>>					m_vecCheck2D;
	QMap<int,QMap<EnumCheckType, QVector<int>> >	m_mapMapTools;		//用作创建工具防止重复
};

class OutLineDetectTool :public FlowchartTool
{
	Q_OBJECT
public:
	OutLineDetectTool();
	~OutLineDetectTool();
public:
	virtual	int SetData(QJsonObject &strData);

	virtual	int GetData(QJsonObject &strData);

	void UpdateParam();
public:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
public:
	virtual void StopExecute();

	virtual EnumNodeResult InitBeforeRun(MiddleParam& param, QString& strError);		//节点执行逻辑

	virtual EnumNodeResult PrExecute(MiddleParam& param, QString& strError);			//节点执行逻辑

	virtual EnumNodeResult Execute(MiddleParam& param, QString& strError);				//节点执行逻辑
public:
	QString		m_strLinkName;					//连接内容
	bool		m_bEnableLinkFollow = true;
	bool		m_bEnableShowRegion = true;
	bool		m_bEnableShowData = true;
	int			m_iFontSize = 8;
public:
	QPair<QString, QString> GetNodeTypeName();
public:
	static QPair<QString, QString> GetToolName();
public:
	static QPointF FindFoot(QLineF line, QPointF pA);
	static double GetP2PDistance(QPointF&ptA,QPointF&ptB);
	static double GetP2LDistance(QPointF&ptA, QLineF&line);
	static double GetAngleInLines(const QLineF &inline1, const QLineF &inline2);
	static QList<QPointF> calculateLineRectIntersections(const QLineF &line, const QRectF &rect);	//计算交点
	static int CalculateLineRectIntersections(const QLineF &Inline1, const QRectF &rect,QLineF& Outline);	//计算交点
	static bool IsPointInRect(const QPointF& pt,const QRectF &rect);
	static bool IsInLenthLimit(qreal x1, qreal x2);
public:
	HObject						m_ROISearchRegion;		//将ROI参数转化为形状区域
public:
	QVector<CommonData>			m_vecCommonData;		//公共数据ROI
	CheckManager				m_CheckManager;
	QVector<CheckOutLineData>	m_vecCheckOutLineData;	//用来保存的数据
};
#endif