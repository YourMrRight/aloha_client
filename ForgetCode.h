#pragma once

#include <QWidget>
#include "ui_ForgetCode.h"

class ForgetCode : public QWidget
{
	Q_OBJECT

public:
	ForgetCode(QWidget *parent = Q_NULLPTR);
	~ForgetCode();
private slots:
	void SubmitClicked();

private:
	void initControl();
	Ui::ForgetCode ui;
};
