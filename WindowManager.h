#pragma once

#include <QObject>
#include <CCMainWindow.h>
#include "CCMainWindow.h"
#include "TalkWindow.h"
class WindowManager : public QObject
{
	Q_OBJECT

public:
	WindowManager();
	~WindowManager();

public:
	QWidget* findWindowName(const QString& qsWindowName);
	void deleteWindowName(const QString& qsWindowName);
	void addWindowName(const QString& qsWindowName, QWidget* qWidget);

	static WindowManager* getInstance();
	void addNewTalkWindow(CCMainWindow& parent,const QString& uid, const QString& friendName);
	CCMainWindow* getccTalkWindow();
	TalkWindow* getTalkwindow();
	QString getCreatingID();

private:
	TalkWindow* m_talkwindow;
	CCMainWindow* m_ccTalkwindow;
	QMap<QString, QWidget*> m_windowMap;
	QString m_strCreatingTalkId = "";
};
