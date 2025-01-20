all:
	g++ -I src/include -L src/lib -o CitCatCoe CitCatCoe.cpp -lmingw32 -lSDL2main -lSDL2