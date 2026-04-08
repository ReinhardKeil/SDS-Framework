// SDSIO states

#define SDSIO_INACTIVE    0     // Streaming is not active and not connected to SDSIO module
#define SDSIO_CONNECTED   1     // Streaming is connected to SDSIO module
#define SDSIO_START       2     // Request to start streaming, open streams and get ready for read/write operations
#define SDSIO_ACTIVE      3     // Streaming is active
#define SDSIO_STOP_REQ    4     // Request to stop streaming and close streams
#define SDSIO_STOP_DONE   5     // Streaming is stopped
#define SDSIO_END         6     // Request to end streaming (no more data)



sdsStatusLED () {
  static uint8_t ticks = 0U;
// User code for a status LED:
// - When disconnected           1s on, 100ms off
// - During streaming off:       1s on,    1s off
// - During streaming active: 100ms on, 100ms off

  if (ticks == 0U) {
    if (vioGetSignal (vioLED0)) {
      vioSetSignal (vioLED0, vioLEDoff);
      if (sdsioState == SDSIO_INACTIVE) ticks = 10U;  // 1s LED off when streaming off
      else                              ticks = 1U; 
    }
    else  {
      vioSetSignal (vioLED0, vioLEDon);
      if (sdsioState == SDSIO_START)    ticks = 1U;  // 100ms LED on when streaming on
      else                              ticks = 10U; 

    }    
  }  
  ticks--;
}

sdsControlThread ()  {
  sdsioInit ();                                      // Initialize SDSIO module (outputs info to STDIO)

  // Create algorithm thread

  for (;;) {                                         // Executed every 100ms
    sdsStatusLED ();
    sdsioQuery ();                                   // Connect to SDSIO and query sdsFlags

// Add user code to control sdsFlags


// End user code

    switch (sdsioState) {             
      case SDSIO_INACTIVE:
        if ((sdsFlags & sdsFlagStart)) {             // Request to start streaming
          if (OpenStreams() == 0) {
            sdsioState = SDSIO_START;
          }
        }
        break;

      case SDSIO_ACTIVE:
        if (!(sdsFlags & sdsFlagStart))  {           // Request to stop streaming
           sdsioState = SDSIO_STOP;
        }
        break;

      case SDSIO_STOP:
        if (CloseStreams() == 0) {
          sdsioState = SDSIO_INACTIVE;
        }
        break;
    }

    interval_time += 100U;
    osDelayUntil(interval_time);
  }
}


// This function is part of sdsio layer

sdsioQuery () {
// for USB it 'pings' the server until a reply is received
// for Ethernet it is assumed that the ping is checked in sdsioInit ();
}