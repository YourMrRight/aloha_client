#include "CCMainWindow.h"
#include "basicwindow.h"
#include "NotifyManager.h"
#include <QHBoxLayout>
#include <QProxyStyle>
#include <QPainter>
#include <QTimer>
#include <QEvent>
#include <QSqlQueryModel>
//#include "CommonUtils.h"
#include "ContactItem.h"
#include "SysTray.h"
#include <QTreeWidgetItem>
#include <qlistwidget.h>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include "RootContatItem.h"
#include <QSqlQuery>
#include "WindowManager.h"
#include <QMessageBox>
#include "TalkWindowItem.h"
#include "RecevieFile.h"
QString gstrLoginHeadPath;
extern QString gLoginEmployeeID;
const int gUdpPort = 6666;

// ȫ�ֱ������ļ�����
QString gfileName;
// ȫ�ֱ������ļ�����
QString gfileData;


class CustomProxyStyle :public QProxyStyle
{
public:
	virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
		QPainter* painter, const QWidget* widget = nullptr) const
	{
		if (element == PE_FrameFocusRect)
		{
			return;
		}
		else
		{
			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
	}
};

CCMainWindow::CCMainWindow(QWidget* parent)
	:BasicWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::Tool);
	//initTitleBar();
	loadStyleSheet("CCMainWindow");
	setHeadPixmap(getHeadPicPath());
	initControl();
	initTcpSocket();
	initUdpSocket();
	QFile file("Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (!file.size())
	{
		QStringList employeesIDList;
		getEmployeesID(employeesIDList);
		if (!createJSFile(employeesIDList))
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("����js�ļ�����ʧ�ܣ�"));
		}
	}
}

CCMainWindow::~CCMainWindow()
{
}

void CCMainWindow::initControl()
{
	//����ȡ����ʱ�����Ʊ߿�
	ui.treeWidget->setStyle(new CustomProxyStyle);
	setHeadPixmap(":/Resources/MainWindow/main_logo.png");
	setStatusMenuIcon(":/Resources/MainWindow/StatusSucceeded.png");
	setAddNewFriendIcon(":/Resources/MainWindow/aio_toobar_addhuman.png");
	initContactTree();

	//����ǩ��
	ui.lineEdit->installEventFilter(this);
	//��������
	ui.searchLineEdit->installEventFilter(this);
	
	m_emotionWindow = new EmotionWindow;
	m_emotionWindow->hide();		//���ر��鴰��
	connect(m_emotionWindow, SIGNAL(signalEmotionItemClicked(int)), this, SLOT(onEmotionItemClicked(int)));
	connect(ui.sysmin, SIGNAL(clicked(bool)), this, SLOT(onShowHide(bool)));
	connect(ui.sysclose, SIGNAL(clicked(bool)), this, SLOT(onShowClose(bool)));
	//connect(ui.addNewFriend, SIGNAL(clicked(bool), this, SLOT(AddNewFriend()));
	connect(NotifyManager::getInstance(), &NotifyManager::signalSkinChanged, [this]() {
		updateSeachStyle();
		});
	SysTray* systray = new SysTray(this);
	QList<int> leftWidgetSize;
	leftWidgetSize << 154 << width() - 154;
	ui.splitter->setSizes(leftWidgetSize);	//���������óߴ�

	//ui.listWidget->setStyle(new CustomProxyStyle(this));

	connect(ui.listWidget, &QListWidget::itemClicked, this, &CCMainWindow::onTalkWindowItemClicked);

}

void CCMainWindow::initTcpSocket()
{
	m_tcpClientSocket = new QTcpSocket(this);
	m_tcpClientSocket->connectToHost("127.0.0.1", gtcpPort);

}

void CCMainWindow::initUdpSocket()
{
	m_udpReceiver = new QUdpSocket(this);
	for (quint16 port = gUdpPort; port < gUdpPort + 200; ++port)
	{
		if (m_udpReceiver->bind(port, QUdpSocket::ShareAddress))
			break;
	}
	connect(m_udpReceiver, &QUdpSocket::readyRead, this, &CCMainWindow::processPendingData);

}

void CCMainWindow::updateSeachStyle()
{
	ui.searchWidget->setStyleSheet(QString("QWidget#searchWidget{background-color:rgba(%1,%2,%3,50);border-bottom:1px solid rgba(%1,%2,%3,30)}\
											QPushButton#searchBtn{border-image:url(:/Resources/MainWindow/search/search_icon.png)}")
		.arg(m_colorBackGround.red())
		.arg(m_colorBackGround.green())
		.arg(m_colorBackGround.blue()));
}
bool CCMainWindow::createJSFile(QStringList& employeesList)
{
	//��ȡtxt�ļ�����
	QString strFileTxt = "Resources/MainWindow/MsgHtml/msgtmpl.txt";
	QFile fileRead(strFileTxt);
	QString strFile;
	if (fileRead.open(QIODevice::ReadOnly))
	{
		strFile = fileRead.readAll();
		fileRead.close();
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("��ȡ msgtmpl.txt ʧ�ܣ�"));
		return false;
	}
	//�滻��external0��appendHtml0�����Լ�����Ϣʹ�ã�
	QFile fileWrite("Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (fileWrite.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		//���¿�ֵ
		QString strSourceInitNull = "var external = null;";

		//���³�ʼ��
		QString strSourceInit = "external = channel.objects.external;";

		//����newWebChannel
		QString strSourceNew =
			"new QWebChannel(qt.webChannelTransport,\
			function(channel) {\
			external = channel.objects.external;\
		}\
		); \
		";
		//����׷��recvHtml���ű�����˫�����޷�ֱ�ӽ��и�ֵ�����ö��ļ���ʽ
		QString strSourceRecvHtml;
		QFile fileRecvHtml("Resources/MainWindow/MsgHtml/recvHtml.txt");
		if (fileRecvHtml.open(QIODevice::ReadOnly))
		{
			strSourceRecvHtml = fileRecvHtml.readAll();
			fileRecvHtml.close();
		}
		else
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("��ȡ recvHtml.txt ʧ�ܣ�"));
			return false;
		}
		//�����滻��Ľű�
		QString strReplaceInitNull;
		QString strReplaceInit;
		QString strReplaceNew;
		QString strReplaceRecvHtml;

		for (int i = 0; i < employeesList.length(); i++)
		{
			//�༭�滻��Ŀ�ֵ
			QString strInitNull = strSourceInitNull;
			strInitNull.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInitNull += strInitNull;
			strReplaceInitNull += "\n";

			//�༭�滻��ĳ�ʼֵ
			QString strInit = strSourceInit;
			strInit.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInit += strInit;
			strReplaceInit += "\n";

			//�༭�滻��� newWebChannel
			QString strNew = strSourceNew;
			strNew.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceNew += strNew;
			strReplaceNew += "\n";

			//�༭�滻��� recvHtml
			QString strRecvHtml = strSourceRecvHtml;
			strRecvHtml.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strRecvHtml.replace("recvHtml", QString("recvHtml_%1").arg(employeesList.at(i)));
			strReplaceRecvHtml += strRecvHtml;
			strReplaceRecvHtml += "\n";
		}
		strFile.replace(strSourceInitNull, strReplaceInitNull);
		strFile.replace(strSourceInit, strReplaceInit);
		strFile.replace(strSourceNew, strReplaceNew);
		strFile.replace(strSourceRecvHtml, strReplaceRecvHtml);

		QTextStream stream(&fileWrite);
		stream << strFile;
		fileWrite.close();

		return true;
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("д msgtmpl.js ʧ�ܣ�"));
		return false;
	}
}
void CCMainWindow::addCompanyDeps(QTreeWidgetItem* pRootGroupItem, int friendId)
{
	QTreeWidgetItem* pChild = new QTreeWidgetItem;
	qDebug() << friendId;
	QPixmap pix;
	pix.load(":/Resources/MainWindow/head_mask.png");



	QSqlQuery friendName(QString("SELECT userName FROM tab_user WHERE userAccount =%1").arg(friendId));
	friendName.exec();
	friendName.first();
	//����ӽڵ�
	pChild->setData(0, Qt::UserRole, 1);//����������Ϊ1
	pChild->setData(0, Qt::UserRole + 1, friendId);
	

	QString s = QString::number((int)pChild);
	//pChild->setData(0, Qt::UserRole + 1, QString::number((int)pChild));

	ContactItem* pContactItem = new ContactItem(ui.treeWidget);
	pContactItem->setHeadPixmap(getRoundImage(QPixmap(":/Resources/MainWindow/girl.png"), pix, pContactItem->getHeadLabelSize()));
	pContactItem->setUserName(friendName.value(0).toString());

	pRootGroupItem->addChild(pChild);
	ui.treeWidget->setItemWidget(pChild, 0, pContactItem);

	m_groupMap.insert(pChild, friendName.value(0).toString());
}
void CCMainWindow::setHeadPixmap(const QString& headPath)
{
	QPixmap pix;
	pix.load(":/Resources/MainWindow/head_mask.png");
	ui.headLabel->setPixmap(getRoundImage(QPixmap(headPath), pix, ui.headLabel->size()));
}

void CCMainWindow::setUserName(const QString& username)
{
	ui.nameLabel->adjustSize();    //�������ݵ����Լ��ĳߴ�

	//�ı����������ʡ��...
	//fontMetrics()����QFontMetrics�����
	QString name = ui.nameLabel->fontMetrics().elidedText(username, Qt::ElideRight, ui.nameLabel->width());    //Qt::ElideRight ��ʾ���ұ�ʡ��

	ui.nameLabel->setText(name);
}

void CCMainWindow::setStatusMenuIcon(const QString& statusPath)
{
	QPixmap statusBtnPixmap(ui.stausBtn->size());
	statusBtnPixmap.fill(Qt::transparent);

	QPainter painter(&statusBtnPixmap);
	painter.drawPixmap(4, 4, QPixmap(statusPath));

	ui.stausBtn->setIcon(statusBtnPixmap);
	ui.stausBtn->setIconSize(ui.stausBtn->size());
}

void CCMainWindow::setAddNewFriendIcon(const QString& addPath)
{
	QPixmap addNewFriendPixmap(ui.addNewFriend->size());
	addNewFriendPixmap.fill(Qt::transparent);

	QPainter painter(&addNewFriendPixmap);
	painter.drawPixmap(0, 0, QPixmap(addPath));

	ui.addNewFriend->setIcon(addNewFriendPixmap);
	ui.addNewFriend->setIconSize(ui.addNewFriend->size());
}

void CCMainWindow::initContactTree()
{

	//չ��������ʱ���ź�
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onItemExpanded(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(onItemCollapsed(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

	//���ڵ�
	QTreeWidgetItem* pRootGroupItem = new QTreeWidgetItem;
	pRootGroupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	pRootGroupItem->setData(0, Qt::UserRole, 0);//����������Ϊ0

	RootContatItem* pItemName = new RootContatItem(true, ui.treeWidget);


	QString strGroupName = QString::fromLocal8Bit("�ҵĺ���");
	pItemName->setText(strGroupName);

	//�������ڵ�
	ui.treeWidget->addTopLevelItem(pRootGroupItem);
	ui.treeWidget->setItemWidget(pRootGroupItem, 0, pItemName);
	QSqlQuery friendAcout(QString("SELECT friends FROM tab_relationship WHERE userAccount =%1").arg(gLoginEmployeeID));
	int friends = friendAcout.size();
	friendAcout.exec();

	while (friendAcout.next())
	{

		QSqlQuery friendName(QString("SELECT userName FROM tab_user WHERE userAccount =%1").arg(friendAcout.value(0).toInt()));
		friendName.exec();
		friendName.first();

		m_friends << friendName.value(0).toString();
		addCompanyDeps(pRootGroupItem, friendAcout.value(0).toInt());

	}
		


}

void CCMainWindow::onTalkWindowItemClicked(QListWidgetItem* item)
{
	QWidget* talkwindowWidget = m_talkwindowItemMap.find(item).value();
	ui.leftStackedWidget->setCurrentWidget(talkwindowWidget);
}


void CCMainWindow::addTalkWindow(TalkWindow* talkWindow,TalkWindowItem* talkWindowItem, const QString friendeName)
{
	ui.leftStackedWidget->addWidget(talkWindow);
	connect(m_emotionWindow, SIGNAL(signalEmotionWindowHide()),
		talkWindow, SLOT(onSetEmotionBtnStatus()));
	QListWidgetItem* aItem = new QListWidgetItem(ui.listWidget);
	aItem->setSizeHint(QSize(200, 43));
	m_talkwindowItemMap.insert(aItem, talkWindow);

	aItem->setSelected(true);

	talkWindowItem->setHeadPixmap("");//����ͷ��
	ui.listWidget->addItem(aItem);
	ui.listWidget->setItemWidget(aItem, talkWindowItem);

	onTalkWindowItemClicked(aItem);

	connect(talkWindowItem, &TalkWindowItem::signalCloseClicked,
		[talkWindowItem, talkWindow, aItem, this]() {
			m_talkwindowItemMap.remove(aItem);
			talkWindow->close();
			ui.listWidget->takeItem(ui.listWidget->row(aItem));
			delete talkWindowItem;
			ui.leftStackedWidget->removeWidget(talkWindow);
			if (ui.leftStackedWidget->count() < 1)
				close();
		});


}

const QMap<QListWidgetItem*, QWidget*>& CCMainWindow::getTalkWindowItemMap() const
{
	return m_talkwindowItemMap;
}

void CCMainWindow::setCurrentWidget(QWidget* widget)
{
	ui.leftStackedWidget->setCurrentWidget(widget);
}

void CCMainWindow::test1()
{
	QString sum = m_friends.size();
	QMessageBox::information(this, "��ʾ", sum);
}

void CCMainWindow::resizeEvent(QResizeEvent* event)
{
	setUserName(QString::fromLocal8Bit("̫��֮��"));
	BasicWindow::resizeEvent(event);
}

bool CCMainWindow::eventFilter(QObject* obj, QEvent* event)
{
	if (ui.searchLineEdit == obj)
	{
		//���̽����¼�
		if (event->type() == QEvent::FocusIn)
		{
			ui.searchWidget->setStyleSheet(QString("QWidget#searchWidget{background-color:rgb(255,255,255);border-bottom:1px solid rgba(%1,%2,%3,100)}\
													QPushButton#searchBtn{border-image:url(:/Resources/MainWindow/search/main_search_deldown.png)} \
													QPushButton#searchBtn:hover{border-image:url(:/Resources/MainWindow/search/main_search_delhighlight.png)} \
													QPushButton#searchBtn:pressed{border-image:url(:/Resources/MainWindow/search/main_search_delhighdown.png)}")
				.arg(m_colorBackGround.red())
				.arg(m_colorBackGround.green())
				.arg(m_colorBackGround.blue()));
		}
		else if (event->type() == QEvent::FocusOut)
		{
			updateSeachStyle();
		}
	}
	return false;
}
void CCMainWindow::mousePressEvent(QMouseEvent* event)
{
	if (qApp->widgetAt(event->pos()) != ui.searchLineEdit && ui.searchLineEdit->hasFocus())
	{
		ui.searchLineEdit->clearFocus();
	}
	else if (qApp->widgetAt(event->pos()) != ui.lineEdit && ui.lineEdit->hasFocus())
	{
		ui.lineEdit->clearFocus();
	}

	BasicWindow::mousePressEvent(event);
}

void CCMainWindow::getEmployeesID(QStringList& employeesList)
{
	QSqlQueryModel queryModel;
	queryModel.setQuery("SELECT userAccount FROM tab_user WHERE status = 1");

	//����ģ�͵�������(�û�������)
	int employeesNum = queryModel.rowCount();
	QModelIndex index;
	for (int i = 0; i < employeesNum; ++i)
	{
		index = queryModel.index(i, 0);//�У���
		employeesList << queryModel.data(index).toString();
	}

}

void CCMainWindow::handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg)
{
	QMsgTextEdit msgTextEdit;	// ��Ϣ�ı��༭��
	msgTextEdit.setText(strMsg);

	// ����ֻ���� �ı���Ϣ��������Ϣ��
	// �ļ����ͣ��������������
	if (msgType == 1)	// �ı���Ϣ
	{
		// ����Ϣ��ת��Ϊ html
		msgTextEdit.document()->toHtml();
	}
	else if (msgType == 0)	// ������Ϣ
	{
		// ÿ��������ռ���
		const int emotionWidth = 3;
		// ����������������ݳ��� ���� ÿ��������
		int emotionNum = strMsg.length() / emotionWidth;

		// ���������е� ���飬���html��ȥ
		for (int i = 0; i < emotionNum; i++)
		{

			msgTextEdit.addEmotionUrl(strMsg.mid(i * 3, emotionWidth).toInt());
		}
	}

	QString hemlText = msgTextEdit.document()->toHtml();

	if (!hemlText.contains(".png") && !hemlText.contains("</span>"))
	{
		QString fontHtml;
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			// ��html�ļ���� %1�����ַ��� text �滻
			fontHtml.replace("%1", strMsg);
			file.close();
		}
		else
		{
			// this����ǰ���첿������Ϊ������
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("�ļ� msgFont.txt �����ڣ�"));
			return;
		}

		// �ж�ת������û�а��� fontHtml
		if (!hemlText.contains(fontHtml))
		{
			hemlText.replace(strMsg, fontHtml);
		}
	}
	TalkWindow* talkWindow = dynamic_cast<TalkWindow*>(ui.leftStackedWidget->currentWidget());
	talkWindow->ui.msgWidget->appendMsg(hemlText, QString::number(senderEmployeeID));
}

QString CCMainWindow::getHeadPicPath()
{
	QString strPicPath;

		QSqlQuery queryPic(QString("SELECT picture FROM tab_user WHERE userAccount = %1")
			.arg(gLoginEmployeeID));
		queryPic.exec();
		queryPic.next();			// ָ�������ĵ�һ����¼

		strPicPath = queryPic.value(0).toString();


	gstrLoginHeadPath = strPicPath;
	return strPicPath;

}


void CCMainWindow::onItemExpanded(QTreeWidgetItem* item)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();
	if (!bIsChild)
	{
		//dynamic_cast ���������ָ��(������)ת�����̳���ָ��
		RootContatItem* prootItem = dynamic_cast<RootContatItem*>(ui.treeWidget->itemWidget(item, 0));
		if (prootItem)
		{
			prootItem->setExpanded(true);
		}

	}
}

void CCMainWindow::onItemCollapsed(QTreeWidgetItem* item)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();
	if (!bIsChild)
	{
		//dynamic_cast ���������ָ��(������)ת�����̳���ָ��
		RootContatItem* prootItem = dynamic_cast<RootContatItem*>(ui.treeWidget->itemWidget(item, 0));
		if (prootItem)
		{
			prootItem->setExpanded(false);

		}

	}
}
void CCMainWindow::onItemClicked(QTreeWidgetItem* item, int column)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();
	if (!bIsChild)
	{
		item->setExpanded(!item->isExpanded());//δչ����չ������
	}
}
void CCMainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();
	qDebug() << item->data(0, Qt::UserRole + 1).toString();
	//QMessageBox::information(this, QString::fromLocal8Bit("tip"), item->data(0, Qt::UserRole+1).toString());
	if (bIsChild)
	{
		const QString strGroup = m_groupMap.value(item);
		/*WindowManager::getInstance()->addNewTalkWindow(*this,item->data(0, Qt::UserRole + 1).toString(), COMPANY);*/
		for (int nIndex = 0; nIndex < m_friends.length(); nIndex++)
		{

			if (strGroup == m_friends.at(nIndex))
			{
				WindowManager::getInstance()->addNewTalkWindow(*this, item->data(0, Qt::UserRole + 1).toString(),strGroup);
				//QMessageBox::information(this, QString::fromLocal8Bit("tip"), item->data(0, Qt::UserRole + 1).toString());
			}
		}
	}
}
void CCMainWindow::onEmotionBtnClicked(bool)
{
	m_emotionWindow->setVisible(!m_emotionWindow->isVisible());
	QPoint emotionPoint = this->mapToGlobal(QPoint(0, 0));//����ǰ�ؼ������λ��ת��Ϊ��Ļ�ľ���λ��

	emotionPoint.setX(emotionPoint.x() + 170);
	emotionPoint.setY(emotionPoint.y() + 220);
	m_emotionWindow->move(emotionPoint);
}

void CCMainWindow::onEmotionItemClicked(int emotionNum)
{
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.leftStackedWidget->currentWidget());
	if (curTalkWindow)
	{
		curTalkWindow->addEmotionImage(emotionNum);
	}
}

void CCMainWindow::processPendingData()
{
	//�˿�����δ���������
	while (m_udpReceiver->hasPendingDatagrams())
	{
		const static int groupFlagWidth = 1;	//Ⱥ�ı�־ռλ
		const static int groupWidth = 4;		//Ⱥ���
		const static int employeeWidth = 6;		//�û��˺ſ��
		const static int msgTypeWidth = 1;		//��Ϣ���Ϳ��
		const static int msgLengthWidth = 5;	//�ı���Ϣ���ȵĿ��
		const static int pictureWidth = 3;		//����ͼƬ�Ŀ��

		//��ȡudp����
		QByteArray btData;
		btData.resize(m_udpReceiver->pendingDatagramSize());
		m_udpReceiver->readDatagram(btData.data(), btData.size());

		QString strData = btData.data();
		QString strWindowID;//���촰��ID,Ⱥ������Ⱥ�ţ����������û��˺�
		QString strSendEmployeeID, strRecevieEmployeeID;//���ͼ����ն˵��û��˺�
		QString strMsg;		//����

		int msgLength;	//���ݳ���
		int msgType;//��������

		strSendEmployeeID = strData.mid(groupFlagWidth, employeeWidth);

		//�Լ�������Ϣ��������
		if (strSendEmployeeID == gLoginEmployeeID)
		{
			return;
		}

		if (btData[0] == '1')//Ⱥ��
		{
			//Ⱥ��
			strWindowID = strData.mid(groupFlagWidth + employeeWidth, groupWidth);

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + groupWidth];
			if (cMsgType == '1')//�ı���Ϣ
			{
				msgType = 1;
				msgLength = strData.mid(groupFlagWidth + employeeWidth
					+ groupWidth + msgTypeWidth, msgLengthWidth).toInt();
				strMsg = strData.mid(groupFlagWidth + employeeWidth
					+ groupWidth + msgType + msgLengthWidth, msgLength);
			}
			else if (cMsgType == '0')//������Ϣ
			{
				msgType = 0;
				int posImages = strData.indexOf("images");
				strMsg = strData.right(strData.length() - posImages - QString("images").length());
			}
			else if (cMsgType == '2')//�ļ���Ϣ
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int posData_begin = strData.indexOf("data_begin");

				//�ļ�����
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);

				//�ļ�����
				int dataLengthWidth;
				int posData = posData_begin + QString("data_begin").length();
				strMsg = strData.mid(posData);

				//����employeeID��ȡ����������
				QString sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querysenderName(QString("SELECT userName FROM tab_user WHERE userAccount = %1")
					.arg(employeeID));
				querysenderName.exec();

				if (querysenderName.first())
				{
					sender = querysenderName.value(0).toString();
				}

				//�����ļ��ĺ�������������
				RecevieFile* recvFile = new RecevieFile(this);

				//// ���˵���ȡ�������� �����ź�
				connect(recvFile, &RecevieFile::refuseFile, [this]()
					{
						return;
					});

				// �յ�xxx����Ϣ
				QString msgLabel = QString::fromLocal8Bit("�յ�����") + sender
					+ QString::fromLocal8Bit("�������ļ����Ƿ���գ�");
				// ���ı��ַ��������õ���ǩ��
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		else//����
		{
			// ��ȡ�����ߵ��˺�
			strRecevieEmployeeID = strData.mid(groupFlagWidth + employeeWidth, employeeWidth);
			// ��ȡ������ID�ǳƣ��ַ���
			strWindowID = strSendEmployeeID;

			// �����ߵ�ID �� ��½�ߵ�ID ������һ���ģ���ֱ�ӷ���
			if (strRecevieEmployeeID != gLoginEmployeeID)
			{
				return;
			}

			// ��ȡ��Ϣ������
			QChar cMsgType = btData[groupFlagWidth + employeeWidth + employeeWidth];

			// �ж���Ϣ����
			// �ı���Ϣ
			if (cMsgType == '1')
			{
				msgType = 1;

				// ��ȡ���ı���Ϣ�ĳ���
				msgLength = strData.mid(groupFlagWidth + employeeWidth + employeeWidth
					+ msgTypeWidth, msgLengthWidth).toInt();

				// �ı���Ϣ
				strMsg = strData.mid(groupFlagWidth + employeeWidth + employeeWidth
					+ msgTypeWidth + msgLengthWidth, msgLength);
			}
			// ������Ϣ
			else if (cMsgType == '0')
			{
				msgType = 0;
				int posImages = strData.indexOf("images");
				int imagesWidth = QString("images").length();


				strMsg = strData.mid(posImages + imagesWidth);

			}
			// �ļ���Ϣ
			else if (cMsgType == '2')
			{
				msgType = 2;

				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int data_beginWidth = QString("data_begin").length();
				int posData_begin = strData.indexOf("data_begin");

				// �ļ�����
				QString fileName = strData.mid(posBytes + bytesWidth
					, posData_begin - posBytes - bytesWidth);
				gfileName = fileName;

				// �ļ�����
				strMsg = strData.mid(posData_begin + data_beginWidth);
				gfileData = strMsg;

				// �����û��˺Ż�ȡ����������
				QString sender;
				int empID = strSendEmployeeID.toInt();	// ת��������
				QSqlQuery querySenderName(QString("SELECT userName FROM tab_user WHERE userAccount = %1")
					.arg(empID));
				querySenderName.exec();		// ִ������SQL���

				// �жϣ����ݿ��� �Ƿ�������
				if (querySenderName.first())
				{
					sender = querySenderName.value(0).toString();
				}

				// �����ļ��ĺ�������...
				RecevieFile* recvFile = new RecevieFile(this);

				// ���˵���ȡ�������� �����ź�
				connect(recvFile, &RecevieFile::refuseFile, [this]()
					{
						return;
					});

				// �յ�xxx����Ϣ
				QString msgLabel = QString::fromLocal8Bit("�յ�����") + sender
					+ QString::fromLocal8Bit("�������ļ����Ƿ���գ�");
				// ���ı��ַ��������õ���ǩ��
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		// �����촰�ڣ���Ϊ��Ĵ���
		QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);

		// �жϴ����Ƿ��
		if (widget)
		{
			// �Ѵ��ڣ�����Ϊ�����
			this->setCurrentWidget(widget);

			// ����������б�ͬ������
			// ͨ��ӳ�䣬��ȡ�������촰�ڵ�ֵ��
			// ���浽 QListWidgetItem ����������
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);	// ��Ϊѡ�У��״̬
		}
		else
		{
			QSqlQuery friendName(QString("SELECT userName FROM tab_user WHERE userAccount =%1").arg(strWindowID.toInt()));
			friendName.exec();
			friendName.first();
			WindowManager::getInstance()->addNewTalkWindow(*this, strWindowID, friendName.value(0).toString());
			QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);
			this->setCurrentWidget(widget);

			// ����������б�ͬ������
			// ͨ��ӳ�䣬��ȡ�������촰�ڵ�ֵ��
			// ���浽 QListWidgetItem ����������
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);	// ��Ϊѡ�У��״̬

			return;		// �����ڣ�ֱ�ӷ���
		}

		// ����Ϣ�������жϣ�������ļ����ͣ��򲻵��� handleReceivedMsg()
		if (msgType != 2)
		{
			int sendEmployeeID = strSendEmployeeID.toInt();
			// "��ҳ"��׷������
			handleReceivedMsg(sendEmployeeID, msgType, strMsg);
		}
	}

}

//�ı����ݰ���ʽ��Ⱥ�ı�־ + ����Ϣ�˺� + ����Ϣ�û��� + ��Ϣ���� + ���ݳ��� + ����
//�������ݰ���ʽ��Ⱥ�ı�־ + ����Ϣ�˺� + ����Ϣ�û��� + ��Ϣ���� + ������� + images + ����
//msgType 0������Ϣ 1�ı���Ϣ 2�ļ���Ϣ
void CCMainWindow::updateSendTcpMsg(QString& strData, int& msgType, QString fileName)
{
	//��ȡ��ǰ����촰��
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.leftStackedWidget->currentWidget());
	QString talkId = curTalkWindow->getTalkId();

	QString strGroupFlag;
	QString strSend;

	if (talkId.length() == 4)//Ⱥ�ĳ���
	{
		strGroupFlag = "1";
	}
	else
	{
		strGroupFlag = "0";
	}

	int nstrDataLength = strData.length();
	int dataLength = QString::number(nstrDataLength).length();
	QString strdataLength;

	if (msgType == 1)//�����ı���Ϣ
	{
		//�ı���Ϣ�ĳ���Լ��Ϊ5λ
		if (dataLength == 1)
		{
			strdataLength = "0000" + QString::number(nstrDataLength);
		}
		else if (dataLength == 2)
		{
			strdataLength = "000" + QString::number(nstrDataLength);
		}
		else if (dataLength == 3)
		{
			strdataLength = "00" + QString::number(nstrDataLength);
		}
		else if (dataLength == 4)
		{
			strdataLength = "0" + QString::number(nstrDataLength);
		}
		else if (dataLength == 5)
		{
			strdataLength = QString::number(nstrDataLength);
		}
		else
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("����������ݳ��ȣ�"));
		}

		//�ı����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ���� + ���ݳ��� + ����
		strSend = strGroupFlag + gLoginEmployeeID + talkId + "1" + strdataLength + strData;
	}
	else if (msgType == 0)//������Ϣ
	{
		//�������ݰ���ʽ��Ⱥ�ı�־ + ����Ϣ�û��� + ����Ϣ�û���
		//+ ��Ϣ���� + ������� + images + ����
		strSend = strGroupFlag + gLoginEmployeeID + talkId
			+ "0" + strData;
	}
	else if (msgType == 2)//�ļ�
	{
		//�ļ����ݰ���ʽ��Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ�ţ�ȺQQ�ţ� 
		//+ ��Ϣ����(2) + �ļ����� + "bytes" + �ļ����� + "data_begin" + �ļ�����

		QByteArray bt = strData.toUtf8();
		QString strLength = QString::number(bt.length());

		strSend = strGroupFlag + gLoginEmployeeID + talkId
			+ "2" + strLength + "bytes" + fileName + "data_begin" + strData;
	}

	QByteArray dataBt;
	dataBt.resize(strSend.length());
	dataBt = strSend.toUtf8();
	m_tcpClientSocket->write(dataBt);


}
