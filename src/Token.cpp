#include "Token.h"

namespace zephyr {

	Token::Token() {
		name = "";
		avatar = "";
		room = "";
		token = "";
	}

	Token::Token(QString name, QString avatar, QString room, QString token)
		: name(name),
		avatar(avatar),
		room(room),
		token(token)
	{ }

	QString Token::getName() {
		return name;
	}

	QString Token::getAvatar() {
		return avatar;
	}

	QString Token::getRoom() {
		return room;
	}

	QString Token::getToken() {
		return token;
	}

	bool Token::isLoggedIn() {
		return !name.isEmpty() && !avatar.isEmpty() && !room.isEmpty() && !token.isEmpty();
	}

}