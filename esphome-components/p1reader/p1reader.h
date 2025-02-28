//-------------------------------------------------------------------------------------
// ESPHome P1 Electricity Meter custom sensor
// Copyright 2020 Pär Svanström
// 
// History
//  0.1.0 2020-11-05:   Initial release
//  0.x.0 2023-04-18:   Added HDLC support
//  0.2.0 2023-08-14:   Some optimizations when ESPHome started to complain about execution times
//  0.3.0 2023-08-23:   Rework structure to merge above changes with HDLC stuff
//  0.4.0 2025-02-23:   Rework as external component
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

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "parsed_message.h"

namespace esphome
{
    namespace p1_reader
    {
        class P1Reader : public PollingComponent, public uart::UARTDevice
        {
        public:
            P1Reader(uart::UARTComponent *parent): PollingComponent(), uart::UARTDevice(parent)
            {}

            void setup() override;
            void update() override;
        protected:
            float get_setup_priority() const override { return esphome::setup_priority::LATE; }

            // Shared
            int _pollingIntervalMs;

            ParsedMessage _parsedMessage = ParsedMessage();
            char _buffer[BUF_SIZE];
            uint16_t _bufferLen;
            int _uSecondsPerByte;

            sensor::Sensor *cumulative_active_import{nullptr};
            sensor::Sensor *cumulative_active_export{nullptr};

            sensor::Sensor *cumulative_reactive_import{nullptr};
            sensor::Sensor *cumulative_reactive_export{nullptr};

            sensor::Sensor *momentary_active_import{nullptr};
            sensor::Sensor *momentary_active_export{nullptr};

            sensor::Sensor *momentary_reactive_import{nullptr};
            sensor::Sensor *momentary_reactive_export{nullptr};

            sensor::Sensor *momentary_active_import_l1{nullptr};
            sensor::Sensor *momentary_active_export_l1{nullptr};

            sensor::Sensor *momentary_active_import_l2{nullptr};
            sensor::Sensor *momentary_active_export_l2{nullptr};

            sensor::Sensor *momentary_active_import_l3{nullptr};
            sensor::Sensor *momentary_active_export_l3{nullptr};

            sensor::Sensor *momentary_reactive_import_l1{nullptr};
            sensor::Sensor *momentary_reactive_export_l1{nullptr};

            sensor::Sensor *momentary_reactive_import_l2{nullptr};
            sensor::Sensor *momentary_reactive_export_l2{nullptr};

            sensor::Sensor *momentary_reactive_import_l3{nullptr};
            sensor::Sensor *momentary_reactive_export_l3{nullptr};

            sensor::Sensor *voltage_l1{nullptr};
            sensor::Sensor *voltage_l2{nullptr};
            sensor::Sensor *voltage_l3{nullptr};

            sensor::Sensor *current_l1{nullptr};
            sensor::Sensor *current_l2{nullptr};
            sensor::Sensor *current_l3{nullptr};

            void publishSensors(ParsedMessage* parsedMessage);

            // ASCII
            const char* DELIMITERS = "()*:";
            const char* DATA_ID = "1-0";

            size_t readBytesUntilAndIncluding(char terminator, char *buffer, size_t length);

            // HLDC
            const int8_t OUTSIDE_FRAME = 0;
            const int8_t READING_FRAME = 2;
            const int8_t FOUND_FRAME = 2;
            
            int8_t _parseHDLCState = OUTSIDE_FRAME;
            uint16_t _messagePos;
            
            bool parseHDLCStruct();

            // Message read abstraction
            void (P1Reader::*readP1Message)(){nullptr};
            void readP1MessageAscii();
            void readP1MessageHDLC();

        public:
            // Component attribute support
            void set_protocol_type(std::string protocol)
            {
                if (protocol == "ascii")
                    readP1Message = &esphome::p1_reader::P1Reader::readP1MessageAscii;
                else
                    readP1Message = &esphome::p1_reader::P1Reader::readP1MessageHDLC;

                ESP_LOGI("setup", "Protocol is %s", protocol.c_str());
            }

            void set_sensor_cumulative_active_import(sensor::Sensor *sensor)
            {
                cumulative_active_import = sensor;
            }
            void set_sensor_cumulative_active_export(sensor::Sensor *sensor)
            { 
                cumulative_active_export = sensor; 
            }

            void set_sensor_cumulative_reactive_import(sensor::Sensor *sensor)
            {
                cumulative_reactive_import = sensor;
            }
            void set_sensor_cumulative_reactive_export(sensor::Sensor *sensor)
            {
                cumulative_reactive_export = sensor;
            }

            void set_sensor_momentary_active_import(sensor::Sensor *sensor)
            {
                momentary_active_import = sensor;
            }
            void set_sensor_momentary_active_export(sensor::Sensor *sensor)
            {
                momentary_active_export = sensor;
            }

            void set_sensor_momentary_reactive_import(sensor::Sensor *sensor)
            {
                momentary_reactive_import = sensor;
            }
            void set_sensor_momentary_reactive_export(sensor::Sensor *sensor)
            {
                momentary_reactive_export = sensor;
            }

            void set_sensor_momentary_active_import_l1(sensor::Sensor *sensor)
            {
                momentary_active_import_l1 = sensor;
            }
            void set_sensor_momentary_active_export_l1(sensor::Sensor *sensor)
            {
                momentary_active_export_l1 = sensor;
            }

            void set_sensor_momentary_active_import_l2(sensor::Sensor *sensor)
            {
                momentary_active_import_l2 = sensor;
            }
            void set_sensor_momentary_active_export_l2(sensor::Sensor *sensor)
            {
                momentary_active_export_l2 = sensor;
            }

            void set_sensor_momentary_active_import_l3(sensor::Sensor *sensor)
            {
                momentary_active_import_l3 = sensor;
            }
            void set_sensor_momentary_active_export_l3(sensor::Sensor *sensor)
            {
                momentary_active_export_l3 = sensor;
            }

            void set_sensor_momentary_reactive_import_l1(sensor::Sensor *sensor)
            {
                momentary_reactive_import_l1 = sensor;
            }
            void set_sensor_momentary_reactive_export_l1(sensor::Sensor *sensor)
            {
                momentary_reactive_export_l1 = sensor;
            }

            void set_sensor_momentary_reactive_import_l2(sensor::Sensor *sensor)
            {
                momentary_reactive_import_l2 = sensor;
            }
            void set_sensor_momentary_reactive_export_l2(sensor::Sensor *sensor)
            {
                momentary_reactive_export_l2 = sensor;
            }

            void set_sensor_momentary_reactive_import_l3(sensor::Sensor *sensor)
            {
                momentary_reactive_import_l3 = sensor;
            }
            void set_sensor_momentary_reactive_export_l3(sensor::Sensor *sensor)
            {
                momentary_reactive_export_l3 = sensor;
            }

            void set_sensor_voltage_l1(sensor::Sensor* sensor)
            {
                voltage_l1 = sensor;
            }
            void set_sensor_voltage_l2(sensor::Sensor* sensor)
            {
                voltage_l2 = sensor;
            }
            void set_sensor_voltage_l3(sensor::Sensor* sensor)
            {
                voltage_l3 = sensor;
            }

            void set_sensor_current_l1(sensor::Sensor* sensor)
            {
                current_l1 = sensor;
            }
            void set_sensor_current_l2(sensor::Sensor* sensor)
            {
                current_l2 = sensor;
            }
            void set_sensor_current_l3(sensor::Sensor* sensor)
            {
                current_l3 = sensor;
            }
        };
    }
}
