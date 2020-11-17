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
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ��"), QString::fromLocal8Bit("������������벻һ�£����������롣"));
			ui.codeEdit->setText("");
			ui.comfirmcodeEdit->setText("");
			ui.codeEdit->setFocus();
			return;
		}
		else
		{
			//str << ui.usernameEdit->text() << ui.codeEdit->text() << ui.comfirmcodeEdit->text()
			//	<< ui.emileEdit->text() << ui.phoneEdit->text();
			//QMessageBox::information(this, QString::fromLocal8Bit("������Ϣ"), str);
		}
	}
	else
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���������Ϣ������"));
	}

}
void RegistWindow::CancelClick()
{
	this->close();
}
