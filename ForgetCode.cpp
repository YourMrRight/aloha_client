#include "ForgetCode.h"
#include <QMessageBox>
ForgetCode::ForgetCode(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	initControl();
}

ForgetCode::~ForgetCode()
{
}

void ForgetCode::initControl()
{
	connect(ui.submitBtn, &QPushButton::clicked, this,&ForgetCode::SubmitClicked);

}

void ForgetCode::SubmitClicked()
{
	/*QString str;

	if (!ui.userNameEdit->text().isEmpty() && !ui.codeEdit->text().isEmpty() && !ui.comfirmCodeEdit->text().isEmpty()
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
			str << ui.usernameEdit->text() << ui.codeEdit->text() << ui.comfirmcodeEdit->text()
				<< ui.emileEdit->text() << ui.phoneEdit->text();
			QMessageBox::information(this, QString::fromLocal8Bit("调试信息"), str);
		}
	}
	else
	{
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("您输入的信息不完整"));
	}*/
}