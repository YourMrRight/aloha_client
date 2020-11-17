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

// 全局变量，文件名称
QString gfileName;
// 全局变量，文件内容
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
				QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("更新js文件数据失败！"));
		}
	}
}

CCMainWindow::~CCMainWindow()
{
}

void CCMainWindow::initControl()
{
	//树获取焦点时不绘制边框
	ui.treeWidget->setStyle(new CustomProxyStyle);
	setHeadPixmap(":/Resources/MainWindow/main_logo.png");
	setStatusMenuIcon(":/Resources/MainWindow/StatusSucceeded.png");
	setAddNewFriendIcon(":/Resources/MainWindow/aio_toobar_addhuman.png");
	initContactTree();

	//个性签名
	ui.lineEdit->installEventFilter(this);
	//好友搜索
	ui.searchLineEdit->installEventFilter(this);
	
	m_emotionWindow = new EmotionWindow;
	m_emotionWindow->hide();		//隐藏表情窗口
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
	ui.splitter->setSizes(leftWidgetSize);	//分类器设置尺寸

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
	//读取txt文件数据
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
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("读取 msgtmpl.txt 失败！"));
		return false;
	}
	//替换（external0，appendHtml0用作自己发信息使用）
	QFile fileWrite("Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (fileWrite.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		//更新空值
		QString strSourceInitNull = "var external = null;";

		//更新初始化
		QString strSourceInit = "external = channel.objects.external;";

		//更新newWebChannel
		QString strSourceNew =
			"new QWebChannel(qt.webChannelTransport,\
			function(channel) {\
			external = channel.objects.external;\
		}\
		); \
		";
		//更新追加recvHtml，脚本中有双引号无法直接进行赋值，采用读文件方式
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
				QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("读取 recvHtml.txt 失败！"));
			return false;
		}
		//保存替换后的脚本
		QString strReplaceInitNull;
		QString strReplaceInit;
		QString strReplaceNew;
		QString strReplaceRecvHtml;

		for (int i = 0; i < employeesList.length(); i++)
		{
			//编辑替换后的空值
			QString strInitNull = strSourceInitNull;
			strInitNull.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInitNull += strInitNull;
			strReplaceInitNull += "\n";

			//编辑替换后的初始值
			QString strInit = strSourceInit;
			strInit.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInit += strInit;
			strReplaceInit += "\n";

			//编辑替换后的 newWebChannel
			QString strNew = strSourceNew;
			strNew.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceNew += strNew;
			strReplaceNew += "\n";

			//编辑替换后的 recvHtml
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
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("写 msgtmpl.js 失败！"));
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
	//添加子节点
	pChild->setData(0, Qt::UserRole, 1);//子项数据设为1
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
	ui.nameLabel->adjustSize();    //根据内容调整自己的尺寸

	//文本过长则进行省略...
	//fontMetrics()返回QFontMetrics类对象
	QString name = ui.nameLabel->fontMetrics().elidedText(username, Qt::ElideRight, ui.nameLabel->width());    //Qt::ElideRight 表示在右边省略

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

	//展开与收缩时的信号
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onItemExpanded(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(onItemCollapsed(QTreeWidgetItem*)));
	connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

	//根节点
	QTreeWidgetItem* pRootGroupItem = new QTreeWidgetItem;
	pRootGroupItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	pRootGroupItem->setData(0, Qt::UserRole, 0);//根项数据设为0

	RootContatItem* pItemName = new RootContatItem(true, ui.treeWidget);


	QString strGroupName = QString::fromLocal8Bit("我的好友");
	pItemName->setText(strGroupName);

	//插入分组节点
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

	talkWindowItem->setHeadPixmap("");//设置头像
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
	QMessageBox::information(this, "提示", sum);
}

void CCMainWindow::resizeEvent(QResizeEvent* event)
{
	setUserName(QString::fromLocal8Bit("太湖之光"));
	BasicWindow::resizeEvent(event);
}

bool CCMainWindow::eventFilter(QObject* obj, QEvent* event)
{
	if (ui.searchLineEdit == obj)
	{
		//键盘焦点事件
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

	//返回模型的总行数(用户的总数)
	int employeesNum = queryModel.rowCount();
	QModelIndex index;
	for (int i = 0; i < employeesNum; ++i)
	{
		index = queryModel.index(i, 0);//行，列
		employeesList << queryModel.data(index).toString();
	}

}

void CCMainWindow::handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg)
{
	QMsgTextEdit msgTextEdit;	// 信息文本编辑器
	msgTextEdit.setText(strMsg);

	// 这里只处理 文本信息，表情信息。
	// 文件类型，不调用这个方法
	if (msgType == 1)	// 文本信息
	{
		// 将信息，转换为 html
		msgTextEdit.document()->toHtml();
	}
	else if (msgType == 0)	// 表情信息
	{
		// 每个表情所占宽度
		const int emotionWidth = 3;
		// 计算表情数量，数据长度 除以 每个表情宽度
		int emotionNum = strMsg.length() / emotionWidth;

		// 遍历数据中的 表情，添加html里去
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
			// 将html文件里的 %1，用字符串 text 替换
			fontHtml.replace("%1", strMsg);
			file.close();
		}
		else
		{
			// this，当前聊天部件，作为父窗体
			QMessageBox::information(this, QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("文件 msgFont.txt 不存在！"));
			return;
		}

		// 判断转换后，有没有包含 fontHtml
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
		queryPic.next();			// 指向结果集的第一条记录

		strPicPath = queryPic.value(0).toString();


	gstrLoginHeadPath = strPicPath;
	return strPicPath;

}


void CCMainWindow::onItemExpanded(QTreeWidgetItem* item)
{
	bool bIsChild = item->data(0, Qt::UserRole).toBool();
	if (!bIsChild)
	{
		//dynamic_cast 将基类对象指针(或引用)转换到继承类指针
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
		//dynamic_cast 将基类对象指针(或引用)转换到继承类指针
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
		item->setExpanded(!item->isExpanded());//未展开则展开子项
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
	QPoint emotionPoint = this->mapToGlobal(QPoint(0, 0));//将当前控件的相对位置转换为屏幕的绝对位置

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
	//端口中有未处理的数据
	while (m_udpReceiver->hasPendingDatagrams())
	{
		const static int groupFlagWidth = 1;	//群聊标志占位
		const static int groupWidth = 4;		//群宽度
		const static int employeeWidth = 6;		//用户账号宽度
		const static int msgTypeWidth = 1;		//信息类型宽度
		const static int msgLengthWidth = 5;	//文本信息长度的宽度
		const static int pictureWidth = 3;		//表情图片的宽度

		//读取udp数据
		QByteArray btData;
		btData.resize(m_udpReceiver->pendingDatagramSize());
		m_udpReceiver->readDatagram(btData.data(), btData.size());

		QString strData = btData.data();
		QString strWindowID;//聊天窗口ID,群聊则是群号，单聊则是用户账号
		QString strSendEmployeeID, strRecevieEmployeeID;//发送及接收端的用户账号
		QString strMsg;		//数据

		int msgLength;	//数据长度
		int msgType;//数据类型

		strSendEmployeeID = strData.mid(groupFlagWidth, employeeWidth);

		//自己发的信息不做处理
		if (strSendEmployeeID == gLoginEmployeeID)
		{
			return;
		}

		if (btData[0] == '1')//群聊
		{
			//群号
			strWindowID = strData.mid(groupFlagWidth + employeeWidth, groupWidth);

			QChar cMsgType = btData[groupFlagWidth + employeeWidth + groupWidth];
			if (cMsgType == '1')//文本信息
			{
				msgType = 1;
				msgLength = strData.mid(groupFlagWidth + employeeWidth
					+ groupWidth + msgTypeWidth, msgLengthWidth).toInt();
				strMsg = strData.mid(groupFlagWidth + employeeWidth
					+ groupWidth + msgType + msgLengthWidth, msgLength);
			}
			else if (cMsgType == '0')//表情信息
			{
				msgType = 0;
				int posImages = strData.indexOf("images");
				strMsg = strData.right(strData.length() - posImages - QString("images").length());
			}
			else if (cMsgType == '2')//文件信息
			{
				msgType = 2;
				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int posData_begin = strData.indexOf("data_begin");

				//文件名称
				QString fileName = strData.mid(posBytes + bytesWidth, posData_begin - posBytes - bytesWidth);

				//文件内容
				int dataLengthWidth;
				int posData = posData_begin + QString("data_begin").length();
				strMsg = strData.mid(posData);

				//根据employeeID获取发送者姓名
				QString sender;
				int employeeID = strSendEmployeeID.toInt();
				QSqlQuery querysenderName(QString("SELECT userName FROM tab_user WHERE userAccount = %1")
					.arg(employeeID));
				querysenderName.exec();

				if (querysenderName.first())
				{
					sender = querysenderName.value(0).toString();
				}

				//接收文件的后续操作。。。
				RecevieFile* recvFile = new RecevieFile(this);

				//// 用了点了取消，发送 返回信号
				connect(recvFile, &RecevieFile::refuseFile, [this]()
					{
						return;
					});

				// 收到xxx的信息
				QString msgLabel = QString::fromLocal8Bit("收到来自") + sender
					+ QString::fromLocal8Bit("发来的文件，是否接收？");
				// 将文本字符串，设置到标签上
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		else//单聊
		{
			// 获取接收者的账号
			strRecevieEmployeeID = strData.mid(groupFlagWidth + employeeWidth, employeeWidth);
			// 获取接收者ID昵称，字符串
			strWindowID = strSendEmployeeID;

			// 接收者的ID 和 登陆者的ID ，不是一样的，则直接返回
			if (strRecevieEmployeeID != gLoginEmployeeID)
			{
				return;
			}

			// 获取信息的类型
			QChar cMsgType = btData[groupFlagWidth + employeeWidth + employeeWidth];

			// 判断信息类型
			// 文本信息
			if (cMsgType == '1')
			{
				msgType = 1;

				// 提取，文本信息的长度
				msgLength = strData.mid(groupFlagWidth + employeeWidth + employeeWidth
					+ msgTypeWidth, msgLengthWidth).toInt();

				// 文本信息
				strMsg = strData.mid(groupFlagWidth + employeeWidth + employeeWidth
					+ msgTypeWidth + msgLengthWidth, msgLength);
			}
			// 表情信息
			else if (cMsgType == '0')
			{
				msgType = 0;
				int posImages = strData.indexOf("images");
				int imagesWidth = QString("images").length();


				strMsg = strData.mid(posImages + imagesWidth);

			}
			// 文件信息
			else if (cMsgType == '2')
			{
				msgType = 2;

				int bytesWidth = QString("bytes").length();
				int posBytes = strData.indexOf("bytes");
				int data_beginWidth = QString("data_begin").length();
				int posData_begin = strData.indexOf("data_begin");

				// 文件名称
				QString fileName = strData.mid(posBytes + bytesWidth
					, posData_begin - posBytes - bytesWidth);
				gfileName = fileName;

				// 文件内容
				strMsg = strData.mid(posData_begin + data_beginWidth);
				gfileData = strMsg;

				// 根据用户账号获取发送者姓名
				QString sender;
				int empID = strSendEmployeeID.toInt();	// 转换成整形
				QSqlQuery querySenderName(QString("SELECT userName FROM tab_user WHERE userAccount = %1")
					.arg(empID));
				querySenderName.exec();		// 执行以下SQL语句

				// 判断，数据库里 是否有数据
				if (querySenderName.first())
				{
					sender = querySenderName.value(0).toString();
				}

				// 接收文件的后续操作...
				RecevieFile* recvFile = new RecevieFile(this);

				// 用了点了取消，发送 返回信号
				connect(recvFile, &RecevieFile::refuseFile, [this]()
					{
						return;
					});

				// 收到xxx的信息
				QString msgLabel = QString::fromLocal8Bit("收到来自") + sender
					+ QString::fromLocal8Bit("发来的文件，是否接收？");
				// 将文本字符串，设置到标签上
				recvFile->setMsg(msgLabel);
				recvFile->show();
			}
		}
		// 将聊天窗口，设为活动的窗口
		QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);

		// 判断窗口是否打开
		if (widget)
		{
			// 已存在，就设为活动窗口
			this->setCurrentWidget(widget);

			// 将左侧聊天列表，同步激活
			// 通过映射，获取所有聊天窗口的值，
			// 保存到 QListWidgetItem 部件链表里
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);	// 设为选中，活动状态
		}
		else
		{
			QSqlQuery friendName(QString("SELECT userName FROM tab_user WHERE userAccount =%1").arg(strWindowID.toInt()));
			friendName.exec();
			friendName.first();
			WindowManager::getInstance()->addNewTalkWindow(*this, strWindowID, friendName.value(0).toString());
			QWidget* widget = WindowManager::getInstance()->findWindowName(strWindowID);
			this->setCurrentWidget(widget);

			// 将左侧聊天列表，同步激活
			// 通过映射，获取所有聊天窗口的值，
			// 保存到 QListWidgetItem 部件链表里
			QListWidgetItem* item = m_talkwindowItemMap.key(widget);
			item->setSelected(true);	// 设为选中，活动状态

			return;		// 不存在，直接返回
		}

		// 对信息类型做判断，如果是文件类型，则不调用 handleReceivedMsg()
		if (msgType != 2)
		{
			int sendEmployeeID = strSendEmployeeID.toInt();
			// "网页"上追加数据
			handleReceivedMsg(sendEmployeeID, msgType, strMsg);
		}
	}

}

//文本数据包格式：群聊标志 + 发信息账号 + 收信息用户号 + 信息类型 + 数据长度 + 数据
//表情数据包格式：群聊标志 + 发信息账号 + 收信息用户号 + 信息类型 + 表情个数 + images + 数据
//msgType 0表情信息 1文本信息 2文件信息
void CCMainWindow::updateSendTcpMsg(QString& strData, int& msgType, QString fileName)
{
	//获取当前活动聊天窗口
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.leftStackedWidget->currentWidget());
	QString talkId = curTalkWindow->getTalkId();

	QString strGroupFlag;
	QString strSend;

	if (talkId.length() == 4)//群的长度
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

	if (msgType == 1)//发送文本信息
	{
		//文本信息的长度约定为5位
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
				QString::fromLocal8Bit("提示"),
				QString::fromLocal8Bit("不合理的数据长度！"));
		}

		//文本数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号(群QQ号) + 信息类型 + 数据长度 + 数据
		strSend = strGroupFlag + gLoginEmployeeID + talkId + "1" + strdataLength + strData;
	}
	else if (msgType == 0)//表情信息
	{
		//表情数据包格式：群聊标志 + 发信息用户号 + 收信息用户号
		//+ 信息类型 + 表情个数 + images + 数据
		strSend = strGroupFlag + gLoginEmployeeID + talkId
			+ "0" + strData;
	}
	else if (msgType == 2)//文件
	{
		//文件数据包格式：群聊标志 + 发信息员工QQ号 + 收信息员工QQ号（群QQ号） 
		//+ 信息类型(2) + 文件长度 + "bytes" + 文件名称 + "data_begin" + 文件内容

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
