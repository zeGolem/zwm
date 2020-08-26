SOURCES = src/main.cpp \
		  src/FramedWindow.cpp

build/zwm: $(SOURCES)
	g++ -lX11 $(SOURCES) -o build/zwm

build/zwm.dbg: $(SOURCES)
	g++ -ggdb -g3 -lX11 $(SOURCES) -o build/zwm.dbg
 


.PHONY: clean build run

clean:
	rm build/*

build: build/zwm

build-dbg: build/zwm.dbg

xephyr: build
	src/run-xephyr.sh
	build/zwm

xephyr-dbg: build
	src/run-xephyr.sh
	build/zwm.dbg

xephyr-nowm: build
	src/run-xephyr.sh