#pragma once

#include <QWidget>
#include "ui_login.h"
#include<winsock2.h>
class login : public QWidget
{
	Q_OBJECT
signals:
	void isLogin(QString& str);

public:
	login(QWidget *parent = Q_NULLPTR);
	~login();
	void setSocket(SOCKET c);

	private slots:
	void on_RegisterBtn_clicked();
	void on_ReturnLogin_clicked();
	void on_LoginBtn_clicked();
	void on_RokBtn_clicked();
private:
public:
	Ui::login ui;
	SOCKET sclient;

};
