#include <odev2.h>
#define LCD_ENABLE_PIN PIN_D0
#define LCD_RS_PIN PIN_D1
#define LCD_RW_PIN PIN_D2
#define LCD_DATA4 PIN_D4
#define LCD_DATA5 PIN_D5
#define LCD_DATA6 PIN_D6
#define LCD_DATA7 PIN_D7
#define sht_data_pin   PIN_b1//(Data pin veya input)
#define sht_clk_pin    PIN_b0
#include <lcd.c>

//***** SHT75 Ba�latma *****
 
void comstart (void)
{
 output_float(sht_data_pin);  //data high
 output_bit(sht_clk_pin, 0);  //clk low
 delay_us(1);
 output_bit(sht_clk_pin, 1);  //clk high
 delay_us(1);
 output_bit(sht_data_pin, 0); //data low
 delay_us(1);
 output_bit(sht_clk_pin, 0);  //clk low
 delay_us(1);
 output_bit(sht_clk_pin, 1);  //clk high
 delay_us(1);
 output_float(sht_data_pin);  //data high
 delay_us(1);
 
 
 
 
}
 
//***** SHT75 yazma fonksiyonu*****
 
int1 comwrite (int8 iobyte)
{
 int8 i, mask = 0x80;
 int1 ack;
 
 //Komut g�nderilir
 delay_us(4);
 for(i=0; i<8; i++)
  {
   output_bit(sht_clk_pin, 0);                          //clk low
   if((iobyte & mask) > 0) output_float(sht_data_pin);  //data high if MSB high
   else output_bit(sht_data_pin, 0);                    //data low if MSB low
   delay_us(1);
   output_bit(sht_clk_pin, 1);                          //clk high
   delay_us(1);
   mask = mask >> 1;                                    //sonraki bit'e ge�
  }
 
 //Shift in ack
 output_bit(sht_clk_pin, 0);  //clk low
 delay_us(1);
 ack = input(sht_data_pin);   //get ack bit
 output_bit(sht_clk_pin, 1);  //clk high
 delay_us(1);
 output_bit(sht_clk_pin, 0);  //clk low
 return(ack);
}
 
//***** SHT75'ten veri okuma fonksiyonu *****
 
int16 comread (void)
{
 int8 i;
 int16 iobyte = 0;
 const int16 mask0 = 0x0000;
 const int16 mask1 = 0x0001;
 
 //shift in MSB data
 for(i=0; i<8; i++)
  {
   iobyte = iobyte << 1;
   output_bit(sht_clk_pin, 1);                //clk high
   delay_us(1);
   if (input(sht_data_pin)) iobyte |= mask1;  //shift in data bit
   else iobyte |= mask0;
   output_bit(sht_clk_pin, 0);                //clk low
   delay_us(1);
  }
 
 //send ack 0 bit
 output_bit(sht_data_pin, 0); //data low
 delay_us(1);
 output_bit(sht_clk_pin, 1);  //clk high
 delay_us(2);
 output_bit(sht_clk_pin, 0);  //clk low
 delay_us(1);
 output_float(sht_data_pin);  //data high
 
 //shift in LSB data
 for(i=0; i<8; i++)
  {
   iobyte = iobyte << 1;
   output_bit(sht_clk_pin, 1);                //clk high
   delay_us(1);
   if (input(sht_data_pin)) iobyte |= mask1;  //shift in data bit
   else iobyte |= mask0;
   output_bit(sht_clk_pin, 0);                //clk low
   delay_us(1);
  }
 
 //send ack 1 bit
 output_float(sht_data_pin);  //data high
 delay_us(1);
 output_bit(sht_clk_pin, 1);  //clk high
 delay_us(2);
 output_bit(sht_clk_pin, 0);  //clk low
 
 return(iobyte);
}
 
//***** SHT75 okumas�n� beklemek i�in fonksiyon *****
 
void comwait (void)
{
 int16 sht_delay;
 
 output_float(sht_data_pin);                     //data high
 output_bit(sht_clk_pin, 0);                     //clk low
 delay_us(1);
 for(sht_delay=0; sht_delay<30000; sht_delay++)  // wait for max 300ms
  {
   if (!input(sht_data_pin)) break;              //if sht_data_pin low, SHT75 ready
   delay_us(10);
  }
}
 
//***** SHT75 ileti�imini s�f�rlama i�levi *****
 
void comreset (void)
{
 int8 i;
 
 output_float(sht_data_pin);    //data high
 output_bit(sht_clk_pin, 0);    //clk low
 delay_us(2);
 for(i=0; i<9; i++)
  {
   output_bit(sht_clk_pin, 1);  //toggle clk 9 times
   delay_us(2);
   output_bit(sht_clk_pin, 0);
   delay_us(2);
 }
 comstart();
}
 
//***** SHT75 s�f�rlama i�levi *****
 
void sht_soft_reset (void)
{
 comreset();           //SHT75 communication reset
 comwrite(0x1e);       //send SHT75 reset command
 delay_ms(15);         //pause 15 ms
}
 
//***** SHT75 s�cakl���n� �l�me fonksiyonu *****
 
int16 measuretemp (void)
{
 int1 ack;
 int16 iobyte;
 
 comstart();             //alert SHT75
 ack = comwrite(0x03);   //send measure temp command and read ack status
 if(ack == 1) return;
 comwait();              //wait for SHT75 measurement to complete
 iobyte = comread();     //read SHT75 temp data
 return(iobyte);
}
 
//***** SHT75 RH(nem)'yi �l�me fonksiyonu *****
 
int16 measurehumid (void)
{
 int1 ack;
 int16 iobyte;
 
 comstart();            //alert SHT75
 ack = comwrite(0x05);  //send measure RH command and read ack status
 if(ack == 1) return;
 comwait();             //wait for SHT75 measurement to complete
 iobyte = comread();    //read SHT75 temp data
 return(iobyte);
}
 
//***** SHT75 temp & RH'yi hesaplama fonksiyonu *****
 
void calculate_data (int16 temp, int16 humid, float & tc, float & rhlin, float & rhtrue)
{
 float truehumid, rh;
 
 //calculate temperature reading
 tc = ((float) temp * 0.01) - 40.0;
 
 //calculate Real RH reading
 rh = (float) humid;
 
 rhlin = (rh * 0.0405) - (rh * rh * 0.0000028) - 4.0;
 
 //calculate True RH reading
 rhtrue = ((tc - 25.0) * (0.01 + (0.00008 * rh))) + rhlin;
}
 
//***** SHT75 s�cakl��� ve RH'yi �l�en ve hesaplayan fonksiyon*****
 
void sht_rd (float & temp, float & truehumid)
{
 int16 restemp, reshumid;
 float realhumid;
 restemp = 0; truehumid = 0;
 
 restemp = measuretemp();    //measure temp
 reshumid = measurehumid();  //measure RH
calculate_data (restemp, reshumid, temp, realhumid, truehumid);  //calculate temp & RH
}
 
//***** SHT75'i a��l��ta ba�latma i�levi *****
 
void sht_init (void)
{
 comreset();    //reset SHT75
 delay_ms(20);  //delay for power-up
}
void main()
{
 float restemp, truehumid;
 lcd_init();
sht_init();
 
while(1)
 {
  sht_rd (restemp, truehumid);
  printf(lcd_putc,"Temp : %3.1f %cC   ", restemp, 223);
  printf(lcd_putc,"\nRH   : %3.1f %%   ", truehumid);
  delay_ms(500);        //sens�r�n kendi kendine �s�nmas�n� �nlemek i�in okuma aras�nda 500 ms gecikme
 }
}
