QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    extendedwidgets/extendedsqltablemodel.cpp \
    extendedwidgets/extendedtableview.cpp \
    extendedwidgets/extendedsqlquerymodel.cpp \
    main.cpp \
    pns/algorithmabb.cpp \
    pns/algorithmbase.cpp \
    pns/algorithmmsg.cpp \
    pns/algorithmssg.cpp \
    pns/pnsproblem.cpp \
    pns/reducedpnsproblemview.cpp \
    ui/aboutdialog.cpp \
    ui/licensedialog.cpp \
    ui/mainwindow.cpp \
    ui/materialdialog.cpp \
    ui/runalgorithmdialog.cpp \
    ui/runresultviewer.cpp \
    ui/structurebrowser.cpp \
    ui/unitdialog.cpp \
    ui/unitinputoutputdialog.cpp

HEADERS += \
    extendedwidgets/extendedsqltablemodel.h \
    extendedwidgets/extendedtableview.h \
    extendedwidgets/extendedsqlquerymodel.h \
    pns/algorithmabb.h \
    pns/algorithmbase.h \
    pns/algorithmmsg.h \
    pns/algorithmssg.h \
    pns/extendedset.h \
    pns/pnsproblem.h \
    pns/powerset.h \
    pns/reducedpnsproblemview.h \
    ui/aboutdialog.h \
    ui/licensedialog.h \
    ui/mainwindow.h \
    ui/materialdialog.h \
    ui/runalgorithmdialog.h \
    ui/runresultviewer.h \
    ui/structurebrowser.h \
    ui/unitdialog.h \
    ui/unitinputoutputdialog.h

FORMS += \
    ui/aboutdialog.ui \
    ui/licensedialog.ui \
    ui/mainwindow.ui \
    ui/materialdialog.ui \
    ui/runalgorithmdialog.ui \
    ui/runresultviewer.ui \
    ui/structurebrowser.ui \
    ui/unitdialog.ui \
    ui/unitinputoutputdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    generalresources.qrc
