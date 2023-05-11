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
#include <midBand.h>
#include <highBand.h>




/** Defines **/
#define MAX_BUF_SIZE 127


/** Global Variables **/
uint16_t* ADC_DMA_PING_BUFFER = (uint16_t*)0x11000;
uint16_t* ADC_DMA_PONG_BUFFER = (uint16_t*)0x11400;
uint16_t* DAC_DMA_PING_BUFFER = (uint16_t*)0x11800;
uint16_t* DAC_DMA_PONG_BUFFER = (uint16_t*)0x11C00;
// TODO: change thes in the makefile



/** Interrupt Service Routine Declaration Macros **/
RVISR(IRQ_name_VECTOR, ISR_name)



/** Function Declarations **/
int fillDACHB(const uint16_t samples[], uint16_t length, int startIndex);


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

	// calculated ADC0FS to be = 2. This gives 31250kHz sampling rate
	ADC0FS = 2;
	// max buffer size
	ADC0HBS = MAX_BUF_SIZE;

	// DAC setup!!
	DAC->DACCR_.DACCDIV_ = 0; // DACCDIV = 0 no divider on clk src
	DAC->DACCR_.DAC0EN_ = 1; // DAC0 is enabled
	DAC->DACCR_.DAC1EN_ = 0; // DAC1 is disabled
	DAC->DACCR_.DAC0DEN_ = 0; // Both DMAs for DAC0 and DAC1 are disabled (for now...)
	DAC->DACCR_.DAC1DEN_ = 0; 
	DAC->DACCR_.DACCS_ = 2; // DACCS is selecting the HFXT clock as the DAC source clock
	DAC->DACCR_.DACDAL_ = 0; // the DAC data alignment is right-aligned
	DAC->DACFS_.DACFS_ = 14; // Section 19.6 calculation

	// ADC0VAL needs to be stored in da buffers!

	// lets get our samples, shall we?
	while(1) {

		// send audio straight through here! TURN ON ADC DEN
		ADC0CR |= ADCDEN;
		// let it fill up and then stop the ADC from getting new samps until the next time
		while(ADC0->SR.BFIF != 1) {}
		// reset BFIF flag
		ADC0->SR.BFIF = 1;
		// ADC0CR &= ~ADCDEN;

		// TODO: process/filter here

		// outputting to da DAC
		int index = 0;

		DAC->DACCR_.DAC0DEN_ = 0;
		DAC->DACHBS_.DACHBS_= MAX_BUF_SIZE; // max DACHBS of 127 because of da ADC limitations
		DAC->DACCR_.DAC0DEN_ = 1;
		while (DAC->DACSR_.DACBEIF_ == 0){}

		// if the first half is being written, read from second half
		if(ADC0->SR.BH == 0) {
			index = fillDACHB(ADC_DMA_PONG_BUFFER, (MAX_BUF_SIZE + 1) , index);
		}
		else {
			index = fillDACHB(ADC_DMA_PING_BUFFER, (MAX_BUF_SIZE + 1) , index);
		}
	
	}




	// output = input * (loband + midband + hiband + 1)

	return 0;
}



/** Function Definitions **/
int fillDACHB(const uint16_t samples[], uint16_t length, int startIndex){
	if (DAC->DACSR_.DACBH_ == 1){
		for (int i = 0; i < ((DAC->DACHBS_.DACHBS_) + 1); i++){
			DAC_DMA_PING_BUFFER[i*2] = samples[startIndex];
			startIndex++;
			if (startIndex >= length){
				startIndex = 0;
			}			
		}
	} else {
		for (int i = 0; i < ((DAC->DACHBS_.DACHBS_) + 1); i++){
			DAC_DMA_PONG_BUFFER[i*2] = samples[startIndex];
			startIndex++;
			if (startIndex >= length){
				startIndex = 0;
			}
		}
	}
	DAC->DACSR_.DACBEIF_ = 1; //Reset DACBEIF flag
return startIndex;
}


uint16_t* lopass() {

}



/** Interrupt Service Routines **/


