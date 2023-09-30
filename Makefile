PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)

CC    = psp-gcc
CXX   = psp-g++
AS    = psp-gcc
LD    = psp-gcc
FIXUP = psp-fixup-imports

# Add PSPSDK includes and libraries.
INCDIR = $(PSPDEV)/psp/include $(PSPSDK)/include include/
LIBDIR = $(PSPDEV)/psp/lib $(PSPSDK)/lib

CFLAGS   = $(addprefix -I,$(INCDIR)) -G0 -O2 -Wno-write-strings -Wpedantic
CXXFLAGS = $(CFLAGS)
ASFLAGS  = $(CFLAGS)

LDFLAGS = $(addprefix -L,$(LIBDIR)) -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx -nostartfiles -Wl,-zmax-page-size=128

LIBS_KERNEL = libs/libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge
LIBS_USER = libs/libpspsystemctrl_user.a -lc -lpspdebug -lpspge

PSP_FW_VERSION=150

CFLAGS += -D_PSP_FW_VERSION=$(PSP_FW_VERSION)

SRC_DIR = src

TARGET_KERNEL = Allefresher_kernel
SRCS_KERNEL = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
OBJ_DIR_KERNEL=build/kernel
OBJS_KERNEL = $(addsuffix .o,$(addprefix $(OBJ_DIR_KERNEL)/,$(SRCS_KERNEL)))

TARGET_USER = Allefresher_user
SRCS_USER = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
OBJ_DIR_USER=build/user
OBJS_USER = $(addsuffix .o,$(addprefix $(OBJ_DIR_USER)/,$(SRCS_USER)))

FINAL_TARGET_KERNEL = $(TARGET_KERNEL).prx
FINAL_TARGET_USER = $(TARGET_USER).prx

all: $(FINAL_TARGET_KERNEL) $(FINAL_TARGET_USER)

$(OBJ_DIR_KERNEL)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_KERNEL)
	$(CXX) $(CXXFLAGS) -DKERNEL_SPACE=1 -c -o $@ $^

$(OBJ_DIR_USER)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_USER)
	$(CXX) $(CXXFLAGS) -DUSER_SPACE=1 -c -o $@ $^

$(OBJ_DIR_KERNEL):
	mkdir -p $@/$(SRC_DIR)
$(OBJ_DIR_USER):
	mkdir -p $@/$(SRC_DIR)

$(TARGET_KERNEL).elf: $(OBJS_KERNEL) | $(OBJ_DIR_KERNEL)
	$(LINK.c) $^ $(LIBS_KERNEL) -o $@
	$(FIXUP) $@

$(TARGET_USER).elf: $(OBJS_USER) | $(OBJ_DIR_USER)
	$(LINK.c) $^ $(LIBS_USER) -o $@
	$(FIXUP) $@

%.prx: %.elf
	psp-prxgen $< $@

%.c: %.exp
	psp-build-exports -b $< > $@

clean: $(EXTRA_CLEAN)
	-rm -f $(FINAL_TARGET_KERNEL) $(TARGET_KERNEL).elf $(OBJS_KERNEL)
	-rm -f $(FINAL_TARGET_USER) $(TARGET_USER).elf $(OBJS_USER)
	-rm -rf build

rebuild: clean all