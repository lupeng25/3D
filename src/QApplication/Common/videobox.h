﻿#ifndef VIDEOBOX_H
#define VIDEOBOX_H

/**
 * 监控画面切换控件 作者:feiyangqingyun(QQ:517216493) 2021-11-08
 * 1. 将所有通道切换处理全部集中到一个类。
 * 2. 通用整数倍数布局切换函数，可方便拓展到100/255通道等。
 * 3. 通用异形布局切换函数，可以参考进行自定义异形布局。
 * 4. 通道布局切换发出信号通知。
 * 5. 可控每种布局切换菜单是否启用。
 * 6. 支持自定义子菜单布局内容。
 * 7. 支持设置对应的菜单标识比如默认的通道字样改成设备。
 */
#include <QObject>
#include <QWidget>
#include <QMap>

class QMenu;
class QWidget;
class QGridLayout;

class VideoBox : public QObject
{
    Q_OBJECT
public:
    explicit VideoBox(QObject *parent = 0);
private:
    //表格布局存放通道
    QGridLayout *gridLayout;

    //视频控件集合
    QWidgetList widgets;

    //通道数量
    int videoCount;

    //当前画面类型
    QString videoType;

    //启用其他通道画面(2画面/3画面)
    bool enableOther;

    //主菜单子菜单文字标识
    QString menuFlag;
    QString actionFlag;

    //布局方案标识集合
    QMap<QString, QStringList> types;
    void addMenu(QMenu *menu, const QString &type);

public:
	void InitMenus(int iCameraCount, QMenu *menu, const QList<bool> &enable);
	void InitCamera(int iCameraCount,int Index,QMap<QString, QStringList>& types);
	int  CameravideoCount;	//视频通道个数
public Q_SLOTS:
    //获取和设置当前画面类型
    QString getVideoType() const;
    void setVideoType(const QString videoType);

    //获取和设置视频控件集合
    QWidgetList getWidgets() const;
    void setWidgets(QWidgetList widgets);

    //设置表格布局
    void setLayout(QGridLayout *gridLayout);

    //设置启用其他通道画面
    void setEnableOther(bool enableOther);

    //设置主菜单子菜单文字标识
    void setMenuFlag(const QString menuFlag);
    void setActionFlag(const QString actionFlag);

    //设置菜单类型集合
    void setTypes(const QMap<QString, QStringList> &types);

    //初始化菜单
    void initMenu(QMenu *menu, const QList<bool> &enable);

    //通用设置函数
    void show_video(int type, int index);

    //菜单切换布局槽函数
    void show_video();

    //显示和隐藏所有通道
    void show_video_all();
    void hide_video_all();

    //常规及异形通道布局
    void change_video_normal(int index, int row, int column);
    void change_video_custom(int index, int type);

    //设置可见
    void change_video_visible(int start, int end);

    //异形布局
    void change_video_6(const QList<int> &indexs);
    void change_video_8(const QList<int> &indexs);
    void change_video_13(const QList<int> &indexs);

    //具体通道切换函数
    void change_video_1(int index);
    void change_video_2(int index);
    void change_video_3(int index);
    void change_video_4(int index);
    void change_video_6(int index);
    void change_video_8(int index);
    void change_video_9(int index);
    void change_video_13(int index);
    void change_video_16(int index);
    void change_video_25(int index);
    void change_video_36(int index);
    void change_video_64(int index);

Q_SIGNALS:
    //画面布局切换信号
    void changeVideo(int type, const QString videoType, bool videoMax);
};

#endif // VIDEOBOX_H