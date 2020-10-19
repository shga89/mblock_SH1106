//
//  sh1106.cpp
//  
//
//  Created by Daniel Milligan on 8/3/14.
//
//
#include <stdio.h>

// Pull in the wire library to communicate with the LCD over I2C
#include <Wire.h>

#include "sh1106.h"

typedef enum
{
    HORIZONTAL = 0,
    VERTICAL,
    PAGE,
    INVALID,
    END_MEMORY_ADDRESSING_MODES
} MEMORY_ADDRESSING_MODES;

typedef enum
{
    Success = 0,
    DataTooLong,
    NackAddress,
    NackData,
    OtherError,
    END_I2C_STATUS
} I2CStatus;

/* Each character will be an 8x8 character with one space blank to the left and bottom */
#define CHARACTER_WIDTH 8


byte font[][CHARACTER_WIDTH]{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*SPACE*/
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00}, /*  !  */
    {0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00}, /*  "  */
    {0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, 0x00}, /*  #  */
    {0x00, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, 0x00}, /*  $  */
    {0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00}, /*  %  */
    {0x00, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00}, /*  &  */
    {0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00}, /*  '  */
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x00, 0x00}, /*  (  */
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00}, /*  )  */
    {0x00, 0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00, 0x00}, /*  *  */
    {0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00}, /*  +  */
    {0x00, 0xA0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00}, /*  ,  */
    {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00}, /*  -  */
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00}, /*  .  */
    {0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00}, /*  /  */
    {0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00}, /*  0  */
    {0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00}, /*  1  */
    {0x00, 0x62, 0x51, 0x49, 0x49, 0x46, 0x00, 0x00}, /*  2  */
    {0x00, 0x22, 0x41, 0x49, 0x49, 0x36, 0x00, 0x00}, /*  3  */
    {0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00}, /*  4  */
    {0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00}, /*  5  */
    {0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00}, /*  6  */
    {0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00}, /*  7  */
    {0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00}, /*  8  */
    {0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00}, /*  9  */
    {0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00}, /*  :  */
    {0x00, 0x00, 0xAC, 0x6C, 0x00, 0x00, 0x00, 0x00}, /*  ;  */
    {0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00}, /*  <  */
    {0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00}, /*  =  */
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00}, /*  >  */
    {0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00}, /*  ?  */
    {0x00, 0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, 0x00}, /*  @  */
    {0x00, 0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00, 0x00}, /*  A  */
    {0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00}, /*  B  */
    {0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00}, /*  C  */
    {0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00}, /*  D  */
    {0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00}, /*  E  */
    {0x00, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00}, /*  F  */
    {0x00, 0x3E, 0x41, 0x41, 0x51, 0x72, 0x00, 0x00}, /*  G  */
    {0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00}, /*  H  */
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00, 0x00}, /*  I  */
    {0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 0x00}, /*  J  */
    {0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, /*  K  */
    {0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00}, /*  L  */
    {0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00, 0x00}, /*  M  */
    {0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x00}, /*  N  */
    {0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00}, /*  O  */
    {0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00}, /*  P  */
    {0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 0x00}, /*  Q  */
    {0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00}, /*  R  */
    {0x00, 0x26, 0x49, 0x49, 0x49, 0x32, 0x00, 0x00}, /*  S  */
    {0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x00}, /*  T  */
    {0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 0x00}, /*  U  */
    {0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 0x00}, /*  V  */
    {0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 0x00}, /*  W  */
    {0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00}, /*  X  */
    {0x00, 0x03, 0x04, 0x78, 0x04, 0x03, 0x00, 0x00}, /*  Y  */
    {0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00}, /*  Z  */
    {0x00, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00}, /*  [  */
    {0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00}, /*  \  */
    {0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 0x00, 0x00}, /*  ]  */
    {0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00}, /*  ^  */
    {0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00}, /*  _  */
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00}, /*  `  */
    {0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00}, /*  a  */
    {0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00}, /*  b  */
    {0x00, 0x38, 0x44, 0x44, 0x28, 0x00, 0x00, 0x00}, /*  c  */
    {0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, 0x00}, /*  d  */
    {0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00}, /*  e  */
    {0x00, 0x08, 0x7E, 0x09, 0x02, 0x00, 0x00, 0x00}, /*  f  */
    {0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C, 0x00, 0x00}, /*  g  */
    {0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00}, /*  h  */
    {0x00, 0x00, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00}, /*  i  */
    {0x00, 0x80, 0x84, 0x7D, 0x00, 0x00, 0x00, 0x00}, /*  j  */
    {0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00}, /*  k  */
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, /*  l  */
    {0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00}, /*  m  */
    {0x00, 0x7C, 0x08, 0x04, 0x7C, 0x00, 0x00, 0x00}, /*  n  */
    {0x00, 0x38, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00}, /*  o  */
    {0x00, 0xFC, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00}, /*  p  */
    {0x00, 0x18, 0x24, 0x24, 0xFC, 0x00, 0x00, 0x00}, /*  q  */
    {0x00, 0x00, 0x7C, 0x08, 0x04, 0x00, 0x00, 0x00}, /*  r  */
    {0x00, 0x48, 0x54, 0x54, 0x24, 0x00, 0x00, 0x00}, /*  s  */
    {0x00, 0x04, 0x7F, 0x44, 0x00, 0x00, 0x00, 0x00}, /*  t  */
    {0x00, 0x3C, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00}, /*  u  */
    {0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 0x00}, /*  v  */
    {0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, 0x00}, /*  w  */
    {0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00}, /*  x  */
    {0x00, 0x1C, 0xA0, 0xA0, 0x7C, 0x00, 0x00, 0x00}, /*  y  */
    {0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x00}, /*  z  */
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x00}, /*  {  */
    {0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00}, /*  |  */
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00, 0x00, 0x00}, /*  }  */
    {0x00, 0x02, 0x01, 0x01, 0x02, 0x01, 0x00, 0x00}, /*  ~  */
    {0x00, 0x02, 0x05, 0x05, 0x02, 0x00, 0x00, 0x00}  /* DEL */
};



sh1106_lcd *sh1106_lcd::m_pInstance = NULL;

sh1106_lcd *sh1106_lcd::getInstance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = (sh1106_lcd *)malloc(sizeof(sh1106_lcd));
        m_pInstance->Initialize();
    }
    return m_pInstance;
}

void sh1106_lcd::Initialize()
{
    Wire.begin(); // begin without address as a master device
    
    SendCommand(DISPLAY_OFF, StartSend);
    SendCommand(SET_MEMORY_ADDRESSING_MODE, MidSend);
    SendCommand(PAGE, MidSend);
    SendCommand(SET_PAGE_ADDRESS, MidSend); // start at page address 0
    SendCommand(SET_COM_OUTPUT_SCAN_DIRECTION, MidSend);
    SendCommand(LOW_COLUMN_ADDRESS, MidSend);
    SendCommand(HIGH_COLUMN_ADDRESS, MidSend);
    SendCommand(START_LINE_ADDRESS, MidSend);
    SendCommand(SET_CONTRAST_CTRL_REG, MidSend);
    SendCommand(0x7F, MidSend);
    SendCommand(SET_SEGMENT_REMAP, MidSend);
    SendCommand(SET_NORMAL_DISPLAY, MidSend);
    SendCommand(SET_MULTIPLEX_RATIO, MidSend);
    SendCommand(0x3F, MidSend);
    SendCommand(OUTPUT_FOLLOWS_RAM, MidSend);
    SendCommand(SET_DISPLAY_OFFSET, MidSend);
    SendCommand(0x00, MidSend); // no offset
    SendCommand(SET_DISPLAY_CLOCK_DIVIDE, MidSend);
    SendCommand(0xF0, MidSend);
    SendCommand(SET_PRE_CHARGE_PERIOD, MidSend);
    SendCommand(0x22, MidSend);
    SendCommand(SET_COM_PINS_HARDWARE_CONFIG, MidSend);
    SendCommand(0x12, MidSend);
    SendCommand(SET_VCOMH, MidSend);
    SendCommand(0x20, MidSend); // 0.77xVcc
    SendCommand(SET_DC_DC_ENABLE, MidSend);
    SendCommand(0x14, MidSend);
    
    SendCommand(DISPLAY_ON, FinishSend);
    
    memset(m_screen, 0, SCREEN_WIDTH * MAX_PAGE_COUNT);
    m_currentLine = 0;
    m_cursor = 1;
}

void sh1106_lcd::Show()
{
    SendState state = StartSend;
    
    for (int index = 0; index < MAX_PAGE_COUNT; index++)
    {
        SendCommand(SET_PAGE_ADDRESS + index, StartSend);
        SendCommand(0x00, MidSend); // low column start address
        SendCommand(0x10, FinishSend); // high column start address
        for (int pixel = 0; pixel < SCREEN_WIDTH; pixel++)
        {
            SendData(m_screen[index][pixel]);
            
            if (state == StartSend)
            {
                state = MidSend;
            }
            else if (pixel == (SCREEN_WIDTH - 2))
            {
                state = FinishSend;
            }
        }
        state = StartSend;
    }
}

void sh1106_lcd::FillScreen(byte fillData)
{
    for (int index = 0; index < MAX_PAGE_COUNT; index++)
    {
        for (int pixel = 0; pixel < SCREEN_WIDTH; pixel++)
        {
            m_screen[index][pixel] = fillData;
        }
    }

    m_currentLine = 0;
    m_cursor = 1;
    
    Show();
}

void sh1106_lcd::ClearScreen()
{
    FillScreen(0x00);
}

/* 
 * x can be from 0 to 131
 * y can be from 0 to 63
 *
 * Show must be called after all pixel's are drawn
 */
void sh1106_lcd::DrawPixel(byte x, byte y, bool on)
{
    byte pageId = y / MAX_PAGE_COUNT;       // convert from byte to page
    byte bitOffset = y % MAX_PAGE_COUNT;    // establish the bit offset
    
    if (pageId < MAX_PAGE_COUNT)
    {
        if (x < SCREEN_WIDTH)
        {
            if (on == true)
            {
                m_screen[pageId][x] |= 1 << bitOffset; // turn this bit on
            }
            else
            {
                m_screen[pageId][x] &= ~(1 << bitOffset); // turn this bit off
            }
        }
    }
}

void sh1106_lcd::DrawRectangle(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);
    
    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }
        
        if (x2 >= SCREEN_WIDTH)
        {
            x2 = SCREEN_WIDTH - 1;
        }
        
        if (x1 == x2)
        {
            x2++; // give at least one
        }
        // Sets top and bottom line
        for (byte xCord = x1; xCord < x2; xCord++)
        {
            m_screen[pageId1][xCord] |= bit1;
            m_screen[pageId2][xCord] |= bit2;
        }
        
        if (y2 < y1)
        {
            byte temp = y1;
            y1 = y2;
            y2 = temp;
        }
        
        // Sets left and right line
        for (/* set above */; y1 < y2; y1++)
        {
            m_screen[pageId1][x1] |= bit1;
            m_screen[pageId1][x2] |= bit1;
            bit1 <<= 1;
            if (bit1 == 0)
            {
                bit1++;
                    
                pageId1++; // move to next page we just rolled
                    
                if (pageId1 >= MAX_PAGE_COUNT)
                {
                    break;
                }
            }
        }
    }
}

void sh1106_lcd::DrawRectangle(byte x1, byte y1, byte x2, byte y2, byte thickness)
{
    // Cheat and call it a couple of times
    // otherwise my head hurts tring to get the bits right...
    for (byte index = 0; index < thickness; index++)
    {
        DrawRectangle(x1, y1, x2, y2);
        x1++;
        x2--;
        y1++;
        y2--;
        
        if (x1 >= SCREEN_WIDTH || x2 >= SCREEN_WIDTH || y1 >= SCREEN_HEIGHT || y2 >= SCREEN_HEIGHT)
        {
            break;
        }
    }
}

void sh1106_lcd::FillRectangle(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);
    
    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }
        
        if (x2 >= SCREEN_WIDTH - 1)
        {
            x2 = SCREEN_WIDTH;
        }
        
        if (x1 == x2)
        {
            x2++;
        }
        
        if (y2 < y1)
        {
            byte temp = y1;
            y1 = y2;
            y2 = temp;
        }
        
        for (/* set above */; y1 < y2; y1++)
        {
            for (byte xCord = x1; xCord < x2; xCord++)
            {
                m_screen[pageId1][xCord] |= bit1;
            }
                
            bit1 <<= 1;
            if (bit1 == 0)
            {
                bit1++;

                pageId1++; // move to next page we just rolled
                    
                if (pageId1 >= MAX_PAGE_COUNT)
                {
                    break;
                }
            }
        }
    }
}

void sh1106_lcd::DrawLine(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);
    byte dy = y2 - y1;
    byte dx = x2 - x1;
    
    if (x1 > x2)
    {
        dx = x1 - x2;
        
        byte temp = x1;
        x1 = x2;
        x2 = temp;
    }
    
    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }
        
        if (x2 >= SCREEN_WIDTH)
        {
            x2 = SCREEN_WIDTH - 1;
        }
        
        if (x1 == x2)
        {
            x2++; // give us one pixel width
        }
        
        /* Utilizing the Bresenham algorithm */
        int eps = 0;
            
        if (y1 < y2)
        {
            for (/* set above */; y1 < y2; y1++)
            {
                for (byte xCord = x1; xCord < x2; xCord++)
                {
                    m_screen[pageId1][xCord] |= bit1;
                    
                    eps += dy;
                    
                    if ((eps << 1) >= dx)
                    {
                        y1++;
                        bit1 <<= 1;
                        if (bit1 == 0)
                        {
                            pageId1++; // move to next page we just rolled
                                
                            bit1++; // back to bit 1 equal to 1
                
                            if (pageId1 >= MAX_PAGE_COUNT)
                            {
                                break; // done
                            }
                        }
                        eps -= dx;
                    }
                }
            }
        }
        else if (y1 == y2)
        {
            for (byte xCord = x1; xCord < x2; xCord++)
            {
                m_screen[pageId1][xCord] |= bit1;
            }
        }
        else
        {
            dy = y1 - y2;
            for (/* set above */; y1 > y2; y1--)
            {
                for (byte xCord = x1; xCord < x2; xCord++)
                {
                    m_screen[pageId2][xCord] |= bit2;
                        
                    eps += dy;
                        
                    if ((eps << 1) >= dx)
                    {
                        y1--;
                        bit2 >>= 1;
                            
                        if (bit2 == 0)
                        {
                            pageId2--; // move to next page we just rolled
                                
                            bit2 = 0x80; // high order bit set and then will shift down to 0
                                
                            // if it rolled around
                            if (pageId2 > MAX_PAGE_COUNT)
                            {
                                break; // done
                            }
                        }
                        eps -= dx;
                    }
                }
            }
        }
    }
}

void sh1106_lcd::Print(char *data)
{
    PrintData(data, false);
    Show();
}

void sh1106_lcd::PrintLine(char *data)
{
    PrintData(data, true);
    Show();
}

void sh1106_lcd::PrintLine(String data)
{
	char ch[20] = {0};
	data.toCharArray(ch, data.length()+1);
    PrintData(ch, true);
    Show();
}

void sh1106_lcd::PrintData(char *data, bool incrementLine)
{
    bool eol = false;
    int index = 0;
    int lineIndex = 0;
    
    // time to scroll
    if (m_currentLine == MAX_PAGE_COUNT)
    {
        while (lineIndex < (MAX_PAGE_COUNT - 1))
        {
            // Copy the lines up scrolling off the top
            memcpy(m_screen[lineIndex], m_screen[lineIndex + 1], SCREEN_WIDTH);
            lineIndex++;
        }
        // Line index is now MAX_PAGE_COUNT - 1
        memset(m_screen[lineIndex], 0, SCREEN_WIDTH); // clear this line
    }
    else
    {
        lineIndex = m_currentLine;
    }
    
    while (eol == false)
    {
        if (data[index] != NULL)
        {
            byte *pPtr = NULL;
            byte dataByte = data[index++];

			dataByte -= ' '; // Normalize
			pPtr = (byte *)&font[dataByte];
            if (pPtr != NULL)
            {
                for (int numberIndex = 0; numberIndex < CHARACTER_WIDTH; numberIndex++)
                {
                    if (m_cursor < SCREEN_WIDTH)
                    {
                        m_screen[lineIndex][m_cursor++] = pPtr[numberIndex];
                    }
                    else
                    {
                        eol = true;
                        break; // get out
                    }
                }
            }
        }
        else
        {
            eol = true; // done
        }
    }
    
    if (incrementLine == true)
    {
        if (m_currentLine < MAX_PAGE_COUNT)
        {
            m_currentLine++;
        }
        m_cursor = 1;
    }
}

byte sh1106_lcd::SendCommand(byte command, SendState state)
{
    if (state == StartSend || state == Complete)
    {
        Wire.beginTransmission(SH1106_ADDR1);
        Wire.write(SH1106_COMMAND);
    }
    
    return SendByte(command, state);
}

byte sh1106_lcd::SendData(byte data, SendState state)
{
    if (state == StartSend || state == Complete)
    {
        Wire.beginTransmission(SH1106_ADDR1);
        Wire.write(SH1106_DATA);
    }
    
    return SendByte(data, state);
}

byte sh1106_lcd::SendByte(byte data, SendState state)
{
    Wire.write(data);

    byte transmissionStatus = 0;
    
    if (state == FinishSend || state == Complete)
    {
        transmissionStatus = Wire.endTransmission();
    }
    
    return transmissionStatus;
}

