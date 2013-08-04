TEMPLATE = subdirs
SUBDIRS += mts \
    mts/platform/storage/simpleplugin \
    service \
    init/systemd

# install additional files for the CI Integration tests
citests.path = /opt/tests/buteo-mtp/test-definition/
citests.files = tests.xml
INSTALLS += citests
#HEADERS = mts/common/mtp_dbus.h
#SOURCES = mts/common/mtp_dbus.cpp
