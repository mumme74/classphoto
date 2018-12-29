# -------------------------------------------------
# Project created by QtCreator 2010-07-09T14:22:28
# -------------------------------------------------
QT += xml \
    core \
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
    croprenderer.cpp \
    createclassdlg.cpp \
    settings.cpp

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
    croprenderer.h \
    createclassdlg.h \
    mainwindow.h

FORMS += mainwindow.ui \
    studenteditdialog.ui \
    createclassdlg.ui
RESOURCES += icons.qrc \
    stylesheets.qrc
OTHER_FILES += resources/stylesheets/defaultTheme.css

!isEmpty(target.path): INSTALLS += target

# copies the given files to the destination directory
defineTest(copyToDestDir) {
    files = $$1
    dir = $$2

    # replace slashes in destination path for Windows
    win32:dir ~= s,/,\\,g

    for(file, files) {
        # replace slashes in source path for Windows
        win32:file ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_quote($$file) $$shell_quote($$dir) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}



DISTFILES += \
    packaging/config/config.xml \
    packaging/packages/com.mummesoft.classphoto/meta/package.xml \
    packaging/packages/com.mummesoft.classphoto/data/license.txt

# copy packaging files
copyToDestDir($$PWD/packaging, $$OUT_PWD/packaging)
