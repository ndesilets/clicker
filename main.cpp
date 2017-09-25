#include <stack>
#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <RF24/RF24.h>
#include "./websocketpp/websocketpp/common/thread.hpp"
#include "./websocketpp/websocketpp/config/asio_no_tls.hpp"
#include "./websocketpp/websocketpp/server.hpp"

// /usr/local/lib
// /usr/local/include/RF24

#define CHANNEL 40
#define PAYLOAD_SIZE 4

typedef websocketpp::server<websocketpp::config::asio> Server;

// Shared between threads
Server server;
std::stack<uint32_t> payloads;

// From wspp examples
class WSServer {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::server<websocketpp::config::asio> server;

    WSServer() : m_count(0) {
        // set up access channels to only log interesting things
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
        m_endpoint.set_access_channels(websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        m_endpoint.init_asio();

        // Bind the handlers we are using
        using websocketpp::lib::placeholders::_1;
        using websocketpp::lib::bind;
        m_endpoint.set_open_handler(bind(&WSServer::on_open,this,_1));
        m_endpoint.set_close_handler(bind(&WSServer::on_close,this,_1));
    }

    void run(uint16_t port) {
        // listen on specified port
        m_endpoint.listen(port);

        // Start the server accept loop
        m_endpoint.start_accept();

        // Set the initial timer to start telemetry
        set_timer();

        // Start the ASIO io_service run loop
        try {
            m_endpoint.run();
        } catch (websocketpp::exception const & e) {
            std::cout << e.what() << std::endl;
        }
    }

    void set_timer() {
        m_timer = m_endpoint.set_timer(
            1000,
            websocketpp::lib::bind(
                &WSServer::on_timer,
                this,
                websocketpp::lib::placeholders::_1
            )
        );
    }

    void on_timer(websocketpp::lib::error_code const & ec) {
        if (ec) {
            return;
        }
        
        // Broadcast count to all connections
        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            m_endpoint.send(*it, "test", websocketpp::frame::opcode::text);
        }
        
        // set timer for next telemetry check
        set_timer();
    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }

private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    
    server m_endpoint;
    con_list m_connections;
    server::timer_ptr m_timer;
    
    // Telemetry data
    uint64_t m_count;
};

void initRF24(RF24* radio);
void initWSServer();
void *runWSServer(void*);
void scan(RF24* radio);
void addPayload(uint8_t* buf);

int main(int argc, char *argv[]) {
    RF24 radio(RPI_BPLUS_GPIO_J8_15, RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
    pthread_t t;

    pthread_create(&t, NULL, runWSServer, NULL);

    pthread_join(t, NULL);

    initRF24(&radio);
    scan(&radio);

    return 0;
}

void initRF24(RF24* radio) {
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

void *runWSServer(void*) {
    WSServer server;

    server.run(8080);

    return (void*)NULL;
}

void scan(RF24* radio) {
    uint8_t buffer[PAYLOAD_SIZE];
    memset(&buffer, '\0', PAYLOAD_SIZE);

    printf("Scanning... \n\n");
    
    while (1) {
        if (radio->available()) {
            radio->read(&buffer, PAYLOAD_SIZE);
    
            addPayload(buffer);

            memset(&buffer, '\0', PAYLOAD_SIZE);
        }
    }
}

void addPayload(uint8_t* payload) {
    uint32_t output = 0;

    printf("%u\n", payload[3]);

    output |= (uint32_t)(payload[0] << 24);
    output |= (uint32_t)(payload[1] << 16);
    output |= (uint32_t)(payload[2] << 8);
    output |= (uint32_t)(payload[3]);

    printf("%x\n", output);
}