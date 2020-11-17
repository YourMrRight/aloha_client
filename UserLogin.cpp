#include "UserLogin.h"
#include "CCMainWindow.h"
#include<QPainter>
#include <QMessageBox>
#include "CCMainWindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include "RegistWindow.h"
#include "SysTray.h"
QString gLoginEmployeeID;//��¼���˺�
UserLogin::UserLogin(QWidget *parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	initTitleBar();
	setTitleBarTitle("", ":/Resources/MainWindow/bar.PNG");
	loadStyleSheet("UserLogin");
	initControl();
}

UserLogin::~UserLogin()
{
}

void UserLogin::registBtnClicked()
{
	RegistWindow* registWindow = new RegistWindow;
	registWindow->show();
}

void UserLogin::initControl()
{
	QLabel *headlabel = new QLabel(this);
	headlabel->setFixedSize(120, 100);
	QPixmap pix = QPixmap((":/Resources/MainWindow/main_logo.png"));
	QPixmap temp(pix.size());
	temp.fill(Qt::transparent);
	QPainter painter(&temp);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.drawPixmap(0, 0, pix);
	painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
	painter.fillRect(temp.rect(), QColor(0, 0, 0, 100));
	painter.end();
	pix = temp;
	headlabel->setPixmap(pix);
	headlabel->move(width() / 2-60 , ui.titleWidget->height() - 34);
	connect(ui.loginBtn, &QPushButton::clicked, this, &UserLogin::onLoginBtnClicked);
	connect(ui.registBtn, &QPushButton::clicked, this, &UserLogin::registBtnClicked);
	//�������ݿ�
	if (!connectMySql())
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("�������ݿ�ʧ�ܣ�"));
		close();
	}
	SysTray* systray = new SysTray(this);
}
     
bool UserLogin::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setDatabaseName("aloha");	//���ݿ�����
	db.setHostName("124.71.111.16");//������
	db.setUserName("root");		//�û���
	db.setPassword("123456");	//����
	db.setPort(3306);			//�˿�

	if (db.open())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UserLogin::veryfyAccountCode(bool& isAccountLogin, QString& strAccount)
{  
	QString strAccountInput = ui.editUserAccount->text();
	QString strCodeInput = ui.editPassword->text();



	//�˺ŵ�¼
	QString strSqlCode = QString("SELECT code,userAccount FROM tab_accounts WHERE userAccount = '%1'")
		.arg(strAccountInput);
	QSqlQuery queryAccount(strSqlCode);
	queryAccount.exec();
	if (queryAccount.first())
	{
		QString strCode = queryAccount.value(0).toString();

		if (strCode == strCodeInput)
		{
			gLoginEmployeeID = queryAccount.value(1).toString();

			strAccount = strAccountInput;
			isAccountLogin = true;
			return true;
		}
		else
		{
			return false;
		}
	}


	return false;
}

void UserLogin::onLoginBtnClicked()
{
	bool isAccountLogin;
	QString strAccount;//�˺Ż�QQ��

	if (!veryfyAccountCode(isAccountLogin, strAccount))
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("��������˺Ż������������������룡"));
		ui.editPassword->setText("");
		return;
	}

	//���µ�¼״̬Ϊ��¼
	QString strSqlStatus = QString("UPDATE tab_user SET online_status = 2 WHERE employeeID = %1").arg(gLoginEmployeeID);
	QSqlQuery sqlStatus(strSqlStatus);
	sqlStatus.exec();

	close();
	CCMainWindow* mainwindow = new CCMainWindow;
	mainwindow->show();
}