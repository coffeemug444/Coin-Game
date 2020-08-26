main: main.cpp coinboard.cpp coinboard.h
	g++ -g main.cpp coinboard.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system
	./main
