// Copyright (c) 2018 Lars Baumgaertner

#include <SPI.h>
#include <RH_RF95.h>

#define VERSION   "0.1"

// for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// RX indicator LED
#define LED 13

RH_RF95::ModemConfigChoice cur_modem_config = RH_RF95::Bw125Cr45Sf128;
//RH_RF95::ModemConfigChoice cur_modem_config = RH_RF95::Bw125Cr48Sf4096;

byte rx_listen = 1;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  delay(100);
  while (!Serial);

  Serial.println("rf95modem firmware (v" + String(VERSION) +")");
  Serial.setTimeout(2000);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1)
      ;
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  rf95.setModemConfig(cur_modem_config);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void onpacketreceived(uint8_t *buf, uint8_t len)
{
  int lastRssi = rf95.lastRssi();
  int lastSNR = rf95.lastSNR();
  Serial.print("+RX ");
  Serial.print(len, DEC);
  Serial.print(",");
  for (int i = 0; i < len; i++)
  {
    //Serial.printf("%02X", buf[i]);
    Serial.print(buf[i], HEX);
  }
  Serial.print(",");
  Serial.print(lastRssi, DEC);
  Serial.print(",");
  Serial.println(lastSNR, DEC);
}
void handleCommand(String input)
{
  if (input.startsWith("AT+TX="))
  {
    int cmdlen = 6;
    const char *instr = input.c_str();
    int plen = input.length() - cmdlen;
    int blen = plen / 2 + plen % 2;
    if (blen > rf95.maxMessageLength())
    {
      Serial.print("+FAIL: MAX_MESSAGE_LEN EXCEEDED! ");
      Serial.print(blen, DEC);
      Serial.print(" / ");
      Serial.println(rf95.maxMessageLength(), DEC);

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
    Serial.print("+SENT ");
    Serial.print(blen);
    Serial.println(" bytes.");
    /*for (int i = 0; i < blen; i++) {
      //Serial.printf("%02X", buf[i]);
      Serial.print(buf[i], HEX);
    }*/
  }
  else if (input.startsWith("AT+MODE="))
  {
    int number = input.substring(8).toInt();
    cur_modem_config = static_cast<RH_RF95::ModemConfigChoice>(number);
    rf95.setModemConfig(cur_modem_config);
    Serial.println("+ Ok.");
  }
  else if (input.startsWith("AT+RX="))
  {
    int number = input.substring(6).toInt();
    if (number == 0 || number == 1) {
      rx_listen = (byte) number;
      Serial.println("+ Ok.");
    } else {
      Serial.println("+ Failed. Invalid RX mode!");
    }
  }
  else if (input.startsWith("AT+HELP"))
  {
    Serial.println("rf95modem help:");
    Serial.println("AT+HELP             Print this usage information.");
    Serial.println("AT+TX=<hexdata>     Send binary data.");
    Serial.println("AT+RX=<0|1>         Turn receiving on (1) or off (2).");
    Serial.println("AT+INFO             Output status information.");
    Serial.println("AT+MODE=<NUM>       Output status information.");
    Serial.println("                    " + String(RH_RF95::Bw125Cr45Sf128) + " medium range (default)");
    Serial.println("                    Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.");
    Serial.println("                    " + String(RH_RF95::Bw500Cr45Sf128) + " fast+short range");
    Serial.println("                    Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on.");
    Serial.println("                    " + String(RH_RF95::Bw31_25Cr48Sf512) + " slow+long range");
    Serial.println("                    Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on.");
    Serial.println("                    " + String(RH_RF95::Bw125Cr48Sf4096) + " slow+long range");
    Serial.println("                    Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on.");
  }
  else if (input.startsWith("AT+INFO"))
  {
    Serial.println("status info:");
    Serial.println();
    Serial.println("firmware:      " + String(VERSION));
    Serial.print(  "modem config:  ");
    switch (cur_modem_config)
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
    Serial.println("frequency:     " + String(RF95_FREQ));
    Serial.println("rx listener:   " + String(rx_listen));
    Serial.println();
    Serial.println("rx bad:        " + String(rf95.rxBad()));
    Serial.println("rx good:       " + String(rf95.rxGood()));
    Serial.println("tx good:       " + String(rf95.txGood()));
    //rf95.printRegisters();
  }
  else
  {
    Serial.print("Unknown command: ");
    Serial.println(input);
  }
}
#define MAX_COMMAND_LEN 512
void loop()
{
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();
    handleCommand(input);
  }
  if (rx_listen == 1 && rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      onpacketreceived(buf, len);
      delay(10);
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("+ RX failed");
    }
  }
}
