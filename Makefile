all:
	g++ -I src/include -L src/lib -o CitCatCoe CitCatCoe.cpp resources.o -lmingw32 -lSDL2main -lSDL2 -mwindows