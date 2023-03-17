
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = highOrderCircuitSimulate
TEMPLATE = app


SOURCES += main.cpp\
    lib/qcustomplot.cpp \
        mainwindow.cpp \
    wire.cpp \
    allwidget.cpp \
    mainwindowsetcomponentparameter.cpp \
    DrawPlot/dialogformatdatainfo.cpp \
    DrawPlot/mainwindowdraw.cpp

HEADERS  += mainwindow.h \
    lib/qcustomplot.h \
    wire.h \
    allwidget.h \
    mainwindowsetcomponentparameter.h \
    DrawPlot/dialogformatdatainfo.h \
    DrawPlot/mainwindowdraw.h

FORMS    += mainwindow.ui \
    mainwindowsetcomponentparameter.ui \
    DrawPlot/dialogformatdatainfo.ui \
    DrawPlot/mainwindowdraw.ui

RC_ICONS=myico.ico

RESOURCES += \
    component.qrc

DISTFILES += \
    myIco.ico

DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY
