#include "clock.h"

/*----- Global Variable -----*/
extern uint16_t second, minute, hour;

/*----- Thread Clock -----*/
osThreadId_t tid_ThClock;                        // thread id 
void ThClock (void *argument);                   // thread function

/*----- Periodic Timer -----*/
osTimerId_t tim_clock;                            // timer id
void Init_Timer (void);


 /*---------------------------------------------------
	*      								Thread
	*---------------------------------------------------*/

int Init_ThClock (void) {
 
  tid_ThClock = osThreadNew(ThClock, NULL, NULL);
  if (tid_ThClock == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThClock (void *argument) {

	Init_Timer();
	osTimerStart(tim_clock, 1000U); 

  while (1) {
     // Insert thread code here...

		
    osThreadYield();                            // suspend thread
  }
}

 /*---------------------------------------------------
	*      								Timer
	*---------------------------------------------------*/
 
// Periodic Timer Function
static void Timer2_Callback (void const *arg) {
  	second++;
		if(second == 60){
			second = 0;
			minute++;
			if(minute == 60){
				minute = 0;
				hour++;
				if(hour == 24){
					hour = 0;
				}
			}
		}
}
 
// Example: Create and Start timers
void Init_Timer (void) {

	// Create periodic timer
	tim_clock = osTimerNew((osTimerFunc_t)&Timer2_Callback, osTimerPeriodic, NULL, NULL);

}



