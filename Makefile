CFLAGS = -Wall -std=gnu99
CXXFLAGS = -Wall -std=c++11
LDFLAGS =

CFLAGS_DEBUG = -g -O3
CXXFLAGS_DEBUG = -g -DVS_DEBUG
LDFLAGS_DEBUG = `pkg-config --libs opencv` -lsvm

CFLAGS_RELEASE = -O3
CXXFLAGS_RELEASE = -O3
LDFLAGS_RELEASE = `pkg-config --libs opencv` -lsvm

CFLAGS_DEBUG_DLX_SUDOKU = -g -O3 -DUSE_SUDOKU_MAIN
CXXFLAGS_DEBUG_DLX_SUDOKU =
LDFLAGS_DEBUG_DLX_SUDOKU =

TARGET = vs
SRCS = vs.cc VideoSudoku.cc SudokuOCR.cc SVMOCR.cc dlx_sudoku.c dlx.c
OBJS = vs.o VideoSudoku.o SudokuOCR.o SVMOCR.o dlx_sudoku.o dlx.o

SUBTARGET = dlx_sudoku
SUBSRCS = dlx_sudoku.c dlx.c
SUBOBJS = dlx_sudoku.o dlx.o

.PHONY: release
.PHONY: debug
.PHONY: debug_dlx_sudoku
.PHONY: clean

release: CFLAGS += $(CFLAGS_RELEASE)
release: CXXFLAGS += $(CXXFLAGS_RELEASE)
release: LDFLAGS += $(LDFLAGS_RELEASE)
release: $(TARGET)

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: LDFLAGS += $(LDFLAGS_DEBUG)
debug: $(TARGET)

debug_dlx_sudoku: CFLAGS += $(CFLAGS_DEBUG_DLX_SUDOKU)
debug_dlx_sudoku: CXXFLAGS += $(CXXFLAGS_DEBUG_DLX_SUDOKU)
debug_dlx_sudoku: LDFLAGS += $(LDFLAGS_DEBUG_DLX_SUDOKU)
debug_dlx_sudoku: $(SUBTARGET)

clean:
	$(RM) $(TARGET) $(SUBTARGET) $(OBJS) $(SUBOBJS)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(SUBTARGET): $(SUBOBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $^
