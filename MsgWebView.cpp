#include "MsgWebView.h"
#include "CCMainWindow.h"
#include "WindowManager.h"
#include <QFile>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWebChannel>
#include "TalkWindow.h"
#include <QSqlQueryModel>
extern QString gstrLoginHeadPath;
#include <QDebug>
MsgHtmlObj::MsgHtmlObj(QObject* parent) :QObject(parent)
{
	initHtmlTmpl();
}



void MsgHtmlObj::initHtmlTmpl()
{
	m_msgLHtmlTmpl = getMsgTmplHtml("msgleftTmpl");

	//m_msgLHtmlTmpl.replace("%1", m_msgLPicPath);
	m_msgRHtmlTmpl = getMsgTmplHtml("msgrightTmpl");
	//m_msgRHtmlTmpl.replace("%1", gstrLoginHeadPath);
}

QString MsgHtmlObj::getMsgTmplHtml(const QString& code)
{
	QFile file(":/Resources/MainWindow/MsgHtml/" + code + ".html");
	file.open(QFile::ReadOnly);
	QString strData;
	if (file.isOpen())
	{
		strData = QLatin1String(file.readAll());
	}
	else
	{
		QMessageBox::information(nullptr, "Tips", "Failed to init html!");
		return QString("");
	}
	file.close();
	return strData;
}

bool MsgWebPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
	//仅接受qrc:/*.html
	if (url.scheme() == QString("qrc"))//判断url类型
		return true;
	return false;
}

MsgWebView::MsgWebView(QWidget* parent)
	: QWebEngineView(parent)
{
	MsgWebPage* page = new MsgWebPage(this);
	setPage(page);

	QWebChannel* channel = new QWebChannel(this);
	m_msgHtmlObj = new MsgHtmlObj(this);
	channel->registerObject("external", m_msgHtmlObj);
	
	
	CCMainWindow* ccTalkWindow = WindowManager::getInstance()->getccTalkWindow();
	connect(this, &MsgWebView::signalSendMsg, ccTalkWindow,&CCMainWindow::updateSendTcpMsg);
	QString strTalkID = WindowManager::getInstance()->getCreatingID();
	QSqlQueryModel queryEmployeeModel;
	QString strEmployeeID, strPicturePath;
	QString strExternal;
	queryEmployeeModel.setQuery(QString("SELECT picture FROM tab_user WHERE status = 1 AND userAccount = %1")
		.arg(strTalkID));
	// 通过索引，找出图片路径，并转成 字符串
	QModelIndex index = queryEmployeeModel.index(0, 0);
	strPicturePath = queryEmployeeModel.data(index).toString();

	strExternal = "external_" + strTalkID;

	// 构建网页对象
	MsgHtmlObj* msgHtmlObj = new MsgHtmlObj(this);
	channel->registerObject(strExternal, msgHtmlObj);		// 注册

	this->page()->setWebChannel(channel);
	this->load(QUrl("qrc:/Resources/MainWindow/MsgHtml/msgTmpl.html"));
}

MsgWebView::~MsgWebView()
{
}

void MsgWebView::appendMsg(const QString& html, QString strObj)
{
	QJsonObject msgObj;
	QString qsMsg;
	const QList<QStringList> msgLst = parseHtml(html);//解析html
	int imageNum = 0;
	bool isImageMsg = false;
	int msgType = 1;//信息类型：0是表情 1文本 2文件
	QString strData;//发送的数据
	for (int i = 0; i < msgLst.size(); i++)
	{
		if (msgLst.at(i).at(0) == "img")
		{
			QString imagePath = msgLst.at(i).at(1);
			QPixmap pixmap;
			//获取表情名称的位置
			QString strEmotionPath = "qrc:/Resources/MainWindow/emotion/";
			int pos = strEmotionPath.size();
			isImageMsg = true;
			//获取表情名称
			QString strEmotionName = imagePath.mid(pos);
			strEmotionName.replace(".png", "");
			//根据表情名称的长度进行设置表情数据
			//不足3位则补足3位,如23则数据为023
			int emotionNameL = strEmotionName.length();
			if (emotionNameL == 1)
			{
				strData = strData + "00" + strEmotionName;
			}
			else if (emotionNameL == 2)
			{
				strData = strData + "0" + strEmotionName;
			}
			else if (emotionNameL == 3)
			{
				strData = strData + strEmotionName;
			}

			msgType = 0;//表情信息
			imageNum++;
			if (imagePath.left(3) == "qrc")
			{
				pixmap.load(imagePath.mid(3));//去掉表情路径中qrc
			}
			else
			{
				pixmap.load(imagePath);
			}

			//表情图片html格式文本组合
			QString imgPath = QString("<img src=\"%1\" width=\"%2\" height=\"%3\"/>")
				.arg(imagePath).arg(pixmap.width()).arg(pixmap.height());
			qsMsg += imgPath;
		}
		else if (msgLst.at(i).at(0) == "text")
		{
			qsMsg += msgLst.at(i).at(1);
			strData = qsMsg;
		}
	}

	msgObj.insert("MSG", qsMsg);

	const QString& Msg = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);
	if (strObj == "0")//发信息
	{
	this->page()->runJavaScript(QString("appendHtml(%1)").arg(Msg));

		if (isImageMsg)
		{
			strData = QString::number(imageNum) + "images" + strData;
		}
		emit signalSendMsg(strData, msgType);
	}
	else//来信
	{
		this->page()->runJavaScript(QString("recvHtml(%1)").arg(Msg));
	}



	//this->page()->runJavaScript(QString("appendHtml(%1)").arg(Msg));
	//emit signalSendMsg(strData, msgType);
}

//void MsgWebView::test(QString& a, int& b)
//{
//		qDebug() << a << b;
//}

QList<QStringList> MsgWebView::parseHtml(const QString& html)
{
	QDomDocument doc;
	doc.setContent(html);
	const QDomElement& root = doc.documentElement();//节点元素
	const QDomNode& node = root.firstChildElement("body");
	return parseDocNode(node);
}

QList<QStringList> MsgWebView::parseDocNode(const QDomNode& node)
{
	QList<QStringList> attribute;
	const QDomNodeList& list = node.childNodes();//返回左右子节点

	for (int i = 0; i < list.count(); i++)
	{
		const QDomNode& node = list.at(i);


		if (node.isElement())
		{
			//转换元素
			const QDomElement& element = node.toElement();
			if (element.tagName() == "img")
			{
				QStringList attributeList;
				attributeList << "img" << element.attribute("src");
				attribute << attributeList;
			}

			if (element.tagName() == "span")
			{
				QStringList attributeList;
				attributeList << "text" << element.text();
				attribute << attributeList;
			}

			if (node.hasChildNodes())
			{
				attribute << parseDocNode(node);
			}
		}
	}

	return attribute;
}