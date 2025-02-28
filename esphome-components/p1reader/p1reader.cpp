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
                uint32_t start = millis();
    
                while (parsedMessage->sensorsToSend > 0)
                {
                    switch (parsedMessage->sensorsToSend--)
                    {
                        case 1:
                            if (cumulative_active_import != nullptr)
                                cumulative_active_import->publish_state(parsedMessage->cumulativeActiveImport);
                            break;
                        case 2:
                            if (cumulative_active_export != nullptr)
                                cumulative_active_export->publish_state(parsedMessage->cumulativeActiveExport);
                            break;
                        case 3:
                            if (momentary_active_import != nullptr)
                                momentary_active_import->publish_state(parsedMessage->momentaryActiveImport);
                            break;
                        case 4:
                            if (momentary_active_export != nullptr)
                                momentary_active_export->publish_state(parsedMessage->momentaryActiveExport);
                            break;
                        case 5:
                            if (momentary_active_import_l1 != nullptr)
                                momentary_active_import_l1->publish_state(parsedMessage->momentaryActiveImportL1);
                            break;
                        case 6:
                            if (momentary_active_export_l1 != nullptr)
                                momentary_active_export_l1->publish_state(parsedMessage->momentaryActiveExportL1);
                            break;
                        case 7:
                            if (momentary_active_import_l2 != nullptr)
                                momentary_active_import_l2->publish_state(parsedMessage->momentaryActiveImportL2);
                            break;
                        case 8:
                            if (momentary_active_export_l2 != nullptr)
                                momentary_active_export_l2->publish_state(parsedMessage->momentaryActiveExportL2);
                            break;
                        case 9:
                            if (momentary_active_import_l3 != nullptr)
                                momentary_active_import_l3->publish_state(parsedMessage->momentaryActiveImportL3);
                            break;
                        case 10:
                            if (momentary_active_export_l3 != nullptr)
                                momentary_active_export_l3->publish_state(parsedMessage->momentaryActiveExportL3);
                            break;
                        case 11:
                            if (voltage_l1 != nullptr)
                                voltage_l1->publish_state(parsedMessage->voltageL1);
                            break;
                        case 12:
                            if (voltage_l2 != nullptr)
                                voltage_l2->publish_state(parsedMessage->voltageL2);
                            break;
                        case 13:
                            if (voltage_l3 != nullptr)
                                voltage_l3->publish_state(parsedMessage->voltageL3);
                            break;
                        case 14:
                            if (current_l1 != nullptr)
                                current_l1->publish_state(parsedMessage->currentL1);
                            break;
                        case 15:
                            if (current_l2 != nullptr)
                                current_l2->publish_state(parsedMessage->currentL2);
                            break;
                        case 16:
                            if (current_l3 != nullptr)
                                current_l3->publish_state(parsedMessage->currentL3);
                            break;
                        case 17:
                            if (cumulative_reactive_import != nullptr)
                                cumulative_reactive_import->publish_state(parsedMessage->cumulativeReactiveImport);
                            break;
                        case 18:
                            if (cumulative_reactive_export != nullptr)
                                cumulative_reactive_export->publish_state(parsedMessage->cumulativeReactiveExport);
                            break;
                        case 19:
                            if (momentary_reactive_import != nullptr)
                                momentary_reactive_import->publish_state(parsedMessage->momentaryReactiveImport);
                            break;
                        case 20:
                            if (momentary_reactive_export != nullptr)
                                momentary_reactive_export->publish_state(parsedMessage->momentaryReactiveExport);
                            break;
                        case 21:
                            if (momentary_reactive_import_l1 != nullptr)
                                momentary_reactive_import_l1->publish_state(parsedMessage->momentaryReactiveImportL1);
                            break;
                        case 22:
                            if (momentary_reactive_export_l1 != nullptr)
                                momentary_reactive_export_l1->publish_state(parsedMessage->momentaryReactiveExportL1);
                            break;
                        case 23:
                            if (momentary_reactive_import_l2 != nullptr)
                                momentary_reactive_import_l2->publish_state(parsedMessage->momentaryReactiveImportL2);
                            break;
                        case 24:
                            if (momentary_reactive_export_l2 != nullptr)
                                momentary_reactive_export_l2->publish_state(parsedMessage->momentaryReactiveExportL2);
                            break;
                        case 25:
                            if (momentary_reactive_import_l3 != nullptr)
                                momentary_reactive_import_l3->publish_state(parsedMessage->momentaryReactiveImportL3);
                            break;
                        case 26:
                            if (momentary_reactive_export_l3 != nullptr)
                                momentary_reactive_export_l3->publish_state(parsedMessage->momentaryReactiveExportL3);
                            break;
                        default:
                            // Unused
                            break;
                    }

                    if ((millis() - start) > 20)
                    {
                        return; // Wait for next execution slice
                    }
                }

                ESP_LOGI("publish", "Sensors published (complete). CRC: %04X", parsedMessage->crc);
                parsedMessage->initNewTelegram();
            }
            else if (!parsedMessage->crcOk && parsedMessage->telegramComplete)
            {
                parsedMessage->initNewTelegram();
            }
        }
    
        void P1Reader::readP1MessageAscii()
        {
            uint32_t start = millis();
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
                        ESP_LOGV("data", "Partial line [%s] received, busywaiting for one byte", _buffer);
                        // if we did not get a complete line, busywait for a single byte over uart
                        delayMicroseconds(_uSecondsPerByte);
                    }
                }

                if ((millis() - start) > 20)
                {
                    ESP_LOGD("ascii", "Waiting for the next time slice while reading message...");
                    break;
                }
            }
        }

        size_t P1Reader::readBytesUntilAndIncluding(char terminator, char *buffer, size_t length)
        {
            size_t index = 0;
            uint32_t start = millis();
            while (index < length)
            {
                uint8_t c;
                bool hasData = read_byte(&c);
                if (!hasData)
                {
                    break;
                }
                *buffer++ = (char)c;
                index++;
                if (c == terminator)
                {
                    break;
                }
            }

            return index; // return number of characters, not including terminator
        }

        uint16_t crc16_x25(byte* data, int len)
        {
            uint16_t crc = 0xffff;
            for (int i = 0; i < len; i++)
            {
                crc ^= data[i];
                for (unsigned k = 0; k < 8; k++)
                    crc = (crc & 1) != 0 ? (crc >> 1) ^ 0x8408 : crc >> 1;
            }
            return ~crc;
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
                uint32_t start = millis();

                while (_parseHDLCState == OUTSIDE_FRAME)
                {
                    bool hasData = read_byte(&data);
                    if (hasData && data == 0x7e)
                    {
                        uint8_t wait = 10;
                        while (!hasData && wait > 0)
                        {
                            hasData = read_byte(&data);
                            if (!hasData)
                            {
                                delayMicroseconds(_uSecondsPerByte);
                                wait--;
                            }
                        }

                        if (!hasData)
                        {
                            ESP_LOGD("hdlc", "Possibly found end of frame while looking for start of frame, bailing out and trying again later...");
                        }

                        // clean buffer for next packet
                        memset(_buffer, 0, BUF_SIZE);
                        _bufferLen = 0;

                        if (data == 0x7e)
                        {
                            _buffer[_bufferLen++] = data;
                        }
                        else
                        {
                            _buffer[_bufferLen++] = 0x7e;
                            _buffer[_bufferLen++] = data;
                        }

                        _parseHDLCState = READING_FRAME;
                        break;
                        ESP_LOGD("hdlc", "Found start of frame...");
                    }
                }

                while (_parseHDLCState == READING_FRAME)
                {
                    bool hasData = read_byte(&data);
                    if (hasData)
                    {
                        _buffer[_bufferLen++] = data;

                        if (data == 0x7e)
                        {
                            _parseHDLCState = FOUND_FRAME;
                            return; // Always parse in a separate timeslot
                            ESP_LOGD("hdlc", "Found end of frame...");
                        }

                        if (_bufferLen >= BUF_SIZE)
                        {
                            _parseHDLCState = OUTSIDE_FRAME;
                            ESP_LOGE("hdlc", "Failed to read frame, buffer overflow, bailing out...");
                            return;
                        }
                    }
                    else
                    {
                        // No byte available, busywait for a single byte over uart
                        delayMicroseconds(_uSecondsPerByte);
                        if ((millis() - start) > 10)
                        {
                            ESP_LOGD("hdlc", "Failed to fetch expected data within 10ms, bailing out and trying later.");
                            return;
                        }
                    }
                }
            }
            
            if (_parseHDLCState == FOUND_FRAME)
            {
                if (_bufferLen < 17)
                {
                    _parseHDLCState = OUTSIDE_FRAME;
                    ESP_LOGE("hdlc", "Frame to small, skipping to next frame. (%d)", _bufferLen);
                    return;
                }

                uint16_t messageLength = ((_buffer[1] & 0x0f) << 8) + (uint8_t)_buffer[2];
                if (messageLength != (_bufferLen - 2))
                {
                    _parseHDLCState = OUTSIDE_FRAME;
                    ESP_LOGE("hdlc", "Message length (%d) not matching frame length (%d), skipping to next frame.", 
                            messageLength, _bufferLen-2);
                    return;
                }

                uint16_t crc = ((uint8_t)_buffer[_bufferLen-2] << 8) | (uint8_t)_buffer[_bufferLen-3];
                uint16_t crcCalculated = crc16_x25((byte*)_buffer + 1, _bufferLen - 4); // FCS
                if (crc != crcCalculated)
                {
                    _parseHDLCState = OUTSIDE_FRAME;
                    ESP_LOGE("hdlc", "Message crc (%04x) not matching frame crc (%04x), skipping to next frame.", 
                            crc, crcCalculated);
                    return;
                }

                _parsedMessage.crcOk = true;

                _messagePos = 17;

                // Skip date field (normally 0)
                _messagePos += _buffer[_messagePos++];

                // Check for start of struct array
                if (_buffer[_messagePos++] != 0x01)
                {
                    _parseHDLCState = OUTSIDE_FRAME;
                    ESP_LOGE("hdlc", "Message array start tag (0x01) missing, got (%x), skipping to next frame.", 
                            _buffer[_messagePos-1]);
                    return;
                }

                uint8_t structCount = _buffer[_messagePos++];
                ESP_LOGD("hdlc", "Number of structs are %d", structCount);

                for (int i=0; i<structCount; i++) 
                {
                    if (!parseHDLCStruct())
                    {
                        _parseHDLCState = OUTSIDE_FRAME;
                        ESP_LOGE("hdlc", "Failed to parse structs");
                        return;
                    }
                }

                _parsedMessage.telegramComplete = true;
            }
        }

        bool P1Reader::parseHDLCStruct()
        {
            char obis[7];
            memset(obis, 0, 7);
            bool is_signed = false;
            double scaleFactors[10] = { 0.0001, 0.001, 0.01, 0.1, 1.0,
                                        10.0, 100.0, 1000.0,
                                        10000.0, 100000.0 };
            int8_t scale = 0;
            int32_t value = 0;
            uint32_t uvalue = 0xffffffff;

            // Check for start of struct
            if (_buffer[_messagePos++] != 0x02)
            {
                _parseHDLCState = OUTSIDE_FRAME;
                ESP_LOGE("hdlc", "Message struct start tag (0x02) missing, got (%x), skipping to next frame.", 
                        _buffer[_messagePos-1]);
                return false;
            }

            uint8_t structElements = _buffer[_messagePos++];
            ESP_LOGV("hdlc", "Number of struct elements are %d", structElements);

            for (int i=0; i<structElements; i++) 
            {
                if (_messagePos >= _bufferLen)
                {
                    _parseHDLCState = OUTSIDE_FRAME;
                    ESP_LOGE("hdlc", "Reading (%d) past end of message (%d).", 
                            _messagePos, _bufferLen);
                    return false;
                }

                uint8_t tag = _buffer[_messagePos++];
                switch (tag)
                {
                    case 0x02: 
                        {
                            // another inner struct
                            uint8_t innerStructElements = _buffer[_messagePos++];
                            ESP_LOGV("hdlc", "Number of inner struct elements are %d", innerStructElements);

                            for (int j=0; j<innerStructElements; j++) 
                            {
                                uint8_t innerTag = _buffer[_messagePos++];
                                switch (innerTag)
                                {
                                    case 0x0f:
                                        scale = _buffer[_messagePos++]; // 10E(scale)
                                        break;
                                    case 0x16: 
                                    {
                                        // Unit
                                        // 0x1b: (k)W
                                        // 0x1d: (k)VAr
                                        // 0x1e: (k)Wh
                                        // 0x20: (k)VArh
                                        // 0x21: A
                                        // 0x23: V
                                        uint8_t unit = _buffer[_messagePos++];
                                        if (scale == 0 && unit != 0x21 && unit != 0x23)
                                            scale = -3; // ref KILO in sensor.py
                                        break;
                                    }
                                    default:
                                        ESP_LOGE("hdlc", "Unknown tag encountered (%x)", innerTag);
                                        break;
                                }
                            }
                            break;
                        }
                    case 0x06:
                        uvalue = (uint8_t)_buffer[_messagePos + 3] | 
                                ((uint8_t)_buffer[_messagePos + 2] << 8) | 
                                ((uint8_t)_buffer[_messagePos + 1] << 16) | 
                                ((uint8_t)_buffer[_messagePos] << 24);
                        _messagePos += 4;
                        break;
                    case 0x09:
                        {
                            uint8_t rowLen = _buffer[_messagePos++];
                            if (rowLen == 6)
                            {
                                // Map to string for ascii parser
                                if (_buffer[_messagePos + 2] > 9)
                                {
                                    obis[0] = (_buffer[_messagePos + 2] / 10) + 48;
                                    obis[1] = (_buffer[_messagePos + 2] % 10) + 48;
                                    obis[3] = _buffer[_messagePos + 3] + 48;
                                    obis[5] = _buffer[_messagePos + 4] + 48;
                                    obis[2] = obis[4] = '.';
                                }
                                else
                                {
                                    obis[0] = _buffer[_messagePos + 2] + 48;
                                    obis[2] = _buffer[_messagePos + 3] + 48;
                                    obis[4] = _buffer[_messagePos + 4] + 48;
                                    obis[1] = obis[3] = '.';
                                }
                            }
                            _messagePos += rowLen;
                            break;
                        }
                    case 0x10:
                        value = _buffer[_messagePos + 1] | (uint8_t)_buffer[_messagePos + 0] << 8;
                        _messagePos += 2;
                        break;
                    case 0x12:
                        value = (int16_t)(_buffer[_messagePos + 1] | (uint8_t)_buffer[_messagePos + 0] << 8);
                        _messagePos += 2;
                        break;
                    default:
                        ESP_LOGE("hdlc", "Unknown tag encountered (%x)", tag);
                        break;
                }
            }

            if (obis[0] == '\0')
            {
                ESP_LOGV("hdlc", "No data found in struct.");
                return true;
            }

            double scaledValue;
            
            if (uvalue == 0xffffffff)
                scaledValue = scaleFactors[scale + 4] * value;
            else
                scaledValue = scaleFactors[scale + 4] * uvalue;

            ESP_LOGD("hdlc", "VAL %s, %f, %d\n", obis, scaledValue, scale);

            _parsedMessage.parseRow(obis, scaledValue);

            return true;
        }
    }
}
