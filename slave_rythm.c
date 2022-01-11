#include "LPC8xx.h"
#include "syscon.h"
#include "lib_ENS_II1_lcd.h"
#include "rom_api.h"
#include "gpio.h"
#include "ctimer.h"
#include "swm.h"
#include "stdio.h"
#include "UART.h"
#include "utilities.h"
#include "dac.h"
#include "adc.h"
#include "iocon.h"
#include "mrt.h"

#define DBGTXPIN 4
#define DBGRXPIN 0
#define CEN 0
#define CRST 1
#define MCP23_I2C_AD	0x40
#define MCP23_IODIR_REG		0
#define MCP23_IPOL_REG		1
#define MCP23_GPINTEN_REG	2
#define MCP23_DEFVAL_REG	3
#define MCP23_INTCON_REG	4
#define MCP23_IOCON_REG		5
#define MCP23_GPPU_REG		6
#define MCP23_INTF_REG		7
#define MCP23_INTCAP_REG	8
#define MCP23_GPIO_REG		9
#define MCP23_OLAT_REG		10



#define BP1 LPC_GPIO_PORT->B0[13]
#define BP2 LPC_GPIO_PORT->B0[12]
#define LED1 LPC_GPIO_PORT->B0[19]
#define LED2 LPC_GPIO_PORT->B0[17]
#define LED3 LPC_GPIO_PORT->B0[21]
#define LED4 LPC_GPIO_PORT->B0[11]
#define SCLK_PIN P0_19
#define TXD_PIN P0_18
#define RXD_PIN P0_20

int bpm=120;
int following = 0;

void delay(int sec);
void set_freq(float freq);
void set_note(char note);
void play_music(int number);


const uint16_t sine_lut[200] = { 500, 516, 531, 547, 563, 578, 594, 609, 624, 639, 655, 669, 684, 699, 713, 727, 741, 755, 768, 781, 794, 806, 819, 831, 842, 854, 864, 875, 885, 895, 905, 914, 922, 930, 938, 946, 952, 959, 965, 970, 976, 980, 984, 988, 991, 994, 996, 998, 999, 1000, 1000, 1000, 999, 998, 996, 994, 991, 988, 984, 980, 976, 970, 965, 959, 952, 946, 938, 930, 922, 914, 905, 895, 885, 875, 864, 854, 842, 831, 819, 806, 794, 781, 768, 755, 741, 727, 713, 699, 684, 669, 655, 639, 624, 609, 594, 578, 563, 547, 531, 516, 500, 484, 469, 453, 437, 422, 406, 391, 376, 361, 345, 331, 316, 301, 287, 273, 259, 245, 232, 219, 206, 194, 181, 169, 158, 146, 136, 125, 115, 105, 95, 86, 78, 70, 62, 54, 48, 41, 35, 30, 24, 20, 16, 12, 9, 6, 4, 2, 1, 0, 0, 0, 1, 2, 4, 6, 9, 12, 16, 20, 24, 30, 35, 41, 48, 54, 62, 70, 78, 86, 95, 105, 115, 125, 136, 146, 158, 169, 181, 194, 206, 219, 232, 245, 259, 273, 287, 301, 316, 331, 345, 361, 376, 391, 406, 422, 437, 453, 469, 484 };

void CTIMER0_IRQHandler(void) {
    static uint8_t step = 0;

    LPC_CTIMER0->IR |= 0x1;
    LPC_DAC0->CR = (sine_lut[step] << 6);

    if(++step >= 200) {
        step = 0;
    }
}




void MRT_IRQHandler(void)
{
	if((LPC_MRT->Channel[0].STAT & 1 )!=0)//tempo
	{
	LED2=!LED2;
	LPC_MRT->Channel[0].STAT |= 1; //reset interrupt
	}
	if(LPC_MRT->Channel[1].STAT & 1 !=0) //timer_note
	{
	LPC_CTIMER0 ->TCR =(1<<CRST);
	following=1;
	LPC_MRT->Channel[1].STAT |= 1; //reset interrupt
	}


	}

int main(void) {

LPC_PWRD_API -> set_fro_frequency(30000);

	// Activation du périphérique d'entrées/sorties TOR

     LPC_SYSCON -> SYSAHBCLKCTRL0 |= (GPIO | SWM | UART1 | CTIMER0 | MRT | ADC | DAC0 | IOCON | GPIO_INT);

	//Configuration en sortie de la broche P0_19
	LPC_GPIO_PORT->DIR0 |= (1 << 19)|(1 << 17)|(1 << 11)|(1 << 21);



	// ADC DAC
	 LPC_SYSCON->PDRUNCFG &= ~(DAC0_PD | ADC_PD );
	 LPC_SWM->PINENABLE0 &= ~(ADC_7 | ADC_8 | DACOUT0);
	 LPC_IOCON->PIO0_19 = (1 << 16);


	//Interruptions MRT
	NVIC->ISER[0] |= (1<<10); //Interruption MRT
	NVIC_EnableIRQ(MRT_IRQn);



    // Interruptions CTIMER0
    NVIC->ISER[0] |= 1 << CTIMER0_IRQn;
    NVIC->IP[5] &= ~(0x1 << 30);

	// Config de CTIMER0
	LPC_SWM -> PINASSIGN4 = 0xFFFF13FF;
    LPC_CTIMER0->PR =  0;
    LPC_CTIMER0->MCR |= (1 << MR0I | 1 << MR0R);


	//Configuration de MRT
	LPC_MRT->Channel[0].INTVAL = 15000001 ;
	LPC_MRT->Channel[0].CTRL|= (1<<MRT_INTEN);
	LPC_MRT->Channel[1].CTRL|= (MRT_OneShot<<MRT_MODE) | (1<<MRT_INTEN);

    // Initialisation de la liaison série
	//ConfigSWM(U1_SCLK, SCLK_PIN);
	ConfigSWM(U1_TXD, TXD_PIN);
	ConfigSWM(U1_RXD, RXD_PIN);

	int baud_rate = 9600 ;
	int BRGVAL = 15*1000000.0/((15+1)*baud_rate)-0.5;
	LPC_SYSCON->UART1CLKSEL = 0;
	LPC_SYSCON->PRESETCTRL0 &= (UART1_RST_N);
	LPC_SYSCON->PRESETCTRL0 |= ~(UART1_RST_N);

	// baud rate generator
	LPC_USART1->BRG = BRGVAL;
	// réglage liaison série
	LPC_USART1->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1|UART_EN;
	LPC_USART1->STAT = 0xFFFF;
    //LPC_SWM -> PINASSIGN1 = 0xFFFF0004;


	char texte[17];

	init_lcd();
	sprintf(texte, "%s", "rythm  ");
	lcd_puts(texte);

	//demarrage du timer
	LPC_CTIMER0 -> TCR =(1<<CEN);
	LPC_CTIMER0 ->TCR =(1<<CRST);

	while(1){

		if (((LPC_USART1->STAT) & RXRDY) ==1 )
		{
		switch ((LPC_USART1->RXDAT))
		    {
		    case 'l':

		    	init_lcd();
		    	sprintf(texte, "%s", "Launch Music  ");
		    	lcd_puts(texte);
                play_chords(0);

		    default:
		        break;
		    }

		}
		}



	}





void delay(int sec){

	for (int i = 0 ; i<20000*sec;i++);

}

void set_freq(float freq){
	float new_MRf;
	int new_MR;
    new_MRf=15000000/freq/200;
	new_MR=(int)new_MRf;
	LPC_CTIMER0->TCR = (1 << CRST);
	LPC_CTIMER0->MR[0] = new_MR;
	LPC_CTIMER0 ->TCR =(1<<CEN);
}


void set_note(char note){

	 switch (note)
	    {
	    case 'C':
	        set_freq(261.63);

	        break;
	    case 'D':
	        set_freq(293.66);

	        break;
	    case 'E':
	        set_freq(329.63);

	        break;
	    case 'F':
	        set_freq(349.23);

	        break;
	    case 'G':
	        set_freq(392);

	        break;
	    case 'A':
	        set_freq(440);

	        break;
	    case 'B':
	        set_freq(493.88);

	        break;
	    case 'H':
	        set_freq(523.25);

	    case 'S':
	        break;
	    default:
	        break;

}
}


void play_chords(int number){

	char musics[1][17] = { "Les Hobbits", };
	char music_length[1] = { 130 };
	char music_melody[1][200] = {
	        {'S','E','S','E','S','E','E','E','S',
			'E','S','E','S','D','D','D','S',
			'E','S','E','S','E','E','E','S',
			'D','S','D','S','D','D','D','S',
			'A','S','A','S','A','A','A','S',
			'D','S','D','S','B','B','B','S',
			'E','S','E','S','D','D','D','S',
			'E','S','E','S','E','E','E',
			'S','E','S','E','S','E','E','E','S',
			'E','S','E','S','D','D','D','S',
			'E','S','E','S','E','E','E','S',
			'D','S','D','S','D','D','D','S',
			'A','S','A','S','A','A','A','S',
			'D','S','D','S','B','B','B','S',
			'E','S','E','S','D','D','D','S',
			'E','S','E','S','E','E','E','S'

	        }

	    };


	char music_rythm[1][200] = {
	    {2,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
	      }
	    };



	following = 1;
	for (int i=0;i<=music_length[number];i)
	{
		if (following==1){
		set_note(music_melody[number][i]);
		float duree=music_rythm[number][i];
		int periode = 15000001*duree/bpm*30;
		following = 0;
		LPC_MRT->Channel[1].INTVAL = periode;
		i++;
		}

	}
	LPC_CTIMER0 ->TCR =(1<<CRST);





}
