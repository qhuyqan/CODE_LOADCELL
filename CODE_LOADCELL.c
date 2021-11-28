#include <CODE_LOADCELL.h>

#define LCD_ENABLE_PIN PIN_C3
#define LCD_RS_PIN PIN_C1
#define LCD_RW_PIN PIN_C2
#define LCD_DATA4 PIN_D0
#define LCD_DATA5 PIN_D1
#define LCD_DATA6 PIN_D3
#define LCD_DATA7 PIN_D2

#include <lcd.c>

#define HX711_DATA PIN_B2
#define HX711_SCK  PIN_B1

const float weight_noise = 3.072387667;

int32 read_hx711(void)                       
{
   unsigned int32 data,val = 0;
   unsigned int8 i,j;

   for(i=0;i<10;i++)
   {
      output_bit( HX711_DATA, 1);
      output_bit( HX711_SCK, 0);
      data=0;
      
      while(input(HX711_DATA));
      for (j=0;j<24;j++)                     // gain 128
      {
         output_bit( HX711_SCK, 1);
         data = data<<1;
         output_bit( HX711_SCK, 0);
         if(input(HX711_DATA)) 
         {
            data++;                          //serial data 24 bit
         }
      }
      output_bit( HX711_SCK, 1);
      data = data ^ 0x800000;
      val = val + data;                      //10 times weight_val
      output_bit( HX711_SCK, 0);
   }
   val = val / 10;                           //average
   return (val);
}

float weight_val(unsigned int32 offset_val,unsigned int32 read_val)
{
   float val = 0,noise = 0;
   
   if(offset_val >= read_val)
   {
      read_val = (offset_val - read_val);             //weight >= 0
      val = read_val / 1000;                             //mg ---> g
      noise = val * weight_noise;
      val = val - noise;
   }
   else
   {     
      read_val = (read_val - offset_val);             //weight < 0
      val = read_val / 1000;                             //mg ---> g
      noise = val * weight_noise;
      val = (val - noise) * (-1);
   }
   return (val);
}

void weight_display(float weight_val)
{
   lcd_putc("\a");   
   lcd_gotoxy(1,2);
   printf(lcd_putc, "W = %4.0f  g", weight_val);
}

void send_to_com(float weight_val){
   printf("%4.0f", weight_val);
}

void main()
{
   unsigned int32 read_weight = 0,offset = 0;            
   float weight = 0;
   
   lcd_init();
   delay_ms(500);
   lcd_putc("\f");
   lcd_gotoxy(1,1);
   lcd_putc("  CAN DIEN TU");   
   
   offset = read_hx711(); 
   while(TRUE)
   {
      read_weight = read_hx711();
//!      lcd_gotoxy(1,1);
//!      printf(lcd_putc, "Offset = %ld", offset);
//!      lcd_gotoxy(1,2);
//!      printf(lcd_putc, "HX711 = %ld", read_weight);
     
      weight = weight_val(offset, read_weight);       
      delay_us(100);
      weight_display(weight); 
      send_to_com(weight);
      delay_ms(500);     
   }

}
