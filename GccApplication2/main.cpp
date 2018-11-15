/*
 * $safeprojectname$.cpp
 * This Application is linked to LAB 2, Soft-Blink.
 * Created: 09.11.2018 10.03.30
 * Final Edit: 14.11.2018 10.23.01
 * Author : Marcus
 */ 

#include <avr/io.h>
#include <util/delay.h>

void wait_ms(unsigned int wantedDelay);
void wait_us(unsigned int wantedDelay);
void LEDon(const unsigned int pinNumber);
void LEDoff(const unsigned int pinNumber);

int main(void)
{
	DDRD = 0x7F;
	const unsigned int pinNumber = 0;
	const int periodTime = 500;			 //in ms or us, depending on which wait-function is selected
	unsigned int delayOn = 0;			 //in ms or us, depending on which wait-function is selected
	unsigned int delayOff = periodTime;	 //in ms or us, depending on which wait-function is selected
    while (1) 
    {
		while(delayOn < periodTime)
		{
			LEDon(pinNumber);
			wait_us(delayOn);	//hold ON 
			LEDoff(pinNumber);
			wait_us(delayOff);	//hold OFF
			delayOn++;			//increase time ON (increase perceived brightness)
			delayOff=periodTime-delayOn;	//the rest of the period the led is off
		}
		while(delayOn > 0)
		{
			LEDon(pinNumber);
			wait_us(delayOn);
			LEDoff(pinNumber);
			wait_us(delayOff);
			delayOn--; //decrease time ON (decrease perceived brightness)
			delayOff=periodTime-delayOn; 
		}
    }

}


	void wait_ms(unsigned int wantedDelay)
	{
		while(wantedDelay > 0)
		{
			_delay_ms(1);
			wantedDelay --;
		}
	}
	
	void wait_us(unsigned int wantedDelay)
	{
		while(wantedDelay > 0)
		{
			_delay_us(1);
			wantedDelay --;
		}
	}

	void LEDon(const unsigned int pinNumber)
	{
		PORTD |= (1 << pinNumber);
	}
	void LEDoff(const unsigned int pinNumber)
	{
		PORTD &= ~(1 << pinNumber);
	}