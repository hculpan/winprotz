###############################################3
# winprotz Makefile
###############################################3

# Add as you add more files
HEADERS = src\mainpaint.h src\boardpaint.h
OBJ_DEPENDENCIES =  mainpaint.obj boardpaint.obj
EXTRA_OBJS = mainpaint.obj,boardpaint.obj


# Shouldn't need to update
TARGET = winprotz.exe
WATCOM_BASE = C:\WATCOM
PLATFORM = nt_win
COMPILER = wcc386
LINKER = wlink
CC_OPTIONS = -w4 -e25 -zq -od -d2 -6r -bt=nt -fo=.obj -mf

all : $(TARGET)

clean : .SYMBOLIC
	del *.obj >nul 2>&1
	del *.exe >nul 2>&1
	del *.map >nul 2>&1
	del *.sym >nul 2>&1
	del *.err >nul 2>&1

mainpaint.obj : src\mainpaint.c $(HEADERS)
		$(COMPILER) src\mainpaint.c -i="$(WATCOM_BASE)/h" $(CC_OPTIONS)

boardpaint.obj : src\boardpaint.c $(HEADERS)
		$(COMPILER) src\boardpaint.c -i="$(WATCOM_BASE)/h" $(CC_OPTIONS)

main.obj : src\main.c $(OBJ_DEPENDENCIES) $(HEADERS)
	$(COMPILER) src\main.c -i="$(WATCOM_BASE)/h" $(CC_OPTIONS)

$(TARGET) : main.obj
  $(LINKER) name winprotz d all sys $(PLATFORM) op m op maxe=25 op q op symf FIL main.obj,$(EXTRA_OBJS)
