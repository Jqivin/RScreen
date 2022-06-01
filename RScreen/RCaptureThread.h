#pragma once
#include <QThread>
class RCaptureThread:public QThread
{
public:
	//out
	int width = 1280;    //宽
	int height = 720;   //高
	

	//in
	int fps = 10;  //帧率
	int cacheSize = 3;

	//线程安全,返回的空间由用户释放
	char *GetRGB();
	void Start();
	void Stop();
	void run();

	static RCaptureThread* Get()
	{
		static RCaptureThread ct;
		return &ct;
	}
	virtual ~RCaptureThread();

protected:
	RCaptureThread();
	QMutex mutex;
	bool isExit = false;     //记录线程的状态
	std::list<char *> rgbs;   //记录抓屏的数据 char*类型
};

