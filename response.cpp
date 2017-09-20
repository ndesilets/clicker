#include <stdio.h>
#include <stdint.h>

class Response {
    private:
        uint8_t* address;
        uint8_t value;
    public:
        Response(uint8_t* a, uint8_t v);
        uint8_t* getAddress();
        uint8_t getValue();
        void print();
};

Response::Response(uint8_t* a, uint8_t v) {
    address = a;
    value = v;
}

uint8_t* Response::getAddress() {
    return address;
}

uint8_t Response::getValue() {
    return value;
}

void Response::print(){
    printf("[");
    for(int i = 0; i < 3; i++){
        (i < 2) ? printf("0x%x, ", address[i]) : printf("0x%x", address[i]);
    }
    printf("] : %u\n", value);
}