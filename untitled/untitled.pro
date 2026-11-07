QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icons/copy.png \
    icons/cut.png \
    icons/duose-wenbentihuan.png \
    icons/file-add.png \
    icons/folder-open.png \
    icons/font.png \
    icons/list_number.png \
    icons/notebook.png \
    icons/paste.png \
    icons/redo.png \
    icons/save.png \
    icons/search.png \
    icons/undo.png \
    icons/wrap.png

RESOURCES += \
    icons/icons.qrc
