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
#include <ti/sysbios/gates/GateHwi.h>
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
#include "driverlib/fpu.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
//#include "semaphore.h"

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
#include "images.h"
#include "utils/ustdlib.h"


#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable psDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(psDMAControlTable, 1024)
tDMAControlTable psDMAControlTable[64];
#else
tDMAControlTable psDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif

#define TASKSTACKSIZE   512

Task_Struct task0Struct, task1Struct;
Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE];
Clock_Struct clk1Struct;
Clock_Handle clk1Handle;
GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;
Semaphore_Params semParams;
Semaphore_Handle semaphoreHandle;
Swi_Handle SwiHandle;
Hwi_Handle buttonHwiHandle;
Hwi_Handle timerHwiHandle;

uint32_t g_ui32SysClock;
uint32_t g_ui32Panel;
int buttonPressed = 0;
int StartingBlock = 0;
tContext sContext;
tRectangle sRect;

void Testing(UArg arg0, UArg arg1);
void clk1Fxn(UArg arg0);
void gpioButtonFxn0(void);
void initThread(void);
void initLCD(void);
void Screen(UArg arg0, UArg arg1);

void OnControlPanel(tWidget *psWidget);
void OnOptionPanel(tWidget *psWidget);
void OnMainPanel(tWidget *psWidget);

void IntroWid(tWidget *psWidget, tContext *psContext);
void OnOption(tWidget *psWidget, int32_t i32Value);
extern tCanvasWidget g_psPanels[];

//*****************************************************************************
// The Main panel. Contains introductory text, Time, Motor speed and sensor outputs
//*****************************************************************************
Canvas(g_sIntroWid, g_psPanels, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 24,
       320, 166, CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, IntroWid);


//*****************************************************************************
// Control panel. Set the limits here
//*****************************************************************************
Canvas(g_sSliderValueCanvas, g_psPanels + 1, 0, 0,
       &g_sKentec320x240x16_SSD2119, 210, 30, 60, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrSilver,
       &g_sFontCm24, "50%",
       0, 0);

tSliderWidget g_psSliders[] =
{
    SliderStruct(g_psPanels + 1, g_psSliders + 1, 0,
                 &g_sKentec320x240x16_SSD2119, 5, 115, 220, 30, 0, 100, 25,
                 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
                  SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
                 ClrGray, ClrBlack, ClrSilver, ClrWhite, ClrWhite,
                 &g_sFontCm20, "25%", 0, 0, OnOption),
    SliderStruct(g_psPanels + 1, g_psSliders + 2, 0,
                 &g_sKentec320x240x16_SSD2119, 5, 155, 220, 25, 0, 100, 25,
                 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
                  SL_STYLE_TEXT),
                 ClrWhite, ClrBlueViolet, ClrSilver, ClrBlack, 0,
                 &g_sFontCm18, "Foreground Text Only", 0, 0, OnOption),
    SliderStruct(g_psPanels + 1, g_psSliders + 3, 0,
                 &g_sKentec320x240x16_SSD2119, 240, 70, 26, 110, 0, 100, 50,
                 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_VERTICAL |
                  SL_STYLE_OUTLINE | SL_STYLE_LOCKED), ClrDarkGreen,
                  ClrDarkRed, ClrSilver, 0, 0, 0, 0, 0, 0, 0),
    SliderStruct(g_psPanels + 1, g_psSliders + 4, 0,
                 &g_sKentec320x240x16_SSD2119, 280, 30, 30, 150, 0, 100, 75,
                 (SL_STYLE_IMG | SL_STYLE_BACKG_IMG | SL_STYLE_VERTICAL |
                 SL_STYLE_OUTLINE), 0, ClrBlack, ClrSilver, 0, 0, 0,
                 0, g_pui8GettingHotter28x148, g_pui8GettingHotter28x148Mono,
                 OnOption),
    SliderStruct(g_psPanels + 1, g_psSliders + 5, 0,
                 &g_sKentec320x240x16_SSD2119, 5, 30, 195, 37, 0, 100, 50,
                 SL_STYLE_IMG | SL_STYLE_BACKG_IMG, 0, 0, 0, 0, 0, 0,
                 0, g_pui8GreenSlider195x37, g_pui8RedSlider195x37,
                 OnOption),
    SliderStruct(g_psPanels + 1, &g_sSliderValueCanvas, 0,
                 &g_sKentec320x240x16_SSD2119, 5, 80, 220, 25, 0, 100, 50,
                 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_TEXT |
                  SL_STYLE_BACKG_TEXT | SL_STYLE_TEXT_OPAQUE |
                  SL_STYLE_BACKG_TEXT_OPAQUE),
                 ClrBlue, ClrYellow, ClrSilver, ClrYellow, ClrBlue,
                 &g_sFontCm18, "Text in both areas", 0, 0,
                 OnOption),
};

#define SLIDER_TEXT_VAL_INDEX   0
#define SLIDER_LOCKED_INDEX     2
#define SLIDER_CANVAS_VAL_INDEX 4

#define NUM_SLIDERS (sizeof(g_psSliders) / sizeof(g_psSliders[0]))

//*****************************************************************************
//
// An array of canvas widgets, one per panel.  Each canvas is filled with
// black, overwriting the contents of the previous panel.
//
//*****************************************************************************
tCanvasWidget g_psPanels[] =
{
    CanvasStruct(0, 0, &g_sIntroWid, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, g_psSliders, &g_sKentec320x240x16_SSD2119, 0,
                 24, 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
};

//*****************************************************************************
//
// The number of panels.
//
//*****************************************************************************
#define NUM_PANELS              (sizeof(g_psPanels) / sizeof(g_psPanels[0]))

//*****************************************************************************
//
// The names for each of the panels, which is displayed at the bottom of the
// screen.
//
//*****************************************************************************
char *g_pcPanei32Names[] =
{
    "     Instructions     ",
    "     Control     "
};

//*****************************************************************************
//
// The buttons and text across the bottom of the screen.
//
//*****************************************************************************
RectangularButton(g_sControls, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 190,
                  50, 50, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0, ClrSilver,
                  &g_sFontCm20, "Controls", g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0,
                  OnControlPanel);

Canvas(g_sTitle, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 50, 190, 220, 50,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, 0, 0, ClrSilver,
       &g_sFontCm20, 0, 0, 0);

RectangularButton(g_sOption, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 270, 190,
                  50, 50, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0,
                  ClrSilver, &g_sFontCm20, "Options", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, OnOptionPanel);

RectangularButton(g_sMainL, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 190,
                  50, 50, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0, ClrSilver,
                  &g_sFontCm20, "MainL", g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0,
                  OnMainPanel);

RectangularButton(g_sMainR, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 270, 190,
                  50, 50, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0,
                  ClrSilver, &g_sFontCm20, "MainR", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, OnMainPanel);



//  ======== main ========
void main(void)
{

    initThread();
    initLCD();
    BIOS_start();
}


void initLCD(void) {

    FPUEnable();
    FPULazyStackingEnable();

    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    // Initialize display driver and graphics
    Kentec320x240x16_SSD2119Init(g_ui32SysClock);
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

    //
    // Fill the top 24 rows of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "EGH 456 - Motor Control", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    // data transfer init
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&psDMAControlTable[0]);
    uDMAEnable();

    //TouchScreenInit(g_ui32SysClock);

    //
    // Initialize the touch screen driver and have it route its messages to the
    // widget tree.
    //

    //TouchScreenCallbackSet(WidgetPointerMessage);

    //
    // Add the title block and the previous and next buttons to the widget
    // tree.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sControls);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sOption);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sMainL);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sMainR);

    //
    // Add the first panel to the widget tree.
    //
    g_ui32Panel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanels);
    CanvasTextSet(&g_sTitle, g_pcPanei32Names[0]);

    //
    // Issue the initial paint request to the widgets.
    //
    WidgetPaint(WIDGET_ROOT);

    //
    // Loop forever handling widget messages.
    //
}

// Initialize
void initThread(void){
    /* Call board init functions */
    Board_initGeneral();
    // Board_initEMAC();
    Board_initGPIO();
    // Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    // Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initUSBMSCHFatFs();
    // Board_initWatchdog();
    // Board_initWiFi();

    Task_Params taskParams;
    Clock_Params clkParams;

    Clock_Params_init(&clkParams);
    clkParams.period = 1000;
    clkParams.startFlag = FALSE;

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    taskParams.priority = 1;
    Task_construct(&task1Struct, (Task_FuncPtr)Screen, &taskParams, NULL);

    taskParams.stack = &task0Stack;
    taskParams.priority = 2;
    Task_construct(&task0Struct, (Task_FuncPtr)Testing, &taskParams, NULL);

    // install callback and set interrupts
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
            // Process any messages in the widget message queue.
            WidgetMessageQueueProcess();
        }

}

// Clock for session time
void clk1Fxn(UArg arg0)
{
    UInt32 time;
    time = Clock_getTicks();
    System_printf("System time in clk1Fxn = %lu\n", (ULong)time);
    if(buttonPressed == 1 && StartingBlock < 3){
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
        BIOS_exit(0);
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

void
OnControlPanel(tWidget *psWidget)
{
    //
    // There is nothing to be done if the first panel is already being
    // displayed.
    //
    if(g_ui32Panel == 1)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));

    //
    // Decrement the panel index.
    //
    g_ui32Panel = 1;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitle, g_pcPanei32Names[g_ui32Panel]);
    WidgetPaint((tWidget *)&g_sTitle);


    PushButtonImageOff(&g_sControls);
    PushButtonTextOff(&g_sControls);
    PushButtonFillOn(&g_sControls);
    WidgetPaint((tWidget *)&g_sControls);
    PushButtonImageOn(&g_sOption);
    PushButtonTextOn(&g_sOption);
    PushButtonFillOff(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);


    PushButtonImageOn(&g_sMainL);
    PushButtonTextOn(&g_sMainL);
    PushButtonFillOff(&g_sMainL);
    WidgetPaint((tWidget *)&g_sMainL);

}

//*****************************************************************************
//
// Handles presses of the next panel button.
//
//*****************************************************************************
void
OnOptionPanel(tWidget *psWidget)
{
    //
    // There is nothing to be done if the last panel is already being
    // displayed.
    //
    if(g_ui32Panel == 2)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));

    //
    // Increment the panel index.
    //
    g_ui32Panel = 2;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitle, g_pcPanei32Names[g_ui32Panel]);
    WidgetPaint((tWidget *)&g_sTitle);

    //
    // See if the previous panel was the first panel.
    //

    PushButtonImageOn(&g_sMainR);
    PushButtonTextOn(&g_sMainR);
    PushButtonFillOff(&g_sMainR);
    WidgetPaint((tWidget *)&g_sMainR);

    //
    // Clear the next button from the display since the last panel is being
    // displayed.
    //
    PushButtonImageOff(&g_sOption);
    PushButtonTextOff(&g_sOption);
    PushButtonFillOn(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);
    PushButtonImageOn(&g_sControls);
    PushButtonTextOn(&g_sControls);
    PushButtonFillOff(&g_sControls);
    WidgetPaint((tWidget *)&g_sControls);

}

void OnMainPanel(tWidget *psWidget) {

    if(g_ui32Panel == 0)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanels + g_ui32Panel));
    uint32_t temp = g_ui32Panel;
    //
    // Decrement the panel index.
    //
    g_ui32Panel = 0;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ui32Panel));
    WidgetPaint((tWidget *)(g_psPanels + g_ui32Panel));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitle, g_pcPanei32Names[g_ui32Panel]);
    WidgetPaint((tWidget *)&g_sTitle);

    if (temp == 1)
    {
        PushButtonImageOff(&g_sMainR);
        PushButtonTextOff(&g_sMainR);
        PushButtonFillOn(&g_sMainR);
        WidgetPaint((tWidget *)&g_sMainR);
    }
    if (temp == 2)
    {
        PushButtonImageOff(&g_sMainL);
        PushButtonTextOff(&g_sMainL);
        PushButtonFillOn(&g_sMainL);
        WidgetPaint((tWidget *)&g_sMainL);
    }

    PushButtonImageOn(&g_sOption);
    PushButtonTextOn(&g_sOption);
    PushButtonFillOff(&g_sOption);
    WidgetPaint((tWidget *)&g_sOption);
    PushButtonImageOn(&g_sControls);
    PushButtonTextOn(&g_sControls);
    PushButtonFillOff(&g_sControls);
    WidgetPaint((tWidget *)&g_sControls);

}

//*****************************************************************************
//
// Handles paint requests for the IntroWid canvas widget.
//
//*****************************************************************************
void
IntroWid(tWidget *psWidget, tContext *psContext)
{
    //
    // Display the IntroWid text in the canvas.
    //
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


//*****************************************************************************
//
// Handles notifications from the slider controls.
//
//*****************************************************************************
void
OnOption(tWidget *psWidget, int32_t i32Value)
{
    static char pcCanvasText[5];
    static char pcSliderText[5];

    //
    // Is this the widget whose value we mirror in the canvas widget and the
    // locked slider?
    //
    if(psWidget == (tWidget *)&g_psSliders[SLIDER_CANVAS_VAL_INDEX])
    {
        //
        // Yes - update the canvas to show the slider value.
        //
        usprintf(pcCanvasText, "%3d%%", i32Value);
        CanvasTextSet(&g_sSliderValueCanvas, pcCanvasText);
        WidgetPaint((tWidget *)&g_sSliderValueCanvas);

        //
        // Also update the value of the locked slider to reflect this one.
        //
        SliderValueSet(&g_psSliders[SLIDER_LOCKED_INDEX], i32Value);
        WidgetPaint((tWidget *)&g_psSliders[SLIDER_LOCKED_INDEX]);
    }

    if(psWidget == (tWidget *)&g_psSliders[SLIDER_TEXT_VAL_INDEX])
    {
        //
        // Yes - update the canvas to show the slider value.
        //
        usprintf(pcSliderText, "%3d%%", i32Value);
        SliderTextSet(&g_psSliders[SLIDER_TEXT_VAL_INDEX], pcSliderText);
        WidgetPaint((tWidget *)&g_psSliders[SLIDER_TEXT_VAL_INDEX]);
    }
}
