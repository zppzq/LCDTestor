# Microsoft Developer Studio Project File - Name="KadyUsbTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=KadyUsbTest - Win32 Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KadyUsbTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KadyUsbTest.mak" CFG="KadyUsbTest - Win32 Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KadyUsbTest - Win32 Checked" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KadyUsbTest - Win32 Free" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KadyUsbTest - Win32 Checked"

# PROP Use_MFC 0
# PROP Output_Dir ".\objchk\i386"
# PROP Intermediate_Dir ".\objchk\i386"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /Gz /W3 /Zi /Oi /Gy /I "$(DRIVERWORKS)\..\common\include\stl" /I "$(DRIVERWORKS)\include" /I "$(DRIVERWORKS)\source" /I "$(TARGET_INC_PATH)" /I "$(CRT_INC_PATH)" /I "$(DDK_INC_PATH)" /I "$(WDM_INC_PATH)" /I "$(DRIVERWORKS)\include\dep_vxd" /I "$(DRIVERWORKS)\include\dep_wdm" /I "$(DRIVERWORKS)\include\dep_ndis" /FI"warning.h" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D DBG=1 /D "DEPRECATE_DDK_FUNCTIONS" /D _WIN32_WINNT=$(_WIN32_WINNT) /D WINVER=$(WINVER) /D _WIN32_IE=$(_WIN32_IE) /D NTDDI_VERSION=$(NTDDI_VERSION) /D NTVERSION='WDM' /Fd".\objchk\i386\KadyUsbTest.pdb" /Zel -cbstring /GF       /c
# ADD BASE RSC /l 0x409 /i "$(CRT_INC_PATH)" /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(CRT_INC_PATH)" /d "_DEBUG" /d NTVERSION='WDM'
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 ntoskrnl.lib hal.lib  usbd.lib vdw_wdm.lib  /nologo /base:"0x10000" /version:5.0 /stack:0x40000,0x1000 /entry:"DriverEntry" /debug /machine:IX86 /nodefaultlib /out:".\objchk\i386\KadyUsbTest.sys" /libpath:"$(TARGET_LIB_PATH)" /libpath:"$(DRIVERWORKS)\lib\i386\checked" /libpath:"$(DRIVERNETWORKS)\lib\i386\checked" /driver /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text  /FULLBUILD /RELEASE /OPT:REF /OPT:ICF /align:0x80 /osversion:5.00 /subsystem:native,1.10   /ignore:4010,4037,4039,4065,4070,4078,4087,4089,4099,4221,4210
# Begin Custom Build - ---------------------------Build SoftICE Symbols----------------------------
OutDir=.\objchk\i386
TargetName=KadyUsbTest
InputPath=.\objchk\i386\KadyUsbTest.sys
SOURCE="$(InputPath)"

"$(OutDir)\$(TargetName).nms" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(DRIVERWORKS)\bin\nmsym /trans:source,package,always $(OutDir)\$(TargetName).sys

# End Custom Build

!ELSEIF  "$(CFG)" == "KadyUsbTest - Win32 Free"

# PROP Use_MFC 0
# PROP Output_Dir ".\objfre\i386"
# PROP Intermediate_Dir ".\objfre\i386"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /Gz /W3 /Oy /Gy /I "$(DRIVERWORKS)\..\common\include\stl" /I "$(DRIVERWORKS)\include" /I "$(DRIVERWORKS)\source" /I "$(TARGET_INC_PATH)" /I "$(CRT_INC_PATH)" /I "$(DDK_INC_PATH)" /I "$(WDM_INC_PATH)" /I "$(DRIVERWORKS)\include\dep_vxd" /I "$(DRIVERWORKS)\include\dep_wdm" /I "$(DRIVERWORKS)\include\dep_ndis" /FI"warning.h" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D _WIN32_WINNT=$(_WIN32_WINNT) /D WINVER=$(WINVER) /D _WIN32_IE=$(_WIN32_IE) /D NTDDI_VERSION=$(NTDDI_VERSION) /D NTVERSION='WDM' /Zel -cbstring /GF /Oxs      /c
# ADD BASE RSC /l 0x409 /i "$(CRT_INC_PATH)" /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(CRT_INC_PATH)" /d "NDEBUG" /d NTVERSION='WDM'
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 ntoskrnl.lib hal.lib  usbd.lib vdw_wdm.lib  /nologo /base:"0x10000" /version:5.0 /stack:0x40000,0x1000 /entry:"DriverEntry" /machine:IX86 /nodefaultlib /out:"objfre\i386\KadyUsbTest.sys" /libpath:"$(TARGET_LIB_PATH)" /libpath:"$(DRIVERWORKS)\lib\i386\free" /libpath:"$(DRIVERNETWORKS)\lib\i386\free" /driver /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text  /FULLBUILD /RELEASE /OPT:REF /OPT:ICF /align:0x80 /osversion:5.00 /subsystem:native,1.10  /debug:MINIMAL  /ignore:4010,4037,4039,4065,4070,4078,4087,4089,4099,4221,4210
# Begin Custom Build - ---------------------------Build SoftICE Symbols----------------------------
OutDir=.\objfre\i386
TargetName=KadyUsbTest
InputPath=.\objfre\i386\KadyUsbTest.sys
SOURCE="$(InputPath)"

"$(OutDir)\$(TargetName).nms" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(DRIVERWORKS)\bin\nmsym /trans:source,package,always $(OutDir)\$(TargetName).sys

# End Custom Build

!ENDIF 

# Begin Target

# Name "KadyUsbTest - Win32 Checked"
# Name "KadyUsbTest - Win32 Free"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\KadyUsbTestDevice.cpp
DEP_CPP_KADYU=\
	"..\intrface.h"\
	".\function.h"\
	".\KadyUsbTestDevice.h"\
	".\KadyUsbTestDriver.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\common\include\stcinit.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\cpprt.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\cright.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\kndiscpp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\KNdisList.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\KNdisSpinLock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_vxd\vtoolscp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_vxd\vxdntlib.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dwcontrl.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394async.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394Irb.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394isoc.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kadapter.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kaddress.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\karray.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kbus.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kchecker.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kcontrol.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kcsq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdevice.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdevque.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdispobj.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmaxfer.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmqcsq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmqex.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdpc.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdriver.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kerrlog.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kevent.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfile.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfilter.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kgenlock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kheap.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kicount.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kimgsect.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kintrupt.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kiocparm.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kirp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\klist.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\klower.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kmemory.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kmutex.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kndisvdw.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpcicfg.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kphysdev.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpnpdev.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpnplow.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kquery.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kregkey.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kresreq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksemaphr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksfifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kspin.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kstdwmi.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksysthrd.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ktimer.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ktrace.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kunitnam.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kusb.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kusbbusintf.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kustring.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kvirtualcom.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kvxdintf.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\Kwdmfltr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmi.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmiblock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmistr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kworkitm.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ShFifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\vdw.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\source\shfifo.cpp"\
	"D:\WINDDK\2600\inc\crt\basetsd.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\1394.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb100.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb200.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbbusif.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbdlib.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbdrivr.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbiodef.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\wdm.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\afilter.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ia64reg.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\mce.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ndis.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ntddk.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\xfilter.h"\
	"D:\WINDDK\2600\inc\wxp\bugcodes.h"\
	"D:\WINDDK\2600\inc\wxp\evntrace.h"\
	"D:\WINDDK\2600\inc\wxp\guiddef.h"\
	"D:\WINDDK\2600\inc\wxp\ia64reg.h"\
	"D:\WINDDK\2600\inc\wxp\netevent.h"\
	"D:\WINDDK\2600\inc\wxp\ntddndis.h"\
	"D:\WINDDK\2600\inc\wxp\ntddser.h"\
	"D:\WINDDK\2600\inc\wxp\ntdef.h"\
	"D:\WINDDK\2600\inc\wxp\ntiologc.h"\
	"D:\WINDDK\2600\inc\wxp\ntstatus.h"\
	"D:\WINDDK\2600\inc\wxp\usb.h"\
	"D:\WINDDK\2600\inc\wxp\usb100.h"\
	"D:\WINDDK\2600\inc\wxp\usb200.h"\
	"D:\WINDDK\2600\inc\wxp\usbdi.h"\
	"D:\WINDDK\2600\inc\wxp\usbioctl.h"\
	"D:\WINDDK\2600\inc\wxp\usbiodef.h"\
	"D:\WINDDK\2600\inc\wxp\wmistr.h"\
	
# End Source File
# Begin Source File

SOURCE=.\KadyUsbTestDriver.cpp
DEP_CPP_KADYUS=\
	".\function.h"\
	".\KadyUsbTestDevice.h"\
	".\KadyUsbTestDriver.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\common\include\stcinit.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\cpprt.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\cright.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\kndiscpp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\KNdisList.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_ndis\KNdisSpinLock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_vxd\vtoolscp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dep_vxd\vxdntlib.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\dwcontrl.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394async.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394Irb.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\k1394isoc.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kadapter.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kaddress.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\karray.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kbus.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kchecker.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kcontrol.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kcsq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdevice.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdevque.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdispobj.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmaxfer.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmqcsq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdmqex.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdpc.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kdriver.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kerrlog.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kevent.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfile.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kfilter.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kgenlock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kheap.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kicount.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kimgsect.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kintrupt.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kiocparm.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kirp.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\klist.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\klower.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kmemory.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kmutex.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kndisvdw.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpcicfg.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kphysdev.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpnpdev.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kpnplow.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kquery.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kregkey.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kresreq.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksemaphr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksfifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kspin.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kstdwmi.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ksysthrd.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ktimer.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ktrace.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kunitnam.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kusb.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kusbbusintf.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kustring.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kvirtualcom.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kvxdintf.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\Kwdmfltr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmi.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmiblock.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kwmistr.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\kworkitm.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\ShFifo.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\include\vdw.h"\
	"D:\PROGRA~1\Compuware\DriverStudio\DriverWorks\source\shfifo.cpp"\
	"D:\WINDDK\2600\inc\crt\basetsd.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\1394.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb100.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usb200.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbbusif.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbdlib.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbdrivr.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\usbiodef.h"\
	"D:\WINDDK\2600\inc\ddk\wdm\wxp\wdm.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\afilter.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ia64reg.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\mce.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ndis.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\ntddk.h"\
	"D:\WINDDK\2600\inc\ddk\wxp\xfilter.h"\
	"D:\WINDDK\2600\inc\wxp\bugcodes.h"\
	"D:\WINDDK\2600\inc\wxp\evntrace.h"\
	"D:\WINDDK\2600\inc\wxp\guiddef.h"\
	"D:\WINDDK\2600\inc\wxp\ia64reg.h"\
	"D:\WINDDK\2600\inc\wxp\netevent.h"\
	"D:\WINDDK\2600\inc\wxp\ntddndis.h"\
	"D:\WINDDK\2600\inc\wxp\ntddser.h"\
	"D:\WINDDK\2600\inc\wxp\ntdef.h"\
	"D:\WINDDK\2600\inc\wxp\ntiologc.h"\
	"D:\WINDDK\2600\inc\wxp\ntstatus.h"\
	"D:\WINDDK\2600\inc\wxp\usb.h"\
	"D:\WINDDK\2600\inc\wxp\usb100.h"\
	"D:\WINDDK\2600\inc\wxp\usb200.h"\
	"D:\WINDDK\2600\inc\wxp\usbdi.h"\
	"D:\WINDDK\2600\inc\wxp\usbioctl.h"\
	"D:\WINDDK\2600\inc\wxp\usbiodef.h"\
	"D:\WINDDK\2600\inc\wxp\wmistr.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\function.h
# End Source File
# Begin Source File

SOURCE=.\..\intrface.h
# End Source File
# Begin Source File

SOURCE=.\KadyUsbTestDevice.h
# End Source File
# Begin Source File

SOURCE=.\KadyUsbTestDriver.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;mc;mof"
# Begin Source File

SOURCE=.\KadyUsbTest.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\KadyUsbTest.cat
# End Source File
# Begin Source File

SOURCE=.\KadyUsbTest.inf
# End Source File
# Begin Source File

SOURCE=.\makefile
# End Source File
# Begin Source File

SOURCE=.\sources
# End Source File
# End Target
# End Project
