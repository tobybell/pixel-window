build/app: build/app.o build/paint.o
	swiftc -o $@ $^

build/app.o: app.swift header.h
	swiftc -o $@ -c app.swift -O -import-objc-header header.h

build/paint.o: paint.cc header.h
	clang++ -o $@ -c $< -std=c++20 -O -isysroot $(SYSROOT)
