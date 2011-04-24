OBJ_DIR    = .
SRC_DIR    = .
INS_DIR    = ..
SMP_DIR    = .

CC         = gcc
CFLAGS     = $(OPTIMIZE) $(INCLUDES) $(DEFINES)
LFLAGS     = $(LIBS) $(LIBDIR)

INCLUDES   = -I$(INS_DIR)

TARGET     = $(OBJ_DIR)/ppm.o

LIBDIR     = -L../
LIBS       = -lm

# ----- ルール ------

all: $(TARGET)

clean:
	rm $(TARGET)

$(OBJ_DIR)/ppm.o: $(SRC_DIR)/ppm.c
	$(CC) -c $(SRC_DIR)/ppm.c $(CFLAGS) -o $(OBJ_DIR)/ppm.o


