/////////////////////////// GenieC 07/08/2020 //////////////////////////////
//
//      Library to utilize the 4D Systems Genie interface to displays
//      that have been created using the Visi-Genie creator platform.
//      This is intended to be used a generic library for platforms
//      supporting the C programming language.
//
//		Note that this library will require additional setup for user's 
//      choice of microcontroller
//
//      Improvements/Updates by (based on geneArduino library)
//		  4D Systems Engineering, August 2020, www.4dsystems.com.au
//		  4D Systems Engineering, July 2020, www.4dsystems.com.au
//        4D Systems Engineering, January 2016, www.4dsystems.com.au
//        4D Systems Engineering, October 2015, www.4dsystems.com.au
//        4D Systems Engineering, September 2015, www.4dsystems.com.au
//        4D Systems Engineering, August 2015, www.4dsystems.com.au
//        4D Systems Engineering, May 2015, www.4dsystems.com.au
//        Matt Jenkins, March 2015, www.majenko.com
//        Clinton Keith, January 2015, www.clintonkeith.com
//        4D Systems Engineering, July 2014, www.4dsystems.com.au
//        Clinton Keith, March 2014, www.clintonkeith.com
//        Clinton Keith, January 2014, www.clintonkeith.com
//        4D Systems Engineering, January 2014, www.4dsystems.com.au
//        4D Systems Engineering, September 2013, www.4dsystems.com.au
//      Written by
//        Rob Gray (GRAYnomad), June 2013, www.robgray.com
//      Based on code by
//        Gordon Henderson, February 2013, <projects@drogon.net>
//
//      Copyright (c) 2012-2020 4D Systems Pty Ltd, Sydney, Australia
/*********************************************************************
 * This file is part of GenieC:
 *    GenieC is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    GenieC is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with GenieC.
 *    If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/


#include  "visi_genie.h"

// Public Functions

bool genieBegin() {

	pendingACK = 0;
	pingRequest = 0;
	recover_pulse = 50;
	autoPing = 0;
	GENIE_CMD_TIMEOUT = 1250;
	autoPingTimer = 0;
	displayDetected = 0;
	displayDetectTimer = 0;
	currentForm = -1;
	nakInj = 0;
	badByteCounter = 0;
	delayedCycles = 0;
	display_uptime = 0;
	ping_spacer;
	genieStart = 1;

	UserHandler = NULL;
	UserMagicByteHandler = NULL;
	UserMagicDByteHandler = NULL;
	UserDebuggerHandler = NULL;

	display_uptime = millis(); // start uptime timer (ms)
	genieStart = 1; // start form request on startup
	genieReadObject(GENIE_OBJ_FORM, (uint8_t)0x00); // send form request
	uint32_t timeout_start = millis(); // timeout timer
	while ( millis() - timeout_start <= 250 ) { // blocking loop, releases after 150ms to timeout, or sooner if display's detected.
		if ( genieDoEvents() == GENIE_REPORT_OBJ && !genieStart ) return true; // form is updated.
	}
	displayDetected = false;
	return 0; // timeout occurred, status offline.
}


uint8_t genieReadObject(uint16_t object, uint16_t index) {
	if ( !displayDetected )
		return -1;
	uint8_t checksum;
	geniePutByte((uint8_t)GENIE_READ_OBJ);
	checksum = GENIE_READ_OBJ;
	geniePutByte(object);
	checksum ^= object;
	geniePutByte(index);
	checksum ^= index;
	geniePutByte(checksum);
	return 0;
}

uint16_t genieWriteObject(uint16_t object, uint16_t index, uint16_t data) {
    if (!displayDetected)
        return -1;
    uint8_t checksum;
    pendingACK = 1;
    geniePutByte(GENIE_WRITE_OBJ);
    checksum = GENIE_WRITE_OBJ;
    geniePutByte(object);
    checksum ^= object;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte(highByte(data));
    checksum ^= highByte(data);
    geniePutByte(lowByte(data));
    checksum ^= lowByte(data);
    geniePutByte(checksum);
    uint32_t timeout_write = millis();
    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
        uint8_t command_return = genieDoEvents();
        if (command_return == GENIE_ACK) {
            return 1;
        }
        if (command_return == GENIE_NAK) {
            return 0;
        }
    }
    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write Object didn't receive any reply\r\n");
    displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
    return -1; // timeout

}

uint16_t genieWriteShortToIntLedDigits (uint16_t index, int16_t data) {
    return genieWriteObject(GENIE_OBJ_ILED_DIGITS_L, index, data);
}

uint16_t genieWriteFloatToIntLedDigits (uint16_t index, float data) {
    FloatLongFrame frame;
    frame.floatValue = data;
    uint16_t retval;
    retval = genieWriteObject(GENIE_OBJ_ILED_DIGITS_H, index, frame.wordValue[1]);
    if (retval == 1) return retval;
    return genieWriteObject(GENIE_OBJ_ILED_DIGITS_L, index, frame.wordValue[0]);
}

uint16_t genieWriteLongToIntLedDigits (uint16_t index, int32_t data) {
    FloatLongFrame frame;
    frame.longValue = data;
    uint16_t retval;
    retval = genieWriteObject(GENIE_OBJ_ILED_DIGITS_H, index, frame.wordValue[1]);
    if (retval == 1) return retval;
    return genieWriteObject(GENIE_OBJ_ILED_DIGITS_L, index, frame.wordValue[0]);
}

uint8_t genieWriteContrast(uint16_t value) {
    if (!displayDetected)
        return -1;
    uint8_t checksum;
    pendingACK = 1;
    geniePutByte(GENIE_WRITE_CONTRAST);
    checksum = GENIE_WRITE_CONTRAST;
    geniePutByte(value);
    checksum ^= value;
    geniePutByte(checksum);
    uint32_t timeout_write = millis();
    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
        uint8_t command_return = genieDoEvents();
        if (command_return == GENIE_ACK) {
            return 1;
        }
        if (command_return == GENIE_NAK) {
            return 0;
        }
    }
    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write Contrast didn't receive any reply\r\n");
    displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
    return -1; // timeout
}

uint16_t genieWriteStr(uint16_t index, char *string) {
    if (!displayDetected)
        return -1;
    char* p;
    uint8_t checksum;
    pendingACK = 1;
    int len = strlen(string);
    if (len > 255)
        return -1;
    geniePutByte(GENIE_WRITE_STR);
    checksum = GENIE_WRITE_STR;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte((unsigned char)len);
    checksum ^= len;
    for (p = string; *p; ++p) {
        geniePutByte(*p);
        checksum ^= *p;
    }
    geniePutByte(checksum);
    uint32_t timeout_write = millis();
    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
        uint8_t command_return = genieDoEvents();
        if (command_return == GENIE_ACK) {
            return 1;
        }
        if (command_return == GENIE_NAK) {
            return 0;
        }
    }
    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write String didn't receive any reply\r\n");
//    displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
    return -1; // timeout
}

uint16_t genieWriteStrU(uint16_t index, uint16_t *string) {
    if (!displayDetected)
        return -1;
    uint16_t * p = string;
    int len = 0;
    while (*p++) len++;

    uint8_t checksum;
    pendingACK = 1;
    geniePutByte(GENIE_WRITE_STRU);
    checksum = GENIE_WRITE_STRU;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte((unsigned char)len);
    checksum ^= len;
    for (p = string; *p; ++p) {
        geniePutByte(*p >> 8);
        checksum ^= *p >> 8;
        geniePutByte(*p);
        checksum ^= *p & 0xff;
        p++;
    }
    geniePutByte(checksum);
    uint32_t timeout_write = millis();
    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
        uint8_t command_return = genieDoEvents();
        if (command_return == GENIE_ACK) {
            return 1;
        }
        if (command_return == GENIE_NAK) {
            return 0;
        }
    }
    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write Unicode String didn't receive any reply\r\n");
//    displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
    return -1; // timeout
}

uint16_t genieWriteInhLabelDefault(uint16_t index) {
    return genieWriteObject(GENIE_OBJ_ILABELB, index, -1);  
}

uint16_t genieWriteInhLabel(uint16_t index, char * string) {
    if (!displayDetected)
        return -1;
    char* p;
    uint8_t checksum;
    pendingACK = 1;
    int len = strlen(string);
    if (len > 255)
        return -1;
    geniePutByte(GENIE_WRITE_INH_LABEL);
    checksum = GENIE_WRITE_INH_LABEL;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte((unsigned char)len);
    checksum ^= len;
    for (p = string; *p; ++p) {
        geniePutByte(*p);
        checksum ^= *p;
    }
    geniePutByte(checksum);
    uint32_t timeout_write = millis();
    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
        uint8_t command_return = genieDoEvents();
        if (command_return == GENIE_ACK) {
            return 1;
        }
        if (command_return == GENIE_NAK) {
            return 0;
        }
    }
    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write String didn't receive any reply\r\n");
//    displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
    return -1; // timeout
}

bool genieEventIs(genieFrame * e, uint8_t cmd, uint8_t object, uint8_t index) {
    return (e->reportObject.cmd == cmd && e->reportObject.object == object && e->reportObject.index == index);
}

uint16_t genieGetEventData(genieFrame * e) {
    return (e->reportObject.data_msb << 8) + e->reportObject.data_lsb;
}

uint8_t genieDequeueEvent(genieFrame * buff) {
    if (EventQueue.n_events > 0) {
        memcpy(buff, &EventQueue.frames[EventQueue.rd_index], GENIE_FRAME_SIZE);
        EventQueue.rd_index++;
        EventQueue.rd_index &= MAX_GENIE_EVENTS - 1;
        EventQueue.n_events--;
    }
    return false;
}

uint8_t genieDoEvents() {
    geniePing(); // used to keep lcd connection alive

    uint8_t rx_data[6]; // array for receiving command, payload, and crc.
    uint8_t checksumVerify; // used to calculate a matching (or not) checksum.
//    if (UserDebuggerHandler != 0) UserDebuggerHandler("Genie Bytes Available : %i\r\n", genieGetByteCount());
    // ######################################
    // ## SLOW USER CODE? NO PROBLEM! #######
    // ######################################

    if (millis() - delayedCycles >= DISPLAY_TIMEOUT) {
        displayDetectTimer = millis(); // reset counter to prevent false disconnections.
    }
    delayedCycles = millis(); // reset the doevents function timeout, every cycle.

    if (genieOnline()) {
//        if (UserDebuggerHandler != 0) UserDebuggerHandler(" Current Time : %lu ms\r\nPrevious Time : %lu ms\r\n", millis(), displayDetectTimer);
        if (millis() - displayDetectTimer > DISPLAY_TIMEOUT) { // code online, but lcd is not?
            displayDetectTimer = millis();
            displayDetected = 0;
            pingRequest = 0;
            rx_data[0] = GENIE_PING;
            rx_data[1] = GENIE_DISCONNECTED;
            rx_data[2] = 0;
            rx_data[3] = 0;
            rx_data[4] = 1;
            rx_data[5] = 0;
            if (UserDebuggerHandler != 0) UserDebuggerHandler("Display was not responding for quite a while. Is it disconnected?\r\n");
            genieEnqueueEvent(rx_data);
            currentForm = -1; // reset form holder
            return -1;
        }
    }

    if (!displayDetected) { // not online?
        pendingACK = 0; // reset pending ACK check
        currentForm = -1; // reset form holder
        display_uptime = 0; // keeps timer reset
    }

    // ######################################
    // ## Main State Machine ################
    // ######################################

    if (genieGetByteCount() > 0) {
        uint8_t b = geniePeekByte(); // Look at the next byte but don't pull it yet.
        if (!displayDetected && (b == GENIEM_REPORT_BYTES || b == GENIEM_REPORT_DBYTES))
            b = 0xFF; // force bad bytes instead of false triggering genie magic switches.
        switch (b) { // We're going to parse what we see into the proper switch.

        case GENIE_ACK:
            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            genieGetByte(); // remove ACK
            badByteCounter = 0; // reset the bad byte counter
            pendingACK = 0;
            nakInj = 0; // reset NAK counter
            return GENIE_ACK;

        case GENIE_NAK:
            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            genieGetByte(); // remove NAK
            nakInj++; // increment consecutive NAK counter.
            while (geniePeekByte() == GENIE_NAK)
                genieGetByte(); // remove trailing naks for next test
            if (nakInj >= 2) { // if NAK's are consecutive 2 or more times...
                nakInj = 0; // reset the counter
                geniePutByte(0xFF); // inject a byte into the tx buffer to attempt recovery.
            }
            pendingACK = 0;
            return GENIE_NAK;

        case GENIEM_REPORT_BYTES:
            if (genieGetByteCount() < 3)
                break; // magic report event less than 3 bytes? check again.
            rx_data[0] = genieGetByte();
            rx_data[1] = genieGetByte();
            rx_data[2] = genieGetByte();
            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            badByteCounter = 0; // reset the bad byte counter
            if (UserMagicByteHandler != NULL)
                UserMagicByteHandler(rx_data[1], rx_data[2]);
            else
                for (int i = 0; i < rx_data[2]; i++)
                    genieGetByte();
            (void)genieGetNextByte();
            return GENIEM_REPORT_BYTES;

        case GENIEM_REPORT_DBYTES:
            if (genieGetByteCount() < 3)
                break; // magic report event less than 3 bytes? check again.
            rx_data[0] = genieGetByte();
            rx_data[1] = genieGetByte();
            rx_data[2] = genieGetByte();
            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            badByteCounter = 0; // reset the bad byte counter
            if (UserMagicDByteHandler != NULL)
                UserMagicDByteHandler(rx_data[1], rx_data[2]);
            else
                for (int i = 0; i < 2 * rx_data[2]; i++)
                    genieGetByte();
            (void)genieGetNextByte();
            return GENIEM_REPORT_DBYTES;

        case GENIE_REPORT_EVENT:
            if (genieGetByteCount() < 6)
                break; // report event less than 6 bytes? check again.
            rx_data[0] = genieGetByte();
            rx_data[1] = genieGetByte();
            rx_data[2] = genieGetByte();
            rx_data[3] = genieGetByte();
            rx_data[4] = genieGetByte();
            rx_data[5] = genieGetByte();
            checksumVerify = rx_data[0];
            checksumVerify ^= rx_data[1];
            checksumVerify ^= rx_data[2];
            checksumVerify ^= rx_data[3];
            checksumVerify ^= rx_data[4];
            if (checksumVerify != rx_data[5])
                return 0; //discard this packet, CRC is bad.
            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            badByteCounter = 0; // reset the bad byte counter
            if (rx_data[1] == GENIE_OBJ_FORM)
                currentForm = rx_data[2];
            genieEnqueueEvent(rx_data);
            return GENIE_REPORT_EVENT;

        case GENIE_REPORT_OBJ:
            if (genieGetByteCount() < 6) {
            //	if (UserDebuggerHandler != 0) UserDebuggerHandler("Genie Report Object has not enough bytes: %i\r\n", genieGetByteCount());
                break; // report event less than 6 bytes? check again.
            }
            rx_data[0] = genieGetByte();
            rx_data[1] = genieGetByte();
            rx_data[2] = genieGetByte();
            rx_data[3] = genieGetByte();
            rx_data[4] = genieGetByte();
            rx_data[5] = genieGetByte();
            checksumVerify = rx_data[0];
            checksumVerify ^= rx_data[1];
            checksumVerify ^= rx_data[2];
            checksumVerify ^= rx_data[3];
            checksumVerify ^= rx_data[4];
            if (checksumVerify != rx_data[5]){
            	if (UserDebuggerHandler != 0) UserDebuggerHandler("Genie Report Object has bad CRC\r\n");
            	return 0; //discard this packet, CRC is bad.
            }

            displayDetectTimer = millis(); // reset display timeout since the packet is good.
            badByteCounter = 0; // reset the bad byte counter
            if (rx_data[1] == GENIE_OBJ_FORM) {
                currentForm = rx_data[4];
                if (UserDebuggerHandler != 0) UserDebuggerHandler("Got Current Form\r\n");
            }
            // if ( genieStart ) { genieStart = 0; return GENIE_REPORT_OBJ; } // disable startup form checker

            if ((autoPing || pingRequest) && rx_data[1] == GENIE_OBJ_FORM) {
                if (autoPing) {
                    autoPing = 0; //switch off after queueing event
                    if (!displayDetected) { // if previously disconnected and now is connected...
                        display_uptime = millis(); // start uptime timer (ms)
                        rx_data[0] = GENIE_PING;
                        rx_data[1] = GENIE_READY;
                        rx_data[2] = 0;
                        rx_data[3] = 0;
                        rx_data[4] = 0;
                        rx_data[5] = 0;
                        genieEnqueueEvent(rx_data); // send ready state to user handler.
                        while (genieGetByteCount() > 0)
                            genieGetByte(); // clear on new connect
                        displayDetected = 1; // turn on functions
                    }
                    if (genieStart) {
                        genieStart = 0;
                        return GENIE_REPORT_OBJ;
                    }
                    break;
                }

                if (pingRequest) {
                    pingRequest = 0; //switch off after queueing event
                    rx_data[0] = GENIE_PING;
                    rx_data[1] = GENIE_ACK;
                    rx_data[2] = 0;
                    rx_data[3] = 0;
                    rx_data[4] = 0;
                    rx_data[5] = 0;
                    genieEnqueueEvent(rx_data); // send ACK to user ping request in handler.
                }
                break;
            }

            genieEnqueueEvent(rx_data);
            return GENIE_REPORT_OBJ; // all other reading of object data flow to event handler

        default: // unknown bytes found, shift out and start count for possible disconnection.
//            uint8_t bad_byte = genieGetByte();
        	(void) genieGetByte();
            badByteCounter++; // We count consecutively to 10 bytes in a row and assume display offline.
            if (badByteCounter > 10) {
                badByteCounter = 0; // let DoEvents do the disconnection.
                if (UserDebuggerHandler != 0) UserDebuggerHandler("Bad bytes received. Manual disconnect\r\n");
                displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000;
            }
            return GENIE_NAK;
        }
    }
    if (!pendingACK && EventQueue.n_events > 0 && UserHandler != NULL)
        UserHandler(); // trigger userhandler if queues exist.
    return 0;
}

void genieAttachDebugger(UserDebuggerHandlerPtr handler) {
	UserDebuggerHandler = handler;
}

void genieAttachEventHandler(UserEventHandlerPtr handler) {
    UserHandler = handler;
    uint8_t rx_data[6];
    // display status already collected from Begin function, user just enabled handler, so give a status.
    if (displayDetected) {
        rx_data[0] = GENIE_PING;
        rx_data[1] = GENIE_READY;
        rx_data[2] = 0;
        rx_data[3] = 0;
        rx_data[4] = 0;
        rx_data[5] = 0;
    }
    else {
        rx_data[0] = GENIE_PING;
        rx_data[1] = GENIE_DISCONNECTED;
        rx_data[2] = 0;
        rx_data[3] = 0;
        rx_data[4] = 0;
        rx_data[5] = 0;
        if (UserDebuggerHandler != 0) UserDebuggerHandler("Display was disconnected while attaching event handler\r\n");
    }
    genieEnqueueEvent(rx_data); // send current state to user handler.
}

void genieAttachMagicByteReader(UserMagicByteHandlerPtr handler) {
    UserMagicByteHandler = handler;
}

void genieAttachMagicDoubleByteReader(UserMagicDByteHandlerPtr handler) {
    UserMagicDByteHandler = handler;
}


bool genieOnline() {
    return displayDetected;
}

uint32_t genieUptime() {
    if (displayDetected)
        return millis() - display_uptime;
    else
        return 0;
}

uint8_t	genieCurrentForm() {
	return currentForm;
}

void genieActivateForm(uint8_t form) {
    genieWriteObject(GENIE_OBJ_FORM, form, (uint8_t)0x00);
}

void genieRecover(uint8_t pulses) {
    recover_pulse = pulses;
}

uint8_t genieTimeout(uint16_t value) {
    if (value < 50)
        return 0; // no less than 50 recommended! this will trigger the disconnect flag!
    GENIE_CMD_TIMEOUT = value;
    return 1;
}

uint8_t geniePing() {
    uint16_t geniePingTimerChanger;
    if (displayDetected)
        geniePingTimerChanger = AUTO_PING_CYCLE; // preset online pinger
    if (!displayDetected)
        geniePingTimerChanger = recover_pulse; // 50ms offline pinger
    if (millis() - autoPingTimer > geniePingTimerChanger) {
        autoPingTimer = millis();
        autoPing = 1;
        uint8_t checksum;
        geniePutByte((uint8_t)GENIE_READ_OBJ);
        checksum = GENIE_READ_OBJ;
        geniePutByte((uint8_t)GENIE_OBJ_FORM);
        checksum ^= (uint8_t)GENIE_OBJ_FORM;
        geniePutByte((uint8_t)0x00);
        checksum ^= (uint8_t)0x00;
        geniePutByte(checksum);
        if (UserDebuggerHandler != 0) UserDebuggerHandler("Sending Read Form as Ping\r\n");
    }
    return 1;
}

uint16_t genieEnableAutoPing(uint16_t interval) {
    if (displayDetected) {
        if (millis() - ping_spacer < interval)
            return 0;
        ping_spacer = millis();
        pingRequest = 1;
        uint8_t checksum;
        geniePutByte((uint8_t)GENIE_READ_OBJ);
        checksum = GENIE_READ_OBJ;
        geniePutByte((uint8_t)GENIE_OBJ_FORM);
        checksum ^= (uint8_t)GENIE_OBJ_FORM;
        geniePutByte((uint8_t)0x00);
        checksum ^= (uint8_t)0x00;
        geniePutByte(checksum);
        return 1;
    }
    if (!displayDetected) {
        if (millis() - ping_spacer > interval) {
            ping_spacer = millis();
            uint8_t rx_data[6];
            rx_data[0] = GENIE_PING;
            rx_data[1] = GENIE_NAK;
            rx_data[2] = 0;
            rx_data[3] = 0;
            rx_data[4] = 0;
            rx_data[5] = 0;
            genieEnqueueEvent(rx_data);
        }
    }
    return 0;
}


// Genie Magic functions (ViSi-Genie Pro Only)

uint16_t _genieWriteMagicBytes(uint16_t index, uint8_t *bytes, uint16_t len, uint8_t report) {
    if (!displayDetected)
        return -1;
    uint8_t checksum;
    geniePutByte(GENIEM_WRITE_BYTES);
    checksum = GENIEM_WRITE_BYTES;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte(len);
    checksum ^= len;
    for (int i = 0; i < len; i++) {
        geniePutByte(bytes[i]);
        checksum ^= bytes[i];
    }
    geniePutByte(checksum);

    if (!report) return 1;
	pendingACK = 1;

    uint32_t timeout_write = millis();

    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
		uint8_t command_return = genieDoEvents();
		if (command_return == GENIE_ACK) {
			return 1;
		}
		if (command_return == GENIE_NAK) {
			return 0;
		}
	}

    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write Magic Bytes didn't receive any reply\r\n");
	displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
	return -1; // timeout
}

uint16_t _genieWriteMagicDBytes(uint16_t index, uint16_t *shorts, uint16_t len, uint8_t report) {
    if (!displayDetected)
        return -1;
    uint8_t checksum;
    geniePutByte(GENIEM_WRITE_DBYTES);
    checksum = GENIEM_WRITE_DBYTES;
    geniePutByte(index);
    checksum ^= index;
    geniePutByte(len);
    checksum ^= len;
    for (int i = 0; i < len; i++) {
        geniePutByte(shorts[i] >> 8);
        checksum ^= shorts[i] >> 8;
        geniePutByte(shorts[i] & 0xFF);
        checksum ^= shorts[i] & 0xff;
    }
    geniePutByte(checksum);

    if (!report) return 1;
	pendingACK = 1;

    uint32_t timeout_write = millis();

    while (millis() - timeout_write <= GENIE_CMD_TIMEOUT) {
		uint8_t command_return = genieDoEvents();
		if (command_return == GENIE_ACK) {
			return 1;
		}
		if (command_return == GENIE_NAK) {
			return 0;
		}
	}

    if (UserDebuggerHandler != 0) UserDebuggerHandler("Write Magic Double Bytes didn't receive any reply\r\n");
	displayDetectTimer = millis() + DISPLAY_TIMEOUT + 10000; //manual disconnect
	return -1; // timeout
}

uint8_t genieGetNextByte(void) {
    if (!displayDetected)
        return -1; // user code may keep requesting, block till ready.
    uint8_t rx_data[6];
    uint32_t timeout = millis();
    while (genieGetByteCount() < 1) {
        delayedCycles = millis();
        displayDetectTimer = millis();
        if (millis() - timeout >= 2000) { // we issue an immediate manual disconnect.
            displayDetectTimer = millis();
            displayDetected = 0;
            while (genieGetByteCount() > 0)
                genieGetByte();
            rx_data[0] = GENIE_PING;
            rx_data[1] = GENIE_DISCONNECTED;
            rx_data[2] = 0;
            rx_data[3] = 0;
            rx_data[4] = 0;
            rx_data[5] = 0;
            if (UserDebuggerHandler != 0) UserDebuggerHandler("Display was disconnected while waiting for next byte\r\n");
            genieEnqueueEvent(rx_data);
            return -1;
        }
        continue;
    }
    delayedCycles = millis();
    displayDetectTimer = millis();
    return genieGetByte();
}

uint16_t genieGetNextDoubleByte(void) {
    if (!displayDetected)
        return -1; // user code may keep requesting, block till ready.
    uint8_t rx_data[6];
    uint16_t out;
    uint32_t timeout = millis();
    while (genieGetByteCount() < 2) {
        delayedCycles = millis();
        displayDetectTimer = millis();
        if (millis() - timeout >= 2000) { // we issue an immediate manual disconnect.
            displayDetectTimer = millis();
            displayDetected = 0;
            while (genieGetByteCount() > 0)
                genieGetByte();
            rx_data[0] = GENIE_PING;
            rx_data[1] = GENIE_DISCONNECTED;
            rx_data[2] = 0;
            rx_data[3] = 0;
            rx_data[4] = 0;
            rx_data[5] = 0;
            if (UserDebuggerHandler != 0) UserDebuggerHandler("Display was disconnected while waiting for next two bytes\r\n");
            genieEnqueueEvent(rx_data);
            return -1;
        }
        continue;
    }
    delayedCycles = millis();
    displayDetectTimer = millis();
    out = (genieGetByte()) << 8;
    out |= genieGetByte();
    return out;
}


// Private Functions

uint8_t genieEnqueueEvent(uint8_t * data) {
    if (EventQueue.n_events < MAX_GENIE_EVENTS - 2) {
        int i, j;
        bool fnd = 0;
        j = EventQueue.wr_index;
        for (i = EventQueue.n_events; i > 0; i--) {
            j--;
            if (j < 0)
                j = MAX_GENIE_EVENTS - 1;
            if ((EventQueue.frames[j].reportObject.cmd == data[0]) && (EventQueue.frames[j].reportObject.object == data[1]) && (EventQueue.frames[j].reportObject.index == data[2])) {
                EventQueue.frames[j].reportObject.data_msb = data[3];
                EventQueue.frames[j].reportObject.data_lsb = data[4];
                fnd = 1;
                break;
            }
        }
        if (!fnd) {
            memcpy(&EventQueue.frames[EventQueue.wr_index], data, GENIE_FRAME_SIZE);
            EventQueue.wr_index++;
            EventQueue.wr_index &= MAX_GENIE_EVENTS - 1;
            EventQueue.n_events++;
        }
        return fnd;
    }
    return false;
}
