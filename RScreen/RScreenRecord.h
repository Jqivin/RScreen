#pragma once
#include <QThread>
class RScreenRecord:protected QThread
{
public:
public:
	//in
	int fps = 10;
	int outWidth = 1280;
	int outHeight = 720;

	bool Start(const char *filename);
	void Stop();
	void run();
	static RScreenRecord *Get()
	{
		static RScreenRecord ct;
		return &ct;
	}

	
	virtual ~RScreenRecord();

protected:
	RScreenRecord();
	bool isExit = false;
	QMutex mutex;
};

