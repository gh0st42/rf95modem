// Copyright (c) 2018 Lars Baumgaertner

#include <Arduino.h>

#include <SPI.h>
#include <RH_RF95.h>

#include "modem.h"

#ifdef USE_DISPLAY
#include <SSD1306.h>
#include <OLEDDisplayUi.h>
#endif // USE_DISPLAY

#ifdef USE_BLE
#include "ble.h"
#endif

#ifdef USE_DISPLAY
// Singleton for display connection
SSD1306 display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
OLEDDisplayUi ui(&display);
#endif // USE_DISPLAY

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.1
//#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Singleton configuration struct
struct RF95ModemConfig
{
    RH_RF95::ModemConfigChoice modem_config;
    float frequency;
    byte rx_listen;
    byte big_ble_frames;
} conf = {RH_RF95::Bw125Cr45Sf128, RF95_FREQ, 1, 0};

void out_print(String text)
{
    Serial.print(text);
#ifdef USE_BLE
    ble_print(text, conf.big_ble_frames);
#endif
}
void out_println(String text)
{
    out_print(text + "\n");
}

void modem_setup()
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
}
void initRF95()
{
    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95.init())
    {
        out_println("+FAIL: LoRa radio init");
        while (1)
            ;
    }

    // Defaults after init are 868.1MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(conf.frequency))
    {
        out_println("+FAIL: Setting frequency");
        while (1)
            ;
    }
    out_print("+FREQ: ");
    out_println(String(conf.frequency));

    // Defaults after init are 868.1MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    rf95.setModemConfig(conf.modem_config);

    // The default transmitter power is 23dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(23, false);
}

#ifdef USE_DISPLAY
void initDisplay()
{
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);

    display.init();
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    display.clear();
}

void printDisplay()
{
    String modeStr;
    switch (conf.modem_config)
    {
    case RH_RF95::Bw125Cr45Sf128:
        modeStr = String("MR");
        break;
    case RH_RF95::Bw125Cr48Sf4096:
        modeStr = String("SLR");
        break;
    case RH_RF95::Bw31_25Cr48Sf512:
        modeStr = String("SLR");
        break;
    case RH_RF95::Bw500Cr45Sf128:
        modeStr = String("FSR");
        break;
    default:
        modeStr = String("??");
    }

    display.clear();

    display.drawString(0, 0, "Frequency:");
    display.drawString(70, 0, String(conf.frequency) + String(", ") + modeStr);
    display.drawString(0, 10, "Transmitted:");
    display.drawString(70, 10, String(rf95.txGood()));
    display.drawString(0, 20, "Received:");
    display.drawString(70, 20, String(rf95.rxGood()));
    display.drawString(0, 30, "Received bad:");
    display.drawString(70, 30, String(rf95.rxBad()));
    display.drawString(0, 40, "RSSI:");
    display.drawString(70, 40, String(rf95.lastRssi()));
    display.drawString(0, 50, "SNR:");
    display.drawString(70, 50, String(rf95.lastSNR()));
    display.display();
}
#endif // USE_DISPLAY

void onpacketreceived(uint8_t *buf, uint8_t len)
{
    int lastRssi = rf95.lastRssi();
    int lastSNR = rf95.lastSNR();
    String output = "+RX ";
    output += String(len, DEC);
    output += ",";
    for (int i = 0; i < len; i++)
    {
        //printf("%02X", buf[i]);
        if (buf[i] < 0x10)
        {
            output += "0";
        }
        output += String(buf[i], HEX);
    }
    output += ",";
    output += String(lastRssi, DEC);
    output += ",";
    output += String(lastSNR, DEC);
    out_println(output);

#ifdef USE_DISPLAY
    printDisplay();
#endif // USE_DISPLAY
}
void handleCommand(String input)
{
    if (input.startsWith("AT+TX="))
    {
        int cmdlen = 6;
        input.toLowerCase();
        const char *instr = input.c_str();
        int plen = input.length() - cmdlen;
        int blen = plen / 2 + plen % 2;
        if (blen > rf95.maxMessageLength())
        {
            String err = "+FAIL: MAX_MESSAGE_LEN EXCEEDED! ";
            err += String(blen, DEC);
            err += " / ";
            err += String(rf95.maxMessageLength(), DEC);
            out_println(err);

            return;
        }
        //Serial.println(plen);
        //Serial.println(blen);
        auto getNum = [](char c) {
            return c > '9' ? c - 'a' + 10 : c - '0';
        };
        uint8_t buf[blen];
        int j = 0;
        int hilo = 0;
        for (int i = cmdlen; i < cmdlen + plen; i++)
        {
            if (hilo == 0)
            {
                buf[j] = getNum(instr[i]) << 4;
            }
            else
            {
                buf[j] |= getNum(instr[i]);
            }
            hilo++;
            if (hilo == 2)
            {
                hilo = 0;
                j++;
            }
        }
        rf95.send((uint8_t *)buf, blen);
        rf95.waitPacketSent();
        String output = "+SENT ";
        output += String(blen, DEC);
        output += " bytes.";
        out_println(output);

#ifdef USE_DISPLAY
        printDisplay();
#endif // USE_DISPLAY
    }
    else if (input.startsWith("AT+MODE="))
    {
        int number = input.substring(8).toInt();
        conf.modem_config = static_cast<RH_RF95::ModemConfigChoice>(number);
        rf95.setModemConfig(conf.modem_config);
        out_println("+OK");
#ifdef USE_DISPLAY
        printDisplay();
#endif // USE_DISPLAY
    }
    else if (input.startsWith("AT+RX="))
    {
        int number = input.substring(6).toInt();
        if (number == 0 || number == 1)
        {
            conf.rx_listen = (byte)number;
            out_println("+OK");
        }
        else
        {
            out_println("+FAIL: Invalid RX mode!");
        }
    }
#ifdef USE_BLE
    else if (input.startsWith("AT+BFB="))
    {
        int number = input.substring(6).toInt();
        if (number == 0 || number == 1)
        {
            conf.big_ble_frames = (byte)number;
            out_println("+OK");
        }
        else
        {
            out_println("+FAIL: Invalid BFB mode!");
        }
    }
#endif
    else if (input.startsWith("AT+FREQ="))
    {
        conf.frequency = input.substring(8).toFloat();

        // Reinitialize the RF95 to use the new frequency
        initRF95();

#ifdef USE_DISPLAY
        printDisplay();
#endif // USE_DISPLAY
    }
    else if (input.startsWith("AT+HELP"))
    {
        out_println("+HELP:");
        out_println("AT+HELP             Print this usage information.");
        out_println("AT+TX=<hexdata>     Send binary data.");
        out_println("AT+RX=<0|1>         Turn receiving on (1) or off (2).");
#ifdef USE_BLE
        out_println("AT+BFB=<0|1>        Turn send Big Fine BLE-Frames on (1) or off (2).");
#endif
        out_println("AT+FREQ=<freq>      Changes the frequency.");
        out_println("AT+INFO             Output status information.");
        out_println("AT+MODE=<NUM>       Set modem config:");
        out_println("                    " + String(RH_RF95::Bw125Cr45Sf128) + " - medium range (default)");
        out_println("                     Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.");
        out_println("                    " + String(RH_RF95::Bw500Cr45Sf128) + " - fast+short range");
        out_println("                     Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.");
        out_println("                    " + String(RH_RF95::Bw31_25Cr48Sf512) + " - slow+long range");
        out_println("                     Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on.");
        out_println("                    " + String(RH_RF95::Bw125Cr48Sf4096) + " - slow+long range");
        out_println("                     Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on.");
        out_println("+OK");
    }
    else if (input.startsWith("AT+INFO"))
    {
        Serial.println("+STATUS:");
        Serial.println();
        Serial.println("firmware:      " + String(VERSION));
        Serial.print("modem config:  ");
        Serial.print(String(conf.modem_config) + " | ");
        switch (conf.modem_config)
        {
        case RH_RF95::Bw125Cr45Sf128:
            Serial.println("medium range");
            break;
        case RH_RF95::Bw125Cr48Sf4096:
            Serial.println("slow+long range");
            break;
        case RH_RF95::Bw31_25Cr48Sf512:
            Serial.println("slow+long range");
            break;
        case RH_RF95::Bw500Cr45Sf128:
            Serial.println("fast+short range");
            break;
        default:
            Serial.println("unknown modem config!");
        }
        Serial.println("max pkt size:  " + String(rf95.maxMessageLength()));
        Serial.println("frequency:     " + String(conf.frequency));
        Serial.println("rx listener:   " + String(conf.rx_listen));
        Serial.println("BFB:           " + String(conf.big_ble_frames));
        Serial.println();
        Serial.println("rx bad:        " + String(rf95.rxBad()));
        Serial.println("rx good:       " + String(rf95.rxGood()));
        Serial.println("tx good:       " + String(rf95.txGood()));
        //rf95.printRegisters();
        out_println("+OK");
    }
    else
    {
        String output = "+FAIL: Unknown command: ";
        output += input;
        out_println(output);
    }
}
#define MAX_COMMAND_LEN 512

void modem_loop_tick()
{
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toUpperCase();
        handleCommand(input);
    }
    if (conf.rx_listen == 1 && rf95.available())
    {
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (rf95.recv(buf, &len))
        {
#ifdef LED
            digitalWrite(LED, HIGH);
#endif // LED

            onpacketreceived(buf, len);
            delay(10);

#ifdef LED
            digitalWrite(LED, LOW);
#endif // LED
        }
        else
        {
            out_println("+RX failed");
        }
    }
}