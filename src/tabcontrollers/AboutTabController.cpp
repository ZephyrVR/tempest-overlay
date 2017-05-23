#include "AboutTabController.h"
#include <QQuickWindow>
#include <QDesktopServices>
#include <QNetworkInterface>
#include "../overlaycontroller.h"

namespace zephyr {


void AboutTabController::initStage1() {
	
}

void AboutTabController::initStage2(OverlayController * parent, QQuickWindow * widget) {
	this->parent = parent;
	this->widget = widget;
}

void AboutTabController::eventLoopTick() {

}

QString AboutTabController::getApiVersion() const {
	return QString(OverlayController::apiVersionString);
}

void AboutTabController::setSettingsFile(QString str) {
	settingsFile = str;
}

QString AboutTabController::getSettingsFile() const {
	return settingsFile;
}

void AboutTabController::setLogConfiguration(QString str) {
	logConfiguration = str;
}

QString AboutTabController::getLogConfiguration() const {
	return logConfiguration;
}

void AboutTabController::setVrRuntime(QString str) {
	vrRuntime = str;
}

QString AboutTabController::getVrRuntime() const {
	return vrRuntime;
}

void AboutTabController::openUrl(QString url) {
	// TODO: Open URL in Steam if able: https://partner.steamgames.com/documentation/game_overlay#activating
	QDesktopServices::openUrl(QUrl(url));
}

}
