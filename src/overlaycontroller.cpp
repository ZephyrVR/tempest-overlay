#include "overlaycontroller.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QQuickView>
#include <QApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QOpenGLExtraFunctions>
#include <QCursor>
#include <QProcess>
#include <QMessageBox>
#include <exception>
#include <iostream>
#include <cmath>
#include <openvr.h>
#include "logging.h"
#include "utils/Matrix.h"

namespace zephyr {

std::unique_ptr<OverlayController> OverlayController::singleton;

QSettings* OverlayController::_appSettings = nullptr;

OverlayController::~OverlayController() {
	Shutdown();
}

void OverlayController::Init(QQmlEngine* qmlEngine) {
	auto initError = vr::VRInitError_None;
	vr::VR_Init(&initError, vr::VRApplication_Overlay);
	if (initError != vr::VRInitError_None) {
		if (initError == vr::VRInitError_Init_HmdNotFound || initError == vr::VRInitError_Init_HmdNotFoundPresenceFailed) {
			QMessageBox::critical(nullptr, "Zephyr", "Could not find HMD!");
		}

		throw std::runtime_error(std::string("Failed to initialize OpenVR: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(initError))));
	}

	m_runtimePathUrl = QUrl::fromLocalFile(vr::VR_RuntimePath());
	LOG(INFO) << "VR Runtime Path: " << m_runtimePathUrl.toLocalFile();
	aboutTabController.setVrRuntime(m_runtimePathUrl.toLocalFile());

	QSurfaceFormat format;
	format.setVersion(2, 1);
	format.setDepthBufferSize(16);
	format.setStencilBufferSize(8);
	format.setSamples(16);

	m_pOpenGLContext.reset(new QOpenGLContext());
	m_pOpenGLContext->setFormat(format);
	if (!m_pOpenGLContext->create()) {
		throw std::runtime_error("Could not create OpenGL context");
	}

	m_pOffscreenSurface.reset(new QOffscreenSurface());
	m_pOffscreenSurface->setFormat(m_pOpenGLContext->format());
	m_pOffscreenSurface->create();
	m_pOpenGLContext->makeCurrent(m_pOffscreenSurface.get());

	if (!vr::VROverlay()) {
		QMessageBox::critical(nullptr, "Zephyr", "Is OpenVR running?");
		throw std::runtime_error(std::string("No Overlay interface"));
	}

	aboutTabController.initStage1();

	qmlEngine->rootContext()->setContextProperty("applicationVersion", getVersionString());
	qmlEngine->rootContext()->setContextProperty("vrRuntimePath", getVRRuntimePathUrl());

	qmlRegisterSingletonType<OverlayController>("texasgamer.zephyr", 1, 0, "OverlayController", [](QQmlEngine*, QJSEngine*) {
		QObject* obj = getInstance();
		QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
		return obj;
	});
	qmlRegisterSingletonType<AboutTabController>("texasgamer.zephyr", 1, 0, "AboutTabController", [](QQmlEngine*, QJSEngine*) {
		QObject* obj = &getInstance()->aboutTabController;
		QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
		return obj;
	});

	socketHandler = new SocketHandler();

    QString tokenPath = QString::fromStdString(QApplication::applicationDirPath().toStdString() + "/token.json");

	loadTokenFromFile(tokenPath);

	watcher = new QFileSystemWatcher();
	watcher->addPath(tokenPath);
	QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onTokenModified(QString)));
}

void OverlayController::Shutdown() {
	if (m_pPumpEventsTimer) {
		disconnect(m_pPumpEventsTimer.get(), SIGNAL(timeout()), this, SLOT(OnTimeoutPumpEvents()));
		m_pPumpEventsTimer->stop();
		m_pPumpEventsTimer.reset();
	}

	if (m_pRenderTimer) {
		disconnect(m_pRenderControl.get(), SIGNAL(renderRequested()), this, SLOT(OnRenderRequest()));
		disconnect(m_pRenderControl.get(), SIGNAL(sceneChanged()), this, SLOT(OnRenderRequest()));
		disconnect(m_pRenderTimer.get(), SIGNAL(timeout()), this, SLOT(renderOverlay()));
		m_pRenderTimer->stop();
		m_pRenderTimer.reset();
	}

	m_pWindow.reset();
	m_pRenderControl.reset();
	m_pFbo.reset();
	m_pOpenGLContext.reset();
	m_pOffscreenSurface.reset();
}

void OverlayController::SetWidget(QQuickItem* quickItem, const std::string& name, const std::string& key) {
	if (!desktopMode) {
		vr::VROverlayError overlayError = vr::VROverlay()->CreateDashboardOverlay(key.c_str(), name.c_str(), &m_ulOverlayHandle, &m_ulOverlayThumbnailHandle);
		if (overlayError != vr::VROverlayError_None) {
			if (overlayError == vr::VROverlayError_KeyInUse) {
				QMessageBox::critical(nullptr, "Zephyr", "Another instance is already running.");
			}
			throw std::runtime_error(std::string("Failed to create Overlay: " + std::string(vr::VROverlay()->GetOverlayErrorNameFromEnum(overlayError))));
		}

		vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 2.5f);
		vr::VROverlay()->SetOverlayInputMethod(m_ulOverlayHandle, vr::VROverlayInputMethod_Mouse);
		vr::VROverlay()->SetOverlayFlag(m_ulOverlayHandle, vr::VROverlayFlags_SendVRScrollEvents, true);
		
        std::string thumbIconPath = QApplication::applicationDirPath().toStdString() + "/res/overlay-icon.png";
		if (QFile::exists(QString::fromStdString(thumbIconPath))) {
			vr::VROverlay()->SetOverlayFromFile(m_ulOverlayThumbnailHandle, thumbIconPath.c_str());
		} else {
			LOG(ERROR) << "Could not find thumbnail icon \"" << thumbIconPath << "\"";
		}

		m_pRenderTimer.reset(new QTimer());
		m_pRenderTimer->setSingleShot(true);
		m_pRenderTimer->setInterval(5);
		connect(m_pRenderTimer.get(), SIGNAL(timeout()), this, SLOT(renderOverlay()));

		QOpenGLFramebufferObjectFormat fboFormat;
		fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		fboFormat.setTextureTarget(GL_TEXTURE_2D);
		m_pFbo.reset(new QOpenGLFramebufferObject(quickItem->width(), quickItem->height(), fboFormat));

		m_pRenderControl.reset(new QQuickRenderControl());
		m_pWindow.reset(new QQuickWindow(m_pRenderControl.get()));
		m_pWindow->setRenderTarget(m_pFbo.get());
		quickItem->setParentItem(m_pWindow->contentItem());
		m_pWindow->setGeometry(0, 0, quickItem->width(), quickItem->height());
		m_pRenderControl->initialize(m_pOpenGLContext.get());

		vr::HmdVector2_t vecWindowSize = {
			(float)quickItem->width(),
			(float)quickItem->height()
		};
		vr::VROverlay()->SetOverlayMouseScale(m_ulOverlayHandle, &vecWindowSize);

		connect(m_pRenderControl.get(), SIGNAL(renderRequested()), this, SLOT(OnRenderRequest()));
		connect(m_pRenderControl.get(), SIGNAL(sceneChanged()), this, SLOT(OnRenderRequest()));
	}

	m_pPumpEventsTimer.reset(new QTimer());
	connect(m_pPumpEventsTimer.get(), SIGNAL(timeout()), this, SLOT(OnTimeoutPumpEvents()));
	m_pPumpEventsTimer->setInterval(20);
	m_pPumpEventsTimer->start();

	aboutTabController.initStage2(this, m_pWindow.get());
}

void OverlayController::OnRenderRequest() {
	if (m_pRenderTimer && !m_pRenderTimer->isActive()) {
		m_pRenderTimer->start();
	}
}

void OverlayController::renderOverlay() {
	if (!desktopMode) {
		if (!vr::VROverlay() || !vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle) && !vr::VROverlay()->IsOverlayVisible(m_ulOverlayThumbnailHandle))
			return;

		m_pRenderControl->polishItems();
		m_pRenderControl->sync();
		m_pRenderControl->render();

		GLuint unTexture = m_pFbo->texture();
		if (unTexture != 0) {
#if defined _WIN64 || defined _LP64
			vr::Texture_t texture = { (void*)((uint64_t)unTexture), vr::API_OpenGL, vr::ColorSpace_Auto };
#else
			vr::Texture_t texture = { (void*)unTexture, vr::API_OpenGL, vr::ColorSpace_Auto };
#endif
			vr::VROverlay()->SetOverlayTexture(m_ulOverlayHandle, &texture);
		}
		m_pOpenGLContext->functions()->glFlush();
	}
}


void OverlayController::OnTimeoutPumpEvents() {
	if (!vr::VRSystem())
		return;

	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0f, devicePoses, vr::k_unMaxTrackedDeviceCount);
	aboutTabController.eventLoopTick();

	vr::VREvent_t vrEvent;
	while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent))) {
		switch (vrEvent.eventType) {
			case vr::VREvent_MouseMove: {
				QPoint ptNewMouse(vrEvent.data.mouse.x, vrEvent.data.mouse.y);
				if (ptNewMouse != m_ptLastMouse) {
					QMouseEvent mouseEvent( QEvent::MouseMove, ptNewMouse, m_pWindow->mapToGlobal(ptNewMouse), Qt::NoButton, m_lastMouseButtons, 0 );
					m_ptLastMouse = ptNewMouse;
					QCoreApplication::sendEvent(m_pWindow.get(), &mouseEvent);
					OnRenderRequest();
				}
			}
			break;

			case vr::VREvent_MouseButtonDown: {
				QPoint ptNewMouse(vrEvent.data.mouse.x, vrEvent.data.mouse.y);
				Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
				m_lastMouseButtons |= button;
				QMouseEvent mouseEvent(QEvent::MouseButtonPress, ptNewMouse, m_pWindow->mapToGlobal(ptNewMouse), button, m_lastMouseButtons, 0);
				QCoreApplication::sendEvent(m_pWindow.get(), &mouseEvent);
			}
			break;

			case vr::VREvent_MouseButtonUp: {
				QPoint ptNewMouse(vrEvent.data.mouse.x, vrEvent.data.mouse.y);
				Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
				m_lastMouseButtons &= ~button;
				QMouseEvent mouseEvent(QEvent::MouseButtonRelease, ptNewMouse, m_pWindow->mapToGlobal(ptNewMouse), button, m_lastMouseButtons, 0);
				QCoreApplication::sendEvent(m_pWindow.get(), &mouseEvent);
			}
			break;

			case vr::VREvent_Scroll: {
				QWheelEvent wheelEvent(m_ptLastMouse, m_pWindow->mapToGlobal(m_ptLastMouse), QPoint(),
					QPoint(vrEvent.data.scroll.xdelta * 360.0f * 8.0f, vrEvent.data.scroll.ydelta * 360.0f * 8.0f),
					0, Qt::Vertical, m_lastMouseButtons, 0);
				QCoreApplication::sendEvent(m_pWindow.get(), &wheelEvent);
			}
			break;

			case vr::VREvent_OverlayShown: {
				m_pWindow->update();
			}
			break;

			case vr::VREvent_Quit: {
				LOG(INFO) << "Received quit request.";
				vr::VRSystem()->AcknowledgeQuit_Exiting();
				Shutdown();
				QApplication::exit();
				return;
			}
			break;

			case vr::VREvent_DashboardActivated: {
				LOG(DEBUG) << "Dashboard activated";
				dashboardVisible = true;
			}
			break;

			case vr::VREvent_DashboardDeactivated: {
				LOG(DEBUG) << "Dashboard deactivated";
				dashboardVisible = false;
			}
			break;

			case vr::VREvent_KeyboardDone: {
				char keyboardBuffer[1024];
				vr::VROverlay()->GetKeyboardText(keyboardBuffer, 1024);
				emit keyBoardInputSignal(QString(keyboardBuffer), vrEvent.data.keyboard.uUserValue);
			}
			break;
		}
	}

	if (m_ulOverlayThumbnailHandle != vr::k_ulOverlayHandleInvalid) {
		while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayThumbnailHandle, &vrEvent, sizeof(vrEvent))) {
			switch (vrEvent.eventType) {
				case vr::VREvent_OverlayShown: {
					m_pWindow->update();
				}
				break;
			}
		}
	}
}

QString OverlayController::getVersionString() {
	return QString(applicationVersionString);
}

QUrl OverlayController::getVRRuntimePathUrl() {
	return m_runtimePathUrl;
}

bool OverlayController::soundDisabled() {
	return noSound;
}

void OverlayController::showKeyboard(QString existingText, unsigned long userValue) {
	vr::VROverlay()->ShowKeyboardForOverlay(m_ulOverlayHandle, vr::k_EGamepadTextInputModeNormal, vr::k_EGamepadTextInputLineModeSingleLine, "Zephyr", 1024, existingText.toStdString().c_str(), false, userValue);
}

void OverlayController::promptLogin() {
    // TODO: Investigate why this doesn't play nice with Steam
    // QString file = QString::fromStdString(QApplication::applicationDirPath().toStdString() + "/login-util/Zephyr.exe");
    // QProcess::startDetached(file);
}

void OverlayController::onTokenModified(const QString &path) {
	loadTokenFromFile(path);
}

void OverlayController::loadTokenFromFile(QFile token) {
	if (token.exists()) {
		token.open(QIODevice::ReadOnly | QIODevice::Text);
		QString val = token.readAll();
		token.close();

		QJsonDocument jsonToken = QJsonDocument::fromJson(val.toUtf8());
		QJsonObject jsonTokenObj = jsonToken.object();

		QString name = jsonTokenObj.value("name").toString();
		QString avatar = jsonTokenObj.value("avatar").toString();
		QString room = jsonTokenObj.value("room").toString();
		QString tok = jsonTokenObj.value("token").toString();

		userToken = Token(name, avatar, room, tok);

        if (!userToken.isLoggedIn()) {
            logout();
            emit loggedInChanged();
            return;
        }

		socketHandler->connect();
	} else {
        logout();
	}

	emit loggedInChanged();
}

void OverlayController::logout() {
	userToken = Token();

    socketHandler->disconnect();

    QString tokenPath = QString::fromStdString(QApplication::applicationDirPath().toStdString() + "/token.json");
    QFile tokenFile(tokenPath);

    QJsonDocument jsonToWrite;

    tokenFile.open(QFile::WriteOnly);
    tokenFile.write(jsonToWrite.toJson());
    tokenFile.close();

	emit loggedInChanged();
}

void OverlayController::showNotification(QString title, QString text) {
    if (desktopMode) {
        LOG(INFO) << title.toStdString() << " " << text.toStdString();
        return;
    }

    vr::EVRInitError eError;
    vr::IVRNotifications * notif = (vr::IVRNotifications *) vr::VR_GetGenericInterface(vr::IVRNotifications_Version, &eError);
    vr::VROverlayHandle_t handle;
    vr::VROverlay()->FindOverlay("texasgamer.Zephyr", &handle);
    vr::VRNotificationId id;
    QString notifContent(title + "\n" + text);
    notif->CreateNotification(handle, 0, vr::EVRNotificationType_Transient, notifContent.toStdString().c_str(), vr::EVRNotificationStyle_Application, NULL, &id);
}

}
