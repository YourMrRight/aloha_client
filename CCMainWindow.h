#pragma once

#include <QtWidgets/QWidget>
#include "basicwindow.h"
#include "ui_CCMainWindow.h"
#include "TalkWindow.h"
#include "EmotionWindow.h"
#include "TalkWindowItem.h"
#include <QTcpSocket>
#include <QUdpSocket>
const int gtcpPort = 8888;
class QTreeWidgetItem;
class TalkWindow;
class CCMainWindow : public BasicWindow
{
	Q_OBJECT
public:
	CCMainWindow(QWidget* parent = Q_NULLPTR);
	~CCMainWindow();
	void setUserName(const QString& username);	//设置用户名
	void setHeadPixmap(const QString& headPath);//设置头像
	void setStatusMenuIcon(const QString& statusPath);//设置状态
	void setAddNewFriendIcon(const QString& addPath); //设置添加好友
	void initContactTree();
	void addTalkWindow(TalkWindow* talkWindow,TalkWindowItem* talkwindowItem,const QString friendeName);
	const QMap<QListWidgetItem*, QWidget*>& getTalkWindowItemMap()const;
	void setCurrentWidget(QWidget* widget);
	void test1();

private:
//	void initTimer();	//初始化计时器
	void initControl();
	void initTcpSocket();	//初始化TCP
	void initUdpSocket();	//初始化UDP
	void updateSeachStyle(); //更新搜索样式
	bool createJSFile(QStringList& employeesList);
	void addCompanyDeps(QTreeWidgetItem* pRootGroupItem, int friendId);

private:
	void resizeEvent(QResizeEvent* event);      // 尺寸事件
	bool eventFilter(QObject* obj, QEvent* event);    //obj 监视对象 
	void mousePressEvent(QMouseEvent* event);
	void getEmployeesID(QStringList& employeesList);  //获取所有用户账号
	void handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg);
	QString getHeadPicPath();		// 获取头像路径
private slots:
	void onItemClicked(QTreeWidgetItem* item, int column);
	void onItemExpanded(QTreeWidgetItem* item);
	void onItemCollapsed(QTreeWidgetItem* item);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
	void onEmotionBtnClicked(bool);
	void onEmotionItemClicked(int emotionNum);	//表情被选中

private slots:
	void onTalkWindowItemClicked(QListWidgetItem* item);//左侧列表点击后执行的槽函数
	//void AddNewwFriend();
	void processPendingData();//处理UDP广播收到的数据
private:
	Ui::CCMainWindowClass ui;
	QMap<QTreeWidgetItem*, QString> m_groupMap;//所有分组的分组项
	QMap<QListWidgetItem*, QWidget*> m_talkwindowItemMap;//打开的聊天窗口
	EmotionWindow* m_emotionWindow;	//表情窗口
	QStringList m_friends;
private:
	QTcpSocket* m_tcpClientSocket;	//Tcp客户端
	QUdpSocket* m_udpReceiver;		//udp接收端
public slots:
	void updateSendTcpMsg(QString& strData, int& msgType, QString sFile = "");
};