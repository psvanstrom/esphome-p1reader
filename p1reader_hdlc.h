//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2020-11-05:   Initial release
//  0.x.0 2023-04-18:   Added HDLC support
//  0.2.0 2023-08-14:   Some optimizations when ESPHome started to complain about execution times
//  0.3.0 2023-08-23:   Rework structure to merge above changes with HDLC stuff
//
// MIT License
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.
//-------------------------------------------------------------------------------------

#include "esphome.h"
#include "p1reader_base.h"

class P1ReaderHDLC : public P1ReaderBase {
  const int8_t OUTSIDE_FRAME = 0;
  const int8_t FOUND_FRAME = 1;

  int8_t parseHDLCState = OUTSIDE_FRAME;

  public:
    P1ReaderHDLC(UARTComponent *parent) : P1ReaderBase(parent) {}

  private:
    bool readHDLCStruct() {
      if (!read_array((uint8_t*)_buffer, 3))
        return false;

      if (_buffer[0] != 0x02) {
        return false;
      }

      char obis[7];

      uint8_t struct_len = _buffer[1];
      //ESP_LOGD("hdlc", "Struct length is %d", struct_len);

      uint8_t tag = _buffer[2];

      if (tag != 0x09) {
        ESP_LOGE("hdlc", "Unexpected tag %X in struct, bailing out", tag);
        return false;
      }
      
      uint8_t str_length = read();
      if (read_array((uint8_t*)_buffer, str_length) != str_length) {
        ESP_LOGE("hdlc", "Unable to read %d bytes of OBIS code", str_length);
        return false;
      }
      _buffer[str_length] = 0; // Null-terminate
      sprintf(obis, "%d.%d.%d", _buffer[2], _buffer[3], _buffer[4]);

      tag = read();

      bool is_signed = false;
      uint32_t uvalue = 0;
      int32_t value = 0;
      if (tag == 0x09) {
        str_length = read();
        if (read_array((uint8_t*)_buffer, str_length) != str_length) {
          ESP_LOGE("hdlc", "Unable to read %d bytes of string", str_length);
          return false;
        }

        _buffer[str_length] = 0;
        //ESP_LOGD("hdlc", "Read string length %d", str_length);
      } else if (tag == 0x06) {
        read_array((uint8_t*)_buffer, 4);
        //uvalue = _buffer[0] | _buffer[1] << 8 | _buffer[2] << 16 | _buffer[3] << 24;
        uvalue = _buffer[3] | _buffer[2] << 8 | _buffer[1] << 16 | _buffer[0] << 24;
        //ESP_LOGD("hdlc", "Value of uvalue is %u", uvalue);
      } else if (tag == 0x10) {
        read_array((uint8_t*)_buffer, 2); 
        //value = _buffer[0] | _buffer[1] << 8; // 
        is_signed = true;
        value = _buffer[1] | _buffer[0] << 8;
        //ESP_LOGD("hdlc", "(Signed) Value of value is %d", value);
      } else if (tag == 0x12) {
        read_array((uint8_t*)_buffer, 2); 
        //uvalue = _buffer[0] | _buffer[1] << 8; // 
        uvalue = _buffer[1] | _buffer[0] << 8;
        //ESP_LOGD("hdlc", "(Unsigned) Value of uvalue is %u", uvalue);
      } else {
        ESP_LOGE("hdlc", "unknown tag %X", tag);
      }

      int8_t scaler;
      uint8_t unit;
      if (struct_len == 3) {
        read_array((uint8_t*)_buffer, 6);
        scaler = _buffer[3];
        unit = _buffer[5];
        //ESP_LOGD("hdlc", "Scaler %u", scaler);
        //ESP_LOGD("hdlc", "Unit %d", _buffer[5]);

      if (!is_signed)
        value = uvalue;

        double scaled_value = pow(10, scaler) * value;

        // Volt and Ampere are the only two units where p1reader.yaml doesn't specify 
        // we should report in 1/1000, all others should be divided.
        if(unit != 33 && unit != 35)
          scaled_value = scaled_value / 1000;
        
        _parsedMessage.parseRow(obis, scaled_value);
      }

      return true;
    }

    /* Reads messages formatted according to "Branschrekommendation v1.2", which
       at the time of writing (20210207) is used by Tekniska Verken's Aidon 6442SE
       meters. This is a binary format, with a HDLC Frame. 

       This code is in no way a generic HDLC Frame parser, but it does the job
       of decoding this particular data stream.
    */
    void readP1Message() {
      if (available())
      {
        uint8_t data = 0;
        uint16_t crc = 0x0000;

        while (parseHDLCState == OUTSIDE_FRAME)
        {
          data = read();
          if (data == 0x7e)
          {
            // ESP_LOGD("hdlc", "Found start of frame");
            parseHDLCState = FOUND_FRAME;
            break;

            int8_t next = peek();

            // ESP_LOGD("hdlc", "Next is %d", next);

            if (next == 0x7e)
            {
              read(); // We were actually at the end flag, consume the start flag of the next frame.
            } else if (next == -1) {
              ESP_LOGE("hdlc", "No char available after flag, out of sync. Returning");
              parseHDLCState = OUTSIDE_FRAME;
              return;
            }
          }
        }

        if (parseHDLCState == FOUND_FRAME)
        {
          // Read various static HDLC Frame information we don't care about
          int len = read_array((uint8_t*)_buffer, 12);
          if (len != 12) {
            ESP_LOGE("hdlc", "Expected 12 bytes, got %d bytes - out of sync. Returning", len);
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
          // ESP_LOGD("hdlc", "Got %d HDLC bytes, now reading 4 Invoke ID And Priority bytes", len);          
          len = read_array((uint8_t*)_buffer, 4);
          if (len != 4 || _buffer[0] != 0x40 || _buffer[1] != 0x00 || _buffer[2] != 0x00 || _buffer[3] != 0x00)
          {
            ESP_LOGE("hdlc", "Expected 0x40 0x00 0x00 0x00, got %X %X %X %X - out of sync, returning.", _buffer[0], _buffer[1], _buffer[2], _buffer[3]);
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
        }

        data = read(); // Expect length of time field, usually 0
        //ESP_LOGD("hdlc", "Length of datetime field is %d", data);
        read_array((uint8_t*)_buffer, data);

        data = read();      
        ESP_LOGD("hdlc", "Expect 0x01 (array tag), got 0x%02x", data);
        if (data != 0x01) {
          parseHDLCState = OUTSIDE_FRAME;
          return;
        }

        uint8_t array_length = read();
        ESP_LOGD("hdlc", "Array length is %d", array_length);

        for (int i=0;i<array_length;i++) {
          if(!readHDLCStruct()) {
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
        }

        _parsedMessage.crcOk = _parsedMessage.telegramComplete = true;

        while ((data = read()) != -1)
        {
          //ESP_LOGD("hdlc", "Read char %02X", data);
          if (data == 0x7e)
          {
            ESP_LOGD("hdlc", "Found end of frame");
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
        }
      }
    }
};