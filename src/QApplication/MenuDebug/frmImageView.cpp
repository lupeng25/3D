#include "frmImageView.h"
#include "Data/dataVar.h"
#include "qmutex.h"

FrmImageView::FrmImageView(QWidget *parent)
    : QWidget(parent)
{	
    ui.setupUi(this);
}

FrmImageView::~FrmImageView()
{
	this->deleteLater();
}

int FrmImageView::GetData(QJsonObject & strData)
{
	return 0;
}

int FrmImageView::SetData(QJsonObject & strData)
{
	return 0;
}

int FrmImageView::NewProject()
{
	return 0;
}

//void FrmImageView::AddImgViewSlot(int iNewIndex, QString strCamera, int icount)
//{
//	ui.widget->changevideocount(icount);
//}
//
//void FrmImageView::DeleteImgViewSlot(int iNewIndex, QString strCamera, int icount)
//{
//	ui.widget->changevideocount(icount);
//}
//
//void FrmImageView::ModifyImgViewSlot(int iNewIndex, QString strCamera, int icount)
//{
//	ui.widget->changevideocount(icount);
//}

void FrmImageView::slot_ChangeVideoCount(int icount)
{
	ui.widget->changevideocount(icount);
}