#pragma once

#include "BasicWindow.h"
#include "ui_UserLogin.h"
#include "RegistWindow.h"

class UserLogin : public BasicWindow
{
	Q_OBJECT

public:
	UserLogin(QWidget *parent = Q_NULLPTR);
	~UserLogin();

private slots:
	void onLoginBtnClicked();
	void registBtnClicked();

private:
	void initControl();
	bool connectMySql();
	bool veryfyAccountCode(bool& isAccountLogin, QString& strAccount);

private:
	Ui::UserLogin ui;
};
