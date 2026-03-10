CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
SRC = main.c src/app.c src/game.c src/render.c src/collision.c src/math3d.c
OBJ = $(SRC:.c=.o)
TARGET = dark_museum.exe
LIBS = -lSDL2 -lopengl32

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

src/app.o: src/app.c
	$(CC) $(CFLAGS) -c src/app.c -o src/app.o

src/game.o: src/game.c
	$(CC) $(CFLAGS) -c src/game.c -o src/game.o

src/render.o: src/render.c
	$(CC) $(CFLAGS) -c src/render.c -o src/render.o

src/collision.o: src/collision.c
	$(CC) $(CFLAGS) -c src/collision.c -o src/collision.o

src/math3d.o: src/math3d.c
	$(CC) $(CFLAGS) -c src/math3d.c -o src/math3d.o

clean:
	del /Q *.o src\*.o $(TARGET) 2>nul