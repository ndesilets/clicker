#include <stdint.h>
#include <chrono>
#include <thread>
#include "./nRF24L01.h"
extern "C" {
    #include <wiringPi.h>
}

#define CE 9
#define CSN 10
#define CHANNEL 40

class Nrf24 {
    private:
        uint8_t cmdrxStatus;
        void initGpio();
    public:
        Nrf24();
        uint8_t read(uint8_t, uint8_t);
        uint8_t read(uint8_t, uint8_t, uint8_t);
        uint8_t read();
        uint8_t write(uint8_t);
        uint8_t write(uint8_t, uint8_t);
        uint8_t write(uint8_t, uint8_t*, uint8_t);
        void initRx();
};

void Nrf24::initGpio() {
    wiringPiSetup();
    pinMode(CE, OUTPUT);
    pinMode(CSN, OUTPUT);
}

Nrf24::Nrf24() {
    
}

uint8_t Nrf24::read(uint8_t cmd, uint8_t res) {
    return 0;
}

uint8_t Nrf24::read(uint8_t cmd, uint8_t res, uint8_t len) {
    return 0;
}

uint8_t Nrf24::write(uint8_t cmd) {
    return 0;
}

uint8_t Nrf24::write(uint8_t cmd, uint8_t val) {
    return 0;
}

uint8_t Nrf24::write(uint8_t cmd, uint8_t* val, uint8_t len) {
    return 0;
}

void Nrf24::initRx() {
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
    this->write(W_REGISTER | RX_ADDR_P0, {0x56, 0x34, 0x12}, 3);  // Set MAC to listen for
    
    digitalWrite(CE, HIGH);                     // Deselect device

    return;
}