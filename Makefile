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

INCLUDEDIR = include
BINDIR = bin
SRCDIR = src
OBJDIR = obj

INCLUDE = -I$(INCLUDEDIR)

TARGET_NAME = vs
SRC_NAMES = vs.cc VideoSudoku.cc SudokuOCR.cc SVMOCR.cc dlx_sudoku.c dlx.c

SUBTARGET_NAME = dlx_sudoku
SUBSRC_NAMES = dlx_sudoku.c dlx.c

TARGET = $(addprefix $(BINDIR)/, $(TARGET_NAME))
SRCS = $(addprefix $(SRCDIR)/, $(SRC_NAMES))
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(SRC_NAMES))))

SUBTARGET = $(addprefix $(BINDIR)/, $(SUBTARGET_NAME))
SUBSRCS = $(addprefix $(SRCDIR)/, $(SUBSRC_NAMES))
SUBOBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(SUBSRC_NAMES))))

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
	@ -mkdir -p $(BINDIR)
	$(CXX) $(LDFLAGS) -o $@ $^

$(SUBTARGET): $(SUBOBJS)
	@ -mkdir -p $(BINDIR)
	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ -mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@ -mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $^
