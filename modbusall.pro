TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += src/modbus.pro
SUBDIRS += examples/client/democlient.pro
SUBDIRS += examples/client/mbclient.pro
SUBDIRS += examples/server/demoserver.pro
SUBDIRS += examples/server/mbserver.pro
SUBDIRS += tests/test.pro
