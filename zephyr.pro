QT       += core gui qml quick webengine network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Zephyr
TEMPLATE = app

RC_FILE = src/Zephyr.rc

CONFIG+=c++11

SOURCES += src/main.cpp\
		src/overlaycontroller.cpp \
		src/tabcontrollers/AboutTabController.cpp \
		src/SocketHandler.cpp \
                src/Token.cpp

HEADERS  += src/overlaycontroller.h \
		src/logging.h \
		src/tabcontrollers/AboutTabController.h \
		src/SocketHandler.h \
                src/Token.h

INCLUDEPATH += third-party/openvr/include \
                third-party/easylogging++ \
                third-party/socketio/include

LIBS += -L$$PWD/third-party/openvr/lib/win64/ -lopenvr_api \
        -L$$PWD/third-party/openssl -llibeay32 -lssleay32 \
        -L$$PWD/third-party/socketio -lsioclient_tls

DESTDIR = bin/win64

win32:CONFIG(release, debug|release): LIBS += -L$$(BOOST_ROOT)/out/lib/ -lboost
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(BOOST_ROOT)/out/lib/ -lboostd

INCLUDEPATH += $$(BOOST_ROOT)
DEPENDPATH += $$(BOOST_ROOT)
