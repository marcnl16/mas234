/*
 * GccApplication3.cpp
 * This Application is linked to LAB 1 Task 7 "interrupt", using a button to turn off and on a LED
 * Created:		10.11.2018	12.43.34
 * Final Edit:	14.11.2018	10.28.54
 * Author : Marcus
 */ 

#include <avr/io.h>
#include<util/delay.h>

void LEDon(const int LEDpin);
void LEDoff(const int LEDpin);

const int LEDpin = 0;

int main(void)
{
	DDRD = 0xFF; //all ports set as output
	DDRD &= ~(1 << PD7); // port D7 set as input
//	PORTD |= (1 << PD7); //internal pull-up-resistor
	
    while (1) 
    {
		if (PIND &= (1<<PD7))
		{
			LEDoff(LEDpin);
		}
		else
			LEDon(LEDpin);
			
		_delay_ms(5); //for debouncing
		
    }
}

void LEDon(const int LEDpin)
{
	PORTD |= (1 << LEDpin);
}

void LEDoff(const int LEDpin)
{
	PORTD &= ~(1 << LEDpin);
}