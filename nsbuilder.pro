m32 {
unix:QMAKE_LIBDIR_QT = /usr/lib
unix:QMAKE_CFLAGS ~= s/-m64/-m32/
unix:QMAKE_CXXFLAGS ~= s/-m64/-m32/
unix:QMAKE_LFLAGS ~= s/-m64/-m32/
}

CONFIG		+= debug
CONFIG		-= release

win32:DEFINES		+= WIN32
win32:QMAKE_YACC	= bison
win32:QMAKE_MOVE	= ren

SVN_VER=$$cat(SVN_VERSION_FILE)

DEFINES                 += SVN_VER=\\\"$$SVN_VER\\\"

# stan obiektów Qt
#DEFINES		+= DEBUG5
# przydzial pamieci
#DEFINES		+= DEBUG4
# bison
#DEFINES		+= DEBUG3
# bison
#DEFINES		+= DEBUG2
# flex
#DEFINES		+= DEBUG1
# enter/exit
#DEFINES		+= DEBUGE
# execute_statement
#DEFINES                 += DEBUGX
# do_validate
#DEFINES                 += DEBUGV
# typeCheck
DEFINES     		+= DEBUGTC
# wiele wymiarów
#DEFINES                += DEBUGD

LEXSOURCES = imp.l
YACCSOURCES = imp.y

QT += xml network

RESOURCES     = nsbuilder.qrc

HEADERS       = mainwindow.h \
                nsscheme.h \
		instruction.h \
		sequence.h \
		iteration.h \
		selection.h \
		instructioneditor.h \
		instructionwizard.h \
		simpleinstruction.h \
		questiondialog.h \
		nsschemewizard.h \
		inputinstruction.h \
		outputinstruction.h \
		callinstruction.h \
                returninstruction.h \
		selectioncondition.h \
		settingsdialog.h \
		executionthread.h \
		variableeditor.h \
		exprtree.h \
		imp_idents.h \
                bisonproxy.h \
    commands.h \
    navevaluedialog.h

SOURCES       = main.cpp \
                mainwindow.cpp \
                nsscheme.cpp \
		instruction.cpp \
		sequence.cpp \
		iteration.cpp \
		selection.cpp \
		instructioneditor.cpp \
		instructionwizard.cpp \
		simpleinstruction.cpp \
		questiondialog.cpp \
		nsschemewizard.cpp \
		inputinstruction.cpp \
		outputinstruction.cpp \
		callinstruction.cpp \                
		selectioncondition.cpp \
		settingsdialog.cpp \
		executionthread.cpp \
		variableeditor.cpp \
		exprtree.cpp \
		imp_idents.cpp \
		bisonproxy.cpp \
    returninstruction.cpp \
    commands.cpp \
    navevaluedialog.cpp

FORMS         = instructioneditor.ui \
                instructionwizard.ui \
		questiondialog.ui \
		nsschemewizard.ui \
		variableeditor.ui \
		settingsdialog.ui \
    navevaluedialog.ui

TRANSLATIONS  = nsbuilder_pl.ts

TARGET        = nsbuilder
target.path   = /nsbuilder
target.files  = nsbuilder nsbuilder_pl.qm

INSTALLS += target

UI_DIR=.ui
MOC_DIR=.moc
OBJECTS_DIR=.obj

OTHER_FILES += \
    nsbuilder.rc

RC_FILE = nsbuilder.rc
