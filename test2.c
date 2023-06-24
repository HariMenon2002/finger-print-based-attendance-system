#include<8052.h>
#include<stdint.h>

#define LCD_PORT 	P2
#define LCD_RS		P3_2
#define LCD_EN		P3_3
#define _baudRate_calc(freq,baud) (256 - ((freq/384) / baud))
#define sw1 P1_0
#define sw2 P1_1
/*ports for switch may change*/
char search_cmd[25]= {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x0A,0x00,0x18};
__code char gen_img[20]   = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x01,0x00,0x05};
__code char img_2_tz[20]   = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x02,0x01,0x00,0x08};
__code char img_2_tz_2[20]  = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x02,0x02,0x00,0x09};
__code char gen_temp[20]  = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x05,0x00,0x09};
 char  store_temp[25]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x06,0x06,0x01,0x00,0x00,0x00,0x0E};


char rx_buff[32];

uint8_t index = 0;

__code char* namelist[]={ "Alan","Shino","Aditya"};

void delay(unsigned int tms){
   uint16_t i,j;
   for(i=0;i<tms;i++)
{
   for(j=0;j<3000;j++);
}

}
void lcd_cmd(uint8_t cmd)
{ 
LCD_RS=0;
  LCD_PORT = cmd;
  LCD_EN=1;
  delay(1);
  LCD_EN = 0;
  
  }

void lcd_data(uint8_t data){
 LCD_RS = 1;
 LCD_PORT =data;

 LCD_EN=1;
 delay(1);
 LCD_EN=0;
 
 
} 

void lcd_str(char *str){
uint8_t  i =0;
while(str[i]!='\0')
{    lcd_data(str[i]);
	i++;
}
}

void lcd_init(void)
{

	LCD_PORT = 0x00;
	LCD_RS = 0;
	delay(10);


	lcd_cmd(0x38);
	lcd_cmd(0x0C);
	lcd_cmd(0x06);
	lcd_cmd(0x01);
	lcd_cmd(0x20);
}
void uart_init(uint32_t OscillatorFrequency,uint32_t baudRate)
{
	 unsigned int autoReloadvalue;
	autoReloadvalue =  _baudRate_calc(OscillatorFrequency,baudRate);
	TMOD  = 0x20;
	SCON  = 0x50;
	TL1    = 0x00;
	TH1    = autoReloadvalue;
	TR1    = 1;
	ES = 1;
	EA = 1;
}

void uart_sendc(char _byte)
{
	EA = 0;
	SBUF = _byte;
	while(!TI);
	TI   = 0;
	EA = 1;
}

 /*void uart_sends(char * str)
{
	uint8_t i = 0;
	while(str[i] != '\0')
	{
		uart_sendc(str[i]);
		i++;
	}
}*/
void uart_irq(void) __interrupt (4)
{
	uint8_t dat;
	EA = 0;
	RI = 0;
	dat = SBUF;
	rx_buff[index++] = dat;
	EA = 1;
}

void tx_packet( char *_data,uint8_t len)
{
	uint8_t k;
	len++;
	for(k=0;k<len;k++)
	{
		uart_sendc(_data[k]);
	}
    
}

void main(void)
{
uint8_t _flag = 0;
uint8_t a;
uint8_t b;
 uint8_t c ;
a=0x30;b=0x30;c=0x30;
P3_4=0;  /*for lcd read/write*/ 

lcd_init();
uart_init(11059200, 9600);
lcd_str("  FPS System  ");

delay(100);
lcd_init();
lcd_str("sw1-enrollment");
lcd_cmd(0xC0);
lcd_str("sw2-attendance");

while(1)
{
	if(sw1==1)
	{       lcd_init();
		lcd_str("Scanning       ");
		index = 0;
		tx_packet(gen_img, 11);
		delay(100);

			if(index >= 11)
			{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Colc Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x02)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Fing det fail ");
				}
				else if(rx_buff[9] == 0x03)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Collect Fail ");
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(img_2_tz, 12);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Gen Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x06)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Dis image    ");
				}
				else if(rx_buff[9] == 0x07)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("smalness err  ");
				}					
				else if(rx_buff[9] == 0x15)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Valid Fail    ");
				}
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;

				tx_packet(gen_img, 11);
				delay(100);

				if(index >= 11)
				{
					if(rx_buff[9] == 0x00)
					{
						_flag = 1;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Colc Success ");
					}
					else if(rx_buff[9] == 0x01)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Error Recvng ");
					}
					else if(rx_buff[9] == 0x02)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Fing det fail ");
					}
					else if(rx_buff[9] == 0x03)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Collect Fail ");
					}
				}
			}			

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(img_2_tz_2, 12);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Gen Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x06)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Dis image ");
				}
				else if(rx_buff[9] == 0x07)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("smalness err  ");
				}					
				else if(rx_buff[9] == 0x15)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Valid Fail    ");
				}
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(gen_temp, 11);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Opr Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x0A)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Fail To Combine");
				}
				}/*brace for index>=11*/
				}/*brace for if flag==1*/

			if (_flag==1)
			{ index=0;
			_flag=0;
			tx_packet(store_temp,12);
			delay(100);

			  if (index>=12)
			    {
			   if(rx_buff[9]==0x00)
			   {_flag=1;
			    lcd_init();
			    lcd_cmd(0xC0);
			    lcd_str("sto success");
			    store_temp[12]+=0x01;
			   }
			  if(rx_buff[9]==0x01)
			  {_flag=0;
			  lcd_init();
			  lcd_cmd(0xC0);
			  lcd_str("error rec");
			  }
			  if(rx_buff[9]==0x18)
			  {_flag=0;
			  lcd_init();
			  lcd_cmd(0xC0);
			  lcd_str("flash error");
			  }
			
                          }


                         }





	} /*brace for switch 1*/

	if(sw2==1)
	{
                lcd_init();
		lcd_str("Scanning       ");
		index = 0;
		tx_packet(gen_img, 11);
		delay(100);

			if(index >= 11)
			{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Colc Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x02)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Fing det fail ");
				}
				else if(rx_buff[9] == 0x03)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Collect Fail ");
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(img_2_tz, 12);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Gen Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x06)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Dis image    ");
				}
				else if(rx_buff[9] == 0x07)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("smalness err  ");
				}					
				else if(rx_buff[9] == 0x15)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Valid Fail    ");
				}
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;

				tx_packet(gen_img, 11);
				delay(100);

				if(index >= 11)
				{
					if(rx_buff[9] == 0x00)
					{
						_flag = 1;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Colc Success ");
					}
					else if(rx_buff[9] == 0x01)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Error Recvng ");
					}
					else if(rx_buff[9] == 0x02)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Fing det fail ");
					}
					else if(rx_buff[9] == 0x03)
					{
						_flag = 0;
						lcd_init();
						lcd_cmd(0xC0);
						lcd_str("Collect Fail ");
					}
				}
			}			

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(img_2_tz_2, 12);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Gen Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x06)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Dis image ");
				}
				else if(rx_buff[9] == 0x07)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("smalness err  ");
				}					
				else if(rx_buff[9] == 0x15)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Valid Fail    ");
				}
				}
			}

			if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(gen_temp, 11);
				delay(100);

				if(index >= 11)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Opr Success ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x0A)
				{
					_flag = 0;
					lcd_init();
					lcd_cmd(0xC0);
					lcd_str("Fail To Combine");
				}
				}/*brace for index>=11*/
				}/*brace for if flag==1*/
		if(_flag == 1) 
			{
				index = 0;
				_flag = 0;
				tx_packet(search_cmd, 16);
				delay(100);

				if(index >= 15)
				{
				if(rx_buff[9] == 0x00)
				{
					_flag = 1;
					lcd_cmd(0xC0);
					lcd_str("Match found  ");
				}
				else if(rx_buff[9] == 0x01)
				{
					_flag = 0;
					lcd_cmd(0xC0);
					lcd_str("Error Recvng ");
				}
				else if(rx_buff[9] == 0x09)
				{
					_flag = 0;
					lcd_cmd(0xC0);
					lcd_str("No matching   ");
				}
				}
			}/*brace for flag==1*/

	          if(_flag==1)
		{ 
		  
		  _flag=0;
		  index=0;
		  switch(rx_buff[11])
		  { case 0x00: {a=a+0x01;
				lcd_init();
				lcd_str(namelist[rx_buff[11]]);
				lcd_cmd(0xC0);
				lcd_data(a);
		                  break;}
		   case  0x01:{ b=b+0x01;
				lcd_init();
				lcd_str(namelist[rx_buff[11]]);
				lcd_cmd(0xC0);
				lcd_data(b);
		                break; }
		    case 0x02: {c=c+0x01;
				lcd_init();
				lcd_str(namelist[rx_buff[11]]);
				lcd_cmd(0xC0);
				lcd_data(c);
		    		break;}
		  }

	        }	  
	}/*brace for switch2*/



}/*brace for while(1)*/


}/*brace for void main()*/