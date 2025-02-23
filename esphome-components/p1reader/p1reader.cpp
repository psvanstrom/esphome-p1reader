//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2020-11-05:   Initial release
//  0.x.0 2023-04-18:   Added HDLC support
//  0.2.0 2023-08-14:   Some optimizations when ESPHome started to complain about execution times
//  0.3.0 2023-08-23:   Rework structure to merge above changes with HDLC stuff
//  0.4.0 2025-02-25:   Migrate to esphome external component
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

#include "p1reader.h"

namespace esphome
{
    namespace p1_reader
    {
        void P1Reader::setup()
        {
            // Calculate pollingInterval for Component given our uart buffer size and the rest
            size_t rxBufferSize = parent_->get_rx_buffer_size();
            uint8_t bits = parent_->get_data_bits() + parent_->get_stop_bits() + 
                            (parent_->get_parity() != uart::UART_CONFIG_PARITY_NONE ? 1 : 0) + 1;
            float secondsPerByte = (float)bits * (1.0f / (float) parent_->get_baud_rate());

            ESP_LOGI("setup", "secondsPerByte calculated as: %f s", secondsPerByte);
            
            _uSecondsPerByte = (int) (secondsPerByte * 1000000.0f);
            // Keep a margin of 20%
            _pollingIntervalMs = (int)((float)rxBufferSize * secondsPerByte * 800.0f);
            
            if (_pollingIntervalMs < 20)
            {
                ESP_LOGE("setup", "Polling interval is too low: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                    _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
            } 
            else if (_pollingIntervalMs < 100)
            {
                ESP_LOGW("setup", "Polling interval is low: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                        _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
            }
            else
            {
                ESP_LOGI("setup", "Polling interval calculated as: %d ms (rx_buffer_size %d, uSecondsPerByte %d)", 
                        _pollingIntervalMs, parent_->get_rx_buffer_size(), _uSecondsPerByte);
            }
                
            set_update_interval(_pollingIntervalMs);

            // Start with a clean buffer
            memset(_buffer, 0, BUF_SIZE);
            _bufferLen = 0;
            ESP_LOGI("setup", "Internal buffer size is %d", BUF_SIZE);

            _parsedMessage.initNewTelegram();
        }

        void P1Reader::update()
        {
            // Deliver a parsed and crc ok message in the calls _after_ actually reading it so we 
            // split the work over more scheduler slices since publish_state is slow (and logging is slow
            // so set log level INFO to avoid all the debug logging slowing things down)
            if (_parsedMessage.telegramComplete)
            {
                publishSensors(&_parsedMessage);

                if (!_parsedMessage.telegramComplete)
                {
                    (this->*readP1Message)();
                }
            }
            else
            {
                (this->*readP1Message)();
            }
        }

        void P1Reader::publishSensors(ParsedMessage* parsedMessage)
        {
            if (parsedMessage->crcOk && parsedMessage->telegramComplete)
            {
                if (parsedMessage->sendBatchOne)
                {
                    if (cumulative_active_import != nullptr)
                        cumulative_active_import->publish_state(parsedMessage->cumulativeActiveImport);
                    if (cumulative_active_export != nullptr)
                        cumulative_active_export->publish_state(parsedMessage->cumulativeActiveExport);

                    if (momentary_active_import != nullptr)
                        momentary_active_import->publish_state(parsedMessage->momentaryActiveImport);
                    if (momentary_active_export != nullptr)
                        momentary_active_export->publish_state(parsedMessage->momentaryActiveExport);

                    if (momentary_active_import_l1 != nullptr)
                        momentary_active_import_l1->publish_state(parsedMessage->momentaryActiveImportL1);
                    if (momentary_active_export_l1 != nullptr)
                        momentary_active_export_l1->publish_state(parsedMessage->momentaryActiveExportL1);

                    if (momentary_active_import_l2 != nullptr)
                        momentary_active_import_l2->publish_state(parsedMessage->momentaryActiveImportL2);
                    if (momentary_active_export_l2 != nullptr)
                        momentary_active_export_l2->publish_state(parsedMessage->momentaryActiveExportL2);
                    
                    if (momentary_active_import_l3 != nullptr)
                        momentary_active_import_l3->publish_state(parsedMessage->momentaryActiveImportL3);
                    if (momentary_active_export_l3 != nullptr)
                        momentary_active_export_l3->publish_state(parsedMessage->momentaryActiveExportL3);

                    if (voltage_l1 != nullptr)
                        voltage_l1->publish_state(parsedMessage->voltageL1);
                    if (voltage_l2 != nullptr)
                        voltage_l2->publish_state(parsedMessage->voltageL2);
                    if (voltage_l3 != nullptr)
                        voltage_l3->publish_state(parsedMessage->voltageL3);
                
                    parsedMessage->sendBatchOne = false;
                    
                    ESP_LOGD("publish", "sensors published (part one). CRC: %04X", parsedMessage->crc);
                }
                else
                {
                    if (current_l1 != nullptr)
                        current_l1->publish_state(parsedMessage->currentL1);
                    if (current_l2 != nullptr)
                        current_l2->publish_state(parsedMessage->currentL2);
                    if (current_l3 != nullptr)
                        current_l3->publish_state(parsedMessage->currentL3);

                    if (cumulative_reactive_import != nullptr)
                        cumulative_reactive_import->publish_state(parsedMessage->cumulativeReactiveImport);
                    if (cumulative_reactive_export != nullptr)
                        cumulative_reactive_export->publish_state(parsedMessage->cumulativeReactiveExport);

                    if (momentary_reactive_import != nullptr)
                        momentary_reactive_import->publish_state(parsedMessage->momentaryReactiveImport);
                    if (momentary_reactive_export != nullptr)
                        momentary_reactive_export->publish_state(parsedMessage->momentaryReactiveExport);

                    if (momentary_reactive_import_l1 != nullptr)
                        momentary_reactive_import_l1->publish_state(parsedMessage->momentaryReactiveImportL1);
                    if (momentary_reactive_export_l1 != nullptr)
                        momentary_reactive_export_l1->publish_state(parsedMessage->momentaryReactiveExportL1);

                    if (momentary_reactive_import_l2 != nullptr)
                        momentary_reactive_import_l2->publish_state(parsedMessage->momentaryReactiveImportL2);
                    if (momentary_reactive_export_l2 != nullptr)
                        momentary_reactive_export_l2->publish_state(parsedMessage->momentaryReactiveExportL2);

                    if (momentary_reactive_import_l3 != nullptr)
                        momentary_reactive_import_l3->publish_state(parsedMessage->momentaryReactiveImportL3);
                    if (momentary_reactive_export_l3 != nullptr)
                        momentary_reactive_export_l3->publish_state(parsedMessage->momentaryReactiveExportL3);

                    ESP_LOGI("publish", "Sensors published (complete). CRC: %04X", parsedMessage->crc);

                    parsedMessage->initNewTelegram();
                }
            }
            else if (!parsedMessage->crcOk && parsedMessage->telegramComplete)
            {
                parsedMessage->initNewTelegram();
            }
        }
    
        void P1Reader::readP1MessageAscii()
        {
            while (available())
            {
                int len = readBytesUntilAndIncluding('\n', _buffer + _bufferLen, BUF_SIZE-_bufferLen);

                if (len > 0)
                {
                    _bufferLen += len;
                    bool lineComplete = _buffer[_bufferLen-1] == '\n';
                
                    if (lineComplete)
                    {
                        // if we've reached the CRC checksum, calculate last CRC and compare
                        if (_buffer[0] == '!')
                        {
                            _parsedMessage.updateCrc16(_buffer[0]);
                            int crcFromMsg = (int) strtol(_buffer + 1, NULL, 16);
                            _parsedMessage.checkCrc(crcFromMsg);

                            ESP_LOGI("crc", "Telegram read. CRC: %04X = %04X. PASS = %s", 
                                    _parsedMessage.crc, crcFromMsg, _parsedMessage.crcOk ? "YES": "NO");

                        // otherwise pass the row through the CRC calculation
                        } 
                        else 
                        {
                            for (int i = 0; i < _bufferLen; i++)
                            {
                                _parsedMessage.updateCrc16(_buffer[i]);
                            }
                        }

                        // Remove CR LF before logging and processing
                        _buffer[_bufferLen-1] = '\0';
                        if (_buffer[_bufferLen-2] == '\r')
                            _buffer[_bufferLen-2] = '\0';

                        ESP_LOGV("data", "Complete line [%s] received", _buffer);

                        // if this is a row containing information
                        if (strchr(_buffer, '(') != NULL)
                        {
                            char* dataId = strtok(_buffer, DELIMITERS);
                            char* obisCode = strtok(NULL, DELIMITERS);

                            // ...and this row is a data row, then parse row
                            if (strncmp(DATA_ID, dataId, strlen(DATA_ID)) == 0)
                            {
                                char* value = strtok(NULL, DELIMITERS);
                                char* unit = strtok(NULL, DELIMITERS);
                                _parsedMessage.parseRow(obisCode, value);
                            }
                        }

                        // clean buffer for next line
                        memset(_buffer, 0, BUF_SIZE);
                        _bufferLen = 0;
                    } 
                    else 
                    {
                        ESP_LOGV("data", "Partial line [%s] received", _buffer);
                        // if we did not get a complete line, busywait for a single byte over uart
                        delayMicroseconds(_uSecondsPerByte);
                    }
                }
            }
        }

        size_t P1Reader::readBytesUntilAndIncluding(char terminator, char *buffer, size_t length)
        {
            size_t index = 0;
            while (index < length)
            {
                uint8_t c;
                bool hasData = read_byte(&c);
                if (!hasData) break;
                *buffer++ = (char)c;
                index++;
                if (c == terminator)
                    break;
            }

            return index; // return number of characters, not including null terminator
        }

        /*  Reads messages formatted according to "Branschrekommendation v1.2", which
            at the time of writing (20210207) is used by Tekniska Verken's Aidon 6442SE
            meters. This is a binary format, with a HDLC Frame. 

            This code is in no way a generic HDLC Frame parser, but it does the job
            of decoding this particular data stream.
        */
        void P1Reader::readP1MessageHDLC() 
        {
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
                        } 
                        else if (next == -1)
                        {
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
                if (data != 0x01)
                {
                    parseHDLCState = OUTSIDE_FRAME;
                    return;
                }

                uint8_t array_length = read();
                ESP_LOGD("hdlc", "Array length is %d", array_length);

                for (int i=0;i<array_length;i++) 
                {
                    if(!readHDLCStruct())
                    {
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

        bool P1Reader::readHDLCStruct()
        {
            if (!read_array((uint8_t*)_buffer, 3))
                return false;

            if (_buffer[0] != 0x02)
                return false;

            char obis[7];
            obis[1] = '.';
            obis[3] = '.';
            obis[5] = '\0';

            uint8_t struct_len = _buffer[1];
            //ESP_LOGD("hdlc", "Struct length is %d", struct_len);

            uint8_t tag = _buffer[2];

            if (tag != 0x09)
            {
                ESP_LOGE("hdlc", "Unexpected tag %X in struct, bailing out", tag);
                return false;
            }

            uint8_t str_length = read();
            if (read_array((uint8_t*)_buffer, str_length) != str_length)
            {
                ESP_LOGE("hdlc", "Unable to read %d bytes of OBIS code", str_length);
                return false;
            }
            _buffer[str_length] = 0; // Null-terminate
            obis[0] = _buffer[2] + 48;
            obis[2] = _buffer[3] + 48;
            obis[4] = _buffer[4] + 48;

            tag = read();

            bool is_signed = false;
            uint32_t uvalue = 0;
            int32_t value = 0;
            if (tag == 0x09)
            {
                str_length = read();
                if (read_array((uint8_t*)_buffer, str_length) != str_length)
                {
                    ESP_LOGE("hdlc", "Unable to read %d bytes of string", str_length);
                    return false;
                }

                _buffer[str_length] = 0;
                //ESP_LOGD("hdlc", "Read string length %d", str_length);
            }
            else if (tag == 0x06)
            {
                read_array((uint8_t*)_buffer, 4);
                //uvalue = _buffer[0] | _buffer[1] << 8 | _buffer[2] << 16 | _buffer[3] << 24;
                uvalue = _buffer[3] | _buffer[2] << 8 | _buffer[1] << 16 | _buffer[0] << 24;
                //ESP_LOGD("hdlc", "Value of uvalue is %u", uvalue);
            } 
            else if (tag == 0x10)
            {
                read_array((uint8_t*)_buffer, 2); 
                //value = _buffer[0] | _buffer[1] << 8; // 
                is_signed = true;
                value = _buffer[1] | _buffer[0] << 8;
                //ESP_LOGD("hdlc", "(Signed) Value of value is %d", value);
            }
            else if (tag == 0x12)
            {
                read_array((uint8_t*)_buffer, 2); 
                //uvalue = _buffer[0] | _buffer[1] << 8; // 
                uvalue = _buffer[1] | _buffer[0] << 8;
                //ESP_LOGD("hdlc", "(Unsigned) Value of uvalue is %u", uvalue);
            } 
            else
            {
                ESP_LOGE("hdlc", "unknown tag %X", tag);
            }

            int8_t scaler;
            uint8_t unit;
            if (struct_len == 3)
            {
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
    }
}
