TARGET=moged
SRC:=$(wildcard src/*.cpp) $(wildcard src/gui/*.cpp) $(wildcard src/render/*.cpp) $(wildcard src/gui/gen/*.cpp) $(wildcard src/anim/*.cpp)
CSRC:= src/sql/sqlite3.c


include Makefile.defs

OBJS = $(patsubst %.cpp,obj/%.o,$(notdir $(SRC)))
COBJS = $(patsubst %.c,obj/%.o,$(notdir $(CSRC)))

.PHONY: default
default: $(TARGET)

$(TARGET) : $(OBJS) $(COBJS)
	$(LINK) $(DBG_LINKFLAGS) -o $@ $^ $(LINK_LIBS)

.SUFFIXES:
obj/%.o : 
	$(CCPP) -c $(DBG_FLAGS) -o $@ $<

obj/sqlite3.o : src/sql/sqlite3.c
	$(CC) -c $(CC_DBG_FLAGS) -o $@ $<

.PHONY: clean
clean :
	-rm -f obj/*.o $(TARGET)

.PHONY: depend
depend:
	-mkdir obj
	-rm -f obj/depend
	$(foreach srcfile,$(SRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.cpp,obj/%.o,$(notdir $(srcfile))) >> obj/depend;)
	$(foreach srcfile,$(CSRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.c,obj/%.o,$(notdir $(srcfile))) >> obj/depend;)

-include obj/depend
