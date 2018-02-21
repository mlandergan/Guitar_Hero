/************** ECE2049 Lab 2 ******************/
/**************  9/22/2017   ******************/
/***************************************************/
#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"


// create a data type GAME_STATE for the switch case
enum GAME_STATE {WAITING = 0, BEGIN_GAME = 1, PLAY_GAME = 2, GAME_OVER = 5};
enum note {A = 0, B_flat = 1, B = 2, c = 3, C_sharp = 4, D = 5, E_flat = 6, E = 7, F = 8, F_sharp = 9, G = 10, A_flat = 11};

int LED1[4] = {A, C_sharp, F};
int LED2[4] = {B_flat, D, F_sharp};
int LED3[4] = {B, E_flat, G};
int LED4[4] = {c, E, A_flat};

unsigned int frequencyTicks[13] = {75, 70, 66, 63, 59, 56, 53, 50, 47, 44, 42, 40};

// Function Prototypes
char button_state();
void configure_buttons();
void config_light_led(char inbits);
char checkLED(int); // input is note, output is low nibble for LED
void PlaySong();
void BuzzerOnFrequency(unsigned int);
void runtimerA2(void);
__interrupt void Timer_A2_ISR(void);
void timerDelay(int);
void displayPoints(void);
void win(void);

// Declare globals variables
unsigned char currKey=0;
unsigned int stareTime;
char inbits;
int numberOfNotes = 32;
unsigned int timer = 0, leap_cnt = 0;
int timerDelayStart = 0;
unsigned int points = 10;
int loose = 0;
enum GAME_STATE state = WAITING; // create an instance of GAME_STATE and intiliaze it to WAITING case

//int clicks [24][2]; // notes and duration (seconds)
int song[4][2] = { // test array
    {A, 1},
    {B_flat, 5},
    {G, 2},
    {E, 3}
 };


int song2[32][2] = { // test array
    {E, 1},
    {F_sharp, 1},
    {E, 1},
    {A, 1},
    {E, 1},
    {F_sharp, 1},
    {E, 1},
    {A, 2},
    {F_sharp, 1},
    {E, 1},
    {D, 2},
    {A, 2},
    {F_sharp, 2},
    {F_sharp, 1},
    {G, 1},
    {F_sharp, 2},
    {E, 2},
    {F_sharp, 1},
    {F_sharp, 1},
    {G, 1},
    {F_sharp, 2},
    {E, 1},
    {D, 1},
    {A, 2},
    {A, 4},
    {C_sharp, 2},
    {D, 4},
    {A, 4},
    {C_sharp, 2},
    {D, 4},
    {E, 4},
    {A, 4}
 };


#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void)
{
   if(leap_cnt < 1024)
   {
      timer++;
      leap_cnt++;
   }
   else{
       timer+=2;   // add extra count to catch up
       leap_cnt=0; // every 1024 intervals
   }
}


// Main
void main(void)

{
    _BIS_SR(GIE); // Global interrupt enable
    WDTCTL = WDTPW | WDTHOLD;      // Stop watchdog timer
    runtimerA2(); // configure A2 timer
    initLeds(); // configure LEDS
    configDisplay(); // configure display
    configKeypad(); // configure keypad
    configure_buttons(); // configure buttons

    //Code setup
    Graphics_clearDisplay(&g_sContext); // Clear the display

    while (1)    // Forever loop
    {
        switch (state){
            case WAITING: // display at the begining of the game
                Graphics_drawStringCentered(&g_sContext, "MSP430 HERO", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Press * to begin", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

               currKey = getKey(); // get the current key being pressed

               if (currKey == '*'){  	// if the current key is a * 
                   state = BEGIN_GAME;  // set state to BEGIN_GAME
                  }else {
                   state = WAITING; // if not remain in WAITING state
                  }
                break;
            case BEGIN_GAME:
                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "Press # to exit", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(400);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "3...", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                config_light_led(0x01);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(400);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "2...", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                config_light_led(0x02);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(400);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "1...", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                config_light_led(0x01);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(400);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                config_light_led(0x03);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(800);
                config_light_led(0x00); // turn LED1 and LED0 off

                Graphics_clearDisplay(&g_sContext); // Clear the display
                state = PLAY_GAME;
                break;

            case PLAY_GAME:

                PlaySong();
                // checkNote(); // assigns points, lights RED or GREEN LEDS
                // display points?

                BuzzerOff(); // turn off buzzer
                if(loose == 1){
                    state = GAME_OVER;
                }
                else {
                state = WAITING;
                }
                break;

            case GAME_OVER:
                //print message, LED flash, buzzer sounds
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "You lose!!!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                timerDelay(800);
                Graphics_clearDisplay(&g_sContext);
                setLeds(0x0); // turn off LEDs
                state = WAITING; // restart game, set state to WAITING
                break;
        }

    }  // end while (1)
}

void displayPoints(void)
{
    char displayPoints[11] = "Points: ";
   // displayPoints = points + '0'; AUTO_STRING_LENGTH (char)
    displayPoints[8] = (char) (points/10) + '0';
    displayPoints[9] = (char) (points % 10) + '0';
    displayPoints[10] = 0;
//    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, displayPoints, AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

void timerDelay(int time)
{
  int time_elapsed = 0;
  timerDelayStart = timer; // get the current time before entering delay
  while(time_elapsed < time)
  {
      // do nothing!
      time_elapsed = timer - timerDelayStart; // calculate elapsed time
  }
}


void runtimerA2(void)
{
   // USE ACLK(TASSEL_1), clock divider of 1 (ID_0)
   // and start time counting in UP mode (MC_1)
   TA2CTL = TASSEL_1 + MC_1 + ID_0;
   TA2CCR0 = 81; // ~0.0025 seconds
   TA2CCTL0 = CCIE; // TA2CCr0 interrupt enabled
}

/*
 * Enable a PWM-controlled buzzer on P3.5
 * This function makes use of TimerB0.
 */
void BuzzerOnFrequency(unsigned int ticks)
{
    // Initialize PWM output on P3.5, which corresponds to TB0.5
    P3SEL |= BIT5; // Select peripheral output mode for P3.5
    P3DIR |= BIT5;

    TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
    TB0CTL  &= ~TBIE;                       // Explicitly Disable timer interrupts for safety

    // Now configure the timer period, which controls the PWM period
    // Doing this with a hard coded values is NOT the best method
    // We do it here only as an example. You will fix this in Lab 2.
    TB0CCR0   = ticks;                   // Set the PWM period in ACLK ticks
    TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

    // Configure CC register 5, which is connected to our PWM pin TB0.5
    TB0CCTL5  = OUTMOD_7;                   // Set/reset mode for PWM
    TB0CCTL5 &= ~CCIE;                      // Disable capture/compare interrupts
    TB0CCR5   = TB0CCR0/2;                  // Configure a 50% duty cycle
}


char checkLED(int note){
    unsigned int i;
    for(i=0; i < 4; i++){
        if(note == LED1[i]){
            return 0x08;
        }
        if(note == LED2[i]){
            return 0x04;
        }
        if(note == LED3[i]){
            return 0x02;
        }
        if(note == LED4[i]){
            return 0x01;
        }
    }
    return 0x00;
}

// lights corresponding LED associated with current note
void PlaySong(){
    int k, currNote, time_elapsed = 0, timeAtStart = 0, duration = 0, pointAchieved = 0, negPointStreak = 0;
    unsigned char LED, buttonState;
    for(k=0; k < numberOfNotes; k++){
        pointAchieved = 0;
        timeAtStart = timer; // get the current time before the note plays
        duration = (song2[k][1])/0.0025; // convert duration so its in the same units as the clock (0.0025 seconds)
        while(time_elapsed < duration){ // keep playing the note for the length of its duration
            currKey = getKey(); // get the current key being pressed
               if (currKey == '#'){     // if the current key is a #
                   state = WAITING;  // restart the game
                   return;
               }
            currNote = song2[k][0];
            LED = checkLED(currNote); // display LED associated with current note
            BuzzerOnFrequency(frequencyTicks[currNote]); // get ticks associated with the current note and play buzzer
            setLeds(LED);
            timerDelay(200);
            if(pointAchieved == 0) // only get one point per note
            {
               if(negPointStreak == 8) // if they miss the notes 8 times in a row, game is over
               {
                 loose = 1;
                 state = GAME_OVER;
                 return;
               }
               else{
                 buttonState = button_state();
                 if(buttonState == LED){
                     points++; // increase points by 1
                     config_light_led(0x01); // activate green LED
                     pointAchieved = 1;
                     negPointStreak = 0;
                 }
                 else{
                     points--; // decrease points by 1
                     config_light_led(0x02); // activate red LED
                     negPointStreak++;
                     pointAchieved = 1;
                 }
               }
               displayPoints();
            }


            time_elapsed = timer - timeAtStart;
       }
      time_elapsed = 0;
    }
    win();
}

void win(void){
    loose = 0;
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "You win!!!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    BuzzerOnFrequency(59);
    Graphics_flushBuffer(&g_sContext);
    timerDelay(800);
    Graphics_clearDisplay(&g_sContext);
    BuzzerOff(); // turn off buzzer
}

/*
configure and light the 2 user LEDs on the MSP430F5529
If BIT0 of the argument = 1, LED1 is lit if BIT0=0 then LED1 is off.
if BIT1 of the argument = 1, LED2 is lit and if BIT1=0 then LED2 is off.
*/

void config_light_led(char inbits){
  //configure for I/o
  P4SEL = P4SEL &~ BIT7;
  P1SEL = P1SEL &~ BIT0;

  //configure for output
  P4DIR = P4DIR | BIT7;
  P1DIR = P1DIR | BIT0;

  //initialize to the LED being off (by setting out to 0)
  P4OUT = P4OUT &~ BIT7;
  P1OUT = P1OUT &~ BIT0;

  //define 2 output holders
  unsigned char outbits47 = 0;
  unsigned char outbits10 = 0;

  if (inbits & BIT0){   //if BIT0 of inbits is a 1
      outbits47 |= BIT0; //fill outbits BIT0 with a 1
  }

  if (inbits & BIT1){   //if BIT1 of inbits is a 1
      outbits10 |= BIT1; //fill outbits BIT1 with a 1
  }

  //shift bits so BIT0 is BIT7
  outbits47 = outbits47 << 7; // xxxxxxx0

  //shift bits so BIT1 is BIT0
  outbits10 = outbits10 >> 1; // 0x

  //write to output
  P4OUT = P4OUT | (outbits47 & BIT7); //P4.7 is BIT0 of inbits
  P1OUT = P1OUT | (outbits10 & BIT0); //P1.0 is BIT1 of inbits
}


void configure_buttons(){
  //P7.0, P3.6, P2.2, P7.4 are the buttons
  P7SEL = P7SEL &~ (BIT0 | BIT4); //DIGITAL I/O P7.0 AND 7.4
  P3SEL = P3SEL &~ (BIT6); // DIGITAL I/O P3.6
  P2SEL = P2SEL &~ (BIT2); //DIGITAL I/O P2.2

  //SET ALL TO INPUTS
  P7DIR = P7DIR &~ (BIT0 | BIT4);
  P3DIR = P3DIR &~ (BIT6);
  P2DIR = P2DIR &~ (BIT2);

  //ENABLE PULLUP/DOWN RESISTORS
  P7REN = P7REN | (BIT0 | BIT4);
  P3REN = P3REN | (BIT6);
  P2REN = P2REN | (BIT2);

  //PULL UP RESISTORS
  P7OUT = P7OUT | (BIT0 | BIT4);
  P3OUT = P3OUT | (BIT6);
  P2OUT = P2OUT | (BIT2);
}

/*returns the state of the lab board buttons with 1=pressed LOGIC 0 and 0=not pressed LOGIC 1.
S_1 is P7.0, S_2 is P3.6, S_3 is P2.2, S_4 is P7.4
*/

char button_state(){
  //define char for each button and for all together
  char button1_pressed;
  char button2_pressed;
  char button3_pressed;
  char button4_pressed;
  char buttons_pressed;

  //just in case clear all of the bits in the button#_pressed chars
  button1_pressed = button1_pressed &~ (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  button2_pressed = button2_pressed &~ (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  button3_pressed = button3_pressed &~ (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  button4_pressed = button4_pressed &~ (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  buttons_pressed = buttons_pressed &~ (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

  //SET button1_pressed BIT 0 to 7.0
  button1_pressed = ~P7IN & BIT0; //in bit 0 position
  button1_pressed = button1_pressed << 3; //moved to bit 3 position


  //SET button2_pressed BIT 1 to P3.6
  button2_pressed = ~P3IN & BIT6; //x1xxxxxx
  button2_pressed = button2_pressed >> 4; //moved to bit 2 position

  //SET button3_pressed BIT 2 to P2.2
  button3_pressed = ~P2IN & BIT2; //in but 2 position
  button3_pressed = button3_pressed >> 1; //moved to bit 1 position

  //SET button4_pressed BIT 3 to P7.4
  button4_pressed = ~P7IN & BIT4;
  button4_pressed = button4_pressed >> 4; //moved to the bit 0 position

  //or the button#_pressed together to get the result
  buttons_pressed = button1_pressed | button2_pressed | button3_pressed | button4_pressed; //combine the statements
  return buttons_pressed;
}



