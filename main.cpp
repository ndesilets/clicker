#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <chrono>
#include <thread>
#include "./nrf24.cpp"
#include "./response.cpp"

void scan(Nrf24* nrf24, std::vector<Response*> responses);

int main(int argc, char **argv){
    Nrf24* nrf24 = new Nrf24();
    std::vector<Response*> responses;

    printf("Hello World!\n");

    scan(nrf24, responses);

    return 0;
}

void scan(Nrf24* nrf24, std::vector<Response*> responses) {
    while (1) {
        uint8_t status = nrf24->getRxStatus();

        if (status < 0b0110) {
            printf("status: %u\n", status);

            uint8_t packet[4];

            nrf24->read(R_RX_PAYLOAD, packet, 4);
            nrf24->write(FLUSH_RX);

            for(int i = 0; i < 4; i++){
                printf("%u ", packet[i]);
            }

            printf("\n");
        } else {
            printf("empty\n");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}