#include <stdio.h>
#include <stdint.h>
#include <RF24/RF24.h>
#include "./websocketpp/websocketpp/config/asio_no_tls.hpp"
#include "./websocketpp/websocketpp/server.hpp"

// /usr/local/lib
// /usr/local/include/RF24

#define CHANNEL 40
#define PAYLOAD_SIZE 4

typedef websocketpp::server<websocketpp::config::asio> Server;

void initRf24(RF24* radio);
void initWSServer(Server* server);
void scan(RF24* radio, Server* server);
void onPayload(uint8_t* buf, uint8_t len);

int main(int argc, char *argv[]) {
    RF24 radio(RPI_BPLUS_GPIO_J8_15, RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
    Server server;

    printf("Hello World!\n");

    initRf24(&radio);
    initWSServer(&server);
    scan(&radio, &server);

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

void initWSServer(Server* server) {
    server->init_asio();
    server->listen(8080);
    server->start_accept();
    
    try {
        server->run();
    } catch (const std::exception & e) {
        fprintf(stderr, "%s\n", e.what());
        exit(EXIT_FAILURE);
    }
}

void scan(RF24* radio, Server* server) {
    uint8_t buffer[PAYLOAD_SIZE];
    memset(&buffer, '\0', PAYLOAD_SIZE);

    printf("Scanning... \n\n");
    
    while (1) {
        if (radio->available()) {
            radio->read(&buffer, PAYLOAD_SIZE);
    
            for (int i = 0; i < PAYLOAD_SIZE; i++) {
                printf("%u ", buffer[i]);
            }
    
            printf("\n");

            memset(&buffer, '\0', PAYLOAD_SIZE);
        }
    }
}

void emitPayload(uint8_t* buf, uint8_t len) {

}