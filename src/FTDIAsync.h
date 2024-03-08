

#ifndef USBSERIALTOUART_FTDIASYNC_H
#define USBSERIALTOUART_FTDIASYNC_H
#include <cdcftdi.h>

class FTDIAsync : public FTDIAsyncOper {
public:
    uint8_t OnInit(FTDI *pftdi);
};



#endif //USBSERIALTOUART_FTDIASYNC_H
