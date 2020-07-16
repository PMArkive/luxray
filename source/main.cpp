#include "debug.hpp"
#include "lvgl/lvgl.h"
#include "screens/main_screen.hpp"
#include "ui/controller.hpp"
#include "util.hpp"

extern "C" {
u32 __nx_applet_type = AppletType_None;
TimeServiceType __nx_time_service_type = TimeServiceType_System;
char nx_inner_heap[INNER_HEAP_SIZE];

u32 __nx_nv_transfermem_size = 0x15000;

void __libnx_initheap(void);
void __libnx_init_time(void);
void __appInit(void);
void __appExit(void);
}

void __libnx_initheap(void) {
    extern char* fake_heap_start;
    extern char* fake_heap_end;
    fake_heap_start = nx_inner_heap;
    fake_heap_end = nx_inner_heap + sizeof(nx_inner_heap);
}

bool g_appInitSuccessful;
void __appInit(void) {
    g_appInitSuccessful = true;

    // Init services
    Result rc;
    if (R_FAILED(smInitialize())) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    if (R_FAILED(hidInitialize())) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
    if (R_FAILED(fsInitialize())) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    if (R_FAILED(rc = timeInitialize())) {
        // TimeServiceType_System only has 2 available session normally,
        // so we switch to TimeServiceType_User and flag the failure
        timeExit();
        g_appInitSuccessful = false;
        __nx_time_service_type = TimeServiceType_User;
        if (R_FAILED(rc = timeInitialize())) fatalThrow(rc);
    }
    if (R_FAILED(rc = apmInitialize())) fatalThrow(rc);
    if (R_FAILED(rc = setsysInitialize())) fatalThrow(rc);
    if (R_FAILED(rc = nifmInitialize(NifmServiceType_User))) fatalThrow(rc);
    if (R_FAILED(rc = viInitialize(ViServiceType_Manager))) fatalThrow(rc);

    __libnx_init_time();
    if (R_FAILED(fsdevMountSdmc())) fatalThrow(0x5D);

    auto socketConfig = SocketInitConfig{.bsdsockets_version = 1,

                                         .tcp_tx_buf_size = 0x800,
                                         .tcp_rx_buf_size = 0x1000,
                                         .tcp_tx_buf_max_size = 0,
                                         .tcp_rx_buf_max_size = 0,

                                         .udp_tx_buf_size = 0x2400,
                                         .udp_rx_buf_size = 0xA500,

                                         .sb_efficiency = 1};
    TRY_FATAL(socketInitialize(&socketConfig));

    debugInit();

    LOG("service init");
}

void __appExit(void) {
    // Cleanup services.
    debugExit();
    socketExit();
    fsdevUnmountAll();
    viExit();
    nifmExit();
    setsysExit();
    apmExit();
    timeExit();
    fsExit();
    hidExit();
    smExit();

#ifdef TESLA
    envSetNextLoad("sdmc:/switch/.overlays/ovlmenu.ovl", "--skipCombo");
#endif
}

int main(int argc, char* argv[]) {
    if (not g_appInitSuccessful) {
        LOG("__appInit failed");
        return -1;
    }

    LOG("Main start");

    try {
        ui::Controller::show(MainScreen::getInstance());
    } catch (std::runtime_error& e) {
        LOG("runtime_error: %s", e.what());
    }

    LOG("Main exit");
    return 0;
}
