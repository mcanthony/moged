TARGET=moged
SRC:=$(wildcard src/*.cpp) $(wildcard src/gui/*.cpp) $(wildcard src/render/*.cpp) $(wildcard src/gui/gen/*.cpp)

include Makefile.defs

OBJS = $(patsubst %.cpp,obj/%.o,$(notdir $(SRC)))

.PHONY: default
default: $(TARGET)

$(TARGET) : $(OBJS)
	$(LINK) $(DBG_LINKFLAGS) -o $@ $^ $(LINK_LIBS)

.SUFFIXES:
obj/%.o : 
	$(CC) -c $(DBG_FLAGS) -o $@ $<

.PHONY: clean
clean :
	-rm -f obj/*.o $(TARGET)

.PHONY: depend
depend:
	-mkdir obj
	-rm -f obj/depend
	$(foreach srcfile,$(SRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.cpp,obj/%.o,$(notdir $(srcfile))) >> obj/depend;)

-include obj/depend
