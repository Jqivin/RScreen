//#include "client.h"
//
//client::client(QWidget *parent)
//	: QWidget(parent)
//{
//	ui.setupUi(this);
//}
//
//client::~client()
//{
//}
#include "client.h"
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include<winsock2.h>
#include <stdio.h>
#include <iostream>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QMouseEvent>
#include "login.h"
#include <QDialog>
#pragma comment(lib, "ws2_32.lib")

#pragma execution_character_set("utf-8")

#define UIPAddress "192.168.221.128"
#define DIPAddress "192.168.221.133"

client::client(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	//connect(ui.connect, &QPushButton::clicked, this, &client::ConnectToHost);  
	//不需要使用按钮了 打开就与服务器连接
	ConnectToHost();
	isLogined = false;   //登录状态
	log_ui = NULL;
	//ConnectToHost();

	
}

client::~client()
{
}

//void client::mousePressEvent(QMouseEvent* e)
//{
//	//只有点击左键才有操作
//	if (e->button() == Qt::LeftButton)
//	{
//		//记录下来点击鼠标那一刻的位置
//		pt.setX(e->x());
//		pt.setY(e->y());
//	}
//}
//
//void client::mouseMoveEvent(QMouseEvent* e)
//{
//	// move移动的位置是相当于屏幕的坐标系
//	//event->x() , event->y()是相当于窗口的坐标系
//	this->move(e->globalX() - pt.x(), e->globalY() - pt.y());
//}

//连接服务器
void client::ConnectToHost()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return;
	}

	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket!");
		return;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(6000);
	serAddr.sin_addr.S_un.S_addr = inet_addr(UIPAddress);
	if (::connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败
		printf("connect error !");
		closesocket(sclient);
		return;
	}
	std::cout << "connect ok";

	//刷新一下文件列表
	//display_file();
}

//登录按钮槽函数实现
void client::on_loginBtn_clicked()
{
	if (isLogined) {
		//QMessageBox::information(this, "", "您已经登录，无须重复登录");
		int res = QMessageBox::question(this, "", "您已经处于登录状态，是否退出登录？", QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::Cancel);
		if (res == QMessageBox::StandardButton::Yes)
		{
			isLogined = false;
			ui.loginBtn->setText("未登录");
			ui.loginBtn->setStyleSheet("color: rgb(211, 211, 211);");
			ui.listWidget->clear();
		}
		return;
	}
	if(log_ui == NULL)
		log_ui = new login;
	log_ui->ui.stackedWidget->setCurrentWidget(log_ui->ui.pageLogin);
	log_ui->show();
	log_ui->setSocket(sclient);
	connect(log_ui, &login::isLogin, this, [&](QString &str) {
		isLogined = true;
		ui.loginBtn->setText(str);
		ui.loginBtn->setStyleSheet("color: rgb(10, 130, 10);");
		log_ui->close();
		display_file();
	});
}

//上传文件的按钮
void client::on_upLoad_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, "get file", "/", "*.*");
	if (fileName.isEmpty()) {
		qDebug() << "file is not exist 57" << fileName;
		return;
	}
	QByteArray arr = fileName.toUtf8();   //这个不能省略，不能写成fileName.toUtf8().data();
	char* filename = arr.data();
	//up_file(fileName.toStdString().c_str());    error
	up_file(filename);

}

//下载文件的按钮
void client::on_download_clicked()
{
	QString file;
	QListWidgetItem* item = ui.listWidget->currentItem();
	if (!item)
	{
		QMessageBox::warning(this, "Warning", "请先选择要下载的文件");
		return;
	}
	file = item->text();
	QByteArray arr = file.toUtf8();
	char* filename = arr.data();

	std::cout << filename << std::endl;

	QString text = QString("%1%2%3").arg("是否下载文件").arg(filename).arg("?");
	QMessageBox::StandardButton res;
	res = QMessageBox::question(this, "DownLoad", text, QMessageBox::Yes | QMessageBox::Cancel);
	if (res == QMessageBox::Yes)
		down_file(filename);
}

//展示文件的按钮
void client::on_ls_clicked()
{
	//char *cmd = "ls";
	display_file();
}

//删除文件的按钮
void client::on_deleteFile_clicked()
{
	QListWidgetItem* item = ui.listWidget->currentItem();
	QString filename = item->text();
	QString text = QString("%1%2%3").arg("确定删除文件").arg(filename).arg("?");
	QMessageBox::StandardButton res;
	res = QMessageBox::question(this, "确认", text, QMessageBox::Ok | QMessageBox::Cancel);
	if (res == QMessageBox::Ok)
		delete_file(filename);
	else
		return;
}
//删除文件
void client::delete_file(const QString& file)
{
	//char* buff = file.toLocal8Bit().data();
	//char* buff = file.toUtf8().data();
	QByteArray ba = file.toLatin1();   //must 不能写成上面的，只能分为两步完成
	char* buff = ba.data();
	char cmd[128] = "rm ";
	strcat(cmd, buff);
	send(sclient, cmd, strlen(cmd), 0);
	char recv_buff[1024] = { 0 };
	int n = recv(sclient, recv_buff, 1023, 0);
	if (n <= 0)   //服务器关闭
	{
		printf("ser close\n");
	}
	printf("%s\n", recv_buff);
	QMessageBox::information(this, "ok", QString("成功删除文件") + file);
	//刷新一下文件列表
	display_file();

}
//展示文件
void client::display_file()
{
	char buff[128] = "ls";
	send(sclient, buff, strlen(buff), 0); //可能的情况ls，rm a.c b.c , cp a.c b.c
	char recv_buff[1024] = { 0 };
	int n = recv(sclient, recv_buff, 1023, 0);
	if (n <= 0)   //服务器关闭
	{
		printf("ser close\n");
	}
	printf("%s\n", recv_buff);
	//ui.plainTextEdit->appendPlainText(QString(recv_buff));

	CreateListWidget(recv_buff);
}

//更新ListWidget
void client::CreateListWidget(const char* str)
{
	ui.listWidget->clear();
	QString files(str);
	qDebug() << files;
	for (int i = 0;; i++)
	{
		QString tmp = files.section("\n", i, i);
		if (tmp.isEmpty())
			break;
		QListWidgetItem* item = new QListWidgetItem(tmp);
		ui.listWidget->addItem(item);
	}

}

//下载
void client::down_file(char* filename)
{
	if (filename == NULL)
	{
		//printf("please input filename\n");
		QMessageBox::warning(this, "Warning", "请先选择要下载的文件");
		return;
	}
	char str[128] = "get ";
	strcat(str, filename);
	send(sclient, str, strlen(str), 0);  //get a.c
	char buff_file[128] = { 0 };
	int num = recv(sclient, buff_file, 127, 0); //ok#size  , err#xxx

												//服务器异常关闭了
	if (num <= 0)
	{
		printf("ser close\n");
		closesocket(sclient);
		exit(0);
	}

	if (strncmp(buff_file, "ok#", 3) != 0)   //前三个字符!=OK#,表示收到服务器回复的失败报文
	{
		printf("%s\n", buff_file + 4);
		return;
	}
	//成功了，收到了服务器返回的文件的大小
	int filesize = 0;
	sscanf(buff_file + 3, "%d", &filesize); // ok#filesize

	printf("下载的文件大小为：%d\n", filesize);

	QString path = QFileDialog::getExistingDirectory(this, "保存", "/");

	path += "/";
	path += filename;

	//保存的绝对路径
	QByteArray arr = path.toUtf8();
	char* absolutePath = arr.data();
	FILE *fp = fopen(absolutePath, "wb");
	if (!fp) //打开文件失败
	{
		send(sclient, "err", 3, 0);
		return;
	}

	send(sclient, "ok#", 3, 0); //打开成功

	int curr_size = 0; //当前的大小
	char data[1024] = { 0 };

	while (1)
	{
		int num = recv(sclient, data, 1024, 0); //收到的数据数量
		if (num <= 0)
		{
			//服务器关闭
			printf("ser close --  in sending\n");
			closesocket(sclient);
			fclose(fp);
			exit(0);
		}

		fwrite(data, 1, num, fp);
		curr_size += num;
		printf("down:%.2f%%\r", curr_size * 100.0 / filesize); //  
		ui.upProgressBar->setValue(curr_size * 100.0 / filesize);
		//fflush(stdout);
		if (curr_size >= filesize)
		{
			break;
		}
	}
	printf("\n");
	QMessageBox::information(this, "info", "文件下载完成");
	ui.upProgressBar->setValue(0);
	

	fclose(fp);

}
//上传
void client::up_file(const char* filename)
{
	//qDebug() << "up_file is start";
	if (filename == NULL)    //文件名为空
	{
		printf("You haven't entered the filename yet.\n");
		return;
	}
	//int file_fd = open(filename,O_RDONLY);  //打开文件
	FILE* fp = fopen(filename, "rb+");
	if (!fp)
	{
		printf("The file named %s is not exist.75\n ", filename);
		return;
	}

	//qDebug() << "open file is ok";
	//只需要名称 不需要路径

	QString s(filename);
	int pos = s.lastIndexOf("/");
	//文件名字  相对路径 如JAY.mp3
	QString file = s.right(s.length() - pos - 1);
	file = "up " + file;              //格式：up+文件名
	QByteArray arr = file.toUtf8();
	char* str = arr.data();

	//char str[128] = "up haigeshi.mp4";    //测试写死
	send(sclient, str, strlen(str), 0);     //up filename
	char buff_file[128] = { 0 };
	int num = recv(sclient, buff_file, 127, 0);
	if (num <= 0)
	{
		printf("ser close\n");
		closesocket(sclient);
		exit(0);
	}
	if (strncmp(buff_file, "ok#", 3) != 0)   //前三个字符!=OK#,收到服务器回复的失败报文
	{
		printf("%s\n", buff_file + 4);
		return;
	}
	//服务器端良好
	//计算文件的大小
	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	send(sclient, (char*)&file_size, 4, 0);

	//等待服务器端处理
	char recv_buff[128] = { 0 };
	int n = recv(sclient, recv_buff, 127, 0);
	if (n <= 0)    //服务器端异常
	{
		printf("ser close\n");
		closesocket(sclient);
		fclose(fp);
		exit(0);
	}
	if (strcmp(recv_buff, "FINISHED") == 0)
	{
		printf("Finished:up:100.00%\n");    //秒传
		QMessageBox::information(this, "info", "文件秒传完成");
		return;
	}
	else        //开始上传
	{
		int curr_upsize = 0;
		//char size_buff[128] = {0};   //接收服务端发来的报文（上传的大小或者错误信息）
		char send_buff[1024] = { 0 };
		QString str = "";
		std::cout << "start read" << std::endl;
		while (1)
		{
			int n = fread(send_buff, 1, 1024, fp);
			if (n == 0)
			{
				//文件读取完毕
				//
				break;
			}

			send(sclient, send_buff, n, 0);  //发送文件

											 //int m = recv(c,size_buff,127,0);

			int m = recv(sclient, (char*)&curr_upsize, 4, 0);

			if (m <= 0)    //服务器端异常
			{
				printf("ser close\n");
				closesocket(sclient);
				fclose(fp);
				exit(0);
			}
			// if(strncmp(size_buff,"ok#",3) != 0)
			// {
			//     printf("%s\n",size_buff+4);    //打印错误信息，服务器write失败
			// }
			// else
			// {
			//     sscanf(size_buff+3,"%d",&curr_upsize); // ok#curr_size
			//     printf("up:%.2f%%\r",curr_upsize * 100.0 / file_size);
			// }
			printf("up:%.2f%%\r", curr_upsize * 100.0 / file_size);
			ui.upProgressBar->setValue(curr_upsize * 100.0 / file_size);

		}
	}
	QMessageBox::information(this, "info", "文件上传完成");

	//刷新一下文件列表
	display_file();
	ui.upProgressBar->setValue(0);

	printf("\n");
}
