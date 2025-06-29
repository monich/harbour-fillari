PREFIX = harbour
NAME = fillari

TARGET = $${PREFIX}-$${NAME}
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp glib-2.0 gobject-2.0 gio-unix-2.0
QT += network qml quick dbus

DEFINES += NFCDC_NEED_PEER_SERVICE=0
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
LIBGLIBUTIL_DIR = $${_PRO_FILE_PWD_}/libglibutil
LIBGNFCDC_DIR = $${_PRO_FILE_PWD_}/libgnfcdc
LIBQNFCDC_DIR = $${_PRO_FILE_PWD_}/libqnfcdc

# libglibutil

LIBGLIBUTIL_SRC = $${LIBGLIBUTIL_DIR}/src
LIBGLIBUTIL_INCLUDE = $${LIBGLIBUTIL_DIR}/include

INCLUDEPATH += \
    $${LIBGLIBUTIL_INCLUDE}

HEADERS += \
    $${LIBGLIBUTIL_INCLUDE}/*.h

SOURCES += \
    $${LIBGLIBUTIL_SRC}/gutil_log.c \
    $${LIBGLIBUTIL_SRC}/gutil_misc.c \
    $${LIBGLIBUTIL_SRC}/gutil_strv.c

# libgnfcdc

LIBGNFCDC_INCLUDE = $${LIBGNFCDC_DIR}/include
LIBGNFCDC_SRC = $${LIBGNFCDC_DIR}/src
LIBGNFCDC_SPEC = $${LIBGNFCDC_DIR}/spec

INCLUDEPATH += \
    $${LIBGNFCDC_INCLUDE}

HEADERS += \
    $${LIBGNFCDC_INCLUDE}/*.h \
    $${LIBGNFCDC_SRC}/*.h

SOURCES += \
    $${LIBGNFCDC_SRC}/nfcdc_adapter.c \
    $${LIBGNFCDC_SRC}/nfcdc_base.c \
    $${LIBGNFCDC_SRC}/nfcdc_daemon.c \
    $${LIBGNFCDC_SRC}/nfcdc_default_adapter.c \
    $${LIBGNFCDC_SRC}/nfcdc_error.c \
    $${LIBGNFCDC_SRC}/nfcdc_isodep.c \
    $${LIBGNFCDC_SRC}/nfcdc_log.c \
    $${LIBGNFCDC_SRC}/nfcdc_tag.c \
    $${LIBGNFCDC_SRC}/nfcdc_util.c

OTHER_FILES += \
    $${LIBGNFCDC_SPEC}/*.xml

defineTest(generateStub) {
    xml = $${LIBGNFCDC_SPEC}/org.sailfishos.nfc.$${1}.xml
    cmd = gdbus-codegen --generate-c-code org.sailfishos.nfc.$${1} $${xml}

    gen_h = org.sailfishos.nfc.$${1}.h
    gen_c = org.sailfishos.nfc.$${1}.c
    target_h = org_sailfishos_nfc_$${1}_h
    target_c = org_sailfishos_nfc_$${1}_c

    $${target_h}.target = $${gen_h}
    $${target_h}.depends = $${xml}
    $${target_h}.commands = $${cmd}
    export($${target_h}.target)
    export($${target_h}.depends)
    export($${target_h}.commands)

    GENERATED_HEADERS += $${gen_h}
    PRE_TARGETDEPS += $${gen_h}
    QMAKE_EXTRA_TARGETS += $${target_h}

    $${target_c}.target = $${gen_c}
    $${target_c}.depends = $${gen_h}
    export($${target_c}.target)
    export($${target_c}.depends)

    GENERATED_SOURCES += $${gen_c}
    QMAKE_EXTRA_TARGETS += $${target_c}
    PRE_TARGETDEPS += $${gen_c}

    export(QMAKE_EXTRA_TARGETS)
    export(GENERATED_SOURCES)
    export(PRE_TARGETDEPS)
}

generateStub(Adapter)
generateStub(Daemon)
generateStub(IsoDep)
generateStub(Settings)
generateStub(Tag)

# libqnfcdc

LIBQNFCDC_INCLUDE = $${LIBQNFCDC_DIR}/include
LIBQNFCDC_SRC = $${LIBQNFCDC_DIR}/src

INCLUDEPATH += \
    $${LIBQNFCDC_INCLUDE}

HEADERS += \
    $${LIBQNFCDC_INCLUDE}/NfcAdapter.h \
    $${LIBQNFCDC_INCLUDE}/NfcMode.h \
    $${LIBQNFCDC_INCLUDE}/NfcParam.h \
    $${LIBQNFCDC_INCLUDE}/NfcSystem.h \
    $${LIBQNFCDC_INCLUDE}/NfcTech.h

SOURCES += \
    $${LIBQNFCDC_SRC}/NfcAdapter.cpp \
    $${LIBQNFCDC_SRC}/NfcMode.cpp \
    $${LIBQNFCDC_SRC}/NfcParam.cpp \
    $${LIBQNFCDC_SRC}/NfcSystem.cpp \
    $${LIBQNFCDC_SRC}/NfcTech.cpp

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
    src/BikeLogout.h \
    src/BikeObjectQuery.h \
    src/BikeRequest.h \
    src/BikeSession.h \
    src/BikeUser.h \
    src/ToolTipItem.h \
    src/Fillari.h

SOURCES += \
    src/BikeHistoryModel.cpp \
    src/BikeHistoryQuery.cpp \
    src/BikeHistoryStats.cpp \
    src/BikeLogin.cpp \
    src/BikeLogout.cpp \
    src/BikeObjectQuery.cpp \
    src/BikeRequest.cpp \
    src/BikeSession.cpp \
    src/BikeUser.cpp \
    src/Fillari.cpp \
    src/ToolTipItem.cpp \
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

LANGUAGES = fi

addTrFile($${NAME})
for(l, LANGUAGES) {
    addTrFile($${NAME}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm
