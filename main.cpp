#include <thread>
#include <stack>
#include <vector>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <RF24/RF24.h>

// /usr/local/lib
// /usr/local/include/RF24

#define CHANNEL 35
#define PAYLOAD_SIZE 4
#define PORT 8000

std::mutex outputBufferMutex;
std::stack<uint8_t*> outputBuffer;

void initSocket(int* sockFd, struct sockaddr_in* address);
void listenThread();
void initRf24(RF24* radio);
void scanAndEmit(RF24* radio);

int main(int argc, char *argv[]) {
    RF24 radio(RPI_BPLUS_GPIO_J8_15, RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
    std::thread t(&listenThread);

    // Main thread

    initRf24(&radio);
    scanAndEmit(&radio);

    t.join(); // meh

    return EXIT_SUCCESS;
}

void initSocket(int* sockFd, struct sockaddr_in* server) {
    int opt = 1, sockRes = 0, bindRes = 0;
    
    *sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sockFd == 0) {
        perror("Socket creation");
        exit(EXIT_FAILURE);
    }

    sockRes = setsockopt(*sockFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (sockRes != 0) {
        perror("Socket opt");
        exit(EXIT_FAILURE);
    }

    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(PORT);

    bindRes = bind(*sockFd, (struct sockaddr*)server, sizeof(*server));
    if (bindRes < 0) {
        perror("Bind");
        exit(EXIT_FAILURE);
    }
}

void listenThread() {
    int sockFd, nBytes;
    unsigned int clientLen;
    struct sockaddr_in server, client;
    struct hostent *clientp;
    char *clientaddrp;
    unsigned char dummyBuffer[sizeof("HEY IT ME LOL")];

    initSocket(&sockFd, &server);

    printf("[Net thread]: Socket initialized. Waiting for client.\n");

    // Wait for first incoming connection (assumed to be client.py)

    nBytes = recvfrom(sockFd, dummyBuffer, sizeof("HEY IT ME LOL"), 0, (struct sockaddr*)&client, &clientLen);
    if (nBytes < 0) {
        perror("Handshake");
        exit(EXIT_FAILURE);
    }

    printf("[Net thread]: Got handshake message from client. Yee boi.\n");

    // Flush output buffer every ~.5s

    while(1) {
        outputBufferMutex.lock();

        if (!outputBuffer.empty()) {
            for(unsigned int i = 0; i < outputBuffer.size(); i++) {
                unsigned char* output = (unsigned char*)outputBuffer.top();
                
                sendto(sockFd, output, PAYLOAD_SIZE, 0, (struct sockaddr*)&client, sizeof(client));
                //sendto(sockFd, output, PAYLOAD_SIZE, 0, clientRes->ai_addr, clientRes->ai_addrlen);

                outputBuffer.pop();
            }
        }

        outputBufferMutex.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void initRf24(RF24* radio) {
    printf("Initializing transceiver...\n\n");

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

void scanAndEmit(RF24* radio) {
    uint8_t buffer[PAYLOAD_SIZE] = {0, 0, 0, 0};

    printf("Scanning... \n\n");
    
    while (1) {
        if (radio->available()) {
            //uint32_t payloadDWORD = 0;

            radio->read(&buffer, PAYLOAD_SIZE);

            //payloadDWORD |= (uint32_t)(buffer[0] << 24);
            //payloadDWORD |= (uint32_t)(buffer[1] << 16);
            //payloadDWORD |= (uint32_t)(buffer[2] << 8);
            //payloadDWORD |= (uint32_t)(buffer[3]);

            //printf("%x\n", payloadDWORD);

            outputBufferMutex.lock();

            outputBuffer.push(buffer);

            outputBufferMutex.unlock();
        }
    }
}
