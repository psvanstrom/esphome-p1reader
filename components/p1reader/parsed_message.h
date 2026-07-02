#pragma once

namespace esphome
{
    namespace p1_reader
    {
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

            uint16_t crc;
            bool telegramComplete;
            bool crcOk;
            uint8_t sensorsToSend;

            void parseRow(const char* obisCode, const char* value)
            {
                double obisValue = simpleatof(value);

                parseRow(obisCode, obisValue);
            }

            void parseRow(const char* obisCode, double obisValue)
            {
                int obisCodeLen = strnlen(obisCode, 7);
                
                if (obisCode[obisCodeLen-1] == '0' &&
                    obisCode[obisCodeLen-2] == '.' &&
                    obisCode[obisCodeLen-4] == '.')
                {
                    switch (obisCodeLen)
                    {
                        case 5: switch(obisCode[obisCodeLen-3])
                        {
                            case '7': switch (obisCode[0])
                            {
                                case '1': 
                                    momentaryActiveImport = obisValue;
                                    break;
                                case '2': 
                                    momentaryActiveExport = obisValue;
                                    break;
                                case '3': 
                                    momentaryReactiveImport = obisValue;
                                    break;
                                case '4': 
                                    momentaryReactiveExport = obisValue;
                                    break;
                                default: break;
                            }
                            break;
                            case '8': switch (obisCode[0])
                            {
                                case '1': 
                                    cumulativeActiveImport = obisValue;
                                    break;
                                case '2': 
                                    cumulativeActiveExport = obisValue;
                                    break;
                                case '3': 
                                    cumulativeReactiveImport = obisValue;
                                    break;
                                case '4': 
                                    cumulativeReactiveExport = obisValue;
                                    break;
                                default: break;
                            }
                            break;
                            default: break;
                        }
                        break;
                        case 6: if (obisCode[obisCodeLen-3] == '7') {
                            switch(obisCode[1]) {
                                case '1': switch (obisCode[0])
                                {
                                    case '2': 
                                        momentaryActiveImportL1 = obisValue;
                                        break;
                                    case '3': 
                                        currentL1 = obisValue;
                                        break;
                                    case '4': 
                                        momentaryActiveImportL2 = obisValue;
                                        break;
                                    case '5': 
                                        currentL2 = obisValue;
                                        break;
                                    case '6': 
                                        momentaryActiveImportL3 = obisValue;
                                        break;
                                    case '7': 
                                        currentL3 = obisValue;
                                        break;
                                    default: break;
                                }
                                break;
                                case '2': switch (obisCode[0])
                                {
                                    case '2': 
                                        momentaryActiveExportL1 = obisValue;
                                        break;
                                    case '3': 
                                        voltageL1 = obisValue;
                                        break;
                                    case '4': 
                                        momentaryActiveExportL2 = obisValue;
                                        break;
                                    case '5': 
                                        voltageL2 = obisValue;
                                        break;
                                    case '6': 
                                        momentaryActiveExportL3 = obisValue;
                                        break;
                                    case '7': 
                                        voltageL3 = obisValue;
                                        break;
                                    default: break;
                                }
                                break;
                                case '3': switch (obisCode[0])
                                {
                                    case '2': 
                                        momentaryReactiveImportL1 = obisValue;
                                        break;
                                    case '4': 
                                        momentaryReactiveImportL2 = obisValue;
                                        break;
                                    case '6': 
                                        momentaryReactiveImportL3 = obisValue;
                                        break;
                                    default: break;
                                }
                                break;
                                case '4': switch (obisCode[0])
                                {
                                    case '2': 
                                        momentaryReactiveExportL1 = obisValue;
                                        break;
                                    case '4': 
                                        momentaryReactiveExportL2 = obisValue;
                                        break;
                                    case '6': 
                                        momentaryReactiveExportL3 = obisValue;
                                        break;
                                    default: break;
                                }
                                break;
                                default: break;
                            }
                        }
                        break;
                        default: break;
                    }
                }
            }

            // Limitations: 
            //   Numbers larger than 2G will fail (but spec only goes to 99999999.999 so ok)
            //   And numbers have no more than 3 decimals in the spec.
            //   spec == Swedish spec for H1
            double simpleatof(const char* value)
            {
                double decFactors[10] = {10.0, 100.0, 1000.0, 100000.0,
                                        1000000.0, 10000000.0, 100000000.0,
                                        1000000000.0, 10000000000.0,
                                        100000000000.0};
                int idx = 0;
                int intPart = 0;
                bool negative = false;

                if (value[idx] == '-')
                {
                    negative = true;
                    idx++;
                }

                while (value[idx] != '.')
                {
                    intPart = intPart*10 + (value[idx]-'0');
                    idx++;
                }

                int len = strlen(value), startIdx = ++idx;
                int decPart = 0;
                while (idx < len)
                {
                    decPart = decPart*10 + (value[idx]-'0');
                    idx++;
                }

                if (negative)
                {
                    return -(intPart + decPart/decFactors[len-startIdx-1]);
                }
                else
                {
                    return intPart + decPart/decFactors[len-startIdx-1];
                }
            }

            void initNewTelegram()
            {
                crc = 0x0000;
                telegramComplete = false;
                crcOk = false;
                sensorsToSend = 26;
            }

            void updateCrc16(uint8_t a)
            {
                int i;
                crc ^= a;
                for (i = 0; i < 8; ++i)
                {
                    if (crc & 1)
                    {
                        crc = (crc >> 1) ^ 0xA001;
                    }
                    else
                    {
                        crc = (crc >> 1);
                    }
                }
            }
            
            void checkCrc(uint16_t crcFromMessage)
            {
                crcOk = crc == crcFromMessage;
                telegramComplete = true;
            }
        };
    }
}