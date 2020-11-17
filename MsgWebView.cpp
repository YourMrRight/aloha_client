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
	//������qrc:/*.html
	if (url.scheme() == QString("qrc"))//�ж�url����
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
	// ͨ���������ҳ�ͼƬ·������ת�� �ַ���
	QModelIndex index = queryEmployeeModel.index(0, 0);
	strPicturePath = queryEmployeeModel.data(index).toString();

	strExternal = "external_" + strTalkID;

	// ������ҳ����
	MsgHtmlObj* msgHtmlObj = new MsgHtmlObj(this);
	channel->registerObject(strExternal, msgHtmlObj);		// ע��

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
	const QList<QStringList> msgLst = parseHtml(html);//����html
	int imageNum = 0;
	bool isImageMsg = false;
	int msgType = 1;//��Ϣ���ͣ�0�Ǳ��� 1�ı� 2�ļ�
	QString strData;//���͵�����
	for (int i = 0; i < msgLst.size(); i++)
	{
		if (msgLst.at(i).at(0) == "img")
		{
			QString imagePath = msgLst.at(i).at(1);
			QPixmap pixmap;
			//��ȡ�������Ƶ�λ��
			QString strEmotionPath = "qrc:/Resources/MainWindow/emotion/";
			int pos = strEmotionPath.size();
			isImageMsg = true;
			//��ȡ��������
			QString strEmotionName = imagePath.mid(pos);
			strEmotionName.replace(".png", "");
			//���ݱ������Ƶĳ��Ƚ������ñ�������
			//����3λ����3λ,��23������Ϊ023
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

			msgType = 0;//������Ϣ
			imageNum++;
			if (imagePath.left(3) == "qrc")
			{
				pixmap.load(imagePath.mid(3));//ȥ������·����qrc
			}
			else
			{
				pixmap.load(imagePath);
			}

			//����ͼƬhtml��ʽ�ı����
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
	if (strObj == "0")//����Ϣ
	{
	this->page()->runJavaScript(QString("appendHtml(%1)").arg(Msg));

		if (isImageMsg)
		{
			strData = QString::number(imageNum) + "images" + strData;
		}
		emit signalSendMsg(strData, msgType);
	}
	else//����
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
	const QDomElement& root = doc.documentElement();//�ڵ�Ԫ��
	const QDomNode& node = root.firstChildElement("body");
	return parseDocNode(node);
}

QList<QStringList> MsgWebView::parseDocNode(const QDomNode& node)
{
	QList<QStringList> attribute;
	const QDomNodeList& list = node.childNodes();//���������ӽڵ�

	for (int i = 0; i < list.count(); i++)
	{
		const QDomNode& node = list.at(i);


		if (node.isElement())
		{
			//ת��Ԫ��
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