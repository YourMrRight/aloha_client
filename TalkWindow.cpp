#include "TalkWindow.h"
#include "RootContatItem.h"
#include "ContactItem.h"
#include "CommonUtils.h"
#include "WindowManager.h"
#include <QMessageBox>
#include <QToolTip>
#include <SendFile.h>
#include <QFile>
TalkWindow::TalkWindow(QWidget *parent, const QString& uid)
	: QWidget(parent)
	,m_talkId(uid)
{
	ui.setupUi(this);
//	QMessageBox::information(this, QString::fromLocal8Bit("tip"), uid);
	WindowManager::getInstance()->addWindowName(m_talkId, this);
	setAttribute(Qt::WA_DeleteOnClose);
	initControl();
}

TalkWindow::~TalkWindow()
{
	WindowManager::getInstance()->deleteWindowName(m_talkId);
}

void TalkWindow::onFileOpenBtnClicked(bool)
{
	// 构造发送文件对话框的对象，直接显示
	SendFile* sendFile = new SendFile(this);
	sendFile->show();
}

void TalkWindow::addEmotionImage(int emotionNum)
{
	ui.textEdit->setFocus();
	ui.textEdit->addEmotionUrl(emotionNum);

}

void TalkWindow::setWindowName(const QString & name)
{
	ui.nameLabel->setText(name);
}

QString TalkWindow::getTalkId()
{
	return m_talkId;
}

void TalkWindow::onSendBtnClicked(bool)
{
	if (ui.textEdit->toPlainText().isEmpty())
	{
		QToolTip::showText(this->mapToGlobal(QPoint(630, 660)),
			QString::fromLocal8Bit("发送的信息不能为空！"),
			this, QRect(0, 0, 120, 100), 2000);
		return;
	}

	QString& html = ui.textEdit->document()->toHtml();

	//文本html如果没有字体则添加字体 
	if (!html.contains(".png") && !html.contains("</span>"))
	{
		QString fontHtml;
		QString text = ui.textEdit->toPlainText();
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			fontHtml.replace("%1", text);
			file.close();
		}
		else
		{
			QMessageBox::information(this, QString::fromLocal8Bit("提示")
				, QString::fromLocal8Bit("文件 msgFont.txt 不存在！"));
			return;
		}

		if (!html.contains(fontHtml))
		{
			html.replace(text, fontHtml);
		}
	}


	ui.textEdit->clear();
	ui.textEdit->deletAllEmotionImage();

	ui.msgWidget->appendMsg(html);//收信息窗口添加信息
}

void TalkWindow::onItemDoubleClicked()
{

}

void TalkWindow::initControl()
{
	QList<int> rightWidgetSize;
	rightWidgetSize << 600 << 138;
//	ui.bodySplitter->setSizes(rightWidgetSize);

	ui.textEdit->setFontPointSize(10);
	ui.textEdit->setFocus();
	CommonUtils::loadStyleSheet(this,"TalkWindow");
	connect(ui.closeBtn, SIGNAL(clicked(bool)), parent(), SLOT(onShowClose(bool)));
	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmoutionBtnClicked(bool)));
	connect(ui.faceBtn, SIGNAL(clicked(bool)), parent(), SLOT(onEmotionBtnClicked(bool)));
	connect(ui.sendBtn, SIGNAL(clicked(bool)), this, SLOT(onSendBtnClicked(bool)));
	connect(ui.fileopenBtn, SIGNAL(clicked(bool)), this, SLOT(onFileOpenBtnClicked(bool)));
	switch (m_groupType)
	{
	case COMPANY:
	{
		initCompanyTalk();
		break;
	}

	default://单聊
	{
		initPtoPTalk();
		break;
	}	
	}
}

void TalkWindow::initCompanyTalk()
{
}




void TalkWindow::initPtoPTalk()
{

}

void TalkWindow::addPeopInfo(QTreeWidgetItem * pRootGroupItem)
{

}

void TalkWindow::testnihao()
{
	QMessageBox::information(this, QString::fromLocal8Bit("响应槽函数成功"),"niao");
}
 

