.PHONY: all run clean makedirs rewindbuild quick_rerun compile_filegen compile_filehash

all: makedirs compile_filegen compile_filehash run

rerun: clean makedirs run

quick_rerun: rewindbuild run

compile_filegen: filegen.cpp
	g++ filegen.cpp -o build/filegen

compile_filehash: filehash.cpp
	g++ filehash.cpp -o build/filehash

run: main.cpp
	g++ main.cpp -o build/main
	build/main

makedirs:
	mkdir build
	mkdir data

clean:
	rmdir /s /q build
	rmdir /s /q data

rewindbuild:
	rmdir /s /q build
	mkdir build
