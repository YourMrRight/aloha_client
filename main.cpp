//#include "CCMainWindow.h"
#include "UserLogin.h"
#include <QtWidgets/QApplication>
#include "EmotionWindow.h"
#include "TalkWindowItem.h"

int main(int argc, char *argv[]) 
{
	QApplication a(argc, argv);
//	QApplication::addLibraryPath("./plugins");
	UserLogin *userLogin = new UserLogin;
	userLogin->show();
	return a.exec();
}
