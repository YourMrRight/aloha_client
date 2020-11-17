#pragma once

#include <QWidget>
#include "ui_TalkWindow.h"
#include "CCMainWindow.h"
#include "basicwindow.h"
enum GroupType {
	COMPANY = 0,	
	PTOP = 1		
};
class TalkWindow : public QWidget{
	Q_OBJECT

public:
	TalkWindow(QWidget* parent, const QString& uid);
	~TalkWindow();

public:
	void addEmotionImage(int emotionNum);
	void setWindowName(const QString& name);
	QString getTalkId();
private slots:
	void onSendBtnClicked(bool);
	void onItemDoubleClicked();
	void onFileOpenBtnClicked(bool);

private:
	void initControl();

	void initCompanyTalk();
	void initPtoPTalk();		//��ʼ������
	void addPeopInfo(QTreeWidgetItem* pRootGroupItem);

private:
	Ui::TalkWindow ui;
	QString m_talkId;
	GroupType m_groupType;
	QMap<QTreeWidgetItem*, QString> m_groupPeopleMap;//���з�����ϵ������
	friend class CCMainWindow;
public slots:
	void testnihao();

};
