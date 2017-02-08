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

main.obj : src\main.c
	$(COMPILER) src\main.c -i="$(WATCOM_BASE)/h" $(CC_OPTIONS)

$(TARGET) : main.obj
  $(LINKER) name winprotz d all sys $(PLATFORM) op m op maxe=25 op q op symf FIL main.obj
