//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2020-11-05:   Initial release
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

#define BUF_SIZE 50

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

    void loop() override {
      readP1Message();
    }

  private:
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

    void readP1Message() {
      if (available()) {
        uint16_t crc = 0x0000;
        ParsedMessage parsed = ParsedMessage();

        while (available()) {
          int len = readBytesUntil('\n', buffer, BUF_SIZE);

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
        }

        // if the CRC pass, publish sensor values
        if (parsed.crcOk) {
          publishSensors(&parsed);
        }
      }
    }
};
