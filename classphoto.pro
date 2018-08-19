# -------------------------------------------------
# Project created by QtCreator 2010-07-09T14:22:28
# -------------------------------------------------
QT += xml \
    xmlpatterns \
    widgets \
    printsupport \
    testlib

TARGET = classphoto
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    project.cpp \
    studentbase.cpp \
    studentlistitemdelegate.cpp \
    availableitemsmodel.cpp \
    picture.cpp \
    editshowclassname.cpp \
    classnameingraphicsview.cpp \
    studentingraphicsview.cpp \
    studentsview.cpp \
    studenteditdialog.cpp \
    croppicture.cpp \
    croprenderer.cpp

# studentlistitem.cpp \
HEADERS += mainwindow.h \
    project.h \
    settings.h \
    studentbase.h \
    studentlistitemdelegate.h \
    availableitemsmodel.h \
    picture.h \
    editshowclassname.h \
    classnameingraphicsview.h \
    studentingraphicsview.h \
    studentsview.h \
    studenteditdialog.h \
    croppicture.h \
    croprenderer.h

# studentlistitem.h \
FORMS += mainwindow.ui \
    studenteditdialog.ui
RESOURCES += icons.qrc \
    stylesheets.qrc
OTHER_FILES += resources/stylesheets/defaultTheme.css
