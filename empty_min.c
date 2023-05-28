/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>
/* std header files */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Clock.h>
/* TI-RTOS Header files */
// #include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>
/* Board Header file */
#include "Board.h"
/* Tiva C series macros header files */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
/* Tiva C series driverlib header files */
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "semaphore.h"
/* grlib header files */
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/slider.h"
#include "grlib/pushbutton.h"
/* LCD drivers header files */
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"
// Utility
#include "utils/images.h"
#include "utils/ustdlib.h"

#define TASKSTACKSIZE   512

// Tasks & Clock
Task_Struct task0Struct, task1Struct;
Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE];
Clock_Struct clk1Struct;
Clock_Handle clk1Handle;
// panel ensemble
uint32_t g_ui32Panel;
// blocks double start/stop button
int StartingBlock = 0;
// screen
tContext sContext;
tRectangle sRect;

                    /* put in global*/
// limits
volatile int maxSpeed;
volatile int minSpeed;
int maxPower;
int minPower;
int maxSens1;
int minSens1;
int maxSens2;
int minSens2;

// check start/stop
volatile bool buttonPressed = 0;
                    /* put in global*/


void Testing(UArg arg0, UArg arg1);
void clk1Fxn(UArg arg0);
void gpioButtonFxn0(void);
void initThread(void);
void initLCD(void);
void Screen(UArg arg0, UArg arg1);

//void OnPrevious(tWidget *psWidget);
//void OnNext(tWidget *psWidget);
void OnControl(tWidget *psWidget);
void OnOption(tWidget *psWidget);
void OnMain(tWidget *psWidget);

void mainPanel(tWidget *psWidget, tContext *psContext);
void optionPanel(tWidget *psWidget, int32_t i32Value);
void controlPanel (tWidget *psWidget, int32_t i32Value);
extern tCanvasWidget g_psPanels[];

// Main panel
Canvas(g_smainPanel, g_psPanels + 1, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, mainPanel);

// Option panel
Canvas(g_sOptionSlider5, g_psPanels + 2, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 30, 320, 20,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20,
       "MIN              MAX", 0, 0);
Canvas(g_sOptionSlider4, g_psPanels + 2, &g_sOptionSlider5, 0,
       &g_sKentec320x240x16_SSD2119, 135, 60, 50, 20,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20, "Speed", 0, 0);
Canvas(g_sOptionSlider3, g_psPanels + 2, &g_sOptionSlider4, 0,
       &g_sKentec320x240x16_SSD2119, 135, 95, 50, 20,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20, "Power", 0, 0);
Canvas(g_sOptionSlider2, g_psPanels + 2, &g_sOptionSlider3, 0,
       &g_sKentec320x240x16_SSD2119, 135, 130, 50, 20,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20, "Sens1", 0, 0);
Canvas(g_sOptionSlider, g_psPanels + 2, &g_sOptionSlider2, 0,
       &g_sKentec320x240x16_SSD2119, 135, 165, 50, 20,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20, "Sens2", 0, 0);

tSliderWidget g_psSlidersOptions[] =
{
    // Speed
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 1, 0,
            &g_sKentec320x240x16_SSD2119, 5, 55, 125, 30, 0, 100, 25,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrLimeGreen, ClrBlack, ClrLimeGreen, ClrWhite, ClrWhite,
            &g_sFontCm20, "25%", 0, 0, optionPanel),
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 2, 0,
            &g_sKentec320x240x16_SSD2119, 190, 55, 125, 30, 0, 100, 50,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrLimeGreen, ClrBlack, ClrLimeGreen, ClrWhite, ClrWhite,
            &g_sFontCm20, "50%", 0, 0, optionPanel),

    // Power
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 3, 0,
            &g_sKentec320x240x16_SSD2119, 5, 90, 125, 30, 0, 100, 25,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrBlue, ClrBlack, ClrBlue, ClrWhite, ClrWhite,
            &g_sFontCm20, "25%", 0, 0, optionPanel),
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 4, 0,
            &g_sKentec320x240x16_SSD2119, 190, 90, 125, 30, 0, 100, 50,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrBlue, ClrBlack, ClrBlue, ClrWhite, ClrWhite,
            &g_sFontCm20, "50%", 0, 0, optionPanel),

      // Sens1
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 5, 0,
            &g_sKentec320x240x16_SSD2119, 5, 125, 125, 30, 0, 100, 25,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrGold, ClrBlack, ClrGold, ClrWhite, ClrWhite,
            &g_sFontCm20, "25%", 0, 0, optionPanel),
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 6, 0,
            &g_sKentec320x240x16_SSD2119, 190, 125, 125, 30, 0, 100, 50,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrGold, ClrBlack, ClrGold, ClrWhite, ClrWhite,
            &g_sFontCm20, "50%", 0, 0, optionPanel),

        // Sens2
    SliderStruct(g_psPanels + 2, g_psSlidersOptions + 7, 0,
            &g_sKentec320x240x16_SSD2119, 5, 160, 125, 30, 0, 100, 25,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrRed, ClrBlack, ClrRed, ClrWhite, ClrWhite,
            &g_sFontCm20, "25%", 0, 0, optionPanel),
    SliderStruct(g_psPanels + 2, &g_sOptionSlider, 0,
            &g_sKentec320x240x16_SSD2119, 190, 160, 125, 30, 0, 100, 50,
            (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
            SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
            ClrRed, ClrBlack, ClrRed, ClrWhite, ClrWhite,
            &g_sFontCm20, "50%", 0, 0, optionPanel),
};

#define SPEED_LEFT_INDEX   0
#define SPEED_RIGHT_INDEX  1
#define POWER_LEFT_INDEX   2
#define POWER_RIGHT_INDEX  3
#define SENS1_LEFT_INDEX   4
#define SENS1_RIGHT_INDEX  5
#define SENS2_LEFT_INDEX   6
#define SENS2_RIGHT_INDEX  7
#define NUM_SLIDERS (sizeof(g_psSlidersOptions) / sizeof(g_psSlidersOptions[0]))

// Control Panel

Canvas(g_sControlPanel, g_psPanels, 0, 0, &g_sKentec320x240x16_SSD2119,
       245, 35, 60, 20, CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE,
       ClrBlack, 0, ClrWhite, &g_sFontCm20, "315rpm", 0, 0);

tSliderWidget g_psSlidersControl[] =
{
    SliderStruct(g_psPanels, &g_sControlPanel, 0,
         &g_sKentec320x240x16_SSD2119, 5, 30, 230, 30, 0, 1000, 315,
         (SL_STYLE_FILL | SL_STYLE_TEXT | SL_STYLE_OUTLINE |
          SL_STYLE_BACKG_TEXT | SL_STYLE_TEXT_OPAQUE | SL_STYLE_BACKG_TEXT_OPAQUE),
         ClrLimeGreen, ClrBlack, ClrLimeGreen, ClrWhite, ClrWhite,
         &g_sFontCm20, "Motor Speed", 0, 0, controlPanel),
};
#define SPEED_SET_INDEX 0

// Array of black pannels (filled by &*)
tCanvasWidget g_psPanels[] =
{
    CanvasStruct(0, 0, &g_psSlidersControl, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &g_smainPanel, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, g_psSlidersOptions, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),

};
#define NUM_PANELS (sizeof(g_psPanels) / sizeof(g_psPanels[0]))

RectangularButton(g_sOption, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 10, 195,
                  80, 40, PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT | PB_STYLE_AUTO_REPEAT,
                  ClrSilver, 0, 0, ClrWhite, &g_sFontCm20, "Option", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, OnOption);
RectangularButton(g_sControl, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 230, 195,
                  80, 40, PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT | PB_STYLE_AUTO_REPEAT,
                  ClrSilver, 0, 0, ClrWhite, &g_sFontCm20, "Control", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, OnControl);
RectangularButton(g_sMain, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 120, 195,
                  80, 40, PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT | PB_STYLE_AUTO_REPEAT,
                  ClrSilver, 0, 0, ClrWhite, &g_sFontCm20, "Main", 0,
                  0, 0, 0, OnMain);

//  ======== main ========
void main(void)
{

    initThread();
    initLCD();
    BIOS_start();
}
//  ======== main ========

void initLCD(void) {
    Types_FreqHz cpuFreq;
    BIOS_getCpuFreq(&cpuFreq);

    // Initialize display driver and graphics
    Kentec320x240x16_SSD2119Init((uint32_t)cpuFreq.lo);
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "EGH 456 - Motor Control", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    //TouchScreenInit((uint32_t)cpuFreq.lo);
    //TouchScreenCallbackSet(WidgetPointerMessage);
    System_printf("Touch Initialized\n");
    System_flush();

    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sOption);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sControl);
    //WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sMain);

    g_ui32Panel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanels);
    WidgetPaint(WIDGET_ROOT);
}

// Initialize
void initThread(void){

    Task_Params taskParams;
    Clock_Params clkParams;

    Board_initGeneral();
    Board_initGPIO();
    // Board_initEMAC();
    // Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    // Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initUSBMSCHFatFs();
    // Board_initWatchdog();
    // Board_initWiFi();

    // Session time
    Clock_Params_init(&clkParams);
    clkParams.period = 1000;
    clkParams.startFlag = FALSE;

    //Screen interrupts
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    taskParams.priority = 1;
    Task_construct(&task1Struct, (Task_FuncPtr)Screen, &taskParams, NULL);

    // Beat interrupt
    taskParams.stack = &task0Stack;
    taskParams.priority = 2;
    Task_construct(&task0Struct, (Task_FuncPtr)Testing, &taskParams, NULL);

    // Start/Stop
    GPIO_setCallback(Board_BUTTON0, (GPIO_CallbackFxn)gpioButtonFxn0);
    GPIO_enableInt(Board_BUTTON0);

    //Clock
    Clock_construct(&clk1Struct, (Clock_FuncPtr)clk1Fxn, 1000, &clkParams);
    clk1Handle = Clock_handle(&clk1Struct);
    Clock_start(clk1Handle);
}

// Screen handler
void Screen(UArg arg0, UArg arg1)
{
    while(1)
        {
            //here will be all the graphs
            /*
             *
             *
             *
             *
             */

            // Process any messages in the widget message queue.
            WidgetMessageQueueProcess();
            Task_sleep(10);
        }
}

// Clock for session time
void clk1Fxn(UArg arg0)
{
    UInt32 time;
    time = Clock_getTicks();
    System_printf("System time in clk1Fxn = %lu\n", (ULong)time);
    if(buttonPressed == 0){
        StartingBlock = 0;
    }
    else if(buttonPressed == 1 && StartingBlock < 3){
        StartingBlock++;
    }
}

// SW1 button interrupt
void gpioButtonFxn0(void)
{
    if (buttonPressed == 0){
        buttonPressed = 1;
        GPIO_write(Board_LED1, Board_LED_ON);
    }

    else if (StartingBlock >= 2){
        GPIO_write(Board_LED1, Board_LED_OFF);
        buttonPressed = 0;
    }

}

// LED toggle
void Testing(UArg arg0, UArg arg1)
{
    while (1) {
        Task_sleep((unsigned int)arg0);
        GPIO_toggle(Board_LED0);
        if (buttonPressed == 1)
        {
            System_printf("Motor running\n");
        }
    }
}

void controlPanel (tWidget *psWidget, int32_t i32Value)
{
    static char pcCanvasText[5];

    if(psWidget == (tWidget *)&g_psSlidersControl[SPEED_SET_INDEX])
    {
        // semaphore_pend
        // send speed value to motor
        // semaphore_post
        sprintf(pcCanvasText, "%3drpm", i32Value);
        CanvasTextSet(&g_sControlPanel, pcCanvasText);
        WidgetPaint((tWidget *)&g_sControlPanel);
    }
}

void mainPanel(tWidget *psWidget, tContext *psContext)
{
    GrContextFontSet(psContext, &g_sFontCm20);
    GrContextForegroundSet(psContext, ClrWhite);
    GrStringDraw(psContext, "Press SW1 to start the Motor", -1,
                 0, 32, 0);
    GrStringDraw(psContext, "There are 2 panels:", -1, 0, 62, 0);
    GrStringDraw(psContext, "The Control Panel to adjust speed", -1, 0,
                 87, 0);
    GrStringDraw(psContext, "and view the sensors output.", -1, 0,
                 107, 0);
    GrStringDraw(psContext, "The Options Panel to adjust limits.", -1, 0,
                 132, 0);
}

void optionPanel(tWidget *psWidget, int32_t i32Value)
{
    static char pcSliderText[5];

    //SPEED_LEFT_INDEX
    if(psWidget == (tWidget *)&g_psSlidersOptions[SPEED_LEFT_INDEX])
    {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t minSpeed = i32Value;
        SliderTextSet(&g_psSlidersOptions[SPEED_LEFT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SPEED_LEFT_INDEX]);
    }
    //SPEED_RIGHT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[SPEED_RIGHT_INDEX])
        {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t maxSpeed = i32Value;
        SliderTextSet(&g_psSlidersOptions[SPEED_RIGHT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SPEED_RIGHT_INDEX]);
    }
    //POWER_LEFT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[POWER_LEFT_INDEX])
    {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t minPower = i32Value;
        SliderTextSet(&g_psSlidersOptions[POWER_LEFT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[POWER_LEFT_INDEX]);
    }
    //POWER_RIGHT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[POWER_RIGHT_INDEX])
        {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t maxPower = i32Value;
        SliderTextSet(&g_psSlidersOptions[POWER_RIGHT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[POWER_RIGHT_INDEX]);
    }
    //SENS1_LEFT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[SENS1_LEFT_INDEX])
    {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t minSens1 = i32Value;
        SliderTextSet(&g_psSlidersOptions[SENS1_LEFT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SENS1_LEFT_INDEX]);
    }
    //SENS1_RIGHT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[SENS1_RIGHT_INDEX])
        {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t maxSens1 = i32Value;
        SliderTextSet(&g_psSlidersOptions[SENS1_RIGHT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SENS1_RIGHT_INDEX]);
    }
    //SENS2_LEFT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[SENS2_LEFT_INDEX])
    {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t minSens2 = i32Value;
        SliderTextSet(&g_psSlidersOptions[SENS2_LEFT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SENS2_LEFT_INDEX]);
    }
    //SENS2_RIGHT_INDEX
    else if(psWidget == (tWidget *)&g_psSlidersOptions[SENS2_RIGHT_INDEX])
        {
        sprintf(pcSliderText, "%3d%%", i32Value);
        uint32_t maxSens2 = i32Value;
        SliderTextSet(&g_psSlidersOptions[SENS2_RIGHT_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSlidersOptions[SENS2_RIGHT_INDEX]);
    }
}

void OnOption(tWidget *psWidget) // Option screen = 1
{
    // if click while on panel, return
    if(g_ui32Panel == 1)
    {return;}

    // else, remove the current panel.
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));

    // set to Option panel
    g_ui32Panel = 1;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    // add buttons to link other pages
    PushButtonImageOff(&g_sOption);
    PushButtonTextOff(&g_sOption);
    PushButtonFillOn(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);

    PushButtonImageOn(&g_sControl);
    PushButtonTextOn(&g_sControl);
    PushButtonFillOff(&g_sControl);
    WidgetPaint((tWidget *)&g_sControl);
    PushButtonImageOn(&g_sMain);
    PushButtonTextOn(&g_sMain);
    PushButtonFillOff(&g_sMain);
    WidgetPaint((tWidget *)&g_sMain);
}

void OnMain(tWidget *psWidget) // main Screen = 0
{
    // if click while on panel, return
    if(g_ui32Panel == 0)
    {return;}

    // else, remove the current panel.
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));

    // set to Main panel
    g_ui32Panel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    // add buttons to link other pages
    PushButtonImageOff(&g_sMain);
    PushButtonTextOff(&g_sMain);
    PushButtonFillOn(&g_sMain);
    WidgetPaint((tWidget *)&g_sMain);

    PushButtonImageOn(&g_sControl);
    PushButtonTextOn(&g_sControl);
    PushButtonFillOff(&g_sControl);
    WidgetPaint((tWidget *)&g_sControl);
    PushButtonImageOn(&g_sOption);
    PushButtonTextOn(&g_sOption);
    PushButtonFillOff(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);
}

void OnControl(tWidget *psWidget) // Option screen = 1
{
    // if click while on panel, return
    if(g_ui32Panel == 2)
    {return;}

    // else, remove the current panel.
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));

    // set to control panel
    g_ui32Panel = 2;
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    // add buttons to link other pages
    PushButtonImageOff(&g_sControl);
    PushButtonTextOff(&g_sControl);
    PushButtonFillOn(&g_sControl);
    WidgetPaint((tWidget *)&g_sControl);

    PushButtonImageOn(&g_sOption);
    PushButtonTextOn(&g_sOption);
    PushButtonFillOff(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);
    PushButtonImageOn(&g_sMain);
    PushButtonTextOn(&g_sMain);
    PushButtonFillOff(&g_sMain);
    WidgetPaint((tWidget *)&g_sMain);
}
