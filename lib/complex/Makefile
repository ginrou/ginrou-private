###
### complex 用 Makefile
###
#
# このプログラムは全ての OS でコンパイルできます。
# OS に応じてコンパイルオプションを自動設定してコンパイルします。
#
# make とすることによりプログラムが生成されます。
# make clean とすることにより生成物が消去されます。

OBJ_DIR    = .
SRC_DIR    = .
INS_DIR    = ..
SMP_DIR    = .

CC         = gcc
CFLAGS     = $(OPTIMIZE) $(INCLUDES) $(DEFINES)
LFLAGS     = $(LIBS) $(LIBDIR)

INCLUDES   = -I$(INS_DIR)

TARGET     = $(OBJ_DIR)/complex.o ./comp_test

LIBDIR     = 
LIBS       = -lm
DEFINES    = -DMAC
OPTIMIZE   = -O3

# ----- ルール ------

all: $(TARGET)

clean:
	rm $(TARGET)

$(SMP_DIR)/comp_test: comp_test.c $(OBJ_DIR)/complex.o
	$(CC) $(CFLAGS) -o $(SMP_DIR)/comp_test comp_test.c $(OBJ_DIR)/complex.o $(LFLAGS)

$(OBJ_DIR)/complex.o: $(SRC_DIR)/complex.c
	$(CC) -c $(SRC_DIR)/complex.c $(CFLAGS) -o $(OBJ_DIR)/complex.o


