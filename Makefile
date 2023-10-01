# Get the PSP SDK paths
PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)

# Setup the names of our compilers
CC    = psp-gcc
CXX   = psp-g++
AS    = psp-gcc
LD    = psp-gcc
FIXUP = psp-fixup-imports

# Add PSPSDK includes and libraries.
INCDIR = $(PSPDEV)/psp/include $(PSPSDK)/include include/
LIBDIR = $(PSPDEV)/psp/lib $(PSPSDK)/lib

# Base C/C++/AS flags
CFLAGS   = $(addprefix -I,$(INCDIR)) -G0 -O2 -Wno-write-strings -Wpedantic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  = $(CFLAGS)

# Base linker flags
LDFLAGS = $(addprefix -L,$(LIBDIR)) -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx -specs=$(PSPSDK)/lib/prxspecs -nostartfiles -Wl,-zmax-page-size=128

# The PSP firmware version to target
PSP_FW_VERSION=150

# Add the PSP FW version to the CFLAGS
CFLAGS += -D_PSP_FW_VERSION=$(PSP_FW_VERSION)

# The source directory
SRC_DIR = src

# The target name for the kernel module
TARGET_KERNEL = Allefresher_kernel
# The source files used in the kernel module
SRCS_KERNEL = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
# The dir containing the kernel module's obj files
OBJ_DIR_KERNEL=build/kernel
# The list of obj files the kernel module compiles to
OBJS_KERNEL = $(addsuffix .o,$(addprefix $(OBJ_DIR_KERNEL)/,$(SRCS_KERNEL)))
# Kernel module specific C flags
CFLAGS_KERNEL = -DKERNEL_SPACE=1
# Libs the kernel module links against
LIBS_KERNEL = libs/libpspsystemctrl_kernel.a -lc -lpspkernel -lpspdebug -lpspge

# The target name for the user module
TARGET_USER = Allefresher_user
# The source files used in the user module
SRCS_USER = src/psp.cpp src/roster.cpp src/reader.cpp src/patching.cpp
# The dir containing the user module's obj files
OBJ_DIR_USER=build/user
# The list of obj files the user module compiles to
OBJS_USER = $(addsuffix .o,$(addprefix $(OBJ_DIR_USER)/,$(SRCS_USER)))
# User module specific C flags
CFLAGS_USER = -DUSER_SPACE=1 -fno-pic
# Libs the user module links against
LIBS_USER = libs/libpspsystemctrl_user.a -lc -lpspdebug -lpspge

# The final target name of the kernel PRX
FINAL_TARGET_KERNEL = $(TARGET_KERNEL).prx
# The final target name of the user PRX
FINAL_TARGET_USER = $(TARGET_USER).prx

all: $(FINAL_TARGET_KERNEL) $(FINAL_TARGET_USER)

# For all kernel module object files, invoke the C++ compiler
$(OBJ_DIR_KERNEL)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_KERNEL)
	$(CXX) $(CXXFLAGS) $(CFLAGS_KERNEL) -c -o $@ $^

# For all user module object files, invoke the C++ compiler
$(OBJ_DIR_USER)/src/%.cpp.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_USER)
	$(CXX) $(CXXFLAGS) $(CFLAGS_USER) -c -o $@ $^

# Make the obj dirs
$(OBJ_DIR_KERNEL):
	mkdir -p $@/$(SRC_DIR)
$(OBJ_DIR_USER):
	mkdir -p $@/$(SRC_DIR)

# Link the kernel object files into an ELF, and fixup the imports
$(TARGET_KERNEL).elf: $(OBJS_KERNEL) | $(OBJ_DIR_KERNEL)
	$(LINK.c) $^ $(LIBS_KERNEL) -o $@
	$(FIXUP) $@

# Link the user object files into an ELF, and fixup the imports
$(TARGET_USER).elf: $(OBJS_USER) | $(OBJ_DIR_USER)
	$(LINK.c) $^ $(LIBS_USER) -o $@
	$(FIXUP) $@

# Make a PRX file from all ELF targets
%.prx: %.elf
	psp-prxgen $< $@

# Build C files from any export files
%.c: %.exp
	psp-build-exports -b $< > $@

clean: $(EXTRA_CLEAN)
	-rm -f $(FINAL_TARGET_KERNEL) $(TARGET_KERNEL).elf $(OBJS_KERNEL)
	-rm -f $(FINAL_TARGET_USER) $(TARGET_USER).elf $(OBJS_USER)
	-rm -rf build

rebuild: clean all