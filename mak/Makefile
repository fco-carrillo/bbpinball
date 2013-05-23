
#ROOTFS := /mnt/nfs_root/imx61_rootfs

CROSS_COMPILE := arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc

TARGET = ../bbPinball
SRC = $(wildcard ../build/*.c)
OBJS = $(patsubst %.c,%.o,$(SRC))
CFLAGS += -O3 -D__ARM__  -DUSE_OPENGLES2 -DLINUX -DEGL_API_FB \
-I../include/  -I$(ROOTFS)/usr/include/
LDFLAGS += -L$(ROOTFS)/usr/lib/ -Xlinker -rpath-link=$(ROOTFS)/usr/lib  -lGLESv2 -lEGL -lGAL -lm -ldl
#LDFLAGS += -L$(VIVANTE_SDK_LIB)  -lGLESv2 -lEGL -lGAL -lm -ldl
CC := $(CROSS_COMPILE)gcc

.phony: all
all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): $(SRC)

.phony: clean

clean: distclean

distclean:
	rm -f $(OBJS) $(TARGET)
