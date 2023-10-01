# Get the PSP SDK paths
PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)

VITA_PREFIX = arm-vita-eabi
VITA-CC = $(VITA_PREFIX)-gcc
VITA-CXX = $(VITA_PREFIX)-g++
VITA-LD = $(VITA_PREFIX)-gcc

# Setup the names of our compilers
PSP-CC    = psp-gcc
PSP-CXX   = psp-g++
PSP-AS    = psp-gcc
PSP-LD    = psp-gcc
PSP-FIXUP = psp-fixup-imports

# Add PSPSDK includes and libraries.
PSP-INCDIR = $(PSPDEV)/psp/include $(PSPSDK)/include include/
PSP-LIBDIR = $(PSPDEV)/psp/lib $(PSPSDK)/lib

# Base C/C++/AS flags
CFLAGS   = -O2 -Wno-write-strings -Wpedantic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  = $(CFLAGS)

# The PSP firmware version to target
PSP_FW_VERSION=150

# Add the PSP FW version and include dirs to the CFLAGS
PSP-SHARED-CFLAGS = $(addprefix -I,$(PSP-INCDIR)) -D_PSP_FW_VERSION=$(PSP_FW_VERSION) -G0
PSP-CFLAGS += $(CFLAGS)
PSP-CFLAGS += $(PSP-SHARED-CFLAGS)
PSP-CXXFLAGS += $(CXXFLAGS)
PSP-CXXFLAGS += $(PSP-SHARED-CFLAGS)
PSP-ASFLAGS += $(ASFLAGS)

# Base linker flags
PSP-LDFLAGS = $(addprefix -L,$(PSP-LIBDIR)) -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx -specs=$(PSPSDK)/lib/prxspecs -nostartfiles -Wl,-zmax-page-size=128

# The source directory
SRC_DIR = src

# The target name for the kernel module
PSP_TARGET_KERNEL = Allefresher_kernel
# The source files used in the kernel module
PSP_SRCS_KERNEL = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
# The dir containing the kernel module's obj files
PSP_OBJ_DIR_KERNEL=build/kernel
# The list of obj files the kernel module compiles to
PSP_OBJS_KERNEL = $(addsuffix .o,$(addprefix $(PSP_OBJ_DIR_KERNEL)/,$(PSP_SRCS_KERNEL)))
# Kernel module specific C flags
PSP_CFLAGS_KERNEL = -DKERNEL_SPACE=1
# Libs the kernel module links against
PSP_LIBS_KERNEL = libs/libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge

# The target name for the user module
PSP_TARGET_USER = Allefresher_user
# The source files used in the user module
PSP_SRCS_USER = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
# The dir containing the user module's obj files
PSP_OBJ_DIR_USER=build/user
# The list of obj files the user module compiles to
PSP_OBJS_USER = $(addsuffix .o,$(addprefix $(PSP_OBJ_DIR_USER)/,$(PSP_SRCS_USER)))
# User module specific C flags
PSP_CFLAGS_USER = -DUSER_SPACE=1 -fno-pic
# Libs the user module links against
PSP_LIBS_USER = libs/libpspsystemctrl_user.a -lc -lpspdebug -lpspge

VITA_TARGET_USER = Allefresher_user_vita
VITA_SRCS_USER = src/vita.cpp
VITA_OBJ_DIR_USER = build/user_vita
VITA_OBJS_USER = $(addsuffix .o,$(addprefix $(VITA_OBJ_DIR_USER)/,$(VITA_SRCS_USER)))

VITA-CXXFLAGS = $(CXXFLAGS) -mcpu=cortex-a9 -mthumb-interwork -I$(VITASDK)/$(VITA_PREFIX)/include -Wl,-q -Iinclude
VITA-LDFLAGS = -nostartfiles -Wl,-q
VITA-LIBS = libs/libtaihen_stub.a

# The final target name of the kernel PRX
FINAL_PSP_TARGET_KERNEL = $(PSP_TARGET_KERNEL).prx
# The final target name of the user PRX
FINAL_PSP_TARGET_USER = $(PSP_TARGET_USER).prx

all: $(FINAL_PSP_TARGET_KERNEL) $(FINAL_PSP_TARGET_USER) $(VITA_TARGET_USER).skprx

# For all kernel module object files, invoke the C++ compiler
$(PSP_OBJ_DIR_KERNEL)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(PSP_OBJ_DIR_KERNEL)
	$(PSP-CXX) $(PSP-CXXFLAGS) $(PSP_CFLAGS_KERNEL) -c -o $@ $^

# For all user module object files, invoke the C++ compiler
$(PSP_OBJ_DIR_USER)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(PSP_OBJ_DIR_USER)
	$(PSP-CXX) $(PSP-CXXFLAGS) $(PSP_CFLAGS_USER) -c -o $@ $^

# For all Vita kernel module object files, invoke the C++ compiler
$(VITA_OBJ_DIR_USER)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(VITA_OBJ_DIR_USER)
	$(VITA-CXX) $(VITA-CXXFLAGS) -MMD -MP $(VITA_CFLAGS_KERNEL) -c -o $@ $^
  
# Make the obj dirs
$(PSP_OBJ_DIR_KERNEL):
	mkdir -p $@/$(SRC_DIR)
$(PSP_OBJ_DIR_USER):
	mkdir -p $@/$(SRC_DIR)
$(VITA_OBJ_DIR_USER):
	mkdir -p $@/$(SRC_DIR)

# Link the vita kernel object files into an ELF
$(VITA_TARGET_USER).elf: $(VITA_OBJS_USER) | $(VITA_OBJ_DIR_USER)
	$(VITA-LD) $(VITA-LDFLAGS) $^ $(VITA-LIBS) -o $@

# Link the kernel object files into an ELF, and fixup the imports
$(PSP_TARGET_KERNEL).elf: $(PSP_OBJS_KERNEL) | $(PSP_OBJ_DIR_KERNEL)
	$(PSP-LD) $(PSP-LDFLAGS) $^ $(PSP_LIBS_KERNEL) -o $@
	$(PSP-FIXUP) $@

# Link the user object files into an ELF, and fixup the imports
$(PSP_TARGET_USER).elf: $(PSP_OBJS_USER) | $(PSP_OBJ_DIR_USER)
	$(PSP-LD) $(PSP-LDFLAGS) $^ $(PSP_LIBS_USER) -o $@
	$(PSP-FIXUP) $@

%.skprx: %.velf
	vita-make-fself -c $< $@

%.velf: %.elf
	vita-elf-create -n -e $(basename $@).yml $< $@

# Make a PRX file from all ELF targets
%.prx: %.elf
	psp-prxgen $< $@

# Build C files from any export files
%.c: %.exp
	psp-build-exports -b $< > $@

clean: $(EXTRA_CLEAN)
	-rm -f $(FINAL_PSP_TARGET_KERNEL) $(PSP_TARGET_KERNEL).elf $(PSP_OBJS_KERNEL)
	-rm -f $(FINAL_PSP_TARGET_USER) $(PSP_TARGET_USER).elf $(PSP_OBJS_USER)
	-rm -f $(VITA_TARGET_USER).velf $(VITA_TARGET_USER).skprx $(VITA_TARGET_USER).elf $(VITA_OBJS_USER)
	-rm -rf build

rebuild: clean all