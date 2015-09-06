/*
  This is an example code to load the libwmbus.so library into your project.
  In this example the library must be placed in the same folder as the executable
  file.

  To compile and link the example program use the following command:
  gcc -o main main.c -ldl -lpthread
 */

#include <stdio.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>

#define LIB_WMBUSHCI    "./libwmbushci.so"
#define INVALID_HANDLE  0
#define WMBUS_DEVICE    "/dev/ttyUSB0"

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned long   TWMBusHandle;

typedef void (*TWMBus_CbMsgHandler)(UINT32    msg, UINT32 handle);

typedef int (*open_t)(const char* port);
typedef int (*close_t)(TWMBusHandle handle);
typedef void(*regCbMessage_t)(UINT32 msg, UINT32 handle);
typedef int (*ping_t)(TWMBusHandle handle);
typedef int (*getdevinfo_t)(TWMBusHandle handle, UINT8* buffer, UINT16 bufSize);
typedef int (*hcimessage_t)(TWMBusHandle handle, UINT8*	buffer, UINT16 bufferSize);
typedef int (*getlasterror_t)(TWMBusHandle handle);
typedef int (*geterrorstring_t)(UINT32 error, char* string, int size);
typedef int (*configureAESDecryptionKey_t)(TWMBusHandle handle, UINT8 slotIndex, UINT8* addressFilter, UINT8* key);
typedef int (*dllshutdown_t)();

open_t                      WMBus_OpenDevice;
close_t                     WMBus_CloseDevice;
ping_t                      WMBus_Ping;
getdevinfo_t                WMBus_GetDeviceInfo;
getlasterror_t              WMBus_GetLastError;
geterrorstring_t            WMBus_GetErrorString;
dllshutdown_t               WMBus_DLLShutdown;
hcimessage_t                WMBus_GetHCIMessage;
regCbMessage_t              WMBus_RegisterMsgHandler;
configureAESDecryptionKey_t WMBus_ConfigureAESDecryptionKey;


/* loading library */
bool loadLibWMBusHCI()
{
    void* libHandle = dlopen(LIB_WMBUSHCI, RTLD_NOW);

    if(!libHandle) {
        printf("Error while loading shared object\n");
        return false;
    }

    printf("Loading symbols...\n");
    // loading shared object symbols
    WMBus_OpenDevice = (open_t) dlsym(libHandle, "WMBus_OpenDevice");
    WMBus_CloseDevice = (close_t) dlsym(libHandle, "WMBus_CloseDevice");
    WMBus_Ping = (ping_t) dlsym(libHandle, "WMBus_PingRequest");
    WMBus_GetDeviceInfo = (getdevinfo_t) dlsym(libHandle, "WMBus_GetDeviceInfo");
    WMBus_GetLastError = (getlasterror_t) dlsym(libHandle, "WMBus_GetLastError");
    WMBus_GetErrorString = (geterrorstring_t) dlsym(libHandle, "WMBus_GetErrorString");
    WMBus_DLLShutdown = (dllshutdown_t) dlsym(libHandle, "WMBus_DLLShutdown");
    WMBus_GetHCIMessage = (hcimessage_t) dlsym(libHandle, "WMBus_GetHCIMessage");
    WMBus_RegisterMsgHandler = (regCbMessage_t) dlsym(libHandle, "WMBus_RegisterMsgHandler");
    WMBus_ConfigureAESDecryptionKey = (configureAESDecryptionKey_t)dlsym(libHandle, "WMBus_ConfigureAESDecryptionKey");

    return true;
}

/*
 * ERROR handling
 */
void logWMBusError(int deviceHandle)
{
    char error_str[32];
    int error_code;

    error_code = WMBus_GetLastError(deviceHandle);
    WMBus_GetErrorString(error_code, error_str, sizeof(error_str));
    printf("WMBus device error: %s\n", error_str);
}

/*
 * library shutdown
 */
void unloadLibWMBusHCI()
{
    printf("DLL shutdown\n");
    WMBus_DLLShutdown();
}

/*
 * print HEX buffer
 */
void printBuffer(char* msg, UINT8* buf, UINT16 size)
{
    int i;
    printf("%s: ", msg);
    for(i=0; i<size; i++) {
        printf("0x%02X ", buf[i]);
    }
    printf("\n");
}

static void cbFunc(UINT32 msg, UINT32 handle)
{
    printf("Msg: %d\tHandle %d", msg, handle);

}

/*
 * Main Loop
 */
int main()
{
    // load shared library
    if(!loadLibWMBusHCI())
        return -1;

    // get device handle
    TWMBusHandle deviceHandle = WMBus_OpenDevice(WMBUS_DEVICE);

    // only if device handle is valid the interaction with
    // the shared object is possible
    if(deviceHandle > INVALID_HANDLE) {
        char msg[20] = {0};
        printf("Device open: %d\n", deviceHandle);

        // register cb method
        WMBus_RegisterMsgHandler(cbFunc, deviceHandle);

        // check connection with PING
        if(WMBus_Ping(deviceHandle)) {
            printf("Ping successful\n");
        } else {
            logWMBusError(deviceHandle);
        }

        // get the device configuration
        UINT8 tmp_buf[80] = {0};
        if(WMBus_GetDeviceInfo(deviceHandle, tmp_buf, sizeof(tmp_buf))) {
            sprintf(msg, "Get Device Information");
            printBuffer(msg, tmp_buf, sizeof(tmp_buf));
        } else {
            logWMBusError(deviceHandle);
        }

        UINT16 size = 255;
        UINT8 buf[255] = {0};

        // never ending loop for getting HCI messages
        while(1) {
            // getHCIMessage return true if the queue has some data
            if(WMBus_GetHCIMessage(deviceHandle, buf, size)) {
                sprintf(msg, "HCI Msg");
                printBuffer(msg, buf, size);
            } else { // else it returns false
                printf("empty HCI queue\n");
            }
            sleep(3);
        }

    } else {
        logWMBusError(deviceHandle);
        unloadLibWMBusHCI();
        return 1; //error occurred
    }

    // close device
    WMBus_CloseDevice(deviceHandle);
    // clean up
    unloadLibWMBusHCI();
    return 0;
}

