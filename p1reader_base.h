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

#ifndef P1READER_BASE_H
#define P1READER_BASE_H

#include "esphome.h"
#include "parsed_message.h"

#define BUF_SIZE 60

class P1ReaderBase : public PollingComponent, public UARTDevice {
  int _pollingIntervalMs;
  int _pollingIntervalLog = 30;

  protected:
    ParsedMessage _parsedMessage = ParsedMessage();
    char _buffer[BUF_SIZE];
    uint8_t _bufferLen;
    int _uSecondsPerByte;

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

    P1ReaderBase(UARTComponent *parent) : PollingComponent(), UARTDevice(parent) {
      // Calculate pollingInterval for Component given our uart buffer size and the rest
      size_t rxBufferSize = parent->get_rx_buffer_size();
      uint8_t bits = parent->get_data_bits() + parent->get_stop_bits() + (parent->get_parity() != UART_CONFIG_PARITY_NONE ? 1 : 0) + 1;
      float secondsPerByte = (float)bits * (1.0f / (float) parent->get_baud_rate());

      ESP_LOGD("setup", "secondsPerByte calculated as: %f s", secondsPerByte);
      
      _uSecondsPerByte = (int) (secondsPerByte * 1000000.0f);
      // Keep a margin of 20%
      _pollingIntervalMs = (int)((float)rxBufferSize * secondsPerByte * 800.0f);
      
      set_update_interval(_pollingIntervalMs);
    }

    float get_setup_priority() const override { return esphome::setup_priority::LATE; }

    void setup() override {
      // Start with a clean buffer
      memset(_buffer, 0, BUF_SIZE);
      _bufferLen = 0;
      
      _parsedMessage.initNewTelegram();
    }

    void update() override {
      // Defer setup logging to here (and repeat a few times) as OTA logs do not pick up initial logs otherwise
      if (_pollingIntervalLog >= 0) {
        if (_pollingIntervalMs < 20) {
          ESP_LOGE("setup", "Polling interval is too low: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                   _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
        } else if (_pollingIntervalMs < 100) {
          ESP_LOGW("setup", "Polling interval is low: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                   _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
        } else {
          ESP_LOGI("setup", "Polling interval calculated as: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                   _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
        }
        
        _pollingIntervalLog--;
      }

      // Deliver a parsed and crc ok message in the calls _after_ actually reading it so we 
      // split the work over more scheduler slices since publish_state is slow (and logging is slow
      // so set log level INFO to avoid all teh debug logging slowing things down)
      if (_parsedMessage.telegramComplete) {
        publishSensors(&_parsedMessage);

        if (!_parsedMessage.telegramComplete) {
          readP1Message();
        }
      } else {
        readP1Message();
      }
    }

  protected:

    void publishSensors(ParsedMessage* parsedMessage) {
      if (parsedMessage->crcOk && parsedMessage->telegramComplete)
      {
        if (parsedMessage->sendBatchOne) {
          cumulativeActiveImport->publish_state(parsedMessage->cumulativeActiveImport);
          cumulativeActiveExport->publish_state(parsedMessage->cumulativeActiveExport);
  
          momentaryActiveImport->publish_state(parsedMessage->momentaryActiveImport);
          momentaryActiveExport->publish_state(parsedMessage->momentaryActiveExport);
  
          momentaryActiveImportL1->publish_state(parsedMessage->momentaryActiveImportL1);
          momentaryActiveExportL1->publish_state(parsedMessage->momentaryActiveExportL1);
  
          momentaryActiveImportL2->publish_state(parsedMessage->momentaryActiveImportL2);
          momentaryActiveExportL2->publish_state(parsedMessage->momentaryActiveExportL2);
  
          momentaryActiveImportL3->publish_state(parsedMessage->momentaryActiveImportL3);
          momentaryActiveExportL3->publish_state(parsedMessage->momentaryActiveExportL3);
  
          voltageL1->publish_state(parsedMessage->voltageL1);
          voltageL2->publish_state(parsedMessage->voltageL2);
          voltageL3->publish_state(parsedMessage->voltageL3);
        
          parsedMessage->sendBatchOne = false;
          
          ESP_LOGD("publish", "Sensors published (part one). CRC: %04X", parsedMessage->crc);
        } else {
          currentL1->publish_state(parsedMessage->currentL1);
          currentL2->publish_state(parsedMessage->currentL2);
          currentL3->publish_state(parsedMessage->currentL3);
  
          cumulativeReactiveImport->publish_state(parsedMessage->cumulativeReactiveImport);
          cumulativeReactiveExport->publish_state(parsedMessage->cumulativeReactiveExport);
  
          momentaryReactiveImport->publish_state(parsedMessage->momentaryReactiveImport);
          momentaryReactiveExport->publish_state(parsedMessage->momentaryReactiveExport);
  
          momentaryReactiveImportL1->publish_state(parsedMessage->momentaryReactiveImportL1);
          momentaryReactiveExportL1->publish_state(parsedMessage->momentaryReactiveExportL1);
  
          momentaryReactiveImportL2->publish_state(parsedMessage->momentaryReactiveImportL2);
          momentaryReactiveExportL2->publish_state(parsedMessage->momentaryReactiveExportL2);
  
          momentaryReactiveImportL3->publish_state(parsedMessage->momentaryReactiveImportL3);
          momentaryReactiveExportL3->publish_state(parsedMessage->momentaryReactiveExportL3);
  
          ESP_LOGI("publish", "Sensors published (complete). CRC: %04X", parsedMessage->crc);
  
          parsedMessage->initNewTelegram();
        }
      } else if (!parsedMessage->crcOk && parsedMessage->telegramComplete) {
        parsedMessage->initNewTelegram();
      }
    }
    
    size_t readBytesUntilAndIncluding(char terminator, char *buffer, size_t length) {
      size_t index = 0;
      while (index < length) {
        uint8_t c;
        bool hasData = read_byte(&c);
        if (!hasData) break;
        *buffer++ = (char)c;
        index++;
        if (c == terminator) break;
      }
      return index; // return number of characters, not including null terminator
    }    

    virtual void readP1Message() = 0;
};

#endif