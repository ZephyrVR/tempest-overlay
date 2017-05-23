#pragma once
#define __STDC_LIMIT_MACROS 1
#define _WEBSOCKETPP_NOEXCEPT_ 1
#define _WEBSOCKETPP_CPP11_CHRONO_ 1
#undef emit
#include "sio_client.h"
#include "sio_message.h"
#define emit Q_EMIT
#include <QtCore/QtCore>
#include <QObject>
#include <qnetworkreply.h>

namespace zephyr {

class SocketHandler : public QObject {
	Q_OBJECT
	private:
		QFileSystemWatcher* watcher;
		QNetworkAccessManager* manager;
		QByteArray jsonString;
		QString jwt;
        QString room;

		bool connected;

		sio::client h;

		void OnSocketConnect(std::string const& nsp);

	public slots:
		void gotSession(QNetworkReply* reply);
		void onError(QNetworkReply::NetworkError error);

	public:
		SocketHandler();

		void connect();
		void disconnect();
		bool isConnected();
		
};
}
