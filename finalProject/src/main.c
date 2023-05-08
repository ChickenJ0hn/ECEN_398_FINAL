// MAIN CODE FOR FINAL PROJECT IN ECEN 398
// Written by Andre Tharp and Evan Cornwell
// 5/2/23
// epic
//
// This code will implement a 3-band equalizer on the pingora2
// MCU


/** Includes **/
#include <MemoryMap.h>
#include <irq.h>

#include <lowBand.h>
#include <midBand.h>d
#include <highBand.h>




/** Defines **/



/** Global Variables **/
uint16_t* ADC_DMA_PING_BUFFER = (uint16_t*)0x11000;
uint16_t* ADC_DMA_PONG_BUFFER = (uint16_t*)0x11400;
// TODO: change thes in the makefile



/** Interrupt Service Routine Declaration Macros **/
RVISR(IRQ_name_VECTOR, ISR_name)



/** Function Declarations **/



/** Main Function **/
int main()
{

	// First step, audio input! We can go straight from a 3.5mm aux.
	// Take the samples and toss them thru the ADC

	// we are using interrupts
	enable_all_interrupts();
	BIASCR = BIASEN | BGEN | (37 << BIASADJ_LSB);

	// PART 1: SET UP ADC (might do function for this, we'll see)
	// left aligned: ADCDAL = 1
	ADC0CR = 0
		// | ADCCIE
		// | ADCBFIE
		// | ADCSC 
		| ADCDAL
		| ADCREF // hmm? 
		| ADCCDIV_4
		// | ADCRG
		// | ADCTE
		// | ADCTS
		| ADCCS_HFXT
		// | ADCDEN
		| ADCEN
	;

	// calculated ADC0FS to be = 2
	ADC0FS = 2;
	// max buffer size
	ADC0HBS = 127;

	// need to change stack start location???

	// ADC0VAL needs to be stored in da buffers!

	// send audio straight through here!





	// output = input * (loband + midband + hiband + 1)

	return 0;
}



/** Function Definitions **/
uint16_t* lopass() {

}



/** Interrupt Service Routines **/


