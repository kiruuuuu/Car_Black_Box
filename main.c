/* Name : C kiran kumar
   Date : 21/11/2024
 */


#include <xc.h>
#include "main.h"
#include "matrix_keypad.h"
#include "clcd.h"
#include "adc.h"
#include"eeprom.h"
#include "ds1307.h"
#include "i2c.h"
#include "external_eeprom.h"
#include "uart.h"

// Arrays for storing event data and menu options
unsigned char data[9][3] = {"ON", "GN", "GR", "G1", "G2", "G3", "G4", "G5", " C"};
unsigned char option[4][13] = {"VIEW LOG", "SET TIME", "DOWNLOAD LOG", "CLEAR LOG"};

// Global variables for system state management
unsigned int count = 0; // Event count
unsigned int opt_count = 2; // Option menu count
unsigned int flag = 0; // Flag for log read/write
unsigned int sp; // Speed value
unsigned int wait = 0x00; // EEPROM address counter
unsigned int dashboard_flag = 0; // State for dashboard/menu
unsigned int key_count = 0, key_count1 = 0, key2_count = 0; // Key press counters
unsigned int menu_count = 0; // Menu navigation index
unsigned int value = 0, addr = 0x00; // EEPROM addressing variables
unsigned int dash_count = 0; // Dashboard log count
unsigned char time[9]; // Time buffer
unsigned char clock_reg[3]; // Clock registers
unsigned char date[11]; // Date buffer
unsigned char eeprom_data[10][15]; // EEPROM log buffer
unsigned int d_count = 0, value2 = 0, temp; // Log tracking variables
unsigned char Hr = 0, M = 0, S = 0; // Time variables
unsigned int wait1 = 0, wait2 = 0; // Blinking wait counters
unsigned int key2_flag = 0; // Key flag for time setting
unsigned char key;

// Function to display the current time on the LCD
void display_time(void)
{
	clcd_print(time, LINE2(0));      
}


// Function to retrieve the current time from the RTC module
static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{
        //THis for 12 hrs format
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);  //we are extracting 4th bit 
		time[1] = '0' + (clock_reg[0] & 0x0F);         //we are extracting 0-3 bit   
	}
	else
	{
         //THis for 24 hrs format
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);   //we are extracting 4 and 5th  bit 
		time[1] = '0' + (clock_reg[0] & 0x0F);          //we are extracting 0-3 bit 
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}

// Function to store event data and time in EEPROM
void storing() {                        
    flag=0;                           
    if (wait == 0x64){  // Reset write address after reaching limit
        wait = 0x00;
    }
    int i = 0;
    while (time[i] != '\0') {      // Skip colons
        if (i == 2 || i == 5) {
            i++;
        }
        write_external_eeprom(wait++, time[i]);
        i++;
    }
    write_external_eeprom(wait++, data[count][0]);   // Store event data
    write_external_eeprom(wait++, data[count][1]);
    write_external_eeprom(wait++, '0' + (sp / 10));    // Speed tens place
    write_external_eeprom(wait++, '0' + (sp % 10));    // Speed units place
    dash_count++;
    
}

// Function to display and navigate the main menu
void main_menu(unsigned char key1) {    
         if (menu_count == 0)
         {
            clcd_putch('*', LINE1(0));
            clcd_putch(' ', LINE2(0));
            clcd_print(option[0], LINE1(1));
            clcd_print(option[1], LINE2(1));
         }
    if(key1 == 1) {   // Navigate up
         CLEAR_DISP_SCREEN;
        if( menu_count >= 1 &&  menu_count <=3)
        {
            clcd_print(option[menu_count], LINE2(1));
        }
        if(menu_count>=0)
        {
            menu_count--;
        }
        clcd_putch(' ',LINE2(0));
        clcd_putch('*',LINE1(0));
        if(menu_count >= 0 && menu_count < 3)
            clcd_print(option[menu_count], LINE1(1));
    }
    else if (key1 == 2) {   // Navigate down
        // CLEAR_DISP_SCREEN;
        if(menu_count < 3)
        {
            clcd_print(option[menu_count], LINE1(1));
        }
        if(menu_count < 3)
        {
            menu_count++;
        }
        clcd_putch(' ',LINE1(0));
        clcd_putch('*',LINE2(0));
        if(menu_count >=1 && menu_count < 4)
        {
            clcd_putch('*',LINE2(0));
            clcd_print(option[menu_count], LINE2(1));
        } else if (menu_count == 2) {
            clcd_putch('*',LINE2(0));
           clcd_print(option[menu_count], LINE2(1));
        }
    }

    if(key1 == 11)   // Select menu option
    {
        CLEAR_DISP_SCREEN;
        dashboard_flag=2;   // View log
    }
    if(key1 == 11 && menu_count == 1)
    {
        menu_count=0;
        CLEAR_DISP_SCREEN;
        dashboard_flag=5;     // Set time
        
    }
    if(key1 == 11 && menu_count == 2)
    {
        menu_count=0;
        CLEAR_DISP_SCREEN;
        dashboard_flag=3;   // Download log
    }
     if(key1 == 11 && menu_count == 3)
    {
         menu_count=0;
        CLEAR_DISP_SCREEN;
        dashboard_flag=4;   // Clear log
        
    }
    if(key == 12)           // Exit menu
    {
        CLEAR_DISP_SCREEN;
        dashboard_flag=0;
    }
}
void log(char data[10][15])     //This function for getting data from EEPROM to our 2d array
{
    unsigned char addr1 = 0x00;
   
    
    for(int i = 0; i < 10 ; i++)
    {
        for(int j = 0; j  < 14 ; j++)
        {
            if(j == 2 || j == 5)
            {
                data[i][j] = ':';
            }
            else if (j == 8 || j == 11)
            {
                data[i][j] = ' ';
            }
            else
                data[i][j] =read_external_eeprom(addr1++);
        }
        data[i][14] = '\0';
    }

}
void set_time(unsigned char key2)      //This for set time 
{
     clcd_print("HH:MM:SS",LINE1(0));
     if(wait2 == 0)
     {
            Hr =  ((time[0] - '0')*10) + (time[1] - '0');      // here i am getting hr,min,sec from real time clock
            M  =  ((time[3] - '0')*10) +(time[4] - '0');    
            S  =  ((time[6] - '0')*10) +(time[7] - '0');
            wait2++;
     }
     if(key2 == 2)
     {
         key2_flag=key2_flag+1;
     }
     if(key2_flag == 0)
     {
           if(key2 == 1)
             {
               Hr=Hr+1;                 //here i am incrementing the hour
             if(Hr == 24)
                 Hr=0;
             }
           
     }
     else if( key2_flag == 1)
         {
             if(key2 == 1)
             {
                 M=M+1;                   //here i am incrementing the min
               if(M == 60)
                 M=0;
             }
         }
     else if( key2_flag==2)
         {
              if(key2 == 1)
             {
                  S=S+1;                 //  //here i am incrementing the sec
                 if(S == 60)
                  S=0;
              }
         }
     else
     {
        key2_flag=0; 
      }
     
     if(wait1++ <200)                    //This logic for blinking hour min sec values
     {
     clcd_putch(Hr/10 + '0',LINE2(0));
     clcd_putch(Hr%10 +'0',LINE2(1));
     clcd_putch(':',LINE2(2));
     clcd_putch(M/10 + '0',LINE2(3));
     clcd_putch(M%10 + '0',LINE2(4));
     clcd_putch(':',LINE2(5));
     clcd_putch(S/10 + '0',LINE2(6));
     clcd_putch(S%10 + '0',LINE2(7));
     }
     else if(wait1 > 200 && wait1 < 500)
     {
        if(key2_flag == 0)
            clcd_print("  ",LINE2(0));
        if(key2_flag == 1)
            clcd_print("  ",LINE2(3));
        if(key2_flag == 2)
            clcd_print("  ",LINE2(6));
     }
     else
         wait1=0;
     
     if(key2 == 11)
     {
         write_ds1307(HOUR_ADDR, ((Hr/10) << 4) | Hr%10);
         write_ds1307(MIN_ADDR, ((M/10) << 4) | M%10);
         write_ds1307(SEC_ADDR, ((S/10) << 4) | S%10);
         for(unsigned long int i=250000;i--;);
         dashboard_flag=0;
         menu_count=0;
         key2_flag=0;
         CLEAR_DISP_SCREEN;
     }
     if(key2 == 12)
    {
        CLEAR_DISP_SCREEN;
        dashboard_flag=1;
         menu_count=0;
        key2_flag=0;
    }
}
void clear_log()
{
     dash_count=0;                        //Here i am clearing all the logs which i updated
     wait=0;
     clcd_print("Clear Log",LINE1(0));
     clcd_print("SUCCESSFULL",LINE2(0));
     for(unsigned long int i=250000;i--;);
     dashboard_flag=1;
     menu_count=0;
     CLEAR_DISP_SCREEN;
}

void download_log()
{
    char str1[16]="N0  TIME   EV SP";
    str1[16]='\0';
    puts(str1);
    puts("\n\r");
     if(dash_count == 0)
    {
           clcd_print("NO LOGS :", LINE1(0));
           clcd_print("DOWNLOADS :", LINE2(3));
           for(unsigned long int i=250000;i--;);
             dashboard_flag=1;
              menu_count=0;
             CLEAR_DISP_SCREEN;
             return;
    }
    d_count++;
    if ( flag == 0 )
    {
       log(eeprom_data);
        flag=1;
    }
    if(flag == 1)
    {
        if(d_count == 1)
        {
            if(dash_count>10)
            {
                value2=dash_count%10;
                temp = 10;
            }
            else
            {
                temp=dash_count;
                value2=0;
            }
     for(int i=0;i<temp;i++)
     {
      putch(i + '0');
      putch(' ');
      puts(eeprom_data[(i+value2)%10]);       //here i am displaying our logs into tera term
      puts("\n\r");
     }
     }
    }
    
     clcd_print("SUCCESSFULL",LINE2(0));
     for(unsigned long int i=250000;i--;);
     dashboard_flag=1;    //this is for returning to main menu
     menu_count=0;
     CLEAR_DISP_SCREEN;
     
 
     
}
void view_log(unsigned char key2)    //this logic for view the logs
{
     clcd_print("#", LINE1(0));
    clcd_print("VIEW LOG :", LINE1(4));
     if(dash_count == 0)
    {
           clcd_print("NO LOGS :", LINE1(0));
           clcd_print("DISPLAY :", LINE2(3));
           for(unsigned long int i=250000;i--;);
             dashboard_flag=1;                     //this is for returning to main menu
              menu_count=0;
             CLEAR_DISP_SCREEN;
             return;
    }
    if(flag == 0)
    {
    log(eeprom_data);
    flag=1;
    }
    
     if(key2 == 1)
    {
        key2_count--;
        if(key2_count == -1)
            key2_count=0;
    }
    if( key2 == 2)
    {
        
        if(key2_count >= dash_count-1)
        {
            key2_count = dash_count-1;
        }
        else
        {
            key2_count++;
        }
        if(key2_count == 10)
            key2_count = 9;
    }
    if(dash_count > 10)
    {
    value= dash_count%10;
    }
    else
    {
        value=0;
    }
    clcd_putch(key2_count + '0' ,LINE2(0));
    clcd_print(eeprom_data[(key2_count+value)%10],LINE2(2));
    
    if(key2 == 12)
    {
        CLEAR_DISP_SCREEN;
        dashboard_flag=1;
        key2_count = 0;
        value=0;
    }

}

void dashboard(unsigned char key) {
    get_time();
	display_time();
    unsigned int adc_reg_val;
    clcd_print("  TIME", LINE1(0));
    clcd_print("EV", LINE1(10));
    clcd_print("SP", LINE1(14));
        if (key == 1) {
            
               
            if (count == 8) {
                count = 1;
            } else if (count <= 6) {
                 
                count++;
                storing();
            }
            
        }   
        if (key == 2) {
            if (count == 8) {
                count = 1;
            } else if (count > 1) {
                count--;
                storing();
            } else if(count == 0){
                count = 0;
            }
            else
            {
                count=1;
            }
        }
        if (count < 9 && count >= 0) {
            clcd_print(data[count], LINE2(10));
        } else {
            if (count != 8) {
               count = 0;
            }
        }
    
    if (key == 3 && count != 8 ) {
        count = 8;
        storing();
    }
    adc_reg_val = read_adc(CHANNEL4);
    sp = adc_reg_val / 10;
    if (sp > 99) {
        sp = 99;
    }
    clcd_putch('0' + (sp / 10), LINE2(14));           //storing character by character in clcd
    clcd_putch('0' + (sp % 10), LINE2(15));
    if (key == 11) {
         CLEAR_DISP_SCREEN;
         dashboard_flag = 1;      //going to main menu  u
    }
    
}

void init_config(void) {
    init_matrix_keypad();
    init_clcd();
    init_adc();
    init_i2c();
	init_ds1307();
    init_uart(); 
    clcd_print("ON", LINE2(10));
}

void main(void) {
    init_config();
    while (1) {
        key = read_switches(STATE_CHANGE);
        if (dashboard_flag == 0) {
            dashboard(key);
        } else if(dashboard_flag == 1){
            main_menu(key);
        }
        else if(dashboard_flag == 2)
        {
            view_log(key);
            
        }
        else if(dashboard_flag == 4)
        {
            clear_log();
        }
        else if(dashboard_flag == 3)
        {
            download_log();
        }
        else if(dashboard_flag == 5)
        {
            set_time(key);
        }
    }
}
