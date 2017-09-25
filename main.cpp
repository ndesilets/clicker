#include <thread>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <RF24/RF24.h>

// /usr/local/lib
// /usr/local/include/RF24

#define CHANNEL 40
#define PAYLOAD_SIZE 4
#define OFFSET 98

void initRf24(RF24* radio);
void scan(RF24* radio);
void showAnswers(int* answers, unsigned int count);

int main(int argc, char *argv[]) {
    RF24 radio(RPI_BPLUS_GPIO_J8_15, RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);

    printf("Hello World!\n");

    initRf24(&radio);
    scan(&radio);

    return 0;
}

void initRf24(RF24* radio) {
    radio->begin();

    radio->setChannel(CHANNEL);
    radio->setDataRate(RF24_1MBPS);
    radio->setPALevel(RF24_PA_HIGH);
    radio->setCRCLength(RF24_CRC_16);
    radio->setAddressWidth(3);
    radio->setAutoAck(false);
    radio->setPayloadSize(PAYLOAD_SIZE);

    uint8_t masterMAC[3] = {0x56, 0x34, 0x12};
    radio->openReadingPipe(1, masterMAC); 
    radio->startListening();  

    radio->printDetails();

    printf("\n");
}

void scan(RF24* radio) {
    int answers[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buffer[PAYLOAD_SIZE] = {0, 0, 0, 0};
    std::vector<uint32_t> history (1, 100);

    printf("Scanning... \n\n");
    
    while (1) {
        if (radio->available()) {
            radio->read(&buffer, PAYLOAD_SIZE);
            //printf("%u\n", buffer[3]);
            if (buffer[3] >= 98 && buffer[3] <= 106) {
                uint32_t payloadHash = 0;

                payloadHash |= (uint32_t)(buffer[0] << 24);
                payloadHash |= (uint32_t)(buffer[1] << 16);
                payloadHash |= (uint32_t)(buffer[2] << 8);
                payloadHash |= (uint32_t)(buffer[3]);

                bool exists = false;
                for (unsigned int i = 0; i < history.size(); i++) {
                    if (history[i] == payloadHash) {
                        exists = true;
                    }
                }

                if (!exists) {
                    answers[buffer[3] - OFFSET]++;
                    history.push_back(payloadHash);

                    showAnswers(answers, history.size() - 1);
                }
            }
        }
    }
}

void showAnswers(int* answers, unsigned int count) {
    for (int i = 0; i < 9; i++) {
        printf("[%i]: %i - ", i + 1, answers[i]);
    }
    printf("#%i\n", count);
}