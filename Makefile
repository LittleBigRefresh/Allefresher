TARGET = Allefresher
OBJS = main.o

BUILD_PRX = 1

CFLAGS = -O2 -w -msingle-float
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti  
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge
LDFLAGS = -nostartfiles 

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 