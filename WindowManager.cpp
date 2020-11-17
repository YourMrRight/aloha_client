#include "WindowManager.h"
#include "TalkWindow.h"
#include "CCMainWindow.h"
#include <QMessageBox>
#include <qwidget.h>
#include <QDebug>
#include "TalkWindowItem.h"
//单例模式，创建全局静态对象
Q_GLOBAL_STATIC(WindowManager, theInstance)

WindowManager::WindowManager()
	:QObject(nullptr)
	,m_ccTalkwindow(nullptr)
{

}

WindowManager::~WindowManager()
{
}

QWidget* WindowManager::findWindowName(const QString& qsWindowName)
{
	if (m_windowMap.contains(qsWindowName))
	{
		return m_windowMap.value(qsWindowName);
	}

	return nullptr;
}

void WindowManager::deleteWindowName(const QString& qsWindowName)
{
	m_windowMap.remove(qsWindowName);
}

void WindowManager::addWindowName(const QString& qsWindowName, QWidget* qWidget)
{
	if (!m_windowMap.contains(qsWindowName))
	{
		m_windowMap.insert(qsWindowName, qWidget);
	}
}

WindowManager* WindowManager::getInstance()
{
	return theInstance();
}

void WindowManager::addNewTalkWindow(CCMainWindow& parent, const QString& uid, const QString& friendName)
{
	this->m_ccTalkwindow = &parent;
	QWidget* widget = findWindowName(uid);
	qDebug() << friendName;
	if (!widget)
	{
		m_strCreatingTalkId = uid;
		TalkWindow* talkwindow = new TalkWindow(&parent, uid);
		TalkWindowItem* talkwindowItem = new TalkWindowItem(talkwindow);
		/*talkwindowItem->show();*/
		m_strCreatingTalkId = "";
		talkwindow->setWindowName(friendName);
		talkwindowItem->setMsgLabelContent(friendName);


		parent.addTalkWindow(talkwindow, talkwindowItem,friendName);

	}
	else
	{
		QListWidgetItem* item = parent.getTalkWindowItemMap().key(widget);
		item->setSelected(true);

		//设置右侧当前聊天窗口 
		parent.setCurrentWidget(widget);
	}
	parent.show();
	parent.activateWindow();
}

CCMainWindow* WindowManager::getccTalkWindow()
{
	return m_ccTalkwindow;
}

TalkWindow* WindowManager::getTalkwindow()
{
	return m_talkwindow;
}

QString WindowManager::getCreatingID()
{
	return m_strCreatingTalkId;
}
