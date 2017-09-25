#include <stdint.h>
#include <wiringPiSPI.h>
#include <chrono>
#include <thread>
#include "./nRF24L01.h"
extern "C" {
    #include <wiringPi.h>
}

#define CE 15
#define CSN 24
#define CHANNEL 69

#define SPI_CHN 0
#define SPD_SPD 1000000 // 1MHz

// Adapted from https://stackoverflow.com/a/2602885
// RPi is little endian, NRF24 is big endian
uint8_t swapEndian(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

    return b;
}

class Nrf24 {
    private:
        void init();
        void setRx();
        void setTx();
    public:
        Nrf24();
        void read(uint8_t, uint8_t*);
        void read(uint8_t, uint8_t*, uint8_t);
        void write(uint8_t);
        void write(uint8_t, uint8_t);
        void write(uint8_t, uint8_t*, uint8_t);
        uint8_t getRxStatus();
};

void Nrf24::init() {
    wiringPiSetup();
    pinMode(CE, OUTPUT);
    pinMode(CSN, OUTPUT);

    if (wiringPiSPISetup(SPI_CHN, SPD_SPD) < 0) {
        perror("Error with SPI init");
        exit(EXIT_FAILURE);
    }
}

Nrf24::Nrf24() {
    init();
    setRx();
}

void Nrf24::read(uint8_t cmd, uint8_t* res) {
    cmd = swapEndian(cmd);

    digitalWrite(CSN, LOW);

    //printf("read(): sending cmd = %c\n", cmd);
    
    *res = swapEndian((uint8_t)wiringPiSPIDataRW(SPI_CHN, &cmd, sizeof(uint8_t)));

    //printf("read(): getting response\n");

    wiringPiSPIDataRW(SPI_CHN, 0, sizeof(uint8_t));

    //printf("read(): got %x\n", *res);

    digitalWrite(CSN, HIGH);
}

void Nrf24::read(uint8_t cmd, uint8_t* res, uint8_t len) {
    cmd = swapEndian(cmd);

    digitalWrite(CSN, LOW);
    
    wiringPiSPIDataRW(SPI_CHN, &cmd, sizeof(uint8_t));
    
    for (int i = 0; i < len; i++) {
        res[i] = swapEndian((uint8_t)wiringPiSPIDataRW(SPI_CHN, 0, sizeof(uint8_t)));
    }

    digitalWrite(CSN, HIGH);
}

void Nrf24::write(uint8_t cmd) {
    cmd = swapEndian(cmd);

    digitalWrite(CSN, LOW);

    wiringPiSPIDataRW(SPI_CHN, &cmd, sizeof(uint8_t));

    digitalWrite(CSN, HIGH);
}

void Nrf24::write(uint8_t cmd, uint8_t val) {
    cmd = swapEndian(cmd);
    val = swapEndian(val);

    digitalWrite(CSN, LOW);

    wiringPiSPIDataRW(SPI_CHN, &cmd, sizeof(uint8_t));
    wiringPiSPIDataRW(SPI_CHN, &val, sizeof(uint8_t));

    digitalWrite(CSN, HIGH);
}

void Nrf24::write(uint8_t cmd, uint8_t* val, uint8_t len) {
    cmd = swapEndian(cmd);

    digitalWrite(CSN, LOW);

    wiringPiSPIDataRW(SPI_CHN, &cmd, sizeof(uint8_t));

    for (int i = 0; i < len; i++) {
        wiringPiSPIDataRW(SPI_CHN, &val[i], sizeof(uint8_t));
        val[i] = swapEndian(val[i]);
    }

    digitalWrite(CSN, HIGH);
}

uint8_t Nrf24::getRxStatus() {
    uint8_t status;
    
    digitalWrite(CSN, LOW);

    status = swapEndian((uint8_t)wiringPiSPIDataRW(SPI_CHN, 0, sizeof(uint8_t)));

    //printf("getRxStatus(): %u %u\n", status, swapEndian(status));

    digitalWrite(CSN, HIGH);

    return status &= 0b1110; // Mask RX_P_NO
}

void Nrf24::setRx() {
    uint8_t masterMAC[3] = {0x56, 0x34, 0x12};

    digitalWrite(CE, LOW);                      // Select device

    this->write(W_REGISTER | CONFIG, 0x0A);      // Power on, enable CRC
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    this->write(W_REGISTER | CONFIG, 0x3F);      // 2-byte CRC, 0 to jam
    this->write(W_REGISTER | EN_RXADDR, 0x01);   // Receive on pipe 1
    this->write(W_REGISTER | RX_PW_P0, 0x04);    // 4-byte addr width
    this->write(CONFIG, 0x00);                   // Unset prim rx for recv
    this->write(W_REGISTER | EN_AA, 0x00);       // Disable auto-ack
    this->write(W_REGISTER | RF_CH, CHANNEL);    // Set channel
    this->write(W_REGISTER | SETUP_AW, 0x01);    // 3-byte MAC addr
    this->write(W_REGISTER | RF_SETUP, 0x06);    // 1MBPS data rate, high pwr
    
    this->write(FLUSH_RX);                       // Clear receiver buffer
    this->write(W_REGISTER | STATUS, 0x70);      // Clear interrupts
    this->write(W_REGISTER | RX_ADDR_P0, masterMAC, 3);  // Set MAC to listen for
    
    digitalWrite(CE, HIGH);                     // Deselect device

    return;
}

void Nrf24::setTx() {
    return;
}