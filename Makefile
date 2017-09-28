LINKS = -lpthread -lrf24-bcm

main: main.cpp
	g++ -std=c++14 -Wall main.cpp -o main $(LINKS) -L /usr/local/include/RF24

clean:
	rm main
