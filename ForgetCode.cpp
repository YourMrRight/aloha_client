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
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ��"), QString::fromLocal8Bit("������������벻һ�£����������롣"));
			ui.codeEdit->setText("");
			ui.comfirmcodeEdit->setText("");
			ui.codeEdit->setFocus();
			return;
		}
		else
		{
			str << ui.usernameEdit->text() << ui.codeEdit->text() << ui.comfirmcodeEdit->text()
				<< ui.emileEdit->text() << ui.phoneEdit->text();
			QMessageBox::information(this, QString::fromLocal8Bit("������Ϣ"), str);
		}
	}
	else
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���������Ϣ������"));
	}*/
}