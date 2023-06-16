CFLAGS = -std=c++20 -Ofast -Wunused -isysroot $(SYSROOT)

build/app: build/app.o build/system.o build/triangle.o
	swiftc -o $@ $^

build/app.o: app.swift header.h
	swiftc -o $@ -c app.swift -O -import-objc-header header.h

build/system.o: system.cc header.h canvas.hh
	clang++ -o $@ $(CFLAGS) -c $<

build/triangle.o: triangle.cc canvas.hh
	clang++ -o $@ $(CFLAGS) -c $<

run: build/app
	$<
