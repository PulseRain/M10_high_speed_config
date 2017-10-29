/*
###############################################################################
# Copyright (c) 2017, PulseRain Technology LLC 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License (LGPL) as 
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
*/

//-----------------------------------------------------------------------------
// Firmware Version
//-----------------------------------------------------------------------------
#define FIRMWARE_VERSION 0x0B01


//-----------------------------------------------------------------------------
// registers for Flash Loader 
//-----------------------------------------------------------------------------
#define FLASH_LOADER_CSR_STATUS            0
#define FLASH_LOADER_CSR_CONTROL           1
#define FLASH_LOADER_CSR_READ              2
#define FLASH_LOADER_CSR_WRITE             4
#define FLASH_LOADER_BUF_FILL              8
#define FLASH_LOADER_SAVE_BEGIN            16
#define FLASH_LOADER_SAVE_END              32
#define FLASH_LOADER_FLASH_READ            64

#define FLASH_LOADER_DATA_VALID           16
#define FLASH_LOADER_PING_BUSY            8
#define FLASH_LOADER_PONG_BUSY            4
#define FLASH_LOADER_DONE_FLAG            2
#define FLASH_LOADER_ACTIVE_FLAG          1

#define FRAME_TYPE_FLASH_ERASE           0x37
#define FRAME_TYPE_FLASH_READ            0x33
#define FRAME_TYPE_FLASH_STATUS          0x39
#define FRAME_TYPE_FLASH_WRITE_4_BYTES   0x5D
#define FRAME_TYPE_FLASH_WRITE_128_BYTES 0x5B
#define FRAME_TYPE_FLASH_WRITE_PROTECT   0x31
#define FRAME_TYPE_BUF_FILL_WITH_ACK     0x5E

#define FRAME_TYPE_GET_CHIP_ID_LSW       0x45
#define FRAME_TYPE_GET_CHIP_ID_MSW       0x47

#define FLASH_SEGMENT_SIZE               2048


//-----------------------------------------------------------------------------
// chip ID
//-----------------------------------------------------------------------------

uint8_t chip_id[8];

//-----------------------------------------------------------------------------
// mcu revision
//-----------------------------------------------------------------------------
uint8_t mcu_revision;

//----------------------------------------------------------------------------
// jtag_print_dec_S32()
//
// Parameters:
//      num : 32 bit signed value to be printed 
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit signed value to JTAG console
//----------------------------------------------------------------------------
static void jtag_print_dec_S32 (int32_t num)
{
    uint8_t i;
    uint8_t tmp [10];
    
    if (num < 0) {
        jtag_putchar ('-');
        num = ~num;
        ++num;
    }
    
    for (i = 0; i < 10; ++i) {
        tmp[i] = num % 10;
        num /= 10;
        if (num == 0) {
            break;
        }
    } // End of for loop        

    do {
        jtag_putchar (digital_to_ascii(tmp[i]));
    } while (i--);
    
} // End of jtag_print_dec_S32()


//----------------------------------------------------------------------------
// jtag_print_hex()
//
// Parameters:
//      num : 32 bit unsigned value to be printed 
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit unsigned value in hex format
//  to JTAG console
//----------------------------------------------------------------------------

static void jtag_print_hex (uint32_t num)
{
    uint8_t i;
    uint8_t tmp [8];
    
    for (i = 0; i < 8; ++i) {
        tmp[i] = (uint8_t)(num & 0xF);
       // EA = 0;
        num = num / 16;
       // EA = 1;
       //  __asm__ ("nop");
       
        if (num == 0) {
            break;
        }
    } // End of for loop        
    
    
    do {
        jtag_putchar (digital_to_ascii(tmp[i]));
    } while (i--);
        
} // End of jtag_print_hex()


//----------------------------------------------------------------------------
// jtag_print_oct()
//
// Parameters:
//      num : 32 bit unsigned value to be printed 
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit unsigned value in octal format
//  to JTAG console
//----------------------------------------------------------------------------

static void jtag_print_oct (uint32_t num)
{
    uint8_t i;
    uint8_t tmp [11];
    
    for (i = 0; i < 11; ++i) {
        tmp[i] = (uint8_t)(num & 0x7);
        num = num / 8;
        if (num == 0) {
            break;
        }
    } // End of for loop
    
    do {
        jtag_putchar (digital_to_ascii(tmp[i]));
    } while (i--);
        
} // End of jtag_print_oct()


//----------------------------------------------------------------------------
// jtag_print_bin()
//
// Parameters:
//      num : 32 bit unsigned value to be printed 
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit unsigned value in binary format
//  to JTAG console
//----------------------------------------------------------------------------

static void jtag_print_bin (uint32_t num)
{
    uint8_t i;
    uint8_t tmp [32];
    
    for (i = 0; i < 32; ++i) {
        tmp[i] = (uint8_t)(num & 0x1);
        num = num / 2;
        if (num == 0) {
            break;
        }
    } // End of for loop        
    
    do {
        jtag_putchar (digital_to_ascii(tmp[i]));
    } while (i--);
        
} // End of jtag_print_bin()

//----------------------------------------------------------------------------
// jtag_print_int()
//
// Parameters:
//      num : 32 bit integer to be printed
//      fmt : DEC, HEX or OCT
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit integer value to JTAG console
//----------------------------------------------------------------------------

static void jtag_print_int (int32_t num, uint8_t fmt) __reentrant
{
    if (fmt == BIN) {
        jtag_print_bin((uint32_t) num);
    } else if (fmt == HEX) {
        jtag_print_hex((uint32_t) num);
    } else if (fmt == OCT) {
        jtag_print_oct ((uint32_t) num);
    } else {
        jtag_print_dec_S32 (num);    
    }
} // jtag_print_int()

//----------------------------------------------------------------------------
// jtag_printLn()
//
// Parameters:
//      num : 32 bit integer to be printed
//      fmt : DEC, HEX or OCT
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out 32 bit integer value to JTAG console, plus 
//  a carriage return
//----------------------------------------------------------------------------

static void jtag_printLn(int32_t data, uint8_t fmt) __reentrant 
{
   jtag_print_int (data, fmt);
  // jtag_putchar ('\r');
   jtag_putchar ('\n');
} // End of jtag_printLn()

//----------------------------------------------------------------------------
// jtag_write()
//
// Parameters:
//      buf    : string to write to console
//      length : buffer size
//
// Return Value:
//      None
//
// Remarks:
//      Function to print out string buffer to JTAG console
//----------------------------------------------------------------------------

static void jtag_write (uint8_t* buf, uint16_t length)    
{
    if (length) {
        while (length) {
            jtag_putchar ((*buf++));
            --length;
        } // End of while loop
    } else {
        while (*buf) {
            jtag_putchar ((*buf++));
           
        } // End of while loop
    }
    
} // End of jtag_write()


//----------------------------------------------------------------------------
// jtag_write_reentrant()
//
// Parameters:
//      buf    : string to write to console
//      length : buffer size
//
// Return Value:
//      None
//
// Remarks:
//      Reentry function wrapper to jtag_write()
//----------------------------------------------------------------------------

static void jtag_write_reentrant (uint8_t* buf, uint16_t length) __reentrant
{
    jtag_write (buf, length);
    
} // End of jtag_write_reentrant()


//----------------------------------------------------------------------------
// JTAG_UART_STRUCT
//----------------------------------------------------------------------------

typedef struct {
   void (*_print) (int32_t num, uint8_t fmt) __reentrant;
   void (*_println) (int32_t data, uint8_t fmt) __reentrant;
   void (*_write) (uint8_t* buf, uint16_t length) __reentrant;
} JTAG_UART_STRUCT;

const JTAG_UART_STRUCT JTAG = {
  ._print = jtag_print_int,
  ._println = jtag_printLn,
  ._write = jtag_write_reentrant
};


//----------------------------------------------------------------------------
// Frame Definition
//----------------------------------------------------------------------------


#define FRAME_SYNC_2  0x5A
#define FRAME_SYNC_1  0xA5
#define FRAME_SYNC_0  0x01

#define FRAME_LENGTH 12
#define SYNC_LENGTH  3

#define EXT_FRAME_LENGTH (128 + FRAME_LENGTH)

#define FRAME_BUFFER_SIZE (EXT_FRAME_LENGTH * 2)


#define FRAME_TYPE_MEM_WRITE_WORD  0x37
#define FRAME_TYPE_MEM_WRITE_BYTE  0x38
#define FRAME_TYPE_MEM_WRITE_EXT   0x39
#define FRAME_TYPE_MEM_READ_WORD   0x40
#define FRAME_TYPE_MEM_READ_EXT    0x41
#define FRAME_TYPE_CMD_PLAY        0x50
#define FRAME_TYPE_CMD_RECORD      0x52


#define FRAME_TYPE_ACK             0x34
#define FRAME_TYPE_ACK_EXT         0x35




//----------------------------------------------------------------------------
// Lookup table for CCITT CRC16
//----------------------------------------------------------------------------

#define CRC_PRESET 0xFFFF
uint32_t ccitt_crc = CRC_PRESET;

uint32_t crc_tab_CCITT [256] = {
        0x00000000,
        0x00001021,
        0x00002042,
        0x00003063,
        0x00004084,
        0x000050a5,
        0x000060c6,
        0x000070e7,
        0x00008108,
        0x00009129,
        0x0000a14a,
        0x0000b16b,
        0x0000c18c,
        0x0000d1ad,
        0x0000e1ce,
        0x0000f1ef,
        0x00011231,
        0x00010210,
        0x00013273,
        0x00012252,
        0x000152b5,
        0x00014294,
        0x000172f7,
        0x000162d6,
        0x00019339,
        0x00018318,
        0x0001b37b,
        0x0001a35a,
        0x0001d3bd,
        0x0001c39c,
        0x0001f3ff,
        0x0001e3de,
        0x00022462,
        0x00023443,
        0x00020420,
        0x00021401,
        0x000264e6,
        0x000274c7,
        0x000244a4,
        0x00025485,
        0x0002a56a,
        0x0002b54b,
        0x00028528,
        0x00029509,
        0x0002e5ee,
        0x0002f5cf,
        0x0002c5ac,
        0x0002d58d,
        0x00033653,
        0x00032672,
        0x00031611,
        0x00030630,
        0x000376d7,
        0x000366f6,
        0x00035695,
        0x000346b4,
        0x0003b75b,
        0x0003a77a,
        0x00039719,
        0x00038738,
        0x0003f7df,
        0x0003e7fe,
        0x0003d79d,
        0x0003c7bc,
        0x000448c4,
        0x000458e5,
        0x00046886,
        0x000478a7,
        0x00040840,
        0x00041861,
        0x00042802,
        0x00043823,
        0x0004c9cc,
        0x0004d9ed,
        0x0004e98e,
        0x0004f9af,
        0x00048948,
        0x00049969,
        0x0004a90a,
        0x0004b92b,
        0x00055af5,
        0x00054ad4,
        0x00057ab7,
        0x00056a96,
        0x00051a71,
        0x00050a50,
        0x00053a33,
        0x00052a12,
        0x0005dbfd,
        0x0005cbdc,
        0x0005fbbf,
        0x0005eb9e,
        0x00059b79,
        0x00058b58,
        0x0005bb3b,
        0x0005ab1a,
        0x00066ca6,
        0x00067c87,
        0x00064ce4,
        0x00065cc5,
        0x00062c22,
        0x00063c03,
        0x00060c60,
        0x00061c41,
        0x0006edae,
        0x0006fd8f,
        0x0006cdec,
        0x0006ddcd,
        0x0006ad2a,
        0x0006bd0b,
        0x00068d68,
        0x00069d49,
        0x00077e97,
        0x00076eb6,
        0x00075ed5,
        0x00074ef4,
        0x00073e13,
        0x00072e32,
        0x00071e51,
        0x00070e70,
        0x0007ff9f,
        0x0007efbe,
        0x0007dfdd,
        0x0007cffc,
        0x0007bf1b,
        0x0007af3a,
        0x00079f59,
        0x00078f78,
        0x00089188,
        0x000881a9,
        0x0008b1ca,
        0x0008a1eb,
        0x0008d10c,
        0x0008c12d,
        0x0008f14e,
        0x0008e16f,
        0x00081080,
        0x000800a1,
        0x000830c2,
        0x000820e3,
        0x00085004,
        0x00084025,
        0x00087046,
        0x00086067,
        0x000983b9,
        0x00099398,
        0x0009a3fb,
        0x0009b3da,
        0x0009c33d,
        0x0009d31c,
        0x0009e37f,
        0x0009f35e,
        0x000902b1,
        0x00091290,
        0x000922f3,
        0x000932d2,
        0x00094235,
        0x00095214,
        0x00096277,
        0x00097256,
        0x000ab5ea,
        0x000aa5cb,
        0x000a95a8,
        0x000a8589,
        0x000af56e,
        0x000ae54f,
        0x000ad52c,
        0x000ac50d,
        0x000a34e2,
        0x000a24c3,
        0x000a14a0,
        0x000a0481,
        0x000a7466,
        0x000a6447,
        0x000a5424,
        0x000a4405,
        0x000ba7db,
        0x000bb7fa,
        0x000b8799,
        0x000b97b8,
        0x000be75f,
        0x000bf77e,
        0x000bc71d,
        0x000bd73c,
        0x000b26d3,
        0x000b36f2,
        0x000b0691,
        0x000b16b0,
        0x000b6657,
        0x000b7676,
        0x000b4615,
        0x000b5634,
        0x000cd94c,
        0x000cc96d,
        0x000cf90e,
        0x000ce92f,
        0x000c99c8,
        0x000c89e9,
        0x000cb98a,
        0x000ca9ab,
        0x000c5844,
        0x000c4865,
        0x000c7806,
        0x000c6827,
        0x000c18c0,
        0x000c08e1,
        0x000c3882,
        0x000c28a3,
        0x000dcb7d,
        0x000ddb5c,
        0x000deb3f,
        0x000dfb1e,
        0x000d8bf9,
        0x000d9bd8,
        0x000dabbb,
        0x000dbb9a,
        0x000d4a75,
        0x000d5a54,
        0x000d6a37,
        0x000d7a16,
        0x000d0af1,
        0x000d1ad0,
        0x000d2ab3,
        0x000d3a92,
        0x000efd2e,
        0x000eed0f,
        0x000edd6c,
        0x000ecd4d,
        0x000ebdaa,
        0x000ead8b,
        0x000e9de8,
        0x000e8dc9,
        0x000e7c26,
        0x000e6c07,
        0x000e5c64,
        0x000e4c45,
        0x000e3ca2,
        0x000e2c83,
        0x000e1ce0,
        0x000e0cc1,
        0x000fef1f,
        0x000fff3e,
        0x000fcf5d,
        0x000fdf7c,
        0x000faf9b,
        0x000fbfba,
        0x000f8fd9,
        0x000f9ff8,
        0x000f6e17,
        0x000f7e36,
        0x000f4e55,
        0x000f5e74,
        0x000f2e93,
        0x000f3eb2,
        0x000f0ed1,
        0x000f1ef0
}; // End of crc_tab_CCITT[]


//----------------------------------------------------------------------------
// _update_crc()
//
// Parameters:
//      c : 8 bit input number
//
// Remarks:
//      supporting function to update CRC shift register
//----------------------------------------------------------------------------

void _update_crc(uint8_t c)
{
     uint32_t tmp;

      tmp = (ccitt_crc >> 8) ^ c;
      ccitt_crc = (ccitt_crc << 8) ^ crc_tab_CCITT[tmp & 0xff];
      ccitt_crc = ccitt_crc & 0xffff;
}


//----------------------------------------------------------------------------
// get_crc()
//
// Parameters:
//      in_buf   : data buffer
//      len      : size of the buffer in byte
//
//
// Remarks:
//      calculate the CCITT CRC16 for in_buf. The result is in ccitt_crc.
//----------------------------------------------------------------------------

void get_crc (uint8_t* in_buf, uint32_t len)
{
    uint32_t i;

    ccitt_crc = CRC_PRESET;

    for (i = 0; i < len; ++i) {
        _update_crc (in_buf[i]);
    } // End of for loop

    ccitt_crc = ccitt_crc & 0xFFFF;
} // End of get_crc()


//----------------------------------------------------------------------------
// bit_reverse_8_bit()
//
// Parameters:
//      x : 8 bit input
//
// Return Value:
//      the bit reversed value of x
//
// Remarks:
//      Need to do bit reverse to program flash
//----------------------------------------------------------------------------

uint8_t bit_reverse_8_bit (uint8_t x)
{

        x = (((x & 0xf0)>>4)|((x & 0x0f)<<4));
        x = (((x & 0xcc)>>2)|((x & 0x33)<<2));
        x = (((x & 0xaa)>>1)|((x & 0x55)<<1));

        return x;
} // End of bit_reverse_8_bit()


//----------------------------------------------------------------------------
// bit_reshuffle_32_bit()
//
// Parameters:
//      x : 32 bit input
//
// Return Value:
//      bit reverse each byte of x
//
// Remarks:
//      Need to do bit reverse to program flash
//----------------------------------------------------------------------------

uint32_t bit_reshuffle_32_bit (uint32_t x)
{
        uint8_t y, i;
        uint32_t f = 1;
        uint32_t z = 0;

        for (i = 0; i < 4; ++i) {
            y = (uint8_t)(x & 0xFF);
            y = bit_reverse_8_bit (y);

            z = z + y * f;

            f = f * 256;
            x = x / 256;
        } // End of for loop

        return z;

} // End of bit_reshuffle_32_bit()


//----------------------------------------------------------------------------
// Input circular buffer
//----------------------------------------------------------------------------
uint8_t frame_buffer[FRAME_BUFFER_SIZE];
uint8_t frame_type;
uint16_t read_pointer = 0;
uint16_t write_pointer = 0;

//----------------------------------------------------------------------------
// Input frame buffer
//----------------------------------------------------------------------------

uint8_t input_data[EXT_FRAME_LENGTH];
uint8_t input_counter;



//----------------------------------------------------------------------------
// clear_input_data()
//
// Parameters:
//      None
//
// Remarks:
//      Function to clear input frame buffer
//----------------------------------------------------------------------------

void clear_input_data ()
{
    uint16_t i;

    for (i = 0; i < FRAME_LENGTH; ++i) {
        input_data[i] = 0;
    } // End of for loop

} // End of clear_input_data()


//----------------------------------------------------------------------------
// buffer for memory read
//----------------------------------------------------------------------------

#define READ_WRITE_BUFFER_SIZE (128 + 16)
uint8_t read_write_buffer[READ_WRITE_BUFFER_SIZE];


//----------------------------------------------------------------------------
// send_read_data_back()
//
// Parameters:
//      len : length of the buffer
//
// Remarks:
//      function to send read_write_buffer to UART, with CRC
//----------------------------------------------------------------------------
void send_read_data_back (uint8_t len)
{
    uint8_t t;

    Serial.write(read_write_buffer, len);
    
    get_crc (read_write_buffer, len);

    t = (uint8_t)((ccitt_crc & 0xFF00) >> 8);
    Serial.write(&t, 1);

    t = (uint8_t)(ccitt_crc & 0xFF);
    Serial.write(&t, 1);
    
} // End of send_read_data_back()


//----------------------------------------------------------------------------
// flash_erase_status()
//
// Parameters:
//      None
//
// Return Value:
//      value from CSR Status register
//
// Remarks:
//      function to read flash status
//----------------------------------------------------------------------------

uint32_t flash_erase_status()
{
    uint32_t t;
    uint8_t t0, t1, t2, t3;
    
    FLASH_LOADER_CSR = FLASH_LOADER_CSR_STATUS | FLASH_LOADER_CSR_READ;
    nop_delay (20);

    t0 = FLASH_LOADER_DATA0;
    t1 = FLASH_LOADER_DATA1;
    t2 = FLASH_LOADER_DATA2;
    t3 = FLASH_LOADER_DATA3;

    t = ((uint32_t)t3 << 24) + 
        ((uint32_t)t2 << 16) +
        ((uint32_t)t1 << 8) +
        ((uint32_t)t0);
             
    return t;
} // End of flash_erase_status()


//----------------------------------------------------------------------------
// flash_protect()
//
// Parameters:
//      None
//
//
// Remarks:
//      function to set up the write-protection flag in flash
//----------------------------------------------------------------------------

void flash_protect()
{
    FLASH_LOADER_DATA0 = 0xff;
    FLASH_LOADER_DATA1 = 0xff;
    FLASH_LOADER_DATA2 = 0xff;
    FLASH_LOADER_DATA3 = 0xff;
    
    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_CSR_WRITE;
    nop_delay (20);

} // End of flash_protect()


//----------------------------------------------------------------------------
// send_reply_back()
//
// Parameters:
//      m : 16 bit data
//      n : 32 bit data
//
// Remarks:
//      function to send (m, n) as the payload to reply
//----------------------------------------------------------------------------

void send_reply_back (uint16_t m, uint32_t n)
{
    uint8_t t;
    uint8_t* buf = read_write_buffer;

    *buf++ = FRAME_SYNC_2;
    *buf++ = FRAME_SYNC_1;
    *buf++ = FRAME_SYNC_0;

    *buf++ = FRAME_TYPE_ACK;

    *buf++ = (m >> 8) & 0xFF;
    *buf++ = m & 0xFF;

    *buf++ = (n >> 24) & 0xFF;
    *buf++ = (n >> 16) & 0xFF;
    *buf++ = (n >> 8)  & 0xFF;
    *buf++ = n & 0xFF;


    Serial.write(read_write_buffer, FRAME_LENGTH - 2);
    
    get_crc (read_write_buffer, FRAME_LENGTH - 2);

    t = (uint8_t)((ccitt_crc & 0xFF00) >> 8);
    Serial.write(&t, 1);

    t = (uint8_t)(ccitt_crc & 0xFF);
    Serial.write(&t, 1);

} // End of send_reply_back()


//----------------------------------------------------------------------------
// flash_erase()
//
// Parameters:
//      index : flash sector index, value in [1, 5]
//
// Remarks:
//      function to erase flash sector
//----------------------------------------------------------------------------

void flash_erase (uint8_t index)
{
    uint32_t t;
    uint8_t t0, t1, t2, t3;

    t = ((uint32_t)1 << (index + 22));
    t = ~t;

    if ((index == 3) || (index == 4)) {
       t &= (uint32_t)0xF9FFFFFF;
    } else {
       t &= (uint32_t)0xFE7FFFFF;
    }
    
    t0 = t & 0xFF;
    t1 = ((uint32_t)t >> 8)  & 0xFF;
    t2 = ((uint32_t)t >> 16) & 0xFF;
    t3 = ((uint32_t)t >> 24) & 0xFF;

    FLASH_LOADER_DATA0 = t0;
    FLASH_LOADER_DATA1 = t1;
    FLASH_LOADER_DATA2 = t2;
    FLASH_LOADER_DATA3 = t3;

    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_CSR_WRITE;
    nop_delay (20);
    
    t &= ((uint32_t)index << 20) | 0xff8fffff;

    t0 = t & 0xFF;
    t1 = ((uint32_t)t >> 8)  & 0xFF;
    t2 = ((uint32_t)t >> 16) & 0xFF;
    t3 = ((uint32_t)t >> 24) & 0xFF;

    FLASH_LOADER_DATA0 = t0;
    FLASH_LOADER_DATA1 = t1;
    FLASH_LOADER_DATA2 = t2;
    FLASH_LOADER_DATA3 = t3;

    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_CSR_WRITE;
    nop_delay (20);
    
} // End of flash_erase()

//----------------------------------------------------------------------------
// flash_read()
//
// Parameters:
//      start_addr : start address for flash sector
//      len        : length of the read buffer
//      buf        : pointer to the read buffer
//
// Remarks:
//      function to read data from flash
//----------------------------------------------------------------------------

void flash_read (uint32_t start_addr, uint16_t len, uint8_t* buf)
{
    uint8_t t0, t1, t2, t3;
    uint32_t end_addr;
    uint32_t temp32;

    end_addr = start_addr + len;

    *buf++ = FRAME_SYNC_2;
    *buf++ = FRAME_SYNC_1;
    *buf++ = FRAME_SYNC_0;

    while (start_addr < end_addr) {

        temp32 = start_addr / 4;
        
        t0 = temp32 & 0xFF;
        t1 = ((uint32_t)temp32 >> 8)  & 0xFF;
        t2 = ((uint32_t)temp32 >> 16) & 0xFF;
        t3 = ((uint32_t)temp32 >> 24) & 0xFF;


        FLASH_LOADER_DATA0 = t0;
        FLASH_LOADER_DATA1 = t1;
        FLASH_LOADER_DATA2 = t2;
        FLASH_LOADER_DATA3 = t3;

        FLASH_LOADER_CSR = FLASH_LOADER_FLASH_READ;

        nop_delay (20);
        
        do {
          t0 = FLASH_LOADER_CSR;
          nop_delay (4);
        } while (!(t0 & FLASH_LOADER_DATA_VALID));

      
        
        t0 = FLASH_LOADER_DATA0;
        t1 = FLASH_LOADER_DATA1;
        t2 = FLASH_LOADER_DATA2;
        t3 = FLASH_LOADER_DATA3;
      
        t0 = bit_reverse_8_bit (t0);
        t1 = bit_reverse_8_bit (t1);
        t2 = bit_reverse_8_bit (t2);
        t3 = bit_reverse_8_bit (t3);

        
        *buf++ = t3;
        *buf++ = t2;
        *buf++ = t1;
        *buf++ = t0;

        if (buf >= (read_write_buffer + READ_WRITE_BUFFER_SIZE)) {
            buf = read_write_buffer;
        }

        start_addr += 4;
    } // End of while loop

}


//----------------------------------------------------------------------------
// flash_buf_fill()
//
// Parameters:
//      start_addr : start address for flash sector
//      len        : length of the read buffer
//      buf        : pointer to the read buffer
//
// Remarks:
//      function to read data from flash
//----------------------------------------------------------------------------

void flash_buf_fill (uint32_t start_addr, uint16_t len)
{
    uint32_t t;
    uint8_t t0, t1, t2, t3;

    t = start_addr / 4;
    
    t0 = t & 0xFF;
    t1 = ((uint32_t)t >> 8)  & 0xFF;
    t2 = ((uint32_t)t >> 16) & 0xFF;
    t3 = ((uint32_t)t >> 24) & 0xFF;

    
    FLASH_LOADER_DATA0 = t0;
    FLASH_LOADER_DATA1 = t1;
    FLASH_LOADER_DATA2 = t2;
    FLASH_LOADER_DATA3 = t3;

    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_SAVE_BEGIN;
    nop_delay (20);

    t = start_addr;
    t += (uint32_t)len * FLASH_SEGMENT_SIZE;
    t /= 4;
    --t;
    
    t0 = t & 0xFF;
    t1 = ((uint32_t)t >> 8)  & 0xFF;
    t2 = ((uint32_t)t >> 16) & 0xFF;
    t3 = ((uint32_t)t >> 24) & 0xFF;

    FLASH_LOADER_DATA0 = t0;
    FLASH_LOADER_DATA1 = t1;
    FLASH_LOADER_DATA2 = t2;
    FLASH_LOADER_DATA3 = t3;

    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_SAVE_END;
    nop_delay (20);

    FLASH_LOADER_CSR = FLASH_LOADER_CSR_CONTROL | FLASH_LOADER_BUF_FILL;
    nop_delay (20);
    
}

//----------------------------------------------------------------------------
// input_FSM()
//
// Parameters:
//      input : input from serial console
//      reset : FSM reset
//
// Return Value:
//      None
//
// Remarks:
//      function to read data from flash
//----------------------------------------------------------------------------

enum Input_FSM_State {
    INPUT_STATE_IDLE,
    INPUT_STATE_SYNC_1,
    INPUT_STATE_SYNC_0,
    INPUT_STATE_INPUT_WAIT,
    INPUT_STATE_INPUT_WAIT_EXT,
    INPUT_STATE_FRAME_TYPE
};


void input_FSM(uint8_t input, uint8_t reset)
{
    static enum Input_FSM_State state = INPUT_STATE_IDLE;

    static uint8_t frame_type = 0;
    uint8_t loop_continue;
    uint16_t crc_received;
    uint8_t flash_index;
    uint32_t start_addr;
    uint32_t start_addr_write;
    uint32_t data;
    uint8_t  data_byte;
    uint16_t length;

    uint32_t temp32;

    if (reset) {
      state = INPUT_STATE_IDLE;
      clear_input_data();
    } else {

      do {
        loop_continue = 0;

        switch (state) {
          case INPUT_STATE_IDLE:
            input_counter = 0;

            if (input == FRAME_SYNC_2) {
                state = INPUT_STATE_SYNC_1;
                clear_input_data();
                input_data [input_counter++] = input;
            }
  
            break;
  
          case INPUT_STATE_SYNC_1:
            
            if (input == FRAME_SYNC_1) {
                state = INPUT_STATE_SYNC_0;
                input_data [input_counter++] = input;
            } else {
                state = INPUT_STATE_IDLE;
            }
  
            break;
  
          case INPUT_STATE_SYNC_0:
            
            if (input == FRAME_SYNC_0) {
                state = INPUT_STATE_INPUT_WAIT;
                input_data [input_counter++] = input;
            } else {
                state = INPUT_STATE_IDLE;
            }
  
            break;
  
          case INPUT_STATE_INPUT_WAIT:
            
            input_data [input_counter++] = input;
            frame_type  = input_data[SYNC_LENGTH];
  
            if (input_counter == FRAME_LENGTH) {

              if (frame_type  == FRAME_TYPE_MEM_WRITE_EXT) {
                state = INPUT_STATE_INPUT_WAIT_EXT;
              } else {
                state = INPUT_STATE_FRAME_TYPE;
                loop_continue = 1;
              }
            }
  
            break;
  
          case INPUT_STATE_INPUT_WAIT_EXT:
  
            input_data [input_counter++] = input;
  
            if   ((input_counter == (EXT_FRAME_LENGTH - 2)) && (frame_type == FRAME_TYPE_MEM_WRITE_EXT)) {
                state = INPUT_STATE_FRAME_TYPE;
                loop_continue = 1;
            }
  
            break;
  
          case INPUT_STATE_FRAME_TYPE:
  
            if (frame_type == FRAME_TYPE_MEM_WRITE_EXT) {
              get_crc(input_data, EXT_FRAME_LENGTH - 4);
              crc_received = (input_data [EXT_FRAME_LENGTH - 4] << 8) + input_data [EXT_FRAME_LENGTH - 3];
            } else {
              get_crc(input_data, FRAME_LENGTH - 2);
              crc_received = (input_data [FRAME_LENGTH - 2] << 8) + input_data [FRAME_LENGTH - 1];
            }

            flash_index = input_data[SYNC_LENGTH + 1];
            
            start_addr = ((uint32_t)input_data [SYNC_LENGTH + 3] << 24) +
                         ((uint32_t)input_data [SYNC_LENGTH + 4] << 16) +
                         ((uint32_t)input_data [SYNC_LENGTH + 5] << 8) +
                         ((uint32_t)input_data [SYNC_LENGTH + 6]);
  
            start_addr_write = ((uint32_t)input_data [SYNC_LENGTH + 1] << 24) +
                               ((uint32_t)input_data [SYNC_LENGTH + 2] << 16) +
                               ((uint32_t)input_data [SYNC_LENGTH + 3] << 8) +
                               ((uint32_t)input_data [SYNC_LENGTH + 4]);

            data  = ((uint32_t)input_data [SYNC_LENGTH + 5] << 24) +
                    ((uint32_t)input_data [SYNC_LENGTH + 6] << 16) +
                    ((uint32_t)input_data [SYNC_LENGTH + 7] << 8) +
                    ((uint32_t)input_data [SYNC_LENGTH + 8]);

            data_byte = input_data [SYNC_LENGTH + 5];

            length = ((uint16_t)input_data[SYNC_LENGTH + 1] << 8) +
                     ((uint16_t)input_data[SYNC_LENGTH + 2]);


            state = INPUT_STATE_IDLE;

            frame_type >>= 1;
            
            if (crc_received == ccitt_crc) {
               if (frame_type ==  FRAME_TYPE_GET_CHIP_ID_LSW) {
                  temp32 = ((uint32_t)chip_id[4] << 24) + ((uint32_t)chip_id[5] << 16) + ((uint32_t)chip_id[6] << 8) + chip_id[7];
                  send_reply_back ((uint16_t)mcu_revision, temp32);       
               } else if (frame_type ==  FRAME_TYPE_GET_CHIP_ID_MSW) {
                  temp32 = ((uint32_t)chip_id[0] << 24) + ((uint32_t)chip_id[1] << 16) + ((uint32_t)chip_id[2] << 8) + chip_id[3];
                  send_reply_back (FIRMWARE_VERSION, temp32);
               } else if (frame_type == FRAME_TYPE_FLASH_READ) {
                  flash_read (start_addr, length, read_write_buffer);
                  send_read_data_back (length + 3);
               } else if (frame_type == FRAME_TYPE_FLASH_ERASE) {
                  flash_erase (flash_index);
                  send_reply_back (0x1234, flash_erase_status());
               } else if (frame_type == FRAME_TYPE_FLASH_STATUS) {
                  send_reply_back (0x1234, flash_erase_status());
               } else if (frame_type == FRAME_TYPE_FLASH_WRITE_PROTECT) {
                  flash_protect ();
                  send_reply_back (0xabcd, start_addr_write);
               } else if (frame_type == FRAME_TYPE_BUF_FILL_WITH_ACK) {
                  flash_buf_fill (start_addr, length);
                  send_reply_back (0xcdef, start_addr);
               }
            }

            default:
              break;
         } // End of switch
      } while (loop_continue);
   }
   
} // End of Input_FSM()


//----------------------------------------------------------------------------
// FSM()
//
// Parameters:
//      None
//
// Return Value:
//      None
//
// Remarks:
//      finite state machine for the main flow
//----------------------------------------------------------------------------

enum FSM_State {
    STATE_INIT,
    STATE_UART  
};

void FSM()
{
    static enum FSM_State state = STATE_INIT;
    uint8_t t;
   
    switch (state) {

        case STATE_INIT:
       
            input_FSM (0, 1);
            state = STATE_UART; 
           
            break;

        case STATE_UART:
               
            if (Serial.available()) {
                t = Serial.read();
                input_FSM(t, 0);
            } 
           
            break;
           
        default:
            state = STATE_UART;
            break;
    } // End of switch

} // End of FSM()



//==========================================================================
// setup()
//==========================================================================

void setup() {

  uint8_t t, i;
  
  Serial.begin(921600);
  delay(1000);

  while (Serial.available()) {
      Serial.read();
  } // End of while loop()
  
  CHIP_ID_DATA_CSR = 0xff;
  __asm__ ("nop");

  for (i = 0; i < 8; ++i) {
    t = CHIP_ID_DATA_CSR;
    __asm__ ("nop");
    chip_id[i] = t;
  } // End of for loop()

  mcu_revision = MCU_REVISION;

} // End of setup()

//==========================================================================
// loop()
//==========================================================================

void loop() {
  
  FSM();

} // End of loop()

