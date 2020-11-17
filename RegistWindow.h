#pragma once

#include <QWidget>
#include "ui_RegistWindow.h"

class RegistWindow : public QWidget
{
	Q_OBJECT

public:
	RegistWindow(QWidget *parent = Q_NULLPTR);
	~RegistWindow();
	void InitConctrol();


private slots:
	void RegistClick();
	void CancelClick();


private:
	Ui::RegistWindow ui;
};
