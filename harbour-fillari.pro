PREFIX = harbour
NAME = fillari

TARGET = $${PREFIX}-$${NAME}
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5
QT += network qml quick dbus

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-unused-parameter

TARGET_DATA_DIR = /usr/share/$${TARGET}
TRANSLATIONS_PATH = $${TARGET_DATA_DIR}/translations

CONFIG(debug, debug|release) {
    DEFINES += DEBUG HARBOUR_DEBUG
}

OTHER_FILES += \
    LICENSE \
    README.md \
    rpm/*.spec \
    *.desktop \
    qml/*.qml \
    qml/images/*.svg \
    icons/*.svg

# Directories

HARBOUR_LIB_DIR = $${_PRO_FILE_PWD_}/harbour-lib

# harbour-lib

HARBOUR_LIB_INCLUDE = $${HARBOUR_LIB_DIR}/include
HARBOUR_LIB_SRC = $${HARBOUR_LIB_DIR}/src
HARBOUR_LIB_QML = $${HARBOUR_LIB_DIR}/qml

INCLUDEPATH += \
    $${HARBOUR_LIB_INCLUDE}

HEADERS += \
    $${HARBOUR_LIB_INCLUDE}/HarbourDebug.h \

SOURCES += \

HARBOUR_QML_COMPONENTS = \
    $${HARBOUR_LIB_QML}/HarbourHighlightIcon.qml \
    $${HARBOUR_LIB_QML}/HarbourPasswordInputField.qml

OTHER_FILES += $${HARBOUR_QML_COMPONENTS}

qml_components.files = $${HARBOUR_QML_COMPONENTS}
qml_components.path = $${TARGET_DATA_DIR}/qml/harbour
INSTALLS += qml_components

# App

HEADERS += \
    src/BikeApp.h \
    src/BikeHistoryModel.h \
    src/BikeHistoryQuery.h \
    src/BikeHistoryStats.h \
    src/BikeLogin.h \
    src/BikeObjectQuery.h \
    src/BikeRequest.h \
    src/BikeSession.h \
    src/BikeUser.h \
    src/Fillari.h

SOURCES += \
    src/BikeHistoryModel.cpp \
    src/BikeHistoryQuery.cpp \
    src/BikeHistoryStats.cpp \
    src/BikeLogin.cpp \
    src/BikeObjectQuery.cpp \
    src/BikeRequest.cpp \
    src/BikeSession.cpp \
    src/BikeUser.cpp \
    src/Fillari.cpp \
    src/main.cpp

# Icons

ICON_SIZES = 86 108 128 172 256
for(s, ICON_SIZES) {
    icon_target = icon_$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    INSTALLS += $${icon_target}
}

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${_PRO_FILE_PWD_}/src

defineTest(addTrFile) {
    rel = translations/harbour-$$1
    in = $${_PRO_FILE_PWD_}/$$rel
    out = $${OUT_PWD}/$$rel

    OTHER_FILES += $${in}.ts
    export(OTHER_FILES)

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    qm_target = qm_$$s

    $${lupdate_target}.commands = lupdate -noobsolete -locations none $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${qm_target}.path = $$TRANSLATIONS_PATH
    $${qm_target}.depends = $${lupdate_target}
    $${qm_target}.commands = lrelease -idbased \"$${out}.ts\" && \
        $(INSTALL_FILE) \"$${out}.qm\" $(INSTALL_ROOT)$${TRANSLATIONS_PATH}/

    QMAKE_EXTRA_TARGETS += $${lupdate_target} $${qm_target}
    INSTALLS += $${qm_target}

    export($${lupdate_target}.commands)
    export($${qm_target}.path)
    export($${qm_target}.depends)
    export($${qm_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(INSTALLS)
}

LANGUAGES =

addTrFile($${NAME})
for(l, LANGUAGES) {
    addTrFile($${NAME}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm
