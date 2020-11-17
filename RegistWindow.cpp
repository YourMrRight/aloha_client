#include "RegistWindow.h"
#include <QMessageBox>
RegistWindow::RegistWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	InitConctrol();

}

RegistWindow::~RegistWindow()
{
}

void RegistWindow::InitConctrol()
{
	
	connect(ui.registBtn, &QPushButton::clicked, this, &RegistWindow::RegistClick);
	connect(ui.cancelBtn, &QPushButton::clicked, this, &RegistWindow::CancelClick);

}
void RegistWindow::RegistClick()
{
	QString str;

	if (!ui.usernameEdit->text().isEmpty() && !ui.codeEdit->text().isEmpty() && !ui.comfirmcodeEdit->text().isEmpty()
		&& !ui.emileEdit->text().isEmpty() && !ui.phoneEdit->text().isEmpty())
	{
		if (ui.codeEdit->text() != ui.comfirmcodeEdit->text())
		{
			QMessageBox::information(this, QString::fromLocal8Bit("提示！"), QString::fromLocal8Bit("两次输入的密码不一致，请重新输入。"));
			ui.codeEdit->setText("");
			ui.comfirmcodeEdit->setText("");
			ui.codeEdit->setFocus();
			return;
		}
		else
		{
			//str << ui.usernameEdit->text() << ui.codeEdit->text() << ui.comfirmcodeEdit->text()
			//	<< ui.emileEdit->text() << ui.phoneEdit->text();
			//QMessageBox::information(this, QString::fromLocal8Bit("调试信息"), str);
		}
	}
	else
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("您输入的信息不完整"));
	}

}
void RegistWindow::CancelClick()
{
	this->close();
}
