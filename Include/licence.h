#pragma once
#include <QString>

#ifndef _LICENCE_EXPORT_
#define _LICENCE_API_ _declspec(dllexport)
#else
#define _LICENCE_API_ _declspec(dllimport)
#endif // !_REGISTER_EXPORT_

#define startname                   "B890E289-52B6-4798-9417-9DC9E22CA58A"		//创建的第一次注册表名称
#define endname                     "EB528F9D-5BD0-49E1-B96E-9B14F51E9A66"		//创建的最新注册表名称

enum EnumLicenceRetn
{
	ELicenceRetn_MechineKeyNotRight = -10,	//当前注册不对
	EnumLicenceRetn_AlreadyRegister,		//已注册
	EnumLicenceRetn_RegisterModifyTime,		//注册时间修改

	EnumLicenceRetn_InitModifyTimeOut,		//修改系统时间
	EnumLicenceRetn_InitDayTimeOut,			//注册到期
	EnumLicenceRetn_InitError,				//注册
	EnumLicenceRetn_NotInit ,				//未注册
	EnumLicenceRetn_Permanent_Inited,		//永久注册
	EnumLicenceRetn_Inited,					//注册
};

class _LICENCE_API_ licence
{
public:
	licence();
	~licence();
public:
	QString AES_encryption(const QString &data);
	QString AES_decryption(const QString &data);
public:
	void detection_regrdit();
	void set_regedit(QString keyName, QString regvalue);
	void get_regedit(QString keyName, QString& regvalue);
	void update_regedit();
	bool licence_valid();
	EnumLicenceRetn GetRemainderLicenceDays(double &dRemainderDays);
	QString	GetMechineKey();
	QString	GetRegisterKey(int days);
	EnumLicenceRetn	RegisterKey(QString keys);
	EnumLicenceRetn CheckLicence();

private:
	QString regPath = "HKEY_CURRENT_USER\\Software\\QApplication";//创建的注册表路径
};