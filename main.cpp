#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "./response.cpp"
//#include "./nrf24.cpp"

int main(int argc, char *argv[]){
    std::vector<Response*> responses;

    printf("Hello World!\n");

    uint8_t address[3] = {0x00, 0x00, 0x1A};
    responses.push_back(new Response(address, 0));
    responses.push_back(new Response(address, 1));
    responses.push_back(new Response(address, 2));

    for(Response* response: responses){
        response->print();
    }

    return 0;
}
