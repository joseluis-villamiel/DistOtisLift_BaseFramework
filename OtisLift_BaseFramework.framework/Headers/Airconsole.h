//
// Airconsole.h
// Airconsole SDK
//
// Copyright (c) 2014 Cloudstore Limited. All rights reserved.
//
// The Airconsole SDK allows you to write iOS applications that interface with the Airconsole
// wireless serial port device
//
// Please see www.get-console.com/airconsole for more details on the Airconsole products
//
//
// The SDK provides three main classes
//
// AirconsoleDevice - a 'bean' style class that contains the details of a discovered device
// AirconsoleMgr - a class that allows discovery of AirconsoleDevice objects
// AirconsoleSession - a class that allows connecting to an AirconsoleDevice object and sending/receiving data from it
//
// A typical workflow would be to create an instance of AirconsoleMgr within your application, set the delegate
// to a class that implements AirconsoleMgrDelegate and call the -(void)scanForDevices method. Your delegate will
// be called back with -(void)deviceAdded:(AirconsoleDevice *) and -(void)deviceRemoved:(AirconsoleDevice *) when
// devices are detected and removed from the network
// An application will then select one of the discovered devices, (or alternatively call the
//    -(AirconsoleDevice *)defaultDevice method on AirconsoleMgr) and pass it to the init method of an
// AirconsoleSession object
// After creating an AirconsoleSession instance an application should then set itself as a delegate
//   (AirconsoleSessionDelegate) and call the -(BOOL)connect method. The delegate will be called back once
// the connection attempt has completed (it may be successful or unsuccessful).
// If the connection attempt is successful the application can
// - call the setLineParameters method to set up baud rate, etc (this can be called before the connect method also)
// - call the 'write' method to send bytes to the serial port
// - call the 'read' method to read any bytes from the serial port (non-blocking); an applications delegate
//     will be called back with the sessionBytesAvailable method when bytes are available for reading
//
// A sample application showing a device browser and simple serial client are included in the SDK
//


#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

// Enumeration specifying valid values for the StopBits property on AirconsoleSession
typedef enum {
    AC_STOPBITS_1 = 1,
    AC_STOPBITS_2 = 2,
    AC_STOPBITS_1_5 = 3
} stopbits_t;

// Enumeration specifying valid values for the FlowControl property on AirconsoleSession
typedef enum {
    AC_FLOW_NONE = 0,
    AC_FLOW_SOFTWARE = 1,
    AC_FLOW_HARDWARE = 2,
    AC_FLOW_HARDWARE_DSR = 3
} flowcontrol_t;

// Enumeration specifying valid values for the Parity property on AirconsoleSession
typedef enum {
    AC_PARITY_NONE = 0,
    AC_PARITY_ODD = 1,
    AC_PARITY_EVEN = 2,
    AC_PARITY_MARK = 3,
    AC_PARITY_SPACE = 4
} parity_t;

// Enumeration specifying valid values for the DataBits property on AirconsoleSession
typedef enum {
    AC_DATABITS_7 = 7,
    AC_DATABITS_8 = 8
} databits_t;

// Anonymous enumeration for mask bits for the MSR and PrevMSR properties of AirconsoleSession
enum {
    AC_MSR_CTS = 0x10,
    AC_MSR_DSR = 0x20,
    AC_MSR_RI  = 0x40,
    AC_MSR_DCD = 0x80
};

// Enumeration specifying valid values for the transport type of an AirconsoleDevice object (IP or Bluetooth Low Energy)
typedef enum {
    AC_TRANSPORT_ANY = 0,
    AC_TRANSPORT_IP = 1,
    AC_TRANSPORT_BLE = 2
} transport_t;

// +------------------+
// | AirconsoleDevice |
// +------------------+
// A bean-style class representing a discovered AirconsoleDevice on the network
@interface AirconsoleDevice : NSObject

// The hostname of this device - typically Airconsole-XX (where XX is two random hex digits)
@property (nonatomic, readonly) NSString *name;

// A value indicating if this device is connected over IP or Bluetooth Low Energy
@property (nonatomic, readonly) transport_t transport;

// The IP address of the device (valid for IP transport only)
@property (nonatomic, readonly) NSString *ipAddress;

// The TCP port that the RFC 2217 service can be located on (valid for IP transport only)
@property (nonatomic, readonly) int port;

// The CBPeripheral object backing this device (valid for BLE transport only)
@property (nonatomic, readonly) CBPeripheral *blePeripheral;

// A string representing the type of device - "airconsole"
@property (nonatomic, readonly) NSString *deviceType;

// A string containing the current firmware version of the detected device (or blank if not detected)
@property (nonatomic, readonly) NSString *firmwareVersion;

// A string containing the current hardware version of the detected device (or blank if not detected)
// Since SDK1.60, blank prior to Airconsole 2.80
@property (nonatomic, readonly) NSString *hardwareVersion;

// The number of serial ports connected to this device (for multi-port Airconsole devices)
@property (nonatomic, readonly) int portCount;

// Returns the customer supplied name for the port number or an empty string if no name has been configured (for multi-port Airconsole devices that have more than one physical serial port). portNumber is in the range 1..portCount (valid for IP transport only)
- (NSString *)portName:(int)portNumber;

@end

// +---------------------------+
// | AirconsoleSessionDelegate |
// +---------------------------+
// A protocol definition that alerts delegates of interesting events within an AirconsoleSession
// All methods are optional, and all methods will be called on the main thread
@protocol AirconsoleSessionDelegate <NSObject>
@optional
// The connect method has been called and a connection to the device will be attempted
- (void)sessionWillConnect:(id)session;

// The session was successfull connected
- (void)sessionDidConnect:(id)session;

// The session did not connect successfully. A descriptive error message is provided
- (void)sessionFailedToConnect:(id)session errorMessage:(NSString *)errorMessage;

// The session has been disconnected
- (void)sessionDidDisconnect:(id)session;

// New data has been recieved from the device and is ready for reading. Applications should call
// the 'read' method on AirconsoleSession to read them
- (void)sessionBytesAvailable:(id)session count:(NSUInteger)count;

// The internal buffer holding data received from the device has overflowed - new data has been discarded
// This is an indication that the application should be calling 'read' more often or processing data more quickly
- (void)sessionDidOverflow:(id)session;

// The line parameters of the device have changed (usually at the application's request). This is called when any
// of BaudRate, DataBits, Parity, StopBits, FlowControl, DTR, RTS have been confirmed as changed
- (void)sessionLinePropertiesChanged:(id)session;

// The modem status bits on the deivce have changed (such as CTS, DSR, RI, DCD) - use the MSR and
// PrevMSR properties to determine the current (and previous) values
- (void)sessionModemStatusChanged:(id)session;

// The Signature field on the AirconsoleSession has changed/become available. Use this to determine software/hardware revision numbers and serial number of the device
// Since SDK1.60
- (void)sessionSignatureChanged:(id)session;

// The session was successfully authenticated
// Since SDK1.61
- (void)sessionDidAuthenticate:(id)session;
- (void)sessionDidFailToAuthenticate:(id)session;

// Since SDK1.63
// The battery level of the Airconsole has changed. Requires an Airconsole with battery level detection hardware. The new current battery level can be read from the AirconsoleSession.BatteryLevel property
- (void)sessionBatteryLevelChanged:(id)session;

@end

// +-------------------+
// | AirconsoleSession |
// +-------------------+
// Allows interaction with a specific Airconsole device (send/receive serial data)
@interface AirconsoleSession : NSObject

// A copy of the AirconsoleDevice that this class was initialised with
@property (nonatomic, readonly) AirconsoleDevice *device;

// Which serial port on the Airconsole this session relates to (for Airconsoles with multiple physical serial ports). Range is from 1 .. portCount
@property (nonatomic, readonly) int portNumber;

// Indicates if the session is currently connected to the device
@property (nonatomic, readonly) BOOL connected;

// Indicates is the session is in the process of connecting
@property (nonatomic, readonly) BOOL connecting;

// Indicates if the session has been (optionally) authenticated
// Since SDK1.61
@property (nonatomic, readonly) BOOL authenticated;

// The total number of bytes received in this session
@property (nonatomic, readonly) int rxByteCount;

// the total number of bytes transmitted in this session
@property (nonatomic, readonly) int txByteCount;

// The delegate
@property (nonatomic, assign) id<AirconsoleSessionDelegate> delegate;

// The current baud rate setting of the remote device (e.g. 9600)
// Note: there is a delay in this field updating after calling setLineParameters until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) int BaudRate;

// The current data bits setting of the remote device (e.g. AC_DATABITS_8)
// Note: there is a delay in this field updating after calling setLineParameters until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) databits_t DataBits;

// The current parity setting of the remote device (e.g. AC_PARITY_NONE)
// Note: there is a delay in this field updating after calling setLineParameters until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) parity_t Parity;

// The current stop bits setting of the remote device (e.g. AC_STOPBITS_1)
// Note: there is a delay in this field updating after calling setLineParameters until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) stopbits_t StopBits;

// The current flow control setting of the remote device (e.g. AC_FLOW_NONE)
// Note: there is a delay in this field updating after calling setFlowControl until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) flowcontrol_t FlowControl;

// The current DTR (Data Terminal Ready) setting of the remote device (e.g. ON)
// Note: there is a delay in this field updating after calling setDTR until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) BOOL DTR;

// The current RTS (Request To Send) setting of the remote device (e.g. ON)
// Note: there is a delay in this field updating after calling setRTS until confirmed by the device
// Note: the delegate method sessionLinePropertiesChanged is called when this field changes
@property (nonatomic, readonly) BOOL RTS;

// The current modem status register of the serial port on the remote device (e.g. AC_MSR_CTS | AC_MSR_DCD)
// Note: the delegate method sessionModemStatusChanged is called when this field changes
@property (nonatomic, readonly) unsigned char MSR;

// The previous modem status register of the serial port on the remote device (e.g. AC_MSR_CTS | AC_MSR_DCD)
// Note: the delegate method sessionModemStatusChanged is called when this field changes
@property (nonatomic, readonly) unsigned char PrevMSR;

// The 'signature' returned by the Airconsole device. This will include the Airconsole hostname and on supported devices the hardware & software revisions along with serial number
// Note: the delegate method sessionSignatureChanged is called when this field changes/becomes available
// Since SDK1.60
@property (nonatomic, readonly) NSString *Signature;

// The current battery level on the Airconsole in percentage from 0 (discharged) to 100 (fully charged) or -1 if battery level is not available
// Since SDK1.63
@property (nonatomic, readonly) int BatteryLevel;

// Application level keep-alive handling
@property (nonatomic, assign) BOOL keepaliveEnabled;
@property (nonatomic, assign) NSTimeInterval keepaliveInterval;
@property (nonatomic, assign) NSTimeInterval keepaliveTimeout;


// Constructor method for a new session. Called by user applications - pass in an AirconsoleDevice object
// obtained from the AirconsoleMgr class
- (id)initWithDevice:(AirconsoleDevice *)device;

// Constructor for a new session (as above) but specifying which physical serial port to connect to on the Airconsole (for multi-port Airconsole devices). portNumber is in the range 1 .. portCount
- (id)initWithDeviceAndPortNumber:(AirconsoleDevice *)device portNumber:(int)portNumber;

// Connect this session to the Airconsole device
// Will return 'YES' if the session is in the process of connecting. The connection progress happens
// on a background thread
// The delegate methods sessionDidConnect or sessionFailedToConnect will be called with the actual result
- (BOOL)connect;

// A synchronous version of the connect method - do not call this from the main thread
- (BOOL)connectSync;

// Disconnect this session from the Airconsole device
- (void)disconnect;

// Discard any data held in local buffers that has been received from the device but not read by the application
- (void)flush;

// Send data to the serial port on the Airconsole device
// - buffer contains a pointer to the data to send
// - length indicates the number of bytes to send
// The method returns the actual number of bytes that were sent
- (NSUInteger)write:(const uint8_t *)buffer length:(NSUInteger)length;

// Read data received from the Airconsole. This method will not block and will return immediately
// - buffer contains a pointer to a buffer to copy the data to
// - bufferLength contains the maximum number of bytes to read
// The method returns the actual number of bytes read
- (NSUInteger)read:(uint8_t *)buffer bufferLength:(NSUInteger)bufferLength;

// Returns the number of bytes of data that have been received from the Airconsole and are held
// in an interbal buffer ready for reading.
// Note that the buffer size of the AirconsoleSession class is limited
- (NSUInteger)bytesAvailable;

// Update the serial paramaters of the Airconsole serial port
// After calling this method a request is sent to the AirconsoleDevice to update the parameters. Once
// the parameters have been changed, they are confirmed by calling the delegate method sessionLinePropertiesChanged
- (void)setLineParameters:(int)baudRate dataBits:(databits_t)dataBits parity:(parity_t)parity stopBits:(stopbits_t)stopBits;

// Update the serial flow control setting of the Airconsole serial port
// After calling this method a request is sent to the AirconsoleDevice to update the flow control. Once
// the parameter has been changed, it is confirmed by calling the delegate method sessionLinePropertiesChanged
- (void)setFlowControl:(flowcontrol_t)flowControl;

// Instructs the Airconsole device to send a break on its serial port (hold at logic 0 for some period of time)
- (void)sendBreak;

// Instructs the Airconsole to set the DTR (Data Terminal Ready) line on the serial port to the sepcified setting
// The delegate method sessionLinePropertiesChanged is called once the line has been changed
- (void)setDTR:(BOOL)enabled;

// Instructs the Airconsole to set the RTS (Request To Send) line on the serial port to the sepcified setting
// The delegate method sessionLinePropertiesChanged is called once the line has been changed
- (void)setRTS:(BOOL)enabled;

// Set the authentication credentials for this session if required
// Since SDK1.61
- (void)setCredentials:(NSString *)user data:(NSData *)data;

@end

// +-----------------------+
// | AirconsoleMgrDelegate |
// +-----------------------+
// Delegate methods called when Airconsole devices are detected on the network
@protocol AirconsoleMgrDelegate <NSObject>

// An Airconsole device has been detected via mDNS on the network
- (void)deviceAdded:(AirconsoleDevice *)device;

// An Airconsole device has been removed from the network (or is no longer published via mDNS)
- (void)deviceRemoved:(AirconsoleDevice *)device;
@end

// +---------------+
// | AirconsoleMgr |
// +---------------+
// Provides a class to scan for AirconsoleDevice objects on the network
@interface AirconsoleMgr : NSObject

// Returns a string representation of the SDK version - e.g. "1.0"
@property (nonatomic, readonly) NSString *SDKVersion;

// The delegate object
@property (nonatomic, assign) id<AirconsoleMgrDelegate> delegate;

// Determines if scans for devices will happen across the WiFi interface (via mDNS). Set this parameter before calling scanForDevices (defauts to YES)
@property (nonatomic, readwrite) BOOL scanWiFi;

// Determines if scans for devices will include Bluetooth BLE devices. Set this parameter before calling scanForDevices (defaults to YES)
@property (nonatomic, readwrite) BOOL scanBluetooth;

// If "scanBluetooth" is set and the bluetooth radio is not powered on, iOS will present a dialog prompting the user to turn on Bluetooth. This flag allows applications to disable this warning (Bluetooth scanning will silently fail if the Bluetooth radio is not turned on). This needs to be set before the first call to "scanForDevices"
@property (nonatomic, readwrite) BOOL disableBluetoothWarning;

// Start scanning the network (via mDNS and/or Bluetooth Low Energy) for Airconsole devices
// When one is found, the delegate method deviceAdded is called
// When one is removed, the delegate method deviceRemoved is called
// Callers should set the properties "scanWiFi" and "scanBluetooth" as appropriately before calling this method
- (void)scanForDevices;

// Stop the scanning process
- (void)stopScanning;

// Return the default/best Airconsole device to connect to. If no devices have been discovered then a dummy
// object containing default settings is returned (which can be useful in environments where mDNS does not function)
- (AirconsoleDevice *)defaultDevice;

// Returns an array of all the currently discovered AirconsoleDevice objects
- (NSArray *)deviceList;

// Returns an array of all the currently discovered AirconsoleDevice objects of the specified transport type
- (NSArray *)deviceListOfType:(transport_t)transport;

@end
