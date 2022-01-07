
/* p4_1.c UART0 transmit
* Sending “YES” to UART0 on Freescale FRDM-KL25Z board.
* UART0 is connected to openSDA debug agent and has
* a virtual connection to the host PC COM port.
* Use TeraTerm to see the message “YES” on a PC.
* By default in SystemInit(), FLL clock output is 20.97152MHz.
* Setting BDH=0, BDL=0x17, and OSR=0x0F yields 115200 Baud.
*/
#include <MKL25Z4.H>
#include <stdlib.h>

void UART0_init(void);
void delayMs(int n);
void strreverse(char* begin, char* end);
void itoa(int value, char* str, int base); 
void TPM0_init(void);
void ADC0_init(void);
int calibrareADC0(void);
void leduri(short int rezultat);
void terminial(short int rezultat);

			uint8_t val[2];
			char str[10];
int main (void) {
	ADC0_init();
	UART0_init();
  //itoa(val, str, 10);
  TPM0_init();
	
	short int result;
	
	while (1) {
		ADC0->SC1[0] = 0; /* start conversion on channel 0 */
		while(!(ADC0->SC1[0] & 0x80)) { } /* wait for conversion complete */
		result = ADC0->R[0]; /* read conversion result and clear COCO flag */
    terminial(result);
		leduri(result);
		}
	



}
/* initialize UART0 to transmit at 115200 Baud */
void UART0_init(void) {
SIM->SCGC4 |= 0x0400; /* enable clock for UART0 */
SIM->SOPT2 |= 0x04000000; /* use FLL output for UART Baud rate generator*/
UART0->C2 = 0; /* turn off UART0 while changing configurations */
UART0->BDH = 0x00;
UART0->BDL = 0x0B; /* 115200 Baud la frecventa de 20.97152MHz */
UART0->C4 = 0x0F; /* Over Sampling Ratio 16 */
UART0->C1 = 0x00; /* 8-bit data */
UART0->C2 = 0x08; /* enable transmit */
SIM->SCGC5 |= 0x0200; /* enable clock for PORTA */
PORTA->PCR[2] = 0x0200; /* make PTA2 UART0_Tx pin */
}
/* Delay n milliseconds */
/* The CPU core clock is set to MCGFLLCLK at 20.97152MHz in SystemInit(). */
void delayMs(int n) {
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 7000; j++) {}
}


void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}
	

void itoa(int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	char* wstr=str;
	
	int sign;

	// Validate base
	
	if (base<2 || base>35){ *wstr='\0'; return; }

	// Take care of sign
	
	if ((sign=value) < 0) value = -value;

	// Conversion. Number is reversed.
	
	do *wstr++ = num[value%base]; while(value/=base);
	
	if(sign<0) *wstr++='-';
	
	*wstr='\0';

	// Reverse string

	strreverse(str,wstr-1);
	
}


void TPM0_init(void){

	SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
	PORTD->PCR[1] = 0x0400; /* PTD1 used by TPM0 */
	SIM->SCGC6 |= 0x01000000; /* enable clock to TPM0 */
	SIM->SOPT2 |= 0x01000000; /* use MCGFLLCLK as timer counter clock */
	TPM0->SC = 0; /* disable timer */
	TPM0->CONTROLS[1].CnSC = 0x20 | 0x08; /* edge-aligned, pulse high */
	TPM0->MOD = 21845; /* Set up modulo register for 60 Hz */
	TPM0->SC = 0x0C; /* enable TPM0 with prescaler /16 */
	TPM0->CONTROLS[1].CnV = 7282; /* Set up channel value for 33% dutycycle */
	delayMs(100);
}


void ADC0_init(void){
		SIM->SCGC5 |= 0x0400; /* clock to PORTB */
		//if (calibrareADC0()){};

		PORTB->PCR[20] = 0; /* PTB1 analog input */
		SIM->SCGC6 |= 0x8000000; /* clock to ADC0 */
		ADC0->SC2 &= ~0x40; /* software trigger */
		/* clock div by 4, long sample time, single ended 12 bit, bus clock */
		ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;
}

int calibrareADC0() {
      unsigned short castigCalibrare;
      //dezactiveaza intreruperea completa a conversiei
      ADC0_SC1A &= ~ADC_SC1_AIEN_MASK;
      //activeaza declansatorul SOFTWARE de conversie
      //al procesului de calibrare 
      ADC0_SC2 &= ~ADC_SC2_ADTRG_MASK;
      //activeaza conversia continua, o conversie sau un set de conversii
      //sterge registrul AVGS pentru urmatoarele scriere
      ADC0_SC3 &= (~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK);
      //activeaza functia de mediere hardware
      //32 de conversii folosite in functia de mediere
      //pentru rezultatul final
      ADC0_SC3 |= (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3));
      //pornirea secventei de calibrare
      ADC0_SC3 |= ADC_SC3_CAL_MASK;   
     
     //asteptare pentru sfarsitul conversiei
      while (!(ADC0_SC1A & ADC_SC1_COCO_MASK)); 
      
      //verificare erorilor de calibrare - CALF = Calibration Failed Flag
      if ((ADC0_SC3 & ADC_SC3_CALF_MASK)) {
          return (1); 
      }
      //calcularea partii plus a calibrarii
      castigCalibrare = 0x00;
      castigCalibrare = ADC0_CLP0;
      castigCalibrare += ADC0_CLP1;
      castigCalibrare += ADC0_CLP2;
      castigCalibrare += ADC0_CLP3;
      castigCalibrare += ADC0_CLP4;
      castigCalibrare += ADC0_CLPS;
      castigCalibrare = castigCalibrare / 2;
      //seteaza MSB
      castigCalibrare |= 0x8000; 
      //castigul partii plus
      ADC0_PG = ADC_PG_PG(castigCalibrare);
      
      //calcularea partii minus a calibrarii
      castigCalibrare = 0x00;
      castigCalibrare = ADC0_CLM0;
      castigCalibrare += ADC0_CLM1;
      castigCalibrare += ADC0_CLM2;
      castigCalibrare += ADC0_CLM3;
      castigCalibrare += ADC0_CLM4;
      castigCalibrare += ADC0_CLMS;
      castigCalibrare = castigCalibrare / 2;
      castigCalibrare |= 0x8000; 
      ADC0_MG = ADC_MG_MG(castigCalibrare);
      
      //seteaza pe 0 bitul de calibrare
      ADC0_SC3 &= ~(ADC_SC3_CAL_MASK);
      return (0);
}




void terminial(short int rezultat){
	

	    short int result = rezultat;
	
			val[0] = result >> 8;     // high byte (0x12)
			val[1] = result & 0x00FF; // low byte (0x34) 
	
    	itoa(val[0], str, 10);
	
			while(!(UART0->S1 & 0x80)) { } /* wait for transmit buffer empty */
			UART0->D = str[0]; /* send a char */
			while(!(UART0->S1 & 0x80)) { }
			UART0->D = str[1]; /* send a char */
			while(!(UART0->S1 & 0x80)) { }
			UART0->D = str[2]; /* send a char */

		  itoa(val[1], str, 10);
			
			while(!(UART0->S1 & 0x80)) { } /* wait for transmit buffer empty */
			UART0->D = str[0]; /* send a char */
			while(!(UART0->S1 & 0x80)) { }
			UART0->D = str[1]; /* send a char */
			while(!(UART0->S1 & 0x80)) { }
			UART0->D = str[2]; /* send a char */			
			while(!(UART0->S1 & 0x80)) { }
		 
			UART0->D = '\r'; /* send a char */
			while(!(UART0->S1 & 0x80)) { }
			UART0->D = '\n'; /* send a char */
			delayMs(10); /* leave a gap between messages */
}

void leduri(short int rezultat){
	//val[0] = rezultat >> 8; 
	
	if(rezultat < 10000)
		TPM0->CONTROLS[1].CnV = 19661;/* Set up channel value for 90% dutycycle */
	if(rezultat > 10000)
		TPM0->CONTROLS[1].CnV = 2185;/* Set up channel value for 10% dutycycle */

}
