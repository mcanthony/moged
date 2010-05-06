TARGET=moged
SRC:=$(wildcard src/*.cpp) $(wildcard src/gui/*.cpp) $(wildcard src/render/*.cpp) $(wildcard src/gui/gen/*.cpp) $(wildcard src/anim/*.cpp)
CSRC:= src/sql/sqlite3.c

include Makefile.defs

OBJS = $(patsubst %.cpp,obj/%.o,$(notdir $(SRC)))
COBJS = $(patsubst %.c,obj/%.o,$(notdir $(CSRC)))

OBJS_Z = $(patsubst %.cpp,obj_z/%.o,$(notdir $(SRC)))
COBJS_Z = $(patsubst %.c,obj_z/%.o,$(notdir $(CSRC)))

.PHONY: default
default: $(TARGET)

.PHONY: release
release: $(TARGET)_z

$(TARGET) : $(OBJS) $(COBJS)
	$(LINK) -o $@ $^ $(LINK_LIBS)

$(TARGET)_z : $(OBJS_Z) $(COBJS_Z)
	$(LINK) -o $@ $^ $(LINK_LIBS)

.SUFFIXES:
obj/%.o : 
	$(CCPP) -c $(DBG_FLAGS) -o $@ $<

obj_z/%.o : 
	$(CCPP) -c $(RELEASE_FLAGS) -o $@ $<

obj/sqlite3.o : src/sql/sqlite3.c
	$(CC) -c $(CC_DBG_FLAGS) -o $@ $<

obj_z/sqlite3.o : src/sql/sqlite3.c
	$(CC) -c $(CC_RELEASE_FLAGS) -o $@ $<

.PHONY: clean
clean :
	-rm -f obj/*.o obj_z/*.o $(TARGET) $(TARGET)_z

.PHONY: depend
depend:
	-mkdir obj
	-mkdir obj_z
	-rm -f obj/depend
	-rm -f obj_z/depend
	$(foreach srcfile,$(SRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.cpp,obj/%.o,$(notdir $(srcfile))) >> obj/depend;)
	$(foreach srcfile,$(CSRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.c,obj/%.o,$(notdir $(srcfile))) >> obj/depend;)
	$(foreach srcfile,$(SRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.cpp,obj_z/%.o,$(notdir $(srcfile))) >> obj_z/depend;)
	$(foreach srcfile,$(CSRC),$(DEPEND) -MM $(srcfile) -MT $(patsubst %.c,obj_z/%.o,$(notdir $(srcfile))) >> obj_z/depend;)

-include obj/depend
-include obj_z/depend
