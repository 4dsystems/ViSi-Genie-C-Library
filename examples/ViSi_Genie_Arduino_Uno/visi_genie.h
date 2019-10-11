/*
 * visi_genie_beta.h
 *
 *  Created on: Sep 16, 2019
 *      Author: cruzj
 */

#ifndef VISI_GENIE_BETA_H_
#define VISI_GENIE_BETA_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "string.h"

#ifndef bool
#define bool	uint8_t
#endif

#ifndef true
#define true	(1==1)
#define false	(!true)
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef lowByte
#define lowByte(x) ((int8_t)((x) & 0xFF))
#endif

#ifndef highByte
#define highByte(x) ((int8_t)(((x) >> 8) & 0xFF))
#endif


// Based on Arduino Library v1.4.5 [04-08-2017]
#define GENIE_VERSION		"v1.0.0-beta - 09-16-2019"

// Do not modify current values. Recommended settings.
#define DISPLAY_TIMEOUT         2000
#define AUTO_PING_CYCLE         1250

// Structure to store replys returned from a display
#define GENIE_FRAME_SIZE        6

#define MAX_GENIE_EVENTS    	16    // MUST be a power of 2

#define GENIE_ACK               0x06
#define GENIE_NAK               0x15
#define GENIE_PING              0x80
#define GENIE_READY             0x81
#define GENIE_DISCONNECTED      0x82


typedef enum {
	GENIE_READ_OBJ,
	GENIE_WRITE_OBJ,
	GENIE_WRITE_STR,
	GENIE_WRITE_STRU,
	GENIE_WRITE_CONTRAST,
	GENIE_REPORT_OBJ,
	GENIE_REPORT_EVENT = 7,
	GENIEM_WRITE_BYTES,
	GENIEM_WRITE_DBYTES,
	GENIEM_REPORT_BYTES,
	GENIEM_REPORT_DBYTES
} GenieCommand;

typedef enum {
	GENIE_OBJ_DIPSW,
	GENIE_OBJ_KNOB,
	GENIE_OBJ_ROCKERSW,
	GENIE_OBJ_ROTARYSW,
	GENIE_OBJ_SLIDER,
	GENIE_OBJ_TRACKBAR,
	GENIE_OBJ_WINBUTTON,
	GENIE_OBJ_ANGULAR_METER,
	GENIE_OBJ_COOL_GAUGE,
	GENIE_OBJ_CUSTOM_DIGITS,
	GENIE_OBJ_FORM,
	GENIE_OBJ_GAUGE,
	GENIE_OBJ_IMAGE,
	GENIE_OBJ_KEYBOARD,
	GENIE_OBJ_LED,
	GENIE_OBJ_LED_DIGITS,
	GENIE_OBJ_METER,
	GENIE_OBJ_STRINGS,
	GENIE_OBJ_THERMOMETER,
	GENIE_OBJ_USER_LED,
	GENIE_OBJ_VIDEO,
	GENIE_OBJ_STATIC_TEXT,
	GENIE_OBJ_SOUND,
	GENIE_OBJ_TIMER,
	GENIE_OBJ_SPECTRUM,
	GENIE_OBJ_SCOPE,
	GENIE_OBJ_TANK,
	GENIE_OBJ_USERIMAGES,
	GENIE_OBJ_PINOUTPUT,
	GENIE_OBJ_PININPUT,
	GENIE_OBJ_4DBUTTON,
	GENIE_OBJ_ANIBUTTON,
	GENIE_OBJ_COLORPICKER,
	GENIE_OBJ_USERBUTTON,
	GENIE_OBJ_MAGIC_RESERVED,
	GENIE_OBJ_ISMARTGAUGE,
	GENIE_OBJ_ISMARTSLIDER,
	GENIE_OBJ_ISMARTKNOB
} GenieObject;

typedef struct {
    uint8_t        cmd;
    uint8_t        object;
    uint8_t        index;
    uint8_t        data_msb;
    uint8_t        data_lsb;
} FrameReportObj;

typedef struct {
    uint8_t         cmd;
    uint8_t         index;
    uint8_t         length;
} MagicReportHeader;

/////////////////////////////////////////////////////////////////////
// The Genie frame definition
//
// The union allows the data to be referenced as an array of uint8_t
// or a structure of type FrameReportObj, eg
//
//    genieFrame f;
//    f.bytes[4];
//    f.reportObject.data_lsb
//
//    both methods get the same byte
//
typedef union {
    uint8_t             bytes[GENIE_FRAME_SIZE];
    FrameReportObj      reportObject;
} genieFrame;

typedef struct {
    genieFrame    	frames[MAX_GENIE_EVENTS];
    uint8_t        	rd_index;
    uint8_t        	wr_index;
    uint8_t        	n_events;
} EventQueueStruct;


typedef void        (*UserEventHandlerPtr) (void);
typedef void        (*UserMagicByteHandlerPtr) (uint8_t, uint8_t);
typedef void        (*UserMagicDByteHandlerPtr) (uint8_t, uint8_t);
//typedef void		(*UserDebuggerHandlerPtr) (char *);
typedef void 		(*UserDebuggerHandlerPtr) (const char *, ...);

EventQueueStruct EventQueue;

UserEventHandlerPtr UserHandler;
UserMagicByteHandlerPtr UserMagicByteHandler;
UserMagicDByteHandlerPtr UserMagicDByteHandler;
UserDebuggerHandlerPtr UserDebuggerHandler;

// used internally by the library, do not modify!
volatile bool     pendingACK; // prevent userhandler if waiting for ACK, and end recursion.
volatile bool     pingRequest; // used internally by the library, do not touch.
volatile uint8_t  recover_pulse; // pulse for offline autoping, use genie.recover(x) to change it from sketch.
volatile bool     autoPing; // used internally by the library, do not touch.
volatile uint16_t GENIE_CMD_TIMEOUT; // force disconnection trigger if ACK times out
volatile uint32_t autoPingTimer; // timer for autoPinger() function
volatile bool     displayDetected; // display is online/offline state
volatile uint32_t displayDetectTimer; // timer for lcd to be aware if connected
volatile uint8_t  currentForm; // current form thats loaded
volatile uint8_t  nakInj; // nak injection counter
volatile uint8_t  badByteCounter; // used for disconnection/debugging purposes
volatile uint32_t delayedCycles; // session protection if latency in user code
volatile uint32_t display_uptime; // uptime of display
volatile uint32_t ping_spacer; // prevent flooding the uart during recovery. non-blocking.
volatile bool     genieStart;


// Public Functions
bool 		genieBegin();

uint8_t     genieReadObject(uint16_t object, uint16_t index);
uint16_t    genieWriteObject(uint16_t object, uint16_t index, uint16_t data);
uint8_t     genieWriteContrast(uint16_t value);
uint16_t    genieWriteStr(uint16_t index, char *string);
uint16_t    genieWriteStrU(uint16_t index, uint16_t *string);
bool        genieEventIs(genieFrame * e, uint8_t cmd, uint8_t object, uint8_t index);
uint16_t    genieGetEventData(genieFrame * e);
bool        genieDequeueEvent(genieFrame * buff);

uint8_t		genieDoEvents();
void        genieAttachEventHandler(UserEventHandlerPtr handler);
void        genieAttachMagicByteReader(UserMagicByteHandlerPtr handler);
void        genieAttachMagicDoubleByteReader(UserMagicDByteHandlerPtr handler);
void 		genieAttachDebugger(UserDebuggerHandlerPtr handler);
bool 		genieOnline();
uint32_t	genieUptime();
uint8_t		genieCurrentForm();
void		genieActivateForm(uint8_t form);
void		genieRecover(uint8_t pulses);
uint8_t		genieTimeout(uint16_t value);
uint8_t		geniePing();
uint16_t	genieEnableAutoPing(uint16_t interval);

// Genie Magic functions (ViSi-Genie Pro Only)

#define		genieWriteMagicBytes(index, bytes, len)		_genieWriteMagicBytes(index, bytes, len, 0);
uint16_t    _genieWriteMagicBytes(uint16_t index, uint8_t *bytes, uint16_t len, uint8_t report);
#define		genieWriteMagicDBytes(index, shorts, len)	_genieWriteMagicDBytes(index, shorts, len, 0);
uint16_t    _genieWriteMagicDBytes(uint16_t index, uint16_t *shorts, uint16_t len, uint8_t report);

uint8_t     genieGetNextByte(void);
uint16_t    genieGetNextDoubleByte(void);

// Private Functions

uint8_t     genieEnqueueEvent(uint8_t * data);

extern unsigned long millis(void);
extern uint16_t genieGetByteCount(void);
extern uint8_t genieGetByte(void);
extern uint8_t geniePeekByte(void);
extern void geniePutByte(uint8_t);

#ifdef __cplusplus
} // extern C
#endif

#endif /* VISI_GENIE_BETA_H_ */
