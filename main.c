#include "define.h"
#include "keypad.h"
#include "sensor.h"
#include "powermgmt.h"
#include "motorctrl.h"
#include "console.h"

#include "FreeRTOS.h"
#include "task.h"

const TickType_t xDelay100 = 100 / portTICK_PERIOD_MS;
const TickType_t xDelay200 = 200 / portTICK_PERIOD_MS;
const TickType_t xDelay500 = 500 / portTICK_PERIOD_MS;

/* The CLI commands are defined in CLI-commands.c. */
void vRegisterCLICommands( void );

xQueueHandle xQueue = NULL;

void SystemSetupStuff(void)
{
	LPC_SC->PCONP |= PCONP_PCGPIO;              // power up GPIO
	LPC_GPIO1->FIODIR |= PIN(25);               // p1.25 Pwr on set output
	LPC_GPIO1->FIOPIN |= PIN(25);               // p1.25 Keep pwr on

	// Configure Timer0
	LPC_SC->PCONP |= PCONP_PCTIM1;              // power up Timer (def on)
	LPC_SC->PCLKSEL0 |= PCLK_TIMER1(CCLK_DIV1); // set Timer0 clock1

	// Configure Power button
	LPC_PINCON->PINMODE3 |= (PINMODE_PULLDOWN << 24);      // Pullup
}

HeapRegion_t xHeapRegions[] = {
    { (uint8_t *) 0x10001000UL, 0x7000 },
    { (uint8_t *) 0x2007C000UL, 0x4000 },
    { (uint8_t *) 0x20080000UL, 0x4000 },
    { NULL, 0 }
};

static void task_DigitalTest(void *pvParameters)
{
//    int Counter1 = 0;
	// Configure LCD backligt
	LPC_GPIO1->FIODIR |= PIN(20);               // P1.20 output mode.
	LPC_GPIO1->FIOSET |= PIN(20);               // p1.20 LCD backlight ON

	LPC_GPIO0->FIODIR |= PIN(18);               // p0.20 output mode.
	LPC_GPIO0->FIODIR |= PIN(19);               // p0.20 output mode.
	LPC_GPIO0->FIODIR |= PIN(20);               // p0.20 output mode.
	
	LPC_PINCON->PINSEL0 &= ~((uint32_t)3 << 30);
	LPC_PINCON->PINSEL1 &= ~(3 << 0);
	LPC_PINCON->PINSEL1 &= ~(3 << 4);
	LPC_PINCON->PINSEL1 &= ~(3 << 6);
	LPC_PINCON->PINSEL1 &= ~(3 << 8);

	LPC_PINCON->PINMODE1 &= ~(3 << 4);
	LPC_PINCON->PINMODE1 |= (2 << 4);
	LPC_PINCON->PINMODE1 &= ~(3 << 6);
	LPC_PINCON->PINMODE1 |= (2 << 6);
	LPC_PINCON->PINMODE1 &= ~(3 << 8);
	LPC_PINCON->PINMODE1 |= (2 << 8);

	for (;;) {
		LPC_GPIO1->FIOCLR |= PIN(20);               // p1.20 LCD backlight OFF
		vTaskDelay(xDelay200);
		LPC_GPIO1->FIOSET |= PIN(20);               // p1.20 LCD backlight ON
		LPC_GPIO0->FIOSET |= PIN(18);							// WAHRSCHEINLICH pin 6 auf JST GHR-08V-S
		LPC_GPIO0->FIOSET |= PIN(19);							// pin 3 auf JST GHR-08V-S
		LPC_GPIO0->FIOSET |= PIN(20);							// pin 4 auf JST GHR-08V-S
		vTaskDelay(xDelay100);
		LPC_GPIO0->FIOCLR |= PIN(18);
		vTaskDelay(xDelay100);
		LPC_GPIO0->FIOCLR |= PIN(19);
		vTaskDelay(xDelay100);
		LPC_GPIO0->FIOCLR |= PIN(20);
  }
}

int main(void)
{
	vPortDefineHeapRegions(xHeapRegions);

	SystemCoreClockUpdate();
	SystemSetupStuff();

	xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));

	if (xQueue != NULL)
	{
		xTaskCreate(task_DigitalTest, "Digital", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
		xTaskCreate(task_Console, "Console", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
		xTaskCreate(task_Keypad, "Keypad", configMINIMAL_STACK_SIZE, NULL, 6, NULL);
		//        xTaskCreate(task_LCD, "LCD", 1024, NULL, 8, NULL);
		xTaskCreate(task_Sensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
		xTaskCreate(task_PowerMgmt, "PowerMgmt", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
		xTaskCreate(task_MotorCtrl, "MotorCtrl", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

		vTaskStartScheduler();
	}
}
