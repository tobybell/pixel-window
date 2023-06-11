build/app: build/app.o build/system.o
	swiftc -o $@ $^

build/app.o: app.swift header.h
	swiftc -o $@ -c app.swift -O -import-objc-header header.h

build/system.o: system.cc header.h
	clang++ -o $@ -c $< -std=c++20 -O -isysroot $(SYSROOT)
