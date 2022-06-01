#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QWidget>
#include "ui_rscreen.h"
#include <QTimer>

class RScreen : public QWidget
{
    Q_OBJECT

public:
    RScreen(QWidget *parent = Q_NULLPTR);
	QPoint pt;                         //记录鼠标位置的点
	int width = 1920;                         //录制视频的宽
	int height = 1080;							//录制的视频的高
	int fps = 10;							//视频帧率
	QString fmt = ".mp4";						//视频格式

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
private:
    Ui::RScreenClass ui;
	void myUiSet();
	void connectFun();
	QTimer timer;           //定时器
	void setLabelPixmap();
public slots:
	void on_changeFileBtn();
	void on_recordButton();
	void on_timeout();
	void on_upLoadBtn();
};
