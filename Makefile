TARGET = Allefresher
OBJS = src/main.o

BUILD_PRX = 1

CFLAGS = -O2 -w -msingle-float -Iinclude/
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti  
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = libs/libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge
LDFLAGS = -nostartfiles 

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 