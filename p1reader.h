//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2020-11-05:   Initial release
//  0.x.0 2023-04-18:   Added HDLC support
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

#define BUF_SIZE 60

class ParsedMessage {
  public:
    double cumulativeActiveImport;
    double cumulativeActiveExport;

    double cumulativeReactiveImport;
    double cumulativeReactiveExport;

    double momentaryActiveImport;
    double momentaryActiveExport;

    double momentaryReactiveImport;
    double momentaryReactiveExport;

    double momentaryActiveImportL1;
    double momentaryActiveExportL1;

    double momentaryActiveImportL2;
    double momentaryActiveExportL2;

    double momentaryActiveImportL3;
    double momentaryActiveExportL3;

    double momentaryReactiveImportL1;
    double momentaryReactiveExportL1;

    double momentaryReactiveImportL2;
    double momentaryReactiveExportL2;

    double momentaryReactiveImportL3;
    double momentaryReactiveExportL3;

    double voltageL1;
    double voltageL2;
    double voltageL3;

    double currentL1;
    double currentL2;
    double currentL3;

    bool crcOk = false;
};

class P1Reader : public Component, public UARTDevice {
  const char* DELIMITERS = "()*:";
  const char* DATA_ID = "1-0";

  protected:
    char buffer[BUF_SIZE];

  public:
    Sensor *cumulativeActiveImport = new Sensor();
    Sensor *cumulativeActiveExport = new Sensor();

    Sensor *cumulativeReactiveImport = new Sensor();
    Sensor *cumulativeReactiveExport = new Sensor();

    Sensor *momentaryActiveImport = new Sensor();
    Sensor *momentaryActiveExport = new Sensor();

    Sensor *momentaryReactiveImport = new Sensor();
    Sensor *momentaryReactiveExport = new Sensor();

    Sensor *momentaryActiveImportL1 = new Sensor();
    Sensor *momentaryActiveExportL1 = new Sensor();

    Sensor *momentaryActiveImportL2 = new Sensor();
    Sensor *momentaryActiveExportL2 = new Sensor();

    Sensor *momentaryActiveImportL3 = new Sensor();
    Sensor *momentaryActiveExportL3 = new Sensor();

    Sensor *momentaryReactiveImportL1 = new Sensor();
    Sensor *momentaryReactiveExportL1 = new Sensor();

    Sensor *momentaryReactiveImportL2 = new Sensor();
    Sensor *momentaryReactiveExportL2 = new Sensor();

    Sensor *momentaryReactiveImportL3 = new Sensor();
    Sensor *momentaryReactiveExportL3 = new Sensor();

    Sensor *voltageL1 = new Sensor();
    Sensor *voltageL2 = new Sensor();
    Sensor *voltageL3 = new Sensor();

    Sensor *currentL1 = new Sensor();
    Sensor *currentL2 = new Sensor();
    Sensor *currentL3 = new Sensor();

    P1Reader(UARTComponent *parent) : UARTDevice(parent) {}

    void setup() override { }

    void loop() override
    {
      readP1Message();
    }

  protected:
    uint16_t crc16_update(uint16_t crc, uint8_t a) {
      int i;
      crc ^= a;
      for (i = 0; i < 8; ++i) {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0xA001;
        } else {
            crc = (crc >> 1);
        }
      }
      return crc;
    }

    void parseRow(ParsedMessage* parsed, char* obisCode, char* value) {
      if (strncmp(obisCode, "1.8.0", 5) == 0) {
        parsed->cumulativeActiveImport = atof(value);

      } else if (strncmp(obisCode, "2.8.0", 5) == 0) {
        parsed->cumulativeActiveExport = atof(value);

      } else if (strncmp(obisCode, "3.8.0", 5) == 0) {
        parsed->cumulativeReactiveImport = atof(value);

      } else if (strncmp(obisCode, "4.8.0", 5) == 0) {
        parsed->cumulativeReactiveExport = atof(value);

      } else if (strncmp(obisCode, "1.7.0", 5) == 0) {
        parsed->momentaryActiveImport = atof(value);

      } else if (strncmp(obisCode, "2.7.0", 5) == 0) {
        parsed->momentaryActiveExport = atof(value);

      } else if (strncmp(obisCode, "3.7.0", 5) == 0) {
        parsed->momentaryReactiveImport = atof(value);

      } else if (strncmp(obisCode, "4.7.0", 5) == 0) {
        parsed->momentaryReactiveExport = atof(value);

      } else if (strncmp(obisCode, "21.7.0", 6) == 0) {
        parsed->momentaryActiveImportL1 = atof(value);

      } else if (strncmp(obisCode, "22.7.0", 6) == 0) {
        parsed->momentaryActiveExportL1 = atof(value);

      } else if (strncmp(obisCode, "41.7.0", 6) == 0) {
        parsed->momentaryActiveImportL2 = atof(value);

      } else if (strncmp(obisCode, "42.7.0", 6) == 0) {
        parsed->momentaryActiveExportL2 = atof(value);

      } else if (strncmp(obisCode, "61.7.0", 6) == 0) {
        parsed->momentaryActiveImportL3 = atof(value);

      } else if (strncmp(obisCode, "62.7.0", 6) == 0) {
        parsed->momentaryActiveExportL3 = atof(value);

      } else if (strncmp(obisCode, "23.7.0", 6) == 0) {
        parsed->momentaryReactiveImportL1 = atof(value);

      } else if (strncmp(obisCode, "24.7.0", 6) == 0) {
        parsed->momentaryReactiveExportL1 = atof(value);

      } else if (strncmp(obisCode, "43.7.0", 6) == 0) {
        parsed->momentaryReactiveImportL2 = atof(value);

      } else if (strncmp(obisCode, "44.7.0", 6) == 0) {
        parsed->momentaryReactiveExportL2 = atof(value);

      } else if (strncmp(obisCode, "63.7.0", 6) == 0) {
        parsed->momentaryReactiveImportL3 = atof(value);

      } else if (strncmp(obisCode, "64.7.0", 6) == 0) {
        parsed->momentaryReactiveExportL3 = atof(value);

      } else if (strncmp(obisCode, "32.7.0", 6) == 0) {
        parsed->voltageL1 = atof(value);

      } else if (strncmp(obisCode, "52.7.0", 6) == 0) {
        parsed->voltageL2 = atof(value);

      } else if (strncmp(obisCode, "72.7.0", 6) == 0) {
        parsed->voltageL3 = atof(value);

      } else if (strncmp(obisCode, "31.7.0", 6) == 0) {
        parsed->currentL1 = atof(value);

      } else if (strncmp(obisCode, "51.7.0", 6) == 0) {
        parsed->currentL2 = atof(value);

      } else if (strncmp(obisCode, "71.7.0", 6) == 0) {
        parsed->currentL3 = atof(value);
      }
    }

    void publishSensors(ParsedMessage* parsed) {
      cumulativeActiveImport->publish_state(parsed->cumulativeActiveImport);
      cumulativeActiveExport->publish_state(parsed->cumulativeActiveExport);

      cumulativeReactiveImport->publish_state(parsed->cumulativeReactiveImport);
      cumulativeReactiveExport->publish_state(parsed->cumulativeReactiveExport);

      momentaryActiveImport->publish_state(parsed->momentaryActiveImport);
      momentaryActiveExport->publish_state(parsed->momentaryActiveExport);

      momentaryReactiveImport->publish_state(parsed->momentaryReactiveImport);
      momentaryReactiveExport->publish_state(parsed->momentaryReactiveExport);

      momentaryActiveImportL1->publish_state(parsed->momentaryActiveImportL1);
      momentaryActiveExportL1->publish_state(parsed->momentaryActiveExportL1);

      momentaryActiveImportL2->publish_state(parsed->momentaryActiveImportL2);
      momentaryActiveExportL2->publish_state(parsed->momentaryActiveExportL2);

      momentaryActiveImportL3->publish_state(parsed->momentaryActiveImportL3);
      momentaryActiveExportL3->publish_state(parsed->momentaryActiveExportL3);

      momentaryReactiveImportL1->publish_state(parsed->momentaryReactiveImportL1);
      momentaryReactiveExportL1->publish_state(parsed->momentaryReactiveExportL1);

      momentaryReactiveImportL2->publish_state(parsed->momentaryReactiveImportL2);
      momentaryReactiveExportL2->publish_state(parsed->momentaryReactiveExportL2);

      momentaryReactiveImportL3->publish_state(parsed->momentaryReactiveImportL3);
      momentaryReactiveExportL3->publish_state(parsed->momentaryReactiveExportL3);

      voltageL1->publish_state(parsed->voltageL1);
      voltageL2->publish_state(parsed->voltageL2);
      voltageL3->publish_state(parsed->voltageL3);

      currentL1->publish_state(parsed->currentL1);
      currentL2->publish_state(parsed->currentL2);
      currentL3->publish_state(parsed->currentL3);
    }

  private:
    int timedRead() {
      const unsigned long _startMillis = millis();
      int c;
      do {
        if (available()) {
            c = read();
            if (c >= 0) return c;
        }
        else {
            delay(1);
        }
      } while(millis() - _startMillis < 1000); // default timeout is 1000ms
      return -1;  // indicates timeout
    }

    int readBytesUntil(const char terminator, char *data, const size_t len) {
        size_t count = 0;
        while (count < len) {
            int c = timedRead();
            if (c < 0 || terminator == (char) c) break;
            data[count] = (char) c;
            count++;
        }
        return count;
    }

    void readP1Message() {
      if (available()) {
        uint16_t crc = 0x0000;
        ParsedMessage parsed = ParsedMessage();
        bool telegramEnded = false;

        while (available()) {
          int len = readBytesUntil('\n', buffer, BUF_SIZE-1);

          if (len > 0) {
          	ESP_LOGD("data", "%s", buffer);

            // put newline back as it is required for CRC calculation
            buffer[len] = '\n';
            buffer[len + 1] = '\0';

            // if we've reached the CRC checksum, calculate last CRC and compare
            if (buffer[0] == '!') {
              crc = crc16_update(crc, buffer[0]);
              int crcFromMsg = (int) strtol(&buffer[1], NULL, 16);
              parsed.crcOk = crc == crcFromMsg;
              ESP_LOGI("crc", "Telegram read. CRC: %04X = %04X. PASS = %s", crc, crcFromMsg, parsed.crcOk ? "YES": "NO");
              telegramEnded = true;

            // otherwise pass the row through the CRC calculation
            } else {
              for (int i = 0; i < len + 1; i++) {
                crc = crc16_update(crc, buffer[i]);
              }
            }

            // if this is a row containing information
            if (strchr(buffer, '(') != NULL) {
              char* dataId = strtok(buffer, DELIMITERS);
              char* obisCode = strtok(NULL, DELIMITERS);

              // ...and this row is a data row, then parse row
              if (strncmp(DATA_ID, dataId, strlen(DATA_ID)) == 0) {
                char* value = strtok(NULL, DELIMITERS);
                char* unit = strtok(NULL, DELIMITERS);
                parseRow(&parsed, obisCode, value);
              }
            }
          }
          // clean buffer
          memset(buffer, 0, BUF_SIZE - 1);
        
          if (!telegramEnded && !available()) {
          	// wait for more data
          	delay(40);
          }
        }

        // if the CRC pass, publish sensor values
        if (parsed.crcOk) {
          publishSensors(&parsed);
        }
      }
    }

    
};

class P1ReaderHDLC : public P1Reader {
  const int8_t OUTSIDE_FRAME = 0;
  const int8_t FOUND_FRAME = 1;


  int8_t parseHDLCState = OUTSIDE_FRAME;

  public:
   P1ReaderHDLC(UARTComponent *parent) : P1Reader(parent) {}

  void loop() override {
    readHDLCMessage();
  }

  private:
    bool readHDLCStruct(ParsedMessage* parsed) {
      if(Serial.readBytes(buffer, 3) != 3)
        return false;

      if(buffer[0] != 0x02) {
        return false;
      }

      char obis[7];

      uint8_t struct_len = buffer[1];
      //ESP_LOGD("hdlc", "Struct length is %d", struct_len);

      uint8_t tag = buffer[2];

      if(tag != 0x09) {
        ESP_LOGE("hdlc", "Unexpected tag %X in struct, bailing out", tag);
        return false;
      }
      
      uint8_t str_length = read();
      if(Serial.readBytes(buffer, str_length) != str_length) {
        ESP_LOGE("hdlc", "Unable to read %d bytes of OBIS code", str_length);
        return false;
      }
      buffer[str_length] = 0; // Null-terminate
      sprintf(obis, "%d.%d.%d", buffer[2], buffer[3], buffer[4]);

      tag = read();

      char value_buf[24];
      bool is_signed = false;
      uint32_t uvalue = 0;
      int32_t value = 0;
      if (tag == 0x09) {
        str_length = read();
        if(Serial.readBytes(buffer, str_length) != str_length) {
          ESP_LOGE("hdlc", "Unable to read %d bytes of string", str_length);
          return false;
        }

        buffer[str_length] = 0;
        //ESP_LOGD("hdlc", "Read string length %d", str_length);
      } else if(tag == 0x06) {
        Serial.readBytes(buffer, 4);
        //uvalue = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
        uvalue = buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
        //ESP_LOGD("hdlc", "Value of uvalue is %u", uvalue);
      } else if (tag == 0x10) {
        Serial.readBytes(buffer, 2); 
        //value = buffer[0] | buffer[1] << 8; // 
        is_signed = true;
        value = buffer[1] | buffer[0] << 8;
        //ESP_LOGD("hdlc", "(Signed) Value of value is %d", value);
      } else if (tag == 0x12) {
        Serial.readBytes(buffer, 2); 
        //uvalue = buffer[0] | buffer[1] << 8; // 
        uvalue = buffer[1] | buffer[0] << 8;
        //ESP_LOGD("hdlc", "(Unsigned) Value of uvalue is %u", uvalue);
      } else {
        ESP_LOGE("hdlc", "unknown tag %X", tag);
      }

      int8_t scaler;
      uint8_t unit;
      if(struct_len == 3) {
        Serial.readBytes(buffer, 6);
        scaler = buffer[3];
        unit = buffer[5];
        //ESP_LOGD("hdlc", "Scaler %u", scaler);
        //ESP_LOGD("hdlc", "Unit %d", buffer[5]);

      if(!is_signed)
        value = uvalue;

        double scaled_value = pow(10, scaler) * value;

        // Volt and Ampere are the only two units where p1reader.yaml doesn't specify 
        // we should report in 1/1000, all others should be divided.
        if(unit != 33 && unit != 35)
          scaled_value = scaled_value / 1000;
        
        snprintf(value_buf, 24, "%f", scaled_value);
        parseRow(parsed, obis, value_buf);
      }


      

      return true;
    }

    /* Reads messages formatted according to "Branschrekommendation v1.2", which
       at the time of writing (20210207) is used by Tekniska Verken's Aidon 6442SE
       meters. This is a binary format, with a HDLC Frame. 

       This code is in no way a generic HDLC Frame parser, but it does the job
       of decoding this particular data stream.
    */
    void readHDLCMessage()
    {
      if (available())
      {
        uint8_t data = 0;
        uint16_t crc = 0x0000;
        ParsedMessage parsed = ParsedMessage();

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
          int len = Serial.readBytes(buffer, 12);
          if (len != 12) {
            ESP_LOGE("hdlc", "Expected 12 bytes, got %d bytes - out of sync. Returning", len);
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
          // ESP_LOGD("hdlc", "Got %d HDLC bytes, now reading 4 Invoke ID And Priority bytes", len);          
          len = Serial.readBytes(buffer, 4);
          if (len != 4 || buffer[0] != 0x40 || buffer[1] != 0x00 || buffer[2] != 0x00 || buffer[3] != 0x00)
          {
            ESP_LOGE("hdlc", "Expected 0x40 0x00 0x00 0x00, got %X %X %X %X - out of sync, returning.", buffer[0], buffer[1], buffer[2], buffer[3]);
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
        }

        data = read(); // Expect length of time field, usually 0
        //ESP_LOGD("hdlc", "Length of datetime field is %d", data);
        Serial.readBytes(buffer, data);

        data = read();      
        ESP_LOGD("hdlc", "Expect 0x01 (array tag), got 0x%02x", data);
        if(data != 0x01) {
          parseHDLCState = OUTSIDE_FRAME;
          return;
        }

        uint8_t array_length = read();
        ESP_LOGD("hdlc", "Array length is %d", array_length);

        for(int i=0;i<array_length;i++) {
          if(!readHDLCStruct(&parsed)) {
            parseHDLCState = OUTSIDE_FRAME;
            return;
          }
        }

        publishSensors(&parsed);


        while (true)
        {
          data = read();
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