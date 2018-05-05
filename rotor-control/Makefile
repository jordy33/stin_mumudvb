CC=gcc

SRC=rotor-control.c
OBJ=rotor-control.o

BIND=/usr/local/bin/

TARGET=rotor-control

CLIB=-lm
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLG) $(OBJ) -o $(TARGET) $(CLIB)

$(OBJ): $(HED)

install: all
	cp $(TARGET) $(BIND)

clean:
	rm -f $(OBJ) $(TARGET) *~

c.o: $(CC) -c $< -o $@
