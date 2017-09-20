LINKS = -lwiringPi -lpthread -lcrypt -lrt

main: main.cpp
	g++ -std=c++14 -Wall main.cpp -o main $(LINKS)

clean:
	rm main
