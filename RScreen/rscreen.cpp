#include "rscreen.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <iostream>
#include <QTimer>
#include <QTime>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include "RScreenRecord.h"
#include <QDateTime>
#include "client.h"

//记录是否开始了录制
static bool isRecord = false;
//记录录制的时间
static QTime time;
//开始录制按钮的样式表
#define RECORDQSS "\
QPushButton:!hover\
{background-image:url(:/RScreen/img/startRecoding.png);}\
QPushButton:hover\
{background-image:url(:/RScreen/img/pressStartRecoding.png);}\
QPushButton:pressed\
{background-image:url(:/RScreen/img/pressStartRecoding.png);\
background-color:rgba(255,255,255,0);}"

RScreen::RScreen(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	myUiSet();
	connectFun();
	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	//定时器初始化
	//timer = new QTimer(this);
	timer.start(1000);
	setLabelPixmap();
}
void RScreen::setLabelPixmap()
{
	static QScreen *src = NULL;
	if (!src)
	{
		src = QGuiApplication::primaryScreen();
	}
	//获取全屏
	QPixmap pix = src->grabWindow(QApplication::desktop()->winId());
	int w = ui.firstImg->width();
	QPixmap pix2 = pix.scaledToWidth(w);
	ui.firstImg->setPixmap(pix2);
}
void RScreen::mousePressEvent(QMouseEvent* e)
{
	//只有点击左键才有操作
	if (e->button() == Qt::LeftButton)
	{
		//记录下来点击鼠标那一刻的位置
		pt.setX(e->x());
		pt.setY(e->y());
	}
}

void RScreen::mouseMoveEvent(QMouseEvent* e)
{
	// move移动的位置是相当于屏幕的坐标系
	//event->x() , event->y()是相当于窗口的坐标系
	this->move(e->globalX() - pt.x(), e->globalY() - pt.y());
}
void RScreen::myUiSet()
{
	//选择录制模式
	/*QStringList comboxAreaText;
	comboxAreaText << "全屏录制" << "选择录制";
	foreach(auto x, comboxAreaText)
	{
		ui.comboxArea->addItem(x.toUtf8().data());
	}*/

	//选择录制帧率
	QStringList comboxFpsText;
	comboxFpsText << "10" << "20" << "24" << "25" << "30" << "60" << "120";
	ui.comboxFps->addItems(comboxFpsText);

	//选择清晰度
	QStringList comboxQingxidu;
	comboxQingxidu << "1080P高清" << "720P高清" << "480P清晰";
	ui.comboxqingxidu->addItems(comboxQingxidu);
 
	//选择视频格式
	QStringList comboxFmtText;
	comboxFmtText << ".mp4" << ".avi" << ".wmv" << ".flv";
	ui.comboxFmt->addItems(comboxFmtText);
}

void RScreen::connectFun()
{
	//登录按钮
	//connect(ui.loginBtn, &QPushButton::clicked, this, &RScreen::on_loginBtn);
	//云按钮
	connect(ui.upLoadBtn, &QPushButton::clicked, this, &RScreen::on_upLoadBtn);
	//更改文件位置的按钮
	connect(ui.changeFileBtn, &QPushButton::clicked, this, &RScreen::on_changeFileBtn);
	//开始录制按钮
	connect(ui.recordButton, &QPushButton::clicked, this, &RScreen::on_recordButton);
	//定时器槽函数连接
	connect(&timer, &QTimer::timeout, this, &RScreen::on_timeout);
	//帧率变化
	void(QComboBox::*fun)(const QString&) = &QComboBox::currentIndexChanged;
	connect(ui.comboxFps, fun, this, [=] {
		fps = ui.comboxFps->currentText().toInt();
		std::cout << fps;
	});

	//清晰度变化
	void(QComboBox::*funQ)(const QString&) = &QComboBox::currentIndexChanged;
	connect(ui.comboxqingxidu, funQ, this, [=] {
		QString str = ui.comboxqingxidu->currentText();
		if (str == "1080P高清")
		{
			width = 1920;
			height = 1080;
		}
		else if (str == "720P高清")
		{
			width = 1280;
			height = 720;
		}
		else if (str == "480P清晰")
		{
			width = 720;
			height = 480;
		}
		std::cout << width << "*" << height;
	});

	//视频格式变化
	void(QComboBox::*funFmt)(const QString&) = &QComboBox::currentIndexChanged;
	connect(ui.comboxFmt, funFmt, this, [=] {
		fmt = ui.comboxFmt->currentText();
		std::cout << fmt.toStdString();
	});

}

//更改文件位置的按钮的槽函数的实现
void RScreen::on_changeFileBtn()
{
	//获得一个路径
	QString strPath = QFileDialog::getExistingDirectory();
	if (!strPath.isEmpty())
	{
		//设置ui文字
		ui.urlEdit->setText(strPath);
		//保存路径
		// ...
	}
}

//开始录制按钮的槽函数
void RScreen::on_recordButton()
{
	isRecord = !isRecord;
	//如果开始了录制，再点击按钮就是关闭录制
	if (!isRecord)
	{
		ui.recordButton->setStyleSheet(RECORDQSS);
		RScreenRecord::Get()->Stop();
	}
	else
	{
		time.restart();
		QString str = "background-image:url(:/RScreen/img/stopRecoding.png);background-color:rgba(255,255,255,0);";
		ui.recordButton->setStyleSheet(str);
		
		
		QDateTime t = QDateTime::currentDateTime();
		//用时间创建文件名
		QString filename = t.toString("yyyyMMdd_hhmmss");
		filename = "rscreen_" + filename;
		//视频格式
		filename += fmt;
		filename = ui.urlEdit->text() + "\\" + filename;

		RScreenRecord::Get()->fps = fps;
		RScreenRecord::Get()->outWidth = width;
		RScreenRecord::Get()->outHeight = height;
		if (RScreenRecord::Get()->Start(filename.toLocal8Bit()))
		{
			return;
		}
		
		//timer->start(1000);    //定时器每秒刷新一次
		//this->showMinimized();   //开始录制之后最小化
		
		isRecord = false;
	}
}

void RScreen::on_timeout()
{
	if (isRecord)
	{
		//流逝的时间 返回值是一个毫秒
		int elapsedTime = time.elapsed() / 1000;
		int Minutes = elapsedTime / 60;
		int Second = elapsedTime % 60;
		int Hour = elapsedTime / 3600;
		ui.timelabel->setText(QString::asprintf("%02d:%02d:%02d",Hour,Minutes, Second));
	}
}

//上传按钮的实现
void RScreen::on_upLoadBtn()
{
	client* cli_ui = new client;
	cli_ui->show();
}

////登录按钮槽函数实现
//void RScreen::on_loginBtn()
//{
//	login* log_ui = new login;
//	log_ui->show();
//}
