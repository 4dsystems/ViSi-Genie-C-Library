#include "visi_genie.h"

// This Demo communicates with a 4D Systems Display, configured with ViSi-Genie, utilising the Genie Arduino Library - https://github.com/4dsystems/ViSi-Genie-Arduino-Library.
// The display has a slider, a cool gauge, an LED Digits, a string box and a User LED.
// The program receives messages from the Slider0 object using the Reported Events. This is triggered each time the Slider changes on the display, and an event
// is generated and sent automatically. Reported Events originate from the On-Changed event from the slider itself, set in the Workshop4 software.
// Coolgauge is written to using Write Object, and the String is updated using the Write String command, showing the version of the library.
// The User LED is updated by the Arduino, by first doing a manual read of the User LED and then toggling it based on the state received back.

// As the slider changes, it sends its value to the Arduino, and the Arduino then
// tells the LED Digit to update its value using genieWriteObject. So the Slider message goes via the Arduino to the LED Digit.
// Coolgauge is updated via simple timer in the Arduino code, and updates the display with its value.
// The User LED is read using genieReadObject, and then updated using genieWriteObject. It is manually read, it does not use an Event.

// The library also automatically detects if the display is disconnected and reconnected. With these features, the program can now restore the values of objects after a reconnect.

// This demo illustrates how to use genieReadObject, genieWriteObject, Reported Messages (Events), genieWriteStr, genieWriteContrast, plus supporting functions.

// Application Notes on the 4D Systems Website that are useful to understand this library are found: http://www.4dsystems.com.au/appnotes
// Good App Notes to read are:
// ViSi-Genie Connecting a 4D Display to an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00017/
// ViSi-Genie Writing to Genie Objects Using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00018/
// ViSi-Genie A Simple Digital Voltmeter Application using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00019/
// ViSi-Genie Connection to an Arduino Host with RGB LED Control - http://www.4dsystems.com.au/appnote/4D-AN-00010/
// ViSi-Genie Displaying Temperature values from an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00015/
// ViSi-Genie Arduino Danger Shield - http://www.4dsystems.com.au/appnote/4D-AN-00025/

#define GenieSerial Serial

//#define GenieSerial Serial1
//#define GenieDebug Serial
#define CW 0
#define CCW 1

void geniePutByte(uint8_t c) {
  GenieSerial.write(c);
}
  
uint16_t genieGetByteCount() {
  return GenieSerial.available();
}

uint8_t geniePeekByte() {
  return GenieSerial.peek();
}

uint8_t genieGetByte() {
  return GenieSerial.read();
}

void setup() {
  pinMode(13, OUTPUT); // indicator led
    
  GenieSerial.begin(115200);  // UART for Genie Display
  
  genieBegin();
  genieAttachEventHandler(myGenieEventHandler);

  /* OR to make sure that the display is connected before proceeding:*/
//  while (!genieBegin()) {
//    Serial.println("display offline!");
//  }
//  if (genieOnline()) Serial.println("display online!");
 
  /* offline/sync recovery
     too low a value can flood the uart of the display during a connection/sync
     too high a value *might* cause lcd to go in limp mode
     most results seem to work at 50ms, if you have issues with slow response or
     have issues with events not running properly after a sync, adjust this. */
  //  genieRecover(50); // default 50ms

  /* This changes the form to another. genieCurrentForm(2) changes to form 2... */
  // genieCurrentForm(0); // go to form 0

  /* returns true (1) if the display is online or false (0) if out of sync/disconnected */
  if (genieOnline()) {
    /* Set the brightness/Contrast of the Display - (Not needed but illustrates how)
      Most Displays, 1 = Display ON, 0 = Display OFF. See below for exceptions and for DIABLO16 displays.
      For uLCD-43, uLCD-220RD, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON. */
    genieWriteContrast(15);
  }
  else
    digitalWrite(13, HIGH); // turn on indicator led (display is not connected)

  /* returns the current form the lcd is on */
  if ( genieCurrentForm() == 0 ) {
    // if lcd is on form 0, do something.
  }

  /* This returns true (1) if the lcd is online, only then will this statement be operational. */
  if ( genieOnline() == 1 ) {// if display is online and on form 0, then do something.
    /* Write a string to the Display to show the version of the library used */
    char buf[20];
    String(GENIE_VERSION).toCharArray(buf, 20);
    genieWriteStr(0, buf);
  }

}

void loop() {
  static unsigned long waitPeriod = millis(); // timer to repeat task
  static int coolgaugeVal;
  static boolean gaugeRotation = CW;

  genieDoEvents();

  /* Ping request at an interval chosen by the user.
    Each interval queues an event, which can be retrieved in the handler.
    This can be ran in your loop and will follow its interval */
  //geniePing(1000);

  /* Uptime status, to see how long your display has been online this session. */
  //Serial.println( genieuptime() ); // prints uptime in millisec to the serial monitor (if used)


  if (millis() >= waitPeriod) {
    if (genieOnline()) { // check if the display is online (connected) and if on Form0
      if (gaugeRotation == CW) {
        coolgaugeVal++;
        if ( coolgaugeVal > 100 ) {
          coolgaugeVal = 100;
          gaugeRotation = CCW;
        }
      }
      else if (gaugeRotation == CCW) {
        coolgaugeVal--;
        if ( coolgaugeVal < 0 ) {
          coolgaugeVal = 0;
          gaugeRotation = CW;
        }
      }
    }
    genieWriteObject(GENIE_OBJ_COOL_GAUGE, 0, coolgaugeVal);

    /* The results of this call will be available to myGenieEventHandler() after the display has responded */
    genieReadObject(GENIE_OBJ_USER_LED, 0); // Do a manual read from the UserLEd0 object

    waitPeriod = millis() + 10; /* rerun this code to update Cool Gauge and Slider in another 50ms time. */
  }
}


/////////////////////////////////////////////////////////////////////
//
// This is the user's event handler.

void myGenieEventHandler() {
  genieFrame Event;
  genieDequeueEvent(&Event);
  static int slider_val = 0;

  /* If the commamd received is from a Reported Event, it will be processed here. */
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT) {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER) {                // If the Reported Message was from a Slider
      if (Event.reportObject.index == 0) {                              // If Slider0 (Index = 0)
        slider_val = genieGetEventData(&Event);                        // Receive the event data from the Slider0
        genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val);         // Write Slider0 value to to LED Digits 0
      }
    }
  }

  /* If the commamd received is from a Reported Object, which occurs if a Read Object (genieReadOject) is requested in the main code, reply processed here. */
  else if (Event.reportObject.cmd == GENIE_REPORT_OBJ) {
    if (Event.reportObject.object == GENIE_OBJ_USER_LED) {            // If the Reported Message was from a User LED
      if (Event.reportObject.index == 0) {                            // If UserLed0 (Index = 0)
        bool UserLed0_val = genieGetEventData(&Event);               // Receive the event data from the UserLed0
        UserLed0_val = !UserLed0_val;                                 // Toggle the state of the User LED Variable
        genieWriteObject(GENIE_OBJ_USER_LED, 0, UserLed0_val);       // Write UserLed0_val value back to to UserLed0
      }
    }
  }

  /********** This can be expanded as more objects are added that need to be captured *************
  *************************************************************************************************
    Event.reportObject.cmd is used to determine the command of that event, such as an reported event
    Event.reportObject.object is used to determine the object type, such as a Slider
    Event.reportObject.index is used to determine the index of the object, such as Slider0
    genieGetEventData(&Event) us used to save the data from the Event, into a variable.
  *************************************************************************************************/


  else if (Event.reportObject.cmd == GENIE_PING) {
    if (Event.reportObject.object == GENIE_DISCONNECTED) {
      /* This function runs once, when the LCD is disconnected, because it was turned off or out of sync.
           You may use this to process necessary code. */
      digitalWrite(13, HIGH);
    }

    else if (Event.reportObject.object == GENIE_READY) {
      /* This function runs once, when the LCD is connected and synchronized.
           You may use this to restore screen widgets, or process other code. */
      digitalWrite(13, LOW);
      genieWriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val); // Restore Leddigits0
      genieWriteObject(GENIE_OBJ_SLIDER, 0, slider_val); // Restore Slider0
      static int recover_times = -1; // how many times did the display recover?
      recover_times++;
      char buf[50];
      ((String(GENIE_VERSION) + "\n\n\tRecovered " + recover_times + " Time(s)!")).toCharArray(buf, 50);
      genieWriteStr(0, buf); // Restore text in Strings0
    }

    else if (Event.reportObject.object == GENIE_ACK) {
      /* If a user issues a geniePing(interval) request and it passes,
         this function will happen every 'interval' times chosen by the user. */
      //digitalWrite(13, HIGH); // here we toggle the led on and off after every successful ping interval.
      //delay(20);
      //digitalWrite(13, LOW);
    }

    else if (Event.reportObject.object == GENIE_NAK) {
      /* If a user issues a geniePing(interval) request and it fails,
         this function will happen every 'interval' times chosen by the user. */
    }
  }


}
