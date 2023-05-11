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

// #include <lowBand.h>
// #include <midBand.h>
// #include <highBand.h>




/** Defines **/
#define MAX_BUF_SIZE 256


/** Global Variables **/
uint16_t* ADC_DMA_PING_BUFFER = (uint16_t*)0x11000;
uint16_t* ADC_DMA_PONG_BUFFER = (uint16_t*)0x11400;
uint16_t* DAC_DMA_PING_BUFFER = (uint16_t*)0x11800;
uint16_t* DAC_DMA_PONG_BUFFER = (uint16_t*)0x11C00;
// TODO: change thes in the makefile



/** Interrupt Service Routine Declaration Macros **/
// RVISR(IRQ_name_VECTOR, ISR_name)



/** Function Declarations **/
void fillDACHB(uint16_t samples[]);


/** Main Function **/
int main()
{

	// First step, audio input! We can go straight from a 3.5mm aux.
	// Take the samples and toss them thru the ADC

	// we are using interrupts
	// enable_all_interrupts();
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
	ADC0HBS = MAX_BUF_SIZE/2 - 1;

	// DAC setup!!
	DAC->DACCR_.DACCDIV_ = 2; // DACCDIV = 0 no divider on clk src
	DAC->DACCR_.DAC0EN_ = 1; // DAC0 is enabled
	DAC->DACCR_.DAC1EN_ = 0; // DAC1 is disabled
	DAC->DACCR_.DAC0DEN_ = 0; // Both DMAs for DAC0 and DAC1 are disabled (for now...)
	DAC->DACCR_.DAC1DEN_ = 0; 
	DAC->DACCR_.DACCS_ = 2; // DACCS is selecting the HFXT clock as the DAC source clock

	// TODO: CHECK THESE TO MAKE SURE THEY LINE UP W ADC
	DAC->DACCR_.DACDAL_ = 1; // the DAC data alignment is left-aligned!!!! ***
	DAC->DACFS_.DACFS_ = 2; // Section 19.6 calculation

	
	DAC->DACHBS_.DACHBS_= MAX_BUF_SIZE - 1; // max DACHBS of 127 because of da ADC limitations

	// ADC0VAL needs to be stored in da buffers!

	// DAC DMA FILL WITH CHEERIOS (zeros)
	for(int i = 0; i < MAX_BUF_SIZE; i++) {
		DAC_DMA_PING_BUFFER[i] = 0;
	}

	// send audio straight through here! TURN ON ADC DEN
	ADC0CR |= ADCDEN;
	DAC->DACCR_.DAC0DEN_ = 1;


	// lets get our samples, shall we?
	while(1) {

		
		// let it fill up and then stop the ADC from getting new samps until the next time
		while(ADC0->SR.BFIF != 1) {}
		// reset BFIF flag
		ADC0->SR.BFIF = 1;
		// ADC0CR &= ~ADCDEN;

		// TODO: process/filter here

		// outputting to da DAC

		while (DAC->DACSR_.DACBEIF_ == 0){}

		// if the first half is being written, read from second half
		if(ADC0->SR.BH == 0) {
			fillDACHB(ADC_DMA_PONG_BUFFER);
		}

		else {
			fillDACHB(ADC_DMA_PING_BUFFER);
		}
	
	}




	// output = input * (loband + midband + hiband + 1)

	return 0;
}



/** Function Definitions **/
void fillDACHB(uint16_t samples[]){
	if (DAC->DACSR_.DACBH_ == 1){
		for (int i = 0; i < MAX_BUF_SIZE; i++){
			DAC_DMA_PING_BUFFER[i*2] = samples[i];
				
		}
	} else {
		for (int i = 0; i < MAX_BUF_SIZE; i++) {
			DAC_DMA_PONG_BUFFER[i*2] = samples[i];
		}
	}
	DAC->DACSR_.DACBEIF_ = 1; //Reset DACBEIF flag
return;
}




/** Interrupt Service Routines **/


