TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += src/modbus.pro
SUBDIRS += examples/client/mbclientc.pro
SUBDIRS += examples/client/mbclient.pro
SUBDIRS += examples/server/mbserverc.pro
SUBDIRS += examples/server/mbserver.pro
SUBDIRS += tests/test.pro
