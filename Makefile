CXXFLAGS =  -std=gnu++11	-O2 -g -Wall -fmessage-length=0

OBJS =		main.o avp.o diameter.o avputil.o entry.o logic.o

LIBS =	-lrocksdb

TARGET =	main

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
