#include <SPI.h>
#include <RF24.h>

uint8_t buffer[60];

unsigned char channel[3] = { 37, 38, 39 };  // logical BTLE channel number (37-39)
unsigned char frequency[3] = { 2, 26, 80 };  // physical frequency (2400+x MHz)

uint8_t currentChannel = 1;

RF24 radio(9, 10);

void setup() {

  Serial.begin(9600);

	radio.begin();

	// set standard parameters
	radio.setAutoAck(false);
	radio.setDataRate(RF24_1MBPS);
	radio.disableCRC();
	radio.setChannel(frequency[currentChannel]);
	radio.setRetries(0, 0);
	radio.setPALevel(RF24_PA_MAX);

	radio.setAddressWidth(4);
	radio.openReadingPipe(0, 0x6B7D9171);

	radio.powerUp();

  Serial.println("Hi !");

	delay(100);

}

void printHex(uint8_t data) {
  char buffer[3];
  sprintf (buffer, "%02x", data);
  Serial.print(buffer);
  Serial.print(" ");
}

void swapbuf(uint8_t* buf, uint8_t len) {

	while (len--) {

		uint8_t a = *buf;
		uint8_t v = 0;

		if (a & 0x80) v |= 0x01;
		if (a & 0x40) v |= 0x02;
		if (a & 0x20) v |= 0x04;
		if (a & 0x10) v |= 0x08;
		if (a & 0x08) v |= 0x10;
		if (a & 0x04) v |= 0x20;
		if (a & 0x02) v |= 0x40;
		if (a & 0x01) v |= 0x80;

		*(buf++) = v;
	}

}

void whiten(uint8_t* buf, uint8_t len) {

	uint8_t lfsr = channel[currentChannel] | 0x40;

	while (len--) {
		uint8_t res = 0;
		// LFSR in "wire bit order"
		for (uint8_t i = 1; i; i <<= 1) {
			if (lfsr & 0x01) {
				lfsr ^= 0x88;
				res |= i;
			}
			lfsr >>= 1;
		}
		*(buf++) ^= res;
	}
}

void crc( uint8_t len, uint8_t* dst) {

	uint8_t* buf = (uint8_t*)&buffer;

	// initialize 24-bit shift register in "wire bit order"
	// dst[0] = bits 23-16, dst[1] = bits 15-8, dst[2] = bits 7-0
	dst[0] = 0xAA;
	dst[1] = 0xAA;
	dst[2] = 0xAA;

	while (len--) {

		uint8_t d = *(buf++);

		for (uint8_t i = 1; i; i <<= 1, d >>= 1) {

			// save bit 23 (highest-value), left-shift the entire register by one
			uint8_t t = dst[0] & 0x01;         dst[0] >>= 1;
			if (dst[1] & 0x01) dst[0] |= 0x80; dst[1] >>= 1;
			if (dst[2] & 0x01) dst[1] |= 0x80; dst[2] >>= 1;

			// if the bit just shifted out (former bit 23) and the incoming data
			// bit are not equal (i.e. bit_out ^ bit_in == 1) => toggle tap bits
			if (t != (d & 1)) {
				// toggle register tap bits (=XOR with 1) according to CRC polynom
				dst[2] ^= 0xDA; // 0b11011010 inv. = 0b01011011 ^= x^6+x^4+x^3+x+1
				dst[1] ^= 0x60; // 0b01100000 inv. = 0b00000110 ^= x^10+x^9
			}
		}
	}
}

unsigned long m = millis();

void loop() {

	radio.startListening();

  while (radio.available()) {

    if ((millis() - m) > 10000) {
      Serial.println(".");
      m = millis();
    }

		radio.read(buffer, sizeof(buffer));
    radio.stopListening();

		swapbuf(buffer, sizeof(buffer));
		whiten(buffer, sizeof(buffer));

    if (((buffer[0] & 8) > 0) && (buffer[1] <= 0x20))
    {

      uint8_t total_size = buffer[1]+2;
      uint8_t in_crc[3];

      // calculate & compare CRC
      bool error = false;
      crc( total_size, in_crc );
      for (uint8_t i = 0; i < 3; i++)
      {
        if (buffer[total_size+i] != in_crc[i])
        {
          error = true;
        }
      }

      //if ((buffer[13] == 0xf1) && (buffer[14] == 0xfe))
      if (!error)
      {

        for (uint8_t i = 0; i < buffer[1] + 2 + 3; i++) {
          printHex(buffer[i]);
        }

        Serial.print("( ");
        for (uint8_t i = 0; i < 3; i++) {
          printHex(in_crc[i]);
        }
        Serial.print(") ");

        Serial.println("");

      }

    }

	}

  // Hop channel
  // currentChannel++;
  // if (currentChannel > 2) currentChannel = 0;

}
