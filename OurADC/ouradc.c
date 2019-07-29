//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#include <msp430.h>
#include "main.h"
#include "driverlib.h"
#include "hal_LCD.h"

char ADCState = 0; //Busy state of the ADC
int32_t ADCResult = 0; //Storage for the ADC conversion result
int DUTY_CYCLE = 0;
volatile int Zone_Counter = 1;

void main(void)
{
    WDT_A_hold(WDT_A_BASE);

    char buttonState = 1;

    //Set LED pins
    P1DIR |= BIT7;
    P2DIR |= BIT7;
//    P2DIR |= BIT5;
    P5DIR |= BIT0;
//    P2OUT ^= BIT7;
    __disable_interrupt();

    // Initializations - see functions for more detail
    Init_GPIO();    //Sets all pins to output low as a default
//    Init_Buzzer();  //Sets up a PWM output

    Timer_A_initCompareModeParam initComp2Param = { 0 };
    initComp2Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    initComp2Param.compareInterruptEnable =
    TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initComp2Param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;

    Init_ADC();     //Sets up the ADC to sample
//    Init_Clock();   //Sets up the necessary system clocks
//    Init_UART();    //Sets up an echo over a COM port
    Init_LCD();     //Sets up the LaunchPad LCD display

    PMM_unlockLPM5(); //Disable the GPIO power-on default high-impedance mode to activate previously configured port settings

    __enable_interrupt();

    int a;
    for (a = 0; a <= 50; a++)
    {
        volatile unsigned int i; // volatile to prevent optimization

        P2OUT ^= BIT7;
        i = 10000;                          // SW Delay
        do
            i--;
        while (i != 0);

    }

    int b;
    for (b = 0; b <= 50; b++)
    {
        volatile unsigned int i; // volatile to prevent optimization

        P1OUT ^= BIT7;
        i = 10000;                          // SW Delay
        do
            i--;
        while (i != 0);

    }
//
//    for(;;) {
//         volatile unsigned int i;            // volatile to prevent optimization
//
//         P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
//
//         i = 10000;                          // SW Delay
//         do i--;
//         while(i != 0);
//    }

//    int b;
//    for (b = 0; b <= 50; b++)
//    {
//        volatile unsigned int i; // volatile to prevent optimization
//
//        P1OUT ^= BIT7;
//        i = 10000;                          // SW Delay
//        do
//            i--;
//        while (i != 0);
//
//    }
//
//    int d;
//    for (d = 0; d <= 50; d++)
//    {
//        volatile unsigned int i; // volatile to prevent optimization
//
//        P5OUT ^= BIT0;
//        i = 10000;                          // SW Delay
//        do
//            i--;
//        while (i != 0);
//
//    }
//
//    int c;
//    for (c = 0; c <= 50; c++)
//    {
//        volatile unsigned int i; // volatile to prevent optimization
//
//        P2OUT ^= BIT5;
//        i = 10000;                          // SW Delay
//        do
//            i--;
//        while (i != 0);
//
//    }
//
//    P2DIR |= BIT7;
//
//    int a;
////    P2OUT ^= BIT7;
//    for (a = 0; a <= 50; a++)
//    {
//        volatile unsigned int i; // volatile to prevent optimization
//
//        P2OUT ^= BIT7;
//        i = 10000;                          // SW Delay
//        do
//            i--;
//        while (i != 0);
//
//    }

    while (1) //Do this when you want an infinite loop of code
    {

        //__delay_cycles(5000);

        Init_ADC();

        //By default Zone 1 is on with no buttons pressed
        if (Zone_Counter == 1)
        {
            ADC_configureMemory(ADC_BASE, ADC_INPUT_A9, ADC_VREFPOS_AVCC,
            ADC_VREFNEG_AVSS);
        }

        if ((GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 1)
                & (buttonState == 0)) //Look for falling edge
        {
            buttonState = 1;
            Zone_Counter++;

            if (Zone_Counter == 2)
            {
                ADC_configureMemory(ADC_BASE, ADC_INPUT_A6, ADC_VREFPOS_AVCC,
                ADC_VREFNEG_AVSS);
            }
            if (Zone_Counter == 3)
            {
                ADC_configureMemory(ADC_BASE, ADC_INPUT_A8, ADC_VREFPOS_AVCC,
                ADC_VREFNEG_AVSS);
            }
            if (Zone_Counter == 4)
            {
                ADC_configureMemory(ADC_BASE, ADC_INPUT_A4, ADC_VREFPOS_AVCC,
                ADC_VREFNEG_AVSS);
            }

        }

        if ((GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
                & (buttonState == 1)) //Look for rising edge
        {
            buttonState = 0;
            if (Zone_Counter == 4)
            {
                Zone_Counter = 0; //reset to default Zone 1
            }
        }

        //Start an ADC conversion (if it's not busy) in Single-Channel, Single Conversion Mode
        if (ADCState == 0)
        {
            showHex(ADCResult); //Put the previous result on the LCD display

            ADCState = 1; //Set flag to indicate ADC is busy - ADC ISR (interrupt) will clear it
            ADC_startConversion(ADC_BASE, ADC_SINGLECHANNEL);

            initComp2Param.compareValue = DUTY_CYCLE;
            Timer_A_initCompareMode(TIMER_A0_BASE, &initComp2Param);
        }
    }
}

void Init_GPIO(void)
{
    // Set all GPIO pins to output low to prevent floating input and reduce power consumption
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P1,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P2,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P3,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P4,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P5,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P6,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P7,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P8,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    GPIO_setAsOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P2,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P3,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P4,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P5,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P6,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P7,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P8,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    //Set LaunchPad switches as inputs - they are active low, meaning '1' until pressed
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN);
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN);

    //Set LED1 and LED2 as outputs
    //GPIO_setAsOutputPin(LED1_PORT, LED1_PIN); //Comment if using UART
    GPIO_setAsOutputPin(LED2_PORT, LED2_PIN);
}

/* Clock System Initialization */
void Init_Clock(void)
{
    /*
     * The MSP430 has a number of different on-chip clocks. You can read about it in
     * the section of the Family User Guide regarding the Clock System ('cs.h' in the
     * driverlib).
     */

    /*
     * On the LaunchPad, there is a 32.768 kHz crystal oscillator used as a
     * Real Time Clock (RTC). It is a quartz crystal connected to a circuit that
     * resonates it. Since the frequency is a power of two, you can use the signal
     * to drive a counter, and you know that the bits represent binary fractions
     * of one second. You can then have the RTC module throw an interrupt based
     * on a 'real time'. E.g., you could have your system sleep until every
     * 100 ms when it wakes up and checks the status of a sensor. Or, you could
     * sample the ADC once per second.
     */
    //Set P4.1 and P4.2 as Primary Module Function Input, XT_LF
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
    GPIO_PIN1 + GPIO_PIN2,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    // Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768);
    // Set ACLK = XT1
    CS_initClockSignal(CS_ACLK, CS_XT1CLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Initializes the XT1 crystal oscillator
    CS_turnOnXT1LF(CS_XT1_DRIVE_1);
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);
}

#pragma vector=USCI_A0_VECTOR
__interrupt
void EUSCIA0_ISR(void)
{
    uint8_t RxStatus = EUSCI_A_UART_getInterruptStatus(
            EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE, RxStatus);

    if (RxStatus)
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE,
                                  EUSCI_A_UART_receiveData(EUSCI_A0_BASE));
    }
}

/* Buzzer PWM Initialization */
//void Init_Buzzer(void)
//{
//    /*
//     * The internal timers (TIMER_A) can auto-generate a PWM signal without needing to
//     * flip an output bit every cycle in software. The catch is that it limits which
//     * pins you can use to output the signal, whereas manually flipping an output bit
//     * means it can be on any GPIO. This function populates a data structure that tells
//     * the API to use the timer as a hardware-generated PWM source.
//     *
//     */
//    //Generate PWM - Timer runs in Up-Down mode
//    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
//    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
//    param.timerPeriod = TIMER_A_PERIOD; //Defined in main.h
//    param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
//    param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
//    param.dutyCycle = HIGH_COUNT; //Defined in main.h
//
//    //BZ1 (defined in main.h) as PWM output
//    GPIO_setAsPeripheralModuleFunctionOutputPin(BZ1_PORT, BZ1_PIN,
//                                                GPIO_PRIMARY_MODULE_FUNCTION);
//}
void Init_ADC(void)
{
    /*
     * To use the ADC, you need to tell a physical pin to be an analog input instead
     * of a GPIO, then you need to tell the ADC to use that analog input. Defined
     * these in main.h for A9 on P8.1.
     */
    // SET UP THE OTHER PORTS FOR ADC
    //Set ADC_IN to input direction
    //Zone 1 takes P8.1 ADC input
    if (Zone_Counter == 1)
    {
        GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P8, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    }
    //Zone 2 takes P1.6 ADC input
    if (Zone_Counter == 2)
    {
        GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P1, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    }
    //Zone 3 takes P8.0 ADC input
    if (Zone_Counter == 3)
    {
        GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P8, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
    }
    //Zone 4 takes P1.4 ADC input
    if (Zone_Counter == 4)
    {
        GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P1, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    }

    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use internal ADC bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC_init(ADC_BASE, ADC_SAMPLEHOLDSOURCE_SC, ADC_CLOCKSOURCE_ADCOSC,
    ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    /*
     * Base Address for the ADC Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC_setupSamplingTimer(ADC_BASE, ADC_CYCLEHOLD_16_CYCLES,
    ADC_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input ADC_IN_CHANNEL
     * Use positive reference of AVcc
     * Use negative reference of AVss
     */

    ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
}

//ADC interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt
void ADC_ISR(void)
{
    uint8_t ADCStatus = ADC_getInterruptStatus(ADC_BASE,
    ADC_COMPLETED_INTERRUPT_FLAG);

    ADC_clearInterrupt(ADC_BASE, ADCStatus);
    // check which channel is activating the interrupt
    if (ADCStatus)
    {
        ADCState = 0; //Not busy anymore
        ADCResult = ADC_getResults(ADC_BASE);
        if (Zone_Counter == 1)
        {

            if (ADCResult > 900)
            {
                int a;
                P2OUT ^= BIT7;
                for (a = 0; a <= 50; a++)
                {
                    volatile unsigned int i; // volatile to prevent optimization

                    P2OUT ^= BIT7;
                    i = 10000;                          // SW Delay
                    do
                        i--;
                    while (i != 0);

                }

                P2OUT &= ~BIT7;
            }
        }

        if (Zone_Counter == 2)
        {

            if (ADCResult > 970)
            {
                int b;
                for (b = 0; b <= 50; b++)
                {
                    volatile unsigned int i; // volatile to prevent optimization

                    P1OUT ^= BIT7;
                    i = 10000;                          // SW Delay
                    do
                        i--;
                    while (i != 0);

                }

                P1OUT &= ~BIT7;
            }
        }

        if (Zone_Counter == 3)
        {

            if (ADCResult > 970)
            {
                int c;
                for (c = 0; c <= 50; c++)
                {
                    volatile unsigned int i; // volatile to prevent optimization

                    P2OUT ^= BIT5;
                    i = 10000;                          // SW Delay
                    do
                        i--;
                    while (i != 0);

                }

                P2OUT &= ~BIT5;
            }
        }

        if (Zone_Counter == 4)
        {

            if (ADCResult > 970)
            {
                int d;
                for (d = 0; d <= 50; d++)
                {
                    volatile unsigned int i; // volatile to prevent optimization

                    P5OUT ^= BIT0;
                    i = 10000;                          // SW Delay
                    do
                        i--;
                    while (i != 0);

                }

                P5OUT &= ~BIT0;
            }

            else
                DUTY_CYCLE = 100;

            printf("%i", ADCResult);
        }
    }
}
