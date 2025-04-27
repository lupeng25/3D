#pragma once
#ifndef QTSPLASHSCREEN_H
#define QTSPLASHSCREEN_H

#include <QObject>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTime>
#include <QCoreApplication>
#include <QLabel>

class QtSplashScreen : public QSplashScreen
{
	Q_OBJECT
public:
	QtSplashScreen(QPixmap pixmap, QWidget* parent = 0);
	~QtSplashScreen();
	void  setRange(int min, int max);
	void  updateNum(int n);
	void  showAppText(QString str);
	void  showMessageText(QString str);
	void  showVisionText(QString str);
	void  showOtherText(QString str);
protected:
	void changeEvent(QEvent *);
private:

signals:

private:
	QProgressBar* m_progressBar;
	QLabel*       m_labelText;
	QLabel*       m_Visionlabel;
	QLabel*       m_SoftVisionlabel;
	QLabel*       m_Otherlabel;


};

#endif // QTSPLASHSCREEN_H
