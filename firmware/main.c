/* Name: main.c
 * Project: hid-keyboard, a very simple HID example
 * Author: Christian Starkjohann, Wojciech Kordalski
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 *            (c) 2014 by Wojciech Kordalski
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.

We use VID/PID 0x046D/0xC00E which is taken from a Logitech mouse. Don't
publish any hardware using these IDs! This is for demonstration only!
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[45] = { /* USB report descriptor, size must match usbconfig.h */
		0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)    
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)    
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)    
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)    
    0xc0                           // END_COLLECTION
};

typedef struct{
    uchar   mod;
    uchar		reserved;
    uchar		keys[6];
}report_t;

static report_t buffer;
static uchar xtra, rate, cntr;

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&buffer;
            return sizeof(buffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &rate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            rate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

static uchar stab, scnt, read, keys;
const uchar smax = 8;



void check()
{
	uchar nk = ~PINB;
	if(nk != stab)
	{
		// nk jest nowym potencjalnie stabilnym
		stab = nk;
		scnt = 0;
	}
	else if(stab != read)
	{
		// nk popiera aktualnego potencjalnie stabilnego
		scnt++;
		// potencjalnie stabilny jest wystarczająco pewny
		if(scnt == smax)
		{
			read = stab;
		}
	}	
}

// should send extra package even if nothing happens?
void idle()
{
	// hadrware counter has overflown
	if(TIFR & (1<<TOV0))
	{
		TIFR |= (1<<TOV0);		// clear bit TOV0 in TIFR
		// TIMER EVENT RIGHT NOW (overflow for every 4ms)
		if(rate != 0)		// we have to send report by time even if nothing happen
		{
			if(cntr > 4) cntr -= 5;	// program counter step
			else
			{
				cntr = rate;	// reset program counter
				xtra = 1;
			}
		}
		// else idle reporting is off
	}
}

void led(int a, int b)
{
  if(a) PORTD |= (1<<3);
  if(b) PORTD |= (1<<4);
  if(!a) PORTD &= ~(1<<3);
  if(!b) PORTD &= ~(1<<4);
}

int __attribute__((noreturn)) main(void)
{
uchar   i, kc;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
		PORTB = 0xff;
		DDRB = 0;
		
		stab = scnt = read = keys = xtra = rate = cntr = 0;
		
		// TIMER
		TCCR0A = 5;		// clk/1024 prescaller
		
		led(1, 0);
		
		
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    led(0, 1);
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    led(1,1);
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    led(0,0);
    for(;;){                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
				check();
				idle();
        if(usbInterruptIsReady() && (read != keys || xtra))
				{
            // todo
						buffer.mod = 0;
						buffer.reserved = 0;
						buffer.keys[0] = 0;
						buffer.keys[1] = 0;
						buffer.keys[2] = 0;
						buffer.keys[3] = 0;
						buffer.keys[4] = 0;
						buffer.keys[5] = 0;
						kc = 0;
						if(read & 1)
						{
							buffer.keys[kc++] = 64;	// F7
						}
						if(read & 2)
						{
							buffer.keys[kc++] = 66;	// F9
						}
						if(read & 4)
						{
							buffer.keys[kc++] = 73;	// Insert
						}
						if(kc == 0) buffer.mod = 0;
						keys = read;
            DBG1(0x03, 0, 0);   /* debug output: interrupt report prepared */
            usbSetInterrupt((void *)&buffer, sizeof(buffer));
        }
    }
}

/* ------------------------------------------------------------------------- */
