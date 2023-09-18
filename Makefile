TARGET = imgwpatch
OBJS = main.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX = 1

# USE_KERNEL_LIBS = 1
# USE_KERNEL_LIBC = 1

CFLAGS = -G0 -w -msingle-float -g -Og
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti  
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge
LDFLAGS = -nostartfiles 

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 