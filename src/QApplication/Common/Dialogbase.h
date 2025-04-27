#pragma once
#ifndef DIALOGBASE_H_
#define DIALOGBASE_H_

#include <QWidget>
class Dialogbase : public QWidget
{
public:
	Dialogbase(QWidget *parent = nullptr);
public:
	virtual	int Init() = 0;
	virtual	int LoadData() = 0;
	virtual	int SaveData() = 0;
	virtual	int UpdateUI() = 0;
public:
	virtual	QString Info() = 0;
	virtual	QString Vision() = 0;
public:
};

#endif // !Dialogbase_H_