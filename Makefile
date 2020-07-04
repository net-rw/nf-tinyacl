obj-m += ntl.o

ntl-objs := \
	ntl-init.o \
	common/ntl-entry.o \
	common/ntl-util.o \
	bridge/ntl-bridge-init.o \
	bridge/ntl-br-entry.o \
	bridge/ntl-nf-bridge.o \

