
#pragma once

#include <QObject>

class QQuickWindow;

namespace zephyr {

class OverlayController;

class AboutTabController : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString apiVersion READ getApiVersion)
	Q_PROPERTY(QString settingsFile READ getSettingsFile)
	Q_PROPERTY(QString logConfiguration READ getLogConfiguration)
	Q_PROPERTY(QString vrRuntime READ getVrRuntime)

private:
	OverlayController* parent;
	QQuickWindow* widget;

	QString settingsFile = "Unknown";
	QString logConfiguration = "Unknown";
	QString logFile = "Unknown";
	QString vrRuntime = "Unknown";

public:
	void initStage1();
	void initStage2(OverlayController* parent, QQuickWindow* widget);
	void eventLoopTick();
	
	QString getApiVersion() const;

	void setSettingsFile(QString str);
	QString getSettingsFile() const;

	void setLogConfiguration(QString str);
	QString getLogConfiguration() const;

	void setVrRuntime(QString str);
	QString getVrRuntime() const;

public slots:
	void openUrl(QString url);
};

}
