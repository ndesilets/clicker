LINKS = -lwiringPi -lpthread -lcrypt -lrt -lrf24-bcm -lboost_system

main: main.cpp
	g++ -std=c++14 -Wall main.cpp -o main $(LINKS) -L /usr/local/include/RF24 -I /home/pi/clicker/websocketpp/

clean:
	rm main
