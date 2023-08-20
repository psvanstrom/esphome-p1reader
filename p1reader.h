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

class P1Reader : public P1ReaderBase {

  const char* DELIMITERS = "()*:";
  const char* DATA_ID = "1-0";

public:    
  P1Reader(UARTComponent *parent) : P1ReaderBase(parent) {}

protected:
  void readP1Message() {
    while (available()) {
      int len = readBytesUntilAndIncluding('\n', _buffer + _bufferLen, BUF_SIZE-_bufferLen);

      if (len > 0) {
        _bufferLen += len;
        bool lineComplete = _buffer[_bufferLen-1] == '\n';
        
        if (lineComplete) {
          // if we've reached the CRC checksum, calculate last CRC and compare
          if (_buffer[0] == '!') {
            _parsedMessage.updateCrc16(_buffer[0]);
            int crcFromMsg = (int) strtol(_buffer + 1, NULL, 16);
            _parsedMessage.checkCrc(crcFromMsg);
            ESP_LOGI("crc", "Telegram read. CRC: %04X = %04X. PASS = %s", 
                      _parsedMessage.crc, crcFromMsg, _parsedMessage.crcOk ? "YES": "NO");

          // otherwise pass the row through the CRC calculation
          } else {
            for (int i = 0; i < _bufferLen; i++) {
              _parsedMessage.updateCrc16(_buffer[i]);
            }
          }

          // Remove CR LF before logging and processing
          _buffer[_bufferLen-1] = '\0';
          if (_buffer[_bufferLen-2] == '\r')
            _buffer[_bufferLen-2] = '\0';
          ESP_LOGD("data", "Complete line [%s] received", _buffer);

          // if this is a row containing information
          if (strchr(_buffer, '(') != NULL) {
            char* dataId = strtok(_buffer, DELIMITERS);
            char* obisCode = strtok(NULL, DELIMITERS);

            // ...and this row is a data row, then parse row
            if (strncmp(DATA_ID, dataId, strlen(DATA_ID)) == 0) {
              char* value = strtok(NULL, DELIMITERS);
              char* unit = strtok(NULL, DELIMITERS);
              _parsedMessage.parseRow(obisCode, value);
            }
          }

          // clean buffer for next line
          memset(_buffer, 0, BUF_SIZE);
          _bufferLen = 0;
        } else {
          ESP_LOGD("data", "Partial line [%s] received", _buffer);
          // if we did not get a complete line, busywait for a single byte over uart
          delayMicroseconds(_uSecondsPerByte);
        }
      }
    }
  }
};
