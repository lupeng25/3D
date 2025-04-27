#pragma once
#include "basemainwindow.h"


class _MYTITLEBAR_API_ frmBase :	public BaseWindow
{
	Q_OBJECT

public:
	frmBase(QWidget* parent = Q_NULLPTR);
	~frmBase();

public:
	virtual int GetData(QJsonObject& strData);
	virtual int SetData(QJsonObject& strData);
	virtual int	NewProject();
};

