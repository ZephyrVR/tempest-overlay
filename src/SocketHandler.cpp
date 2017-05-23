#include "SocketHandler.h"
#include "overlaycontroller.h"
#include <QApplication>
#include <qnetworkrequest.h>
#include <qnetworkaccessmanager.h>

namespace zephyr {

	SocketHandler::SocketHandler() {
		connected = false;
	}

	void SocketHandler::connect() {
		if (connected) {
			return;
		}

		LOG(INFO) << "Connecting to socket server...";

		manager = new QNetworkAccessManager(this);

		QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(gotSession(QNetworkReply*)));
		QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));

		jsonString = ("{\"token\":\"" + OverlayController::getInstance()->userToken.getToken() + "\",\"device\":\"Zephyr Overlay\"}").toUtf8();

		QByteArray postDataSize = QByteArray::number(jsonString.size());

        QUrl serviceURL("https://zephyr.gaubert.io/api/v2/1/verify");
		QNetworkRequest request(serviceURL);

		request.setRawHeader("Authorization", "02ee8c79-a4e2-4214-bf88-99f5a8dec309");
		request.setRawHeader("Content-Type", "application/json");
		request.setRawHeader("Content-Length", postDataSize);

		QNetworkReply* reply = manager->post(request, jsonString);

        LOG(INFO) << "Verifying auth token...";
		QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	}

	void SocketHandler::gotSession(QNetworkReply* reply) {
		LOG(INFO) << "SocketHandler got reply from server.";
        QString serverReply(reply->readAll());
        QJsonDocument json = QJsonDocument::fromJson(serverReply.toUtf8());
		QJsonObject jsonObj = json.object();

		bool valid = jsonObj.value("valid").toBool();

		if (!valid) {
			LOG(WARNING) << "Invalid auth token! Logging out...";
			disconnect();
			OverlayController::getInstance()->logout();
			return;
		}

		LOG(INFO) << "Valid auth token, JWT acquired.";

		jwt = jsonObj.value("jwtToken").toString();

		h.set_socket_open_listener(std::bind(&SocketHandler::OnSocketConnect, this, std::placeholders::_1));

        LOG(INFO) << "Attempting to connect to socket server...";

        h.connect("https://zephyr.gaubert.io/");

		// Failed to authenticate with the socket server
		h.socket()->on("unauthorized", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp) {
			LOG(INFO) << "Failed to authenticate with socket server!";
            LOG(INFO) << data->get_map()["message"]->get_string();

            // Grab new JWT and try again
            disconnect();
            connect();
		}));

		// Successfully authenticated with the socket server
        h.socket()->on("authenticated", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp) {
			LOG(INFO) << "Successfully authenticated with socket server!";

            QString room = OverlayController::getInstance()->userToken.getRoom();

            h.socket()->on(room.toStdString() + "-notifications", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& notifData, bool isAck, sio::message::list &ack_resp) {
                LOG(INFO) << "Received notification, displaying...";

                OverlayController::getInstance()->showNotification(QString::fromStdString(notifData->get_map()["title"]->get_string()), QString::fromStdString(notifData->get_map()["text"]->get_string()));
            }));
		}));
	}

	void SocketHandler::onError(QNetworkReply::NetworkError error) {
		LOG(ERROR) << "SocketHandler got error when connecting: " << error;

        // Grab new JWT and try again
        disconnect();
        connect();
	}

	void SocketHandler::OnSocketConnect(std::string const& nsp) {
        // Connected, now we need to authenticate with the server
        LOG(INFO) << "Connected to server, authenticating using JWT...";

        connected = true;

        sio::object_message::ptr msg = sio::object_message::create();
        msg->get_map()["token"] = sio::string_message::create(jwt.toStdString());

        #undef emit
        h.socket()->emit("authenticate", msg);
        #define emit Q_EMIT
	}

	void SocketHandler::disconnect() {
		if (!connected)
			return;

		LOG(INFO) << "Disconnecting from socket server...";

        h.close();

		LOG(INFO) << "Disconnected from socket server.";

		connected = false;
	}

	bool SocketHandler::isConnected() {
		return connected;
	}

}
