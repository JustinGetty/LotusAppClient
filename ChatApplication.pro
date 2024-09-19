QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Specify the sources (now in the Sources/ folder)
SOURCES += \
    Sources/main.cpp \
    Sources/mainwindow.cpp \
    Sources/networkmanager.cpp \
    Sources/user.cpp

# Specify the headers (now in the Headers/ folder)
HEADERS += \
    Headers/mainwindow.h \
    Headers/networkmanager.h \
    Sources/user.h

# Specify the forms (now in the Forms/ folder)
FORMS += \
    Forms/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Resources (if needed, adjust the path if these have moved as well)
DISTFILES += \
    Resources/send_message_icon.jpeg \
    Resources/send_message_icon.png
