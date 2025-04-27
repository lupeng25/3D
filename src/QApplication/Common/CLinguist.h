#pragma once
// Linguist.h
#pragma once
/*
语言家，配置当前系统的语言
*/

#include <QObject>
#include <QTranslator>
#include <QVector>

enum Language
{
	Language_CN,
	Language_EN,
	Language_JP,
	Language_Ve,
	undefined
};

class CLinguist :public QObject
{
	Q_OBJECT
private:
	CLinguist();

public:
	// 当前语言 简体中文、英文、未定义
	Language m_CurrentLanguage = Language::undefined;
	// 切换语言
	void ChangeLanguage(Language lan);
	static CLinguist& getInstance();
private:
	QVector<QTranslator*>	m_vecpQTranslator;
signals:
	// 语言切换信号，通知当前系统语言已经被切换
	void sigLanguageChaned();
};
