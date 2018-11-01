TARGET = test

LIBS = -l:libcrypto.so.1.1
LIBDIRS =

CXXSOURCES = $(wildcard *.cpp)
CXXOBJECTS = $(patsubst %.cpp,%.o,$(CXXSOURCES))
CXXDEPENDS = $(subst .cpp,.d,$(CXXSOURCES))
CXXFLAGS = -Wall
CXX = g++

LDFLAGS = $(LIBDIRS) $(LIBS)

all: $(TARGET)

$(TARGET): $(CXXOBJECTS)
	$(CXX) -o $@ $(CXXOBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(CXXOBJECTS) $(TARGET) $(CXXDEPENDS)


include $(CXXDEPENDS)

%.d: %.cpp
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$;                      \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;     \
	rm -f $@.$$$$
