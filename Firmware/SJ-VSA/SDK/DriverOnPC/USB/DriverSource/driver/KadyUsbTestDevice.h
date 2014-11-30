// KadyUsbTestDevice.h
//
// Generated by DriverWizard 3.2.0 (Build 2485)
// Requires DDK and DriverWorks
// File created on 4/16/2008
//
// This include file contains the definition of a subclass of KPnpDevice.
// WDM drivers declare a subclass of KPnpDevice and override member
// functions to handle requests (IRPs) from the system.
//
#ifndef __KADYUSBTESTDEVICE_H__
#define __KADYUSBTESTDEVICE_H__

class KadyUsbTestDevice : public KPnpDevice
{
public:
	SAFE_DESTRUCTORS;
	KadyUsbTestDevice(PDEVICE_OBJECT Pdo, ULONG Unit);
	~KadyUsbTestDevice();
	VOID Invalidate(void);

	// Member functions
	DEVMEMBER_DISPATCHERS

	virtual NTSTATUS OnStartDevice(KIrp I);
	virtual NTSTATUS OnStopDevice(KIrp I);
	virtual NTSTATUS OnRemoveDevice(KIrp I);
	virtual	NTSTATUS OnQueryCapabilities(KIrp I);
	MEMBER_COMPLETEIRP(KadyUsbTestDevice, OnQueryCapabilitiesComplete)
	virtual NTSTATUS OnDevicePowerUp(KIrp I);
	virtual NTSTATUS OnDeviceSleep(KIrp I);
	virtual NTSTATUS DefaultPnp(KIrp I);
	virtual NTSTATUS DefaultPower(KIrp I);

	typedef struct _USB_COMPLETION_INFO
	{
		PURB m_pUrb;
		KadyUsbTestDevice* m_pClass;
	} USB_COMPLETION_INFO, *PUSB_COMPLETION_INFO;

	MEMBER_COMPLETEIRPWITHCONTEXT(USB_COMPLETION_INFO, IoComplete)
	void TestBusInterface();

	void LoadRegistryParameters();

protected:
	// Member data
	KUsbLowerDevice		m_Lower;
	KUsbInterface   	m_Interface;
	KUsbPipe			EndPoint1In;	// Pipe for USB endpoint address 81, type INTERRUPT
	KUsbPipe			EndPoint2Out;	// Pipe for USB endpoint address 2, type INTERRUPT
#if (_WDM_ && (WDM_MAJORVERSION > 1 || ((WDM_MAJORVERSION == 1) && (WDM_MINORVERSION >= 0x20))))
	KUsbBusInterface	m_BusIntf;	// Direct client access to USB bus on Windows XP and above
	BOOLEAN				m_fBusIntfAvailable;
#endif
    PWSTR	KadyUsbDvcName;		// Registry string variable
#ifdef __COMMENT_ONLY
		// The following member functions are actually defined by 
		// a DEVMEMBER_XXX or MEMBER_XXX macro (such as DEVMEMBER_DISPATCHERS).
		// The macro __COMMENT_ONLY never gets defined.  These comment-only
		// definitions simply allow easy navigation to the functions within
		// the Visual Studio IDE using the class browser.
	virtual NTSTATUS Create(KIrp I); 				// COMMENT_ONLY
	virtual NTSTATUS Close(KIrp I);					// COMMENT_ONLY
	virtual NTSTATUS Read(KIrp I);			  		// COMMENT_ONLY
	virtual NTSTATUS Write(KIrp I);					// COMMENT_ONLY
	virtual NTSTATUS DeviceControl(KIrp I);			// COMMENT_ONLY
	virtual NTSTATUS CleanUp(KIrp I); 				// COMMENT_ONLY
	virtual NTSTATUS SystemControl(KIrp I);			// COMMENT_ONLY
#endif // __COMMENT_ONLY
};

#endif // __KADYUSBTESTDEVICE_H__