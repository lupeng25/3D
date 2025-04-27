#pragma once

#ifndef _ThreadRun_EXPORT_
#define _ThreadRun_API_ _declspec(dllexport)
#else
#define _ThreadRun_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

enum EnumStackStatus
{
	StackStatus_Disable = -4,
	StackStatus_TempSelectProduct = -3,		//零时选中
	StackStatus_SelectProduct = -2,			//零时选中
	StackStatus_OrderSelectProduct = -1,	//订单选中选中
	StackStatus_NotHaveProduct,				//没有产品
	StackStatus_HaveProduct,				//有产品
	StackStatus_Alarm,						//其他
	StackStatus_Other,						//其他
	StackStatus_OutPut,						//其他
};

struct RowID
{
	int iAddress;
	bool bConnect;
	QVector<int>	vecIOInPutStatus;
	QVector<int>	vecIOOutPutStatus;
	QVector<int>	vecTempStatus;
	QVector<int>	vecDataBaseStatus;
	QVector<int>	vecIOChaneStatus;
	QVector<int>	vecTempChaneStatus;
	QVector<int>	vecTempOutUpFlashStatus;	//上升沿
	//QVector<int>	vecIOFlashstatus;
	RowID()
	{
		bConnect = true;
		vecIOInPutStatus.clear();
		vecIOOutPutStatus.clear();
		vecTempStatus.clear();
		vecDataBaseStatus.clear();
		vecIOChaneStatus.clear();
		vecTempChaneStatus.clear();
		vecTempOutUpFlashStatus.clear();
		//vecIOFlashstatus.clear();

	}
};

class _ThreadRun_API_ ThreadRun
{
public:
	virtual int AutoRun() = 0;
	virtual int AutoUIRun() = 0;

};
