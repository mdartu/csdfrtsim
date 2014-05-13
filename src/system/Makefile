CXX=g++
CXXFLAGS=-std=c++0x
LDFLAGS=

CXXFLAGS += $(shell xml2-config --cflags)
LDFLAGS += $(shell xml2-config --libs)

test_systemloader.bin: test_systemloader.o systemloader.o systemvalidator.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f *.o *.bin

