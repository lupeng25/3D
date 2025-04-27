#ifndef NOTIFY_H
#define NOTIFY_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>


#ifndef LQFORMWIDGET_LIBRARY
#  define LQFORMWIDGETSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LQFORMWIDGETSHARED_EXPORT Q_DECL_IMPORT
#endif


class LQFORMWIDGETSHARED_EXPORT Notify : public QWidget
{
    Q_OBJECT
public:
    explicit Notify(int displayTime, QWidget *parent = 0);

    void setIcon(const QString &value);

    void setTitle(const QString &value);

    void setBody(const QString &value);

    void showGriant();

	void hideGriant();
Q_SIGNALS:
    void disappeared();

private:
    int displayTime;
    QString icon;
    QString title;
    QString body;

    QLabel *backgroundLabel;
    QLabel *iconLabel;
    QLabel *titleLabel;
    QLabel *bodyLabel;
    QPushButton *closeBtn;
    void mousePressEvent(QMouseEvent *event);
};
#endif // NOTIFY_H