#pragma once
#include <QtCore/QtCore>

namespace zephyr {

class Token {
private:
	QString name;
	QString avatar;
	QString room;
	QString token;

public:
	Token();
	Token(QString name, QString avatar, QString room, QString token);

	QString getName();
	QString getAvatar();
	QString getRoom();
	QString getToken();

	bool isLoggedIn();
};
}