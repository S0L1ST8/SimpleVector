TARGET = main.exe
CC = g++
FLAGS = -std=c++17 -Wall -Werror -g -O3

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = $(wildcard $(PREF_SRC)*.cpp)
OBJ = $(patsubst $(PREF_SRC)%.cpp, $(PREF_OBJ)%.o, $(SRC))

$(TARGET): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(TARGET)

$(PREF_OBJ)%.o: $(PREF_SRC)%.cpp
	$(CC) -c -MD $(FLAGS) $< -o $@

-include $(wildcard $(PREF_OBJ)*.d)

clean:
	rm $(TARGET) $(PREF_OBJ)*.d $(PREF_OBJ)*.o
