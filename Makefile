.PHONY: all clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean))) #############

CC = gcc
CXX = g++

CFLAGS := -O3 -flto
# CFLAGS := -O0 -g
CFLAGS += -Wall -fmax-errors=3 -Iinclude
# CFLAGS += -DNDEBUG

# generate .d files during compilation
DEPFLAGS = -MT $@ -MMD -MP -MF .build/$*.d

LIBXML2_CFLAGS := $(shell xml2-config --cflags)
LIBXML2_LIBS := $(shell xml2-config --libs)

FIND_MAIN := \
  find src -type f -regex '.*\.cc?$$' \
  | xargs grep -l '^\s*int\s\+main\s*(' \
  | sed 's:^src/\(.*\)\.c\+$$:bin/\1:'
EXE := $(shell $(FIND_MAIN))

all: $(EXE)

.build/xpath2.o: CFLAGS += $(LIBXML2_CFLAGS)
bin/xpath2: LDLIBS += $(LIBXML2_LIBS)

.build/test.o: CFLAGS += $(LIBXML2_CFLAGS)
bin/test: LDLIBS += $(LIBXML2_LIBS)
bin/test: CXX = $(CC)

.PRECIOUS: .build/%.o

bin/%: .build/%.o
	@mkdir -pv $(dir $@)
	$(CXX) $(LDFLAGS) $(filter %.o,$^) -o $@ $(LDLIBS)

%.so: .build/%.o
	$(CXX) $(LDFLAGS) -shared $(filter %.o,$^) -o $@ $(LDLIBS)

.build/%.o: src/%.cc
	@mkdir -pv $(dir $@)
	$(CXX) -std=c++20 $(CFLAGS) $(DEPFLAGS) -c $(filter %.cc,$^) -o $@

.build/%.o: src/%.c
	@mkdir -pv $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $(filter %.c,$^) -o $@

-include $(shell [ -d '.build' ] && find .build -type f -name '*.d')

endif ###############################################################

clean:
	@rm -frv .build bin

