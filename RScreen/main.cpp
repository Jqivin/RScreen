#include "rscreen.h"
#include <QtWidgets/QApplication>
#include "RCaptureThread.h"
#include "RAudioThread.h"
#include "RScreenRecord.h"
#include <iostream>
#include <QDateTime>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RScreen w;
    w.show();

	/*测试屏幕录制类	
	RCaptureThread::Get()->Start();
	//QThread::msleep(3000);
	for (;;)
	{
		char* c = RCaptureThread::Get()->GetRGB();
		if (c)
		{
			std::cout << "+";
		}
	}
	RCaptureThread::Get()->Stop();
	*/
	/*测试音频录制类
	
	*/
	/*RAudioThread::Get()->Start();
	for (;;)
	{
		char *pcm = RAudioThread::Get()->GetPCM();
		if (pcm)
		{
			std::cout << "*";
		}
	}*/

	/*测试RScreenRecord
	*/
	/*QDateTime t = QDateTime::currentDateTime();
	用时间创建文件名
	QString filename = t.toString("yyyyMMdd_hhmmss"); 
	filename = "rscreen_" + filename;
	filename += ".mp4";
	RScreenRecord::Get()->Start(filename.toLocal8Bit());
	getchar();
	RScreenRecord::Get()->Stop();*/

    return a.exec();
}
