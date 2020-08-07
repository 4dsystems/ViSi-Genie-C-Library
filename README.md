![image](https://4dsystems.com.au/media/downloads/4DLogo.png)

ViSi-Genie-C-Library
==============================================================

C Library for 4D Systems ViSi-Genie Environment

## Information

This library provides high level functions for host controllers programmed in the C programming language, to ease communication with 4D Systems modules when using the module configured with ViSi-Genie.
Workshop4 PRO adds additional features to ViSi-Genie, allowing the User to write 4DGL code which can be executed at will from ViSi-Genie, enabling many advanced features not previously possible.
Please refer to the 4D Systems website, namingly the Workshop 4 Product Page, for documentation regarding Workshop 4, and its environments.

## Setup Instructions
The main library requires a couple of functions to be setup.

- unsigned long millis(void);    
    - returns the system time in milliseconds
- uint16_t genieGetByteCount(void);
    - returns the number of bytes present in the UART RX buffer
- uint8_t genieGetByte(void);
    - returns the next byte and take it out of the buffer
- uint8_t geniePeekByte(void);
    - returns the next byte and retain the current buffer    
- void geniePutByte(uint8_t);
    - sends a byte to the display module

## Example Demos

Inside the library are example demos for different Platforms/IDEs, to assist with getting started on using this library. Inside is also a ViSi-Genie Workshop4 project, which can be used on a range of 4D Systems displays (designed on a gen4-uLCD-32PT however can be changed via Workshop4 menu). It illustrates how to use some of the commands in the library include Read Object, Write Object, Reported Messages, Write Contrast and Write String.

## Tested with

This library has been tested on the STM32F030, however could easily be adapted (or may work as-is) to/with other MCU's running C 

If you've tested this library with other development platforms, please don't hesitate to notify us so we can update the compability list.

Any problems discovered with this library, please contact technical support from the 4D Sytems website so fixes can be put in place, or seek support from our forum.

## Compatible 4D Systems Display Modules

This library will work with all 4D Systems Modules which are capable of using the ViSi-Genie environment. This is therefore all Picaso, Diablo16, Pixxi-28 and Pixxi-44 Display Modules.
The demo included with this library was made for the uLCD-32PTU however can easily be adapted to other size displays.

## General Library Discussion
-----------------------------

This section serves to give brief discussion about the constructor and functions included in the library. For functional examples on how to use these functions in a project, refer to the examples folder.

### genieBegin()
Prepares the variables used by the library to it's initial states and values. This also attempts to sync with the display. Returns 'true' if successful, otherwise, this returns 'false'. 

    // Check if the display doesn't sync successfully
    if (!genieBegin()) {
        // Usually, the display will sync as long as all the extern functions
        // are working as required. However, in case it doesn't, you can process
        // necessary actions here (ex. resetting the display)
    }

### genieWriteContrast(uint16_t value)
Sets the display contrast/brightness to a new *value*

| Parameters | Description |
|:----------:| ----------- |
| value      | New brightness/contrast value. The range of this value depends on the 4D display, Typically 0-15 Range |

    genieWriteContrast(0); // Sets the contrast/brightness value to 0, effectively turning off the backlight
    genieWriteContrast(10); // Sets the contrast/brightness value to 10, about 2/3 max brightness

### genieReadObject(uint16_t object, uint16_t index)
Sends a request to read the value of the widget specified by *object* (ex: GENIE_OBJ_GAUGE) and *index*. The value will be sent as a GENIE_REPORT_OBJECT command.

A full list of available objects (ex: GENIE_OBJ_GAUGE, GENIE_OBJ_SLIDER etc) can be found at the bottom of this Readme.

| Parameters | Description |
|:----------:| ----------- |
| object     | Type of target widget |
| index      | Index number of target widget |

    genieReadObject(GENIE_OBJ_GAUGE, 0); // Request a report of the widget Gauge0

### genieWriteObject(uint16_t object, uint16_t index, uint16_t data)
Updates the widget, specified by *object* (ex: GENIE_OBJ_GAUGE) and *index*, to a new value specified by *data*

A full list of available objects (ex: GENIE_OBJ_GAUGE, GENIE_OBJ_SLIDER etc) can be found at the bottom of this Readme.

| Parameters | Description |
|:----------:| ----------- |
| object     | Type of target widget |
| index      | Index number of target widget |
| data       | New value for the target widget |

    genieWriteObject(GENIE_OBJ_GAUGE, 0, 50); // Sets Gauge0 to 50

### genieWriteShortToIntLedDigits(uint16_t index, int16_t data)
Updates the Internal LedDigits specified by *index* to a new 16-bit value, specified by *data*. The widget parameter *Format* in ViSi Genie project should be set to Int16. Internal LedDigits are availble for Diablo and Pixxi displays.

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target Internal LedDigits |
| data       | New 16-bit integer value for the target Internal LedDigits |

    genieWriteShortToIntLedDigits(0, 50); // Sets ILedDigits0 to 50

### genieWriteFloatToIntLedDigits(uint16_t index, float data)
Updates the Internal LedDigits specified by *index* to a new 32-bit float value, specified by *data*. The widget parameter *Format* in ViSi Genie project should be set to any Float option. Internal LedDigits are availble for Diablo and Pixxi displays.

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target Internal LedDigits |
| data       | New 32-bit float value for the target Internal LedDigits |

    genieWriteFloatToIntLedDigits(0, 3.1416f); // Sets ILedDigits0 to 3.1416

### genieWriteLongToIntLedDigits(uint16_t index, int32_t data)
Updates the Internal LedDigits specified by *index* to a new 32-bit integer value, specified by *data*. The widget parameter *Format* in ViSi Genie project should be set to Int16. Internal LedDigits are availble for Diablo and Pixxi displays.

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target Internal LedDigits |
| data       | New 32-bit integer value for the target Internal LedDigits |

    genieWriteLongToIntLedDigits(0, 1000000L); // Sets ILedDigits0 to 1000000

### genieWriteStr(uint16_t index, char *string)
Updates the String widget specified by *index* with a new character string specified by *string*

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target String |
| string     | Character pointer containing the text to print in the String widget |

    genieWriteStr(0, "Sample String"); // Set text in String0 to "Sample String"

### genieWriteStrU(uint16_t index, uint16_t *string)
Updates the String widget specified by *index* with a new Unicode (16-bit) character string specified by *string*

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target String |
| string     | Character pointer containing the text to print in the String widget |

    uint16_t * unistr = {0x0034, 0x0044, 0x0020, 0x30B7, 0x30B9, 0x30C6, 0x30E0, 0x30BA, 0};
    genieWriteStr(2, unistr); // Writes the Unicode string "4D システムズ" to String2

### genieWriteInhLabelDefault(uint16_t index)
Updates the Inherent Label widget specified by *index* with the default contents defined in Workshop4

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target Inherent Label |

    genieWriteInhLabelDefault(1); // Set text in ILabelB1 to default String
	
### genieWriteInhLabel(uint16_t index, char * string)
Updates the Inherent Label widget specified by *index* with a new character string specified by *string*

| Parameters | Description |
|:----------:| ----------- |
| index      | Index number of target Inherent Label |
| string     | Character pointer containing the text to print in the Inherent Label widget |

    genieWriteInhLabel(0, "Sample String"); // Set text in ILabelB0 to "Sample String"

### genieAttachEventHandler(UserEventHandlerPtr handler)
Attach an event handler to handle messages from the display (ex. GENIE_REPORT_EVENT and GENIE_REPORT_OBJECT). Ideally, the handler function doesn't do anything that blocks for a long period since this would cause the command handling to be delayed.

| Parameters  | Description |
|:-----------:| ----------- |
| userHandler | Pointer to the handler function. The function should follow the format *void userHandler()* |

- Usage Example

      genieAttachEventHandler(myGenieEventHandler); // Attach the event handler
                                                    // for processing messages

- Handler Function Example

      void myGenieEventHandler(void)
      {
          genieFrame Event; // Prepare a genie frame container
          genieDequeueEvent(&Event); // Store next message from display to 'Event'

          // Process event here (quickly), see example below

          // Check if the message stored in 'Event' is a GENIE_REPORT_EVENT from  Slider0
          if (genieEventIs(&Event, GENIE_REPORT_EVENT, GENIE_OBJ_SLIDER, 0))    
          {
              // Receive the event data from the Slider0
              slider_val = genieGetEventData(&Event);
              // Write Slider0 value to LedDigits0
              genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val);
          }          
      }

Please refer to the demos provided for more context of what this looks like when implemented.

### genieDoEvents()
This handles all the receiving of messages from the display and therefore should run as frequent as possible. This also calls the event handlers attached using *AttachEventHandler*, *AttachMagicByteReader* and *AttachMagicDoubleByteReader*
This should be run in your main loop() or in a controlled function which runs as often as possible.

    void loop()
    {
        static long waitPeriod = millis();
        genieDoEvents(); // Run as frequently as possible for best experience

        if (millis() >= waitPeriod)
        {
            // Update the display here

            // Ensure that this code block only runs after ~100ms
            waitPeriod = millis() + 100;
        }
    }

### genieEventIs(genieFrame * e, uint8_t cmd, uint8_t object, uint8_t index)
Returns true if the command stored in genieFrame *e* matches the criteria specified by *cmd*, *object* and *index*. Otherwise, returns false.

| Parameters  | Description |
|:-----------:| ----------- |
| e           | Pointer to a genieFrame structure that specifies where the event under evaluation is stored |
| cmd         | Specifies the type of command expected of the message |
| object      | Specifies the type of widget from which the event is from |
| index       | Specifies the index of widget from which the event is from |

    // Check if the message stored in 'Event' is a GENIE_REPORT_EVENT from Slider0
    if (genieEventIs(&Event, GENIE_REPORT_EVENT, GENIE_OBJ_SLIDER, 0))    
    {
        // Receive the event data from the Slider0
        slider_val = genieGetEventData(&Event);
        // Write Slider0 value to LedDigits0
        genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val);
    }

### genieDequeueEvent(genieFrame * buff)
Remove the next message from the queue and store it to genieFrame *buff*. This function should be used inside the custom event handler.

| Parameters  | Description |
|:-----------:| ----------- |
| buff        | Pointer to a genieFrame structure that specifies where the next event should be stored |

See *AttachEventHandler* for an example.

### genieGetEventData(genieFrame * e)
Retrieves the 16-bit value from genieFrame *e*

| Parameters  | Description |
|:-----------:| ----------- |
| buff        | Pointer to a genieFrame structure that specifies where the data to read is from |

See *AttachEventHandler* for an example.

### genieOnline()
Returns the status of the connection between the display and the host controller.

    // Check if the display is online before writing to a widget
    if (genieOnline())    
    {
        // Write newval to LedDigits0
        genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, newval);
    }

### genieUptime()
Returns the amount of time in milliseconds that the display is connected 

    // Prints a message containing the amount of time the display is connected
    printf("The display is online for %lu milliseconds\n", genieUptime());

### genieCurrentForm()
Returns the last detected form

    // Prints a message containing the amount of time the display is connected
    printf("The display is in Form%d\n", genieCurrentForm());

### genieActivateForm(uint8_t form)
Sends a WriteObject command to active the specified *form*

| Parameters | Description |
|:----------:| ----------- |
| forum      | Index number of target form |

    genieActivateForm(5); // Activates Form5

### genieAttachDebugger(UserDebuggerHandlerPtr handler)
Attach a printf debugger to handle debug messages from the library.

| Parameters  | Description |
|:-----------:| ----------- |
| handler     | Pointer to the handler function. The function should follow the format *void userHandler()* |

    genieAttachDebugger(printf); // Attach a printf function as debugger
                                 // for printing library messages

### genieRecover(uint8_t pulses)
Sets the amount of time in milliseconds used by the library to ping the display when offline

| Parameters  | Description |
|:-----------:| ----------- |
| pulses      | Ping interval in milliseconds when display is offline |

    genieRecover(50); // Sets the library to ping the display
                      // every 50 ms when disconnected

### genieTimeout(uint16_t value)
Sets the amount of time in milliseconds used by the library when waiting for incoming acknowledgement response (ACK or NACK)

| Parameters  | Description |
|:-----------:| ----------- |
| value       | Timeout period in milliseconds to use when waiting for response |

    genieRecover(50); // Sets the library to ping the display
                      // every 50 ms when disconnected

### geniePing()
Attempts to sends a ReadObject to read the current form as a ping request. The ping will not be sent if the next ping cycle time hasn't been reached.


    geniePing(); // Usually not needed since the library
                 // handles this automatically

## ViSi Genie Pro Library Discussion
------------------------------------

### genieAttachMagicByteReader(UserMagicByteHandlerPtr handler)
Attach an event handler to handle magic bytes from the display. Magic Bytes needs to be programmed using Magic features (ex. MagicTouch, MagicRelease, etc.) of ViSi Genie.

It is suggested to store the data and process them later on to prevent a blocking delay.

| Parameters  | Description |
|:-----------:| ----------- |
| userHandler | Pointer to the handler function. The function should follow the format *void UserBytePtr(uint8_t, uint8_t)* |

- Usage Example

      // Attach the event handler for processing magic bytes
      genieAttachMagicByteReader(myMagicByteHandler);

- Handler Function Example

      byte buffer0[255]; // Temporary buffer 0 to store the magic bytes
      byte buffer1[255]; // Temporary buffer 1 to store the magic bytes

      void myMagicByteHandler(uint8_t index, uint8_t len)
      {
          // If the magic bytes is coming from MagicObject0
          if (index == 0) {
              // Store the data to buffer0
              for (int i = 0; i < len; i++) {
                  buffer0[i] = genieGetNextByte();
              }
          }
          // If the magic bytes is coming from MagicObject1
          if (index == 1) {
              // Store the data to buffer1
              for (int i = 0; i < len; i++) {
                  buffer1[i] = genieGetNextByte();
              }
          }
      }

### genieAttachMagicDoubleByteReader(UserMagicDByteHandlerPtr handler)
Attach an event handler to handle Magic Double Bytes from the display. Magic Double Bytes needs to be programmed using Magic features (ex. MagicTouch, MagicRelease, etc.) of ViSi Genie.

It is suggested to store the data and process them later on to prevent a blocking delay.

| Parameters  | Description |
|:-----------:| ----------- |
| userHandler | Pointer to the handler function. The function should follow the format *void UserBytePtr(uint8_t, uint8_t)* |

- Usage Example

      // Attach the event handler for processing magic double bytes
      genieAttachMagicDoubleByteReader(myMagicDoubleByteHandler);

- Handler Function Example

      uint16_t buffer0[255]; // Temporary buffer 0 to store the magic double bytes
      uint16_t buffer1[255]; // Temporary buffer 1 to store the magic double bytes

      void myMagicDoubleByteHandler(uint8_t index, uint8_t len)
      {
          // If the magic double bytes is coming from MagicObject0
          if (index == 0) {
              // Store the data to buffer0
              for (int i = 0; i < len; i++) {
                  buffer0[i] = genieGetNextDoubleByte();
              }
          }
          // If the magic double bytes is coming from MagicObject1
          if (index == 1) {
              // Store the data to buffer1
              for (int i = 0; i < len; i++) {                  
                  buffer1[i] = genieGetNextDoubleByte();
              }
          }
      }

### genieGetNextByte()
This function can be utilized to receive next incoming byte from the display. This should be used when receiving magic bytes in the user byte handler.

See *AttachMagicByteReader* for an example.

### genieGetNextDoubleByte()
This function can be utilized to receive next two (2) incoming bytes from the display. This should be used when receiving magic double bytes in the user double byte handler.

See *AttachMagicDoubleByteReader* for an example.

### genieWriteMagicBytes(uint16_t index, uint8_t *bytes, uint16_t len)
Send magic *bytes* with size *len* to MagicObject specified by *index*. The specified MagicObject should be programmed to handle the magic bytes that will be sent by this function.

| Parameters  | Description |
|:-----------:| ----------- |
| index       | Index of MagicObject that is programmed to receive and process the data |
| bytes       | Pointer to a byte array that specifies where the data is stored|
| len         | Specifies the number of bytes to send |

    uint8_t bytes[] = {0x34, 0x44, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73};
    genieWriteMagicBytes(0, bytes, 10); // Writes 10 magic bytes to MagicObject0

### genieWriteMagicDBytes(uint16_t index, uint16_t *dbytes, uint16_t len)
Send magic * bytes* with size *len* to MagicObject specified by *index*. The specified MagicObject should be programmed to handle the magic bytes that will be sent by this function.

| Parameters  | Description |
|:-----------:| ----------- |
| index       | Index of MagicObject that is programmed to receive and process the data |
| dbytes      | Pointer to a 16-bit array that specifies where the data is stored|
| len         | Specifies the number of double bytes to send |

    uint16_t dbytes[] = {0x0034, 0x0044, 0x0020, 0x30B7, 0x30B9, 0x30C6, 0x30E0, 0x30BA};
    genieWriteMagicDBytes(1, dbytes, 8); // Writes 8 magic double bytes to MagicObject1

## Available Library Object Types
---------------------------------

This section lists all of the objects/widgets that are available to be used by this library, which represent each of the widgets available to be placed using the Workshop4 IDE.

These object names are inserted into the ReadObject and WriteObject functions (listed above), and data received back from them presents itself in the AttachEventHandler function.

For more information on each of the actual Widgets in Workshop4, please refer to the Workshop4 Widgets Reference Manual - available on the Workshop4 Product page of the 4D Systems website.

| Object Name | Description |
| ----------- | ----------- |
| GENIE_OBJ_DIPSW	| Dip Switch Widget |
| GENIE_OBJ_KNOB	| Knob Widget |
| GENIE_OBJ_ROCKERSW	| Rocker Switch Widget |
| GENIE_OBJ_ROTARYSW	| Rotary Switch Widget |
| GENIE_OBJ_SLIDER	| Slider Widget |
| GENIE_OBJ_TRACKBAR	| Track Bar Widget |
| GENIE_OBJ_WINBUTTON	| Winbutton Widget |
| GENIE_OBJ_ANGULAR_METER	| Angular Meter Widget |
| GENIE_OBJ_COOL_GAUGE	| Cool Gauge Widget |
| GENIE_OBJ_CUSTOM_DIGITS	| Custom Digits Widget |
| GENIE_OBJ_FORM	| Form Widget |
| GENIE_OBJ_GAUGE	| Gauge Widget |
| GENIE_OBJ_IMAGE	| Image Widget |
| GENIE_OBJ_KEYBOARD	| Keyboard Widget |
| GENIE_OBJ_LED	| LED Widget |
| GENIE_OBJ_LED_DIGITS	| LED Digits Widget |
| GENIE_OBJ_METER	| Meter Widget |
| GENIE_OBJ_STRINGS	| String Widget |
| GENIE_OBJ_THERMOMETER	| Thermometer Widget |
| GENIE_OBJ_USER_LED	| User LED Widget |
| GENIE_OBJ_VIDEO	| Video Widget |
| GENIE_OBJ_STATIC_TEXT	| Static Text Widget |
| GENIE_OBJ_SOUND	| Sound Widget |
| GENIE_OBJ_TIMER	| Timer Widget |
| GENIE_OBJ_SPECTRUM	| Spectrum Widget |
| GENIE_OBJ_SCOPE	| Scope Widget |
| GENIE_OBJ_TANK	| Tank Widget |
| GENIE_OBJ_USERIMAGES	| User Images Widget |
| GENIE_OBJ_PINOUTPUT	| Pin Output Widget |
| GENIE_OBJ_PININPUT	| Pin Input Widget |
| GENIE_OBJ_4DBUTTON	| 4D Button Widget |
| GENIE_OBJ_ANIBUTTON	| Animated Button Widget |
| GENIE_OBJ_COLORPICKER	| Colour Picker Widget |
| GENIE_OBJ_USERBUTTON	| User Button Widget |
| GENIE_OBJ_SMARTGAUGE	| Smart Gauge Widget (PRO Only) |
| GENIE_OBJ_SMARTSLIDER	| Smart Slider Widget (PRO Only) |
| GENIE_OBJ_SMARTKNOB	| Smart Knob Widget (PRO Only) | 
| GENIE_OBJ_ILED_DIGITS_H	| Internal LED Digits Widget - High Byte |
| GENIE_OBJ_ILED_DIGITS_L	| Internal LED Digits Widget - Low Byte |
| GENIE_OBJ_IANGULAR_METER	| Internal Angular Meter Widget |
| GENIE_OBJ_IGAUGE	| Internal Gauge Widget |
| GENIE_OBJ_ILED	| Internal LED Widget |
| GENIE_OBJ_INEEDLE	| Internal Needle Widget |
| GENIE_OBJ_IRULER	| Internal Ruler Widget |
| GENIE_OBJ_ILED_DIGIT	| Internal LED Digit Widget |
| GENIE_OBJ_ILED_DIGITS	| Internal LED Digits Widget |
| GENIE_OBJ_IBUTTOND	| Internal Button D Widget |
| GENIE_OBJ_IDIAL	| Internal Dial Widget |
| GENIE_OBJ_ISWITCH	| Internal Switch Widget |
| GENIE_OBJ_ISLIDERE	| Internal Slider E Widget |
| GENIE_OBJ_IBUTTONE	| Inherent Button E Widget |
| GENIE_OBJ_ITOGGLE_INPUT	| Inherent Toggle Input Widget |
| GENIE_OBJ_ILABELB	| Inherent Label B Widget |
| GENIE_OBJ_IUSER_GAUGE	| Inherent User Gauge Widget |
| GENIE_OBJ_IMEDIA_BUTTON	| Inherent Media Button Widget |
| GENIE_OBJ_IMEDIA_GAUGE	| Inherent Media Gauge Widget |
| GENIE_OBJ_IMEDIA_THERMOMETER	| Inherent Media Thermometer Widget |
| GENIE_OBJ_IMEDIA_ROTARY	| Inherent Media Rotary Widget |
| GENIE_OBJ_IMEDIA_LED	| Inherent Media LED Widget |
| GENIE_OBJ_IMEDIA_SLIDER	| Inherent Media Slider Widget |
| GENIE_OBJ_IROTARY_INPUT	| Inherent Rotary Input Widget |
| GENIE_OBJ_ISWITCHB	| Inherent Switch B Widget |
| GENIE_OBJ_ISLIDERH	| Inherent Slider H Widget |
| GENIE_OBJ_ISLIDERG	| Inherent Slider G Widget |
| GENIE_OBJ_ISLIDERF	| Inherent Slider F Widget |
| GENIE_OBJ_ISLIDERD	| Inherent Slider D Widget |
| GENIE_OBJ_ISLIDERC	| Inherent Slider C Widget |
| GENIE_OBJ_ILINEAR_INPUT	| Inherent Linear Input Widget |

## Questions/Issues?

Please sign up for our Forum and ask a question there, or submit a Tech Support Ticket from our website.
http://forum.4dsystems.com.au or http://www.4dsystems.com.au/support
