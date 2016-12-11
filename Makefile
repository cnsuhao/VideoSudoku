CXXFLAGS = -g -Wall -std=c++11
CFLAGS = -g -Wall -std=gnu99 -O3
LDFLAGS = -g `pkg-config --libs opencv` -lsvm

TARGET = vs
SUBTARGET = dlx_sudoku
OBJS = vs.o VideoSudoku.o SudokuOCR.o SVMOCR.o dlx_sudoku.o dlx.o

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

$(SUBTARGET): dlx_sudoku.o dlx.o
	$(CC) -o $@ dlx_sudoku.c dlx.c -DUSE_SUDOKU_MAIN

.PHONY : clean
clean:
	-rm -rf $(TARGET) $(SUBTARGET) $(OBJS) *~
