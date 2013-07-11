CC = g++
TARGET = bin/TPCW_Simulation

FLAGS = -I./include -g -Wall `pkg-config --cflags --libs gsl` # -DMEMWATCH -DDEBUG # -DDETAIL

all: $(TARGET)

$(TARGET): src/TPCW_Simulation.o src/TPCW.o src/Client.o src/Server.o src/Event.o src/OSleak.o src/Monitor.o
	$(CC) $^ -o $@ $(FLAGS)

src/TPCW_Simulation.o: src/TPCW_Simulation.cpp include/TPCW_Simulation.h include/OSleak.h include/Monitor.h
	$(CC) $< -c -o $@ $(FLAGS)

src/TPCW.o: src/TPCW.cpp include/TPCW.h
	$(CC) $< -c -o $@ $(FLAGS)
	
src/Client.o: src/Client.cpp include/Client.h
	$(CC) $< -c -o $@ $(FLAGS)
	
src/Server.o: src/Server.cpp include/Server.h
	$(CC) $< -c -o $@ $(FLAGS)

src/Event.o: src/Event.cpp include/Event.h
	$(CC) $< -c -o $@ $(FLAGS)

src/OSleak.o: src/OSleak.cpp include/OSleak.h
	$(CC) $< -c -o $@ $(FLAGS)

src/Monitor.o: src/Monitor.cpp include/Monitor.h
	$(CC) $< -c -o $@ $(FLAGS)
clean:
	-rm $(TARGET) src/*.o

.PHONY: clean install
