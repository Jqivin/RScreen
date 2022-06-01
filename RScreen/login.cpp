#include "login.h"
#include <QIcon>
#include <iostream>
#include<QMessageBox>

#pragma execution_character_set("utf-8")


login::login(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	QIcon icon(":/RScreen/img/recoding.ico");
	setWindowIcon(icon);
	
	
}

//注册按钮
void login::on_RegisterBtn_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.pageRegister);
}

//注册界面返回登录按钮
void login::on_ReturnLogin_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.pageLogin);
}

//登录按钮
void login::on_LoginBtn_clicked()
{
	QString user = ui.LlineEditNum->text();
	QString passwd = ui.LlineEditPasswd->text();

	QString cmd("login ");
	cmd = cmd + user + " " + passwd;
	QByteArray a_cmd = cmd.toUtf8();	
	char* c_cmd = a_cmd.data();

	//将用户名和密码发送给服务器验证
	send(sclient, c_cmd, strlen(c_cmd), 0);
	char recv_buff[1024] = { 0 };
	int n = recv(sclient, recv_buff, 1023, 0);
	if (n <= 0)   //服务器关闭
	{
		printf("ser close\n");
	}

	if (strncmp(recv_buff, "ok#", 3) == 0)
	{
		//std::cout << "login success" << std::endl;
		QMessageBox::information(this, "成功", "登录成功");
		emit isLogin(user);
	}
	else
	{
		QMessageBox::critical(this, "失败", "您输入的用户名或密码错误");
	}


}

//注册按钮
void login::on_RokBtn_clicked()
{
	QString user = ui.RlineEditNumber->text();
	QString passwd = ui.RlineEditPasswd->text();
	QString passwd2 = ui.RlineEditPasswd2->text();
	if (passwd != passwd2) {
		QMessageBox::warning(this, "error", "您两次输入的密码不一致,请重新输入");
		ui.RlineEditPasswd2->clear();
		return;
	}

	//拼装报文
	QString cmd("register ");
	cmd = cmd + user + " " + passwd;
	QByteArray a_cmd = cmd.toUtf8();
	char* c_cmd = a_cmd.data();

	//发送给服务器
	send(sclient, c_cmd, strlen(c_cmd), 0);
	char recv_buff[1024] = { 0 };
	int n = recv(sclient, recv_buff, 1023, 0);
	if (n <= 0)   //服务器关闭
	{
		printf("ser close\n");
	}

	if (strncmp(recv_buff, "exist", 5) == 0)
	{
		QMessageBox::warning(this, "账号已存在", "您输入的账号已存在，换一个吧");
		//std::cout << "user have existed" << std::endl;
	}
	else if (strncmp(recv_buff, "err##", 5) == 0)
	{
		QMessageBox::warning(this, "错误", "服务器繁忙，请稍后再试吧");
		//std::cout << "err failed" << std::endl;
	}
	else if (strncmp(recv_buff, "ok###", 5) == 0)
	{
		QMessageBox::information(this, "成功", "您已经注册成功，去登录吧");
		//std::cout << "register success" << std::endl;
	}
	
}



login::~login()
{

}

//提供给rscreen的接口
void login::setSocket(SOCKET c)
{
	sclient = c;
}
