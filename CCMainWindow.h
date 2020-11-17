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
	void setUserName(const QString& username);	//�����û���
	void setHeadPixmap(const QString& headPath);//����ͷ��
	void setStatusMenuIcon(const QString& statusPath);//����״̬
	void setAddNewFriendIcon(const QString& addPath); //������Ӻ���
	void initContactTree();
	void addTalkWindow(TalkWindow* talkWindow,TalkWindowItem* talkwindowItem,const QString friendeName);
	const QMap<QListWidgetItem*, QWidget*>& getTalkWindowItemMap()const;
	void setCurrentWidget(QWidget* widget);
	void test1();

private:
//	void initTimer();	//��ʼ����ʱ��
	void initControl();
	void initTcpSocket();	//��ʼ��TCP
	void initUdpSocket();	//��ʼ��UDP
	void updateSeachStyle(); //����������ʽ
	bool createJSFile(QStringList& employeesList);
	void addCompanyDeps(QTreeWidgetItem* pRootGroupItem, int friendId);

private:
	void resizeEvent(QResizeEvent* event);      // �ߴ��¼�
	bool eventFilter(QObject* obj, QEvent* event);    //obj ���Ӷ��� 
	void mousePressEvent(QMouseEvent* event);
	void getEmployeesID(QStringList& employeesList);  //��ȡ�����û��˺�
	void handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg);
	QString getHeadPicPath();		// ��ȡͷ��·��
private slots:
	void onItemClicked(QTreeWidgetItem* item, int column);
	void onItemExpanded(QTreeWidgetItem* item);
	void onItemCollapsed(QTreeWidgetItem* item);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
	void onEmotionBtnClicked(bool);
	void onEmotionItemClicked(int emotionNum);	//���鱻ѡ��

private slots:
	void onTalkWindowItemClicked(QListWidgetItem* item);//����б�����ִ�еĲۺ���
	//void AddNewwFriend();
	void processPendingData();//����UDP�㲥�յ�������
private:
	Ui::CCMainWindowClass ui;
	QMap<QTreeWidgetItem*, QString> m_groupMap;//���з���ķ�����
	QMap<QListWidgetItem*, QWidget*> m_talkwindowItemMap;//�򿪵����촰��
	EmotionWindow* m_emotionWindow;	//���鴰��
	QStringList m_friends;
private:
	QTcpSocket* m_tcpClientSocket;	//Tcp�ͻ���
	QUdpSocket* m_udpReceiver;		//udp���ն�
public slots:
	void updateSendTcpMsg(QString& strData, int& msgType, QString sFile = "");
};