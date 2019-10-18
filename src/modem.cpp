#include <Arduino.h>

#include <SPI.h>
#include <RH_RF95.h>
#ifdef BLE
#include "ble.h"
#endif

void out_print(String text)
{
    Serial.print(text);
#ifdef BLE
    ble_print(text);
#endif
}
void out_println(String text)
{
    out_print(text + "\n");
}

#define VERSION "0.2"

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
} conf = {RH_RF95::Bw125Cr45Sf128, RF95_FREQ, 1};

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
        out_println("LoRa radio init failed");
        while (1)
            ;
    }

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(conf.frequency))
    {
        out_println("setFrequency failed");
        while (1)
            ;
    }
    out_print("Set Freq to: ");
    out_println(String(conf.frequency));

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    rf95.setModemConfig(conf.modem_config);

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(23, false);
}

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

        /*for (int i = 0; i < blen; i++) {
      //Serial.printf("%02X", buf[i]);
      Serial.print(buf[i], HEX);
    }
    Serial.println();*/
    }
    else if (input.startsWith("AT+MODE="))
    {
        int number = input.substring(8).toInt();
        conf.modem_config = static_cast<RH_RF95::ModemConfigChoice>(number);
        rf95.setModemConfig(conf.modem_config);
        out_println("+ Ok.");
    }
    else if (input.startsWith("AT+RX="))
    {
        int number = input.substring(6).toInt();
        if (number == 0 || number == 1)
        {
            conf.rx_listen = (byte)number;
            out_println("+ Ok.");
        }
        else
        {
            out_println("+ Failed. Invalid RX mode!");
        }
    }
    else if (input.startsWith("AT+FREQ="))
    {
        conf.frequency = input.substring(8).toFloat();

        // Reinitialize the RF95 to use the new frequency
        initRF95();
    }
    else if (input.startsWith("AT+HELP"))
    {
        out_println("rf95modem help:");
        out_println("AT+HELP             Print this usage information.");
        out_println("AT+TX=<hexdata>     Send binary data.");
        out_println("AT+RX=<0|1>         Turn receiving on (1) or off (2).");
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
    }
    else if (input.startsWith("AT+INFO"))
    {
        Serial.println("status info:");
        Serial.println();
        Serial.println("firmware:      " + String(VERSION));
        Serial.print("modem config:  ");
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
        Serial.println();
        Serial.println("rx bad:        " + String(rf95.rxBad()));
        Serial.println("rx good:       " + String(rf95.rxGood()));
        Serial.println("tx good:       " + String(rf95.txGood()));
        //rf95.printRegisters();
    }
    else
    {
        String output = "Unknown command: ";
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
            out_println("+ RX failed");
        }
    }
}