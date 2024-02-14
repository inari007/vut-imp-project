/******************************************************************************/
/*                                                                            */
/*   Project IMP							                                  */
/*                                                                            */
/*   LED Matrix			                      								  */
/*                                                                            */
/*  Zdenek Dobes, xdobes21								                      */
/*                                                                            */
/******************************************************************************/

/* Header file with all the essential definitions for a given type of MCU */
#include "MK60DZ10.h"

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK	(unsigned int)0x1F
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK))) // 0001 1111

/* Constants specifying delay loop duration */
#define	tdelay1			1000
#define tdelay2 		16

/* Maximal length of a single word on the screen */
#define MaxLengthOfWord 10
#define WidthOfALetter 4
#define SpaceBetweenLetters 1

/* Buttons */
#define BUTTON_UP_MASK 		0b00000100000000000000000000000000
#define BUTTON_LEFT_MASK 	0b00001000000000000000000000000000
#define BUTTON_DOWN_MASK 	0b00000000000000000001000000000000
#define BUTTON_RIGHT_MASK 	0b00000000000000000000010000000000

/* Columns  */
#define C0		0b00000000000000000000000000000000
#define C1		0b00000000000000000000000100000000
#define C2		0b00000000000000000000010000000000
#define C3		0b00000000000000000000010100000000
#define C4		0b00000000000000000000000001000000
#define C5		0b00000000000000000000000101000000
#define C6		0b00000000000000000000010001000000
#define C7		0b00000000000000000000010101000000
#define C8		0b00000000000000000000100000000000
#define C9		0b00000000000000000000100100000000
#define C10		0b00000000000000000000110000000000
#define C11		0b00000000000000000000110100000000
#define C12		0b00000000000000000000100001000000
#define C13		0b00000000000000000000100101000000
#define C14		0b00000000000000000000110001000000
#define C15		0b00000000000000000000110101000000

/* Rows */
#define R0 		0b00000100000000000000000000000000
#define R1 		0b00000001000000000000000000000000
#define R2 		0b00000000000000000000001000000000
#define R3 		0b00000010000000000000000000000000
#define R4 		0b00010000000000000000000000000000
#define R5 		0b00000000000000000000000010000000
#define R6 		0b00001000000000000000000000000000
#define R7 		0b00100000000000000000000000000000

/* Letters */
typedef struct{
	unsigned int COL_0;
	unsigned int COL_1;
	unsigned int COL_2;
	unsigned int COL_3;
}letter;

letter A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, ONE, TWO;

/* Words */
typedef struct{
	unsigned int LENGTH;
	letter TXT[MaxLengthOfWord];
}my_word;

my_word AAAA, HNUP, IMP, MISAHRDINA, NOPE, XDOBES21, UP, DOWN, LEFT, RIGHT;

void SystemConfig();
void mini_delay();
void init_letters();
void init_words();
int select_col(int col);
void write_word(my_word cur_word);
void PORTE_IRQHandler(void);
void init_everything();

/* Configuration of the necessary MCU peripherals */
void SystemConfig() {
	/* Turn on all port clocks */
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;

	/* Set corresponding PTA pins (column activators of for GPIO functionality */
	PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );  // A0
	PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) );  // A2
	PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // A3

	/* Set corresponding PTA pins (rows selectors of 74HC154) for GPIO functionality */
	PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );  // R0
	PORTA->PCR[24] = ( 0|PORT_PCR_MUX(0x01) );  // R1
	PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );   // R2
	PORTA->PCR[25] = ( 0|PORT_PCR_MUX(0x01) );  // R3
	PORTA->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );  // R4
	PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) );   // R5
	PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );  // R6
	PORTA->PCR[29] = ( 0|PORT_PCR_MUX(0x01) );  // R7

	/* Set corresponding PTE pins (output enable of 74HC154) for GPIO functionality */
	PORTE->PCR[28] = ( 0|PORT_PCR_MUX(0x01) ); // #EN

	PORTE->PCR[10] = 0b00000001000010100000000100000011;
	PORTE->PCR[12] = 0b00000001000010100000000100000011;
	PORTE->PCR[26] = 0b00000001000010100000000100000011;
	PORTE->PCR[27] = 0b00000001000010100000000100000011;

	/* Change corresponding PTA port pins as outputs */
	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);

	/* Change corresponding PTE port pins as outputs */
	PTE->PDDR = GPIO_PDDR_PDD( GPIO_PIN(28) );

	NVIC_ClearPendingIRQ(PORTE_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);
}

/* Delay between powering up columns */
void mini_delay(){
	for(int i=0; i < 5; i++) {
			for(int j=0; j < 5; j++);
		}
}

/* Shape of letters displayed on the matrix */
void init_letters(){
	A.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; A.COL_1 = R3 | R7; A.COL_2 = R3 | R7; A.COL_3 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7;
	B.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; B.COL_1 = R0 | R3 | R7; B.COL_2 = R0 | R3 | R7; B.COL_3 = R1 | R2| R3 | R4 | R5 | R6;
	D.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; D.COL_1 = R0 | R7; D.COL_2 = R1 | R6; D.COL_3 = R2 | R3 | R4 | R5;
	E.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; E.COL_1 = R0 | R3 | R7; E.COL_2 = R0 | R3 | R7; E.COL_3 = R0 | R3 | R7;
	F.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; F.COL_1 = R3 | R7; F.COL_2 = R3 | R7; F.COL_3 = R3 | R7;
	H.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; H.COL_1 = R3; H.COL_2 = R3; H.COL_3 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7;
	G.COL_0 = R1 | R2| R3 | R4 | R5 | R6; G.COL_1 = R0 | R7; G.COL_2 = R0 | R3 | R7; G.COL_3 = R1 | R2 | R6;
	I.COL_0 = R0 | R7; I.COL_1 = R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7; I.COL_2 = R0 | R7; I.COL_3 = 0;
	L.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; L.COL_1 = R0; L.COL_2 = R0; L.COL_3 = R0;
	M.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; M.COL_1 = R5 | R6; M.COL_2 = R5 | R6; M.COL_3 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7;
	N.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; N.COL_1 = R4 | R5 | R6; N.COL_2 = R2 | R3; N.COL_3 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7;
	O.COL_0 = R1 | R2 | R3 | R4 | R5 | R6; O.COL_1 = R0 | R7; O.COL_2 = R0 | R7; O.COL_3 = R1 | R2| R3 | R4 | R5 | R6;
	P.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; P.COL_1 = R3 | R7; P.COL_2 = R3 | R7; P.COL_3 = R4 | R5 | R6;
	S.COL_0 = R1 | R4 | R5 | R6; S.COL_1 = R0 | R3 | R7; S.COL_2 = R0 | R3 | R7; S.COL_3 = R1 | R2 | R6;
	R.COL_0 = R0 | R1 | R2| R3 | R4 | R5 | R6 | R7; R.COL_1 = R3 | R7; R.COL_2 = R3 | R7; R.COL_3 = R0 | R1 | R2 | R4 | R5 | R6;
	T.COL_0 = R7; T.COL_1 = R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7; T.COL_2 = R7; T.COL_3 = 0;
	U.COL_0 = R1 | R2 | R3 | R4 | R5 | R6 | R7; U.COL_1 = R0; U.COL_2 = R0; U.COL_3 = R1 | R2 | R3 | R4 | R5 | R6 | R7;
	W.COL_0 = R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7; W.COL_1 = R1 | R2; W.COL_2 = R1 | R2; W.COL_3 = R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7;
	X.COL_0 = R0 | R1 | R2 | R5 | R6 | R7; X.COL_1 = R3 | R4 ; X.COL_2 = R3 | R4; X.COL_3 = R0 | R1 | R2 | R5 | R6 | R7;
	ONE.COL_0 = R4; ONE.COL_1 = R5; ONE.COL_2 = R6; ONE.COL_3 = R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7;
	TWO.COL_0 = R0 | R1 | R5 | R6; TWO.COL_1 = R0 | R2 | R7; TWO.COL_2 = R0 | R3 | R7; TWO.COL_3 = R0 | R4 | R5 | R6;
}

/* Supported words */
void init_words(){
	AAAA.LENGTH = 4; AAAA.TXT[0] = A; AAAA.TXT[1] = A; AAAA.TXT[2] = A; AAAA.TXT[3] = A;
	HNUP.LENGTH = 4; HNUP.TXT[0] = H; HNUP.TXT[1] = N; HNUP.TXT[2] = U; HNUP.TXT[3] = P;
	NOPE.LENGTH = 4; NOPE.TXT[0] = N; NOPE.TXT[1] = O; NOPE.TXT[2] = P; NOPE.TXT[3] = E;
	IMP.LENGTH = 3; IMP.TXT[0] = I; IMP.TXT[1] = M; IMP.TXT[2] = P;
	MISAHRDINA.LENGTH = 10; MISAHRDINA.TXT[0] = M; MISAHRDINA.TXT[1] = I; MISAHRDINA.TXT[2] = S; MISAHRDINA.TXT[3] = A; MISAHRDINA.TXT[4] = H;
	MISAHRDINA.TXT[5] = R; MISAHRDINA.TXT[6] = D; MISAHRDINA.TXT[7] = I; MISAHRDINA.TXT[8] = N; MISAHRDINA.TXT[9] = A;
	XDOBES21.LENGTH = 8; XDOBES21.TXT[0] = X; XDOBES21.TXT[1] = D; XDOBES21.TXT[2] = O; XDOBES21.TXT[3] = B; XDOBES21.TXT[4] = E;
	XDOBES21.TXT[5] = S; XDOBES21.TXT[6] = TWO; XDOBES21.TXT[7] = ONE;
	UP.LENGTH = 2; UP.TXT[0] = U; UP.TXT[1] = P;
	DOWN.LENGTH = 4; DOWN.TXT[0] = D; DOWN.TXT[1] = O; DOWN.TXT[2] = W; DOWN.TXT[3] = N;
	LEFT.LENGTH = 4; LEFT.TXT[0] = L; LEFT.TXT[1] = E; LEFT.TXT[2] = F; LEFT.TXT[3] = T;
	RIGHT.LENGTH = 5; RIGHT.TXT[0] = R; RIGHT.TXT[1] = I; RIGHT.TXT[2] = G; RIGHT.TXT[3] = H; RIGHT.TXT[4] = T;
}

/* Convert number of column to binary */
int select_col(int col){
	switch(col){
		case 0:
			return C0;
			break;
		case 1:
			return C1;
			break;
		case 2:
			return C2;
			break;
		case 3:
			return C3;
			break;
		case 4:
			return C4;
			break;
		case 5:
			return C5;
			break;
		case 6:
			return C6;
			break;
		case 7:
			return C7;
			break;
		case 8:
			return C8;
			break;
		case 9:
			return C9;
			break;
		case 10:
			return C10;
			break;
		case 11:
			return C11;
			break;
		case 12:
			return C12;
			break;
		case 13:
			return C13;
			break;
		case 14:
			return C14;
			break;
		case 15:
			return C15;
			break;
		default:
			return 0;
			break;
	}
}

/* Write word on a display */
void write_word(my_word cur_word){
	unsigned int width = WidthOfALetter+SpaceBetweenLetters;
	unsigned int length_of_a_word = width*cur_word.LENGTH;
	unsigned int LightsToPowerUp[length_of_a_word];
	for(int i = 0; i < width*cur_word.LENGTH; i++){
		switch(i % width){
			case 0:
				LightsToPowerUp[i] = cur_word.TXT[i/width].COL_0;
				break;
			case 1:
				LightsToPowerUp[i] = cur_word.TXT[i/width].COL_1;
				break;
			case 2:
				LightsToPowerUp[i] = cur_word.TXT[i/width].COL_2;
				break;
			case 3:
				LightsToPowerUp[i] = cur_word.TXT[i/width].COL_3;
				break;
			case 4:
				LightsToPowerUp[i] = 0;
				break;
			default:
				break;
		}
	}
	unsigned int cur_col = 1;
	unsigned int cur_light;
	unsigned int cur_light_max;
	int delay_1 = tdelay1;
	int delay_2 = tdelay2;
	int max_length = length_of_a_word-1;
	for(int x = 0; x < length_of_a_word + 16; x++){
		cur_light_max = x;
		for(int a = delay_1/cur_col; a > 0; a--) {
			for(int b = delay_2; b > 0; b--){
				cur_light = cur_light_max;
				for(int i = 0; i < cur_col; i++){
					if(cur_light > max_length){
						mini_delay();
						cur_light--;
					}
					else{
						PTA->PDOR = LightsToPowerUp[cur_light] | select_col(i);
						mini_delay();
						cur_light--;
					}
				}
			}
		}
		if(cur_col != 16){
			cur_col++;
		}
	}
}

/* Handling buttons */
void PORTE_IRQHandler(void) {
	if(PORTE->ISFR & BUTTON_UP_MASK){
		write_word(UP);
		PORTE->ISFR |= BUTTON_UP_MASK;
	}
	else if(PORTE->ISFR & BUTTON_DOWN_MASK){
		write_word(DOWN);
		PORTE->ISFR |= BUTTON_DOWN_MASK;
	}
	else if(PORTE->ISFR & BUTTON_LEFT_MASK){
		write_word(LEFT);
		PORTE->ISFR |= BUTTON_LEFT_MASK;
	}
	else if(PORTE->ISFR & BUTTON_RIGHT_MASK){
		write_word(RIGHT);
		PORTE->ISFR |= BUTTON_RIGHT_MASK;
	}
}

/* Complete initialization */
void init_everything(){
	SystemConfig();
	init_letters();
	init_words();
}

int main(void)
{
	init_everything();

	write_word(XDOBES21);

    for (;;) {
    	//write_word(IMP);
    }

    return 0;
}
