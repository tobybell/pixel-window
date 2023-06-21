CFLAGS = -std=c++20 -Ofast -Wunused -isysroot $(SYSROOT)

MODULES = system triangle bezier round-rect point ring
OBJECTS = $(MODULES:%=build/%.o)

build/app: build/app.o $(OBJECTS)
	swiftc -o $@ $^

build/app.o: app.swift header.h
	swiftc -o $@ -c app.swift -O -import-objc-header header.h

build/%.o: %.cc
	clang++ -o $@ $(CFLAGS) -MD -c $<

run: build/app
	$<

-include $(OBJECTS:.o=.d)