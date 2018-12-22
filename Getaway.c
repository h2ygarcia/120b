

#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char traffic_arr[8] = {0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char robber_arr [8] = {0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char display_arr[8] = {0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
volatile unsigned long cnt  = 0;
volatile unsigned long cnt2 = 0;
volatile unsigned long cnt_y = 0;
volatile unsigned long cnt_traffic = 0;
volatile unsigned long cnt_traffic_dis = 0;
volatile unsigned char r = 0; //robber
volatile unsigned char temp_pos = 0x01;
volatile unsigned char cnt_rand_dis = 0; 
// ====================================================================================================
//shift register functions
// ====================================================================================================

void transmit_data_blue(unsigned char data) {
	unsigned char i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTB |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}

void transmit_data2    (unsigned char data2) {
	unsigned char j;
	for (j = 0; j < 8 ; ++j) {
		// Sets SRCLR to 1, also clears SRCLK
		PORTD = 0x08;
		// set SER = next bit of data to be sent.
		PORTD |= ((data2 >> j) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTD |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTD |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTD = 0x00;
}


// ====================================================================================================
// SM: LED Matrix Displays
// ====================================================================================================
enum SM1_States {sm1_display } state;
void SM1_Traffic_Display() {
	// === Local Variables ===
	static unsigned char row = 0xFE; // grounds selected row
	static unsigned char pos = 0;	
	// === Transitions ===
	switch (state) {
		case sm1_display:
		state = sm1_display;
		break;
		default:
		state = sm1_display;
		break;
	}
	// === Actions ===
	switch (state) {
		case sm1_display:   // If illuminated LED in bottom right corner	
		if(row == 0xFE){  //manipulating row bit by grounding column
			row = 0x7F;
		}
		else{
			row = ((row >> 1) | 0x80);
		}
		if(pos >= 7){
			pos = 0;
		}
		else{
			pos++;
		}
		break;
		default:
		break;
	}
    transmit_data_blue(0xFF);
	//transmit_data_red(0x00);
	transmit_data2(traffic_arr[pos]);
	transmit_data_blue(row);
};

 enum SMR_display_States {smRob_display } stateRD;
 void SM_Robber_Display() {
 	// === Local Variables ===
 	static unsigned char row = 0xFE; // grounds selected row
 	static unsigned char pos = 0;
 	// === Transitions ===
 	switch (stateRD) {
 		case smRob_display:
 		state = smRob_display;
 		break;
 		default:
 		state = smRob_display;
 		break;
 	}
 	// === Actions ===
 	switch (stateRD) {
 		case smRob_display:   // If illuminated LED in bottom right corner
 		if(row == 0xFE){
 		row = 0x7F;
 		}
 		else{
 			row = ((row >> 1) | 0x80);
 		}
 		if(pos >= 7){
 			pos = 0;
 		}
 		else{
 			pos++;	
 		}
 		break;
 		default:
 		break;
 	}
     transmit_data_blue(0xFF);
 	transmit_data2(robber_arr[pos]);
 	transmit_data_blue(row);
 };

// ====================================================================================================
// SM: Traffic   @ 500ms
// ====================================================================================================
enum SM2_States {ahead, behind} state2;
void SM2_Traffic(){
	//local variables
	static unsigned char t_pos = 7;
	static unsigned  char num = 0x00;
	static unsigned  char num1 = 0x00;
	static unsigned char temp= 255;
	//transitions
	switch(state2){
		
		case ahead:
		if(t_pos == 7){
			 state2 = ahead ;
			 
		}
		else
		{
			state2 = behind;
		}
		break;
		
		case behind:
		if(t_pos >=0){
			t_pos--;
			state2 = behind;
		}
		else{
			t_pos  = 7;
			state2 = ahead;
		}
		break;

		default:
		state2 = ahead;
		break;
    }
	//actions
	switch(state2){
		case ahead:
		num = rand() % 252 + 1;
		traffic_arr[t_pos] = num & 0xDB;
		t_pos--;
		break;
		
		case behind:
		traffic_arr[0] = 0x00 & 0xDB;
		break;
		default:
		break;
	}
}
// ====================================================================================================
// Joystick functions
// ====================================================================================================
void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}
void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}
// ====================================================================================================
// SM: Robber
// ====================================================================================================	
 enum SM3_States { up, down , still } state_y; //Remember to modify so i = 7 after i =0
 void SM3_Robber_YAXIS()
 {
 	//local variables
	Set_A2D_Pin(0x01);
 	unsigned short input = ADC;
 	//transitions
 	switch(state_y)
 	{
 		case up:
 		if( (input >= 550) && (r < 7)  ){
		   robber_arr[r]= 0;
		   r++;
 		   state_y = up;
 		}
		else if((input < 450) && (r>0))
		{
			robber_arr[r]= 0;
			r--;
			state_y = down;
		}
		else{
			state_y = still;
		}
		input = ADC;
		break;
 		case down:
 		if ((input < 450) && (r>0)){
			 robber_arr[r]= 0;
			 r--;
 		     state_y = down;
		 }
		 else if((input >= 550) && (r < 7))
		 {
			 robber_arr[r]= 0;
			 r++;
			 state_y = up;
		 }
		 else{
			 state_y = still;
		 }
		 input = ADC;
		 break;
		 case still:
		 if((input >= 550) && (r < 7) )
		 {
			 robber_arr[r]= 0;
			r++;
			 state_y = up;
		 }
		 else if((input < 450) && (r>0)){
			 robber_arr[r]= 0;
			 r--;
			 state_y = down;
		}
		else{
			state_y = still;
		}
		 input = ADC;
		break;
 		
 		default:
 		state_y = up;
 		break;
 	}
 	//actions
 	switch(state_y)
 	{
 		case up:   
 		robber_arr[r] = temp_pos;
		break;
		case down:
		robber_arr[r] = temp_pos;
		break;
		case still:
	    robber_arr[r] = temp_pos;
		break;
 		default:
 		break;
 	}
 }

enum SM4_States { left, right , still2 } state_x; //TEST FIRST
void SM4_Robber_XAXIS()
{
	//local variables
	Set_A2D_Pin(0x00);
	unsigned short input = ADC;
	//transitions
	switch(state_x)
	{
		case left:
		if( (input >= 550) && (temp_pos > 0x01)  ){
			robber_arr[r]= 0;
			temp_pos = temp_pos >> 1;
			state_x = right;
		}
		else if((input < 450) && (temp_pos < 0x80))
		{
			robber_arr[r]= 0;
			temp_pos = temp_pos << 1;
			state_x = left;
		}
		else{
			state_x = still2;
		}
		input = ADC;
		break;
		
		case right:
		if ((input < 450) && (temp_pos < 0x80)){
			robber_arr[r]= 0;
			temp_pos = temp_pos << 1;
			state_x = left;
		}
		else if((input >= 550) && (temp_pos > 0x01))
		{
			robber_arr[r]= 0;
			temp_pos = temp_pos >> 1;
			state_x = right;
		}
		else{
			state_x = still2;
		}
		input = ADC;
		break;
		
		case still2:
		if((input >= 550) && (temp_pos > 0x01) )
		{
			robber_arr[r]= 0;
			temp_pos = temp_pos >> 1;
			state_x = right;
		}
		else if((input < 450) && (temp_pos < 0x80)){
			robber_arr[r]= 0;
			temp_pos = temp_pos << 1;
			state_x = left;
		}
		
		else{
			state_x = still2;
		}
		input = ADC;
		break;
		
		default:
		state_x = left;
		break;
	}
	//actions
	switch(state_x)
	{
		case right:
		robber_arr[r] = temp_pos;
		break;
		
		case left:
		robber_arr[r] = temp_pos;
		break;
		
		case still2:
		robber_arr[r] = temp_pos;
		break;
		
		default:
		break;
	}
	
}
// ====================================================================================================
// MAIN
// ====================================================================================================
int main(void)
{
 	DDRA= 0x00; PORTA = 0xFF;	//input for joystick
 	DDRD= 0xFF; PORTD  = 0x00;   //output pattern display
 	DDRB = 0xFF; PORTB = 0x00;  // output row selecting 
	DDRC = 0xFF; PORTC = 0x00;
	const unsigned long timerPeriod = 1;
	unsigned inc = 0x00;
	TimerSet(timerPeriod);
	TimerOn();
	
 	state   = sm1_display;
 	state2  = ahead;
 	state_y = up;
	state_x = left;

	
 	A2D_init();
	while(1)
	{
     SM2_Traffic();
     SM1_Traffic_Display();
	 SM_Robber_Display();
	 
	
     while(!TimerFlag){}
     TimerFlag = 0;
		
	cnt  += timerPeriod;
	cnt2 += timerPeriod;
	cnt_y += timerPeriod;
	cnt_traffic += timerPeriod;
	cnt_rand_dis += timerPeriod;
	}
	
}

