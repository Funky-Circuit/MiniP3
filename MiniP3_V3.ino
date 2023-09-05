#define F_CPU 2000000UL     /* Define frequency here its 8MHz */
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]



//variables --------------------------------------------------------------------------

int playing_track = 0;
int last_playing_track = 0;
int is_playing = 0;
int button_1_state;
int button_2_state;
int button_3_state;
int last_button_1_state;
int last_button_2_state;
int last_button_3_state;



//functions --------------------------------------------------------------------------

void uart_init(long uart_baudrate)
{
  int baud_prescale;
  UCSR0B |= 0b00011000; //Turn on transmission and reception
  UCSR0C |= 0b10000110; //Use 8-bit character sizes
  baud_prescale = (F_CPU / (uart_baudrate * 2UL)) - 1;
  UBRR0L = baud_prescale; //Load lower 8-bits of the baud rate value
  UBRR0H = baud_prescale >> 8; //Load upper 8-bits
}

void uart_transmit(unsigned char data)
{
  while ( (UCSR0A & 0b00100000) == 0 );
  UDR0 = data;
}

int read_D(int pin)
{
  if((PIND & (1<<pin)) != 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void dfp_send_cmd(byte CMD, byte Par1, byte Par2)
{
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
  Par1, Par2, ((checksum >> 8) & 0x00FF) /*highByte(checksum)*/, (checksum & 0x00FF) /*lowByte(checksum)*/, End_Byte};
  for (byte k=0; k<10; k++)
  {
    uart_transmit( Command_line[k]);
  }
}

void dfp_setup() {
  dfp_send_cmd(0x3F, 0, 0);
  _delay_ms(500);
  dfp_send_cmd(0x06,0,30);
  _delay_ms(2000);
}



//main --------------------------------------------------------------------------

int main()
{
  uart_init(9600);
  DDRD |= 0b00011100;
  dfp_setup();
  while(1)
  {
  button_1_state = read_D(2);
  button_2_state = read_D(3);
  button_3_state = read_D(4);
  if (last_button_1_state != button_1_state && button_1_state == 1)
  {
    playing_track += 1;
    if (playing_track > 10)
    {
      playing_track = 1;
      /*Serial.print("playing track: ");
      Serial.println(playing_track);*/
    }
  }
  if (last_button_2_state != button_2_state && button_2_state == 1)
  {
    playing_track -= 1;
    if (playing_track < 1)
    {
      playing_track = 10;
      /*Serial.print("playing track: ");
      Serial.println(playing_track);*/
    }
  }
  if (last_button_3_state != button_3_state && button_3_state == 1)
  {
    if (is_playing == 1)
    {
      dfp_send_cmd(0x0E,0,0);
      is_playing = 0;
      /*Serial.print("playing state: ");
      Serial.println(is_playing);*/
    }
    else
    {
      dfp_send_cmd(0x03,0,playing_track);
      is_playing = 1;
      /*Serial.print("playing state: ");
      Serial.println(is_playing);*/
    }
  }
  if (last_playing_track != playing_track)
  {
    is_playing = 1;
    dfp_send_cmd(0x03,0,playing_track);
    last_playing_track = playing_track;
    /*Serial.print("playing state: ");
    Serial.println(is_playing);
    Serial.print("playing track: ");
    Serial.println(playing_track);*/
  }
  last_button_1_state = button_1_state;
  last_button_2_state = button_2_state;
  last_button_3_state = button_3_state;
  /*dfp_send_cmd(0x03,0,1);
  _delay_ms(5000);
  dfp_send_cmd(0x03,0,2);
  _delay_ms(5000);*/
  } 
}
