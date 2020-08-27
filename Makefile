SOURCES = src/main.cpp          \
		  src/FramedWindow.cpp  \
		  src/WindowManager.cpp \

build/zwm: $(SOURCES) src/*.h
	g++ -lX11 $(SOURCES) -o build/zwm

build/zwm.dbg: $(SOURCES) src/*.h
	g++ -ggdb -g3 -lX11 $(SOURCES) -o build/zwm.dbg
 


.PHONY: clean build run

clean:
	rm build/*

build: build/zwm

build-dbg: build/zwm.dbg

run-xephyr: build
	src/run-xephyr.sh build/zwm

run-xephyr-dbg: build
	src/run-xephyr.sh build/zwm.dbg