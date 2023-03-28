OUTPUT=project_2
CFLAGS=-g -Wall -Werror -Wunused-variable -fsanitize=address -std=c99
LFLAGS=-lm
OBJ = mysh.o readcommands.o builtinfunctions.o prompts.o piperedirect.o freememory.o
TARGET = mysh

all: $(TARGET)

$(TARGET): $(OBJ)
	gcc $(CFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)
clean:
	rm -f *.o $(OBJ) %(TARGET) *~
