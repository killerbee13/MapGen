CXXFLAGS=-g --std=c++14 -Wall -Wno-missing-braces -Ofast -msse3 -I ./boost_1_64_0 -march=native
LDFLAGS=-L/usr/lib/x86_64-linux-gnu -L./boost_1_64_0/stage/lib -lpng12
CXX=clang++-3.8

objects=main.o fnv.o input.o SimplexNoise.o OlsenNoise.o map.o


inputDeps=input.cpp input.h
fnvDeps=fnv.cpp fnv.h
SimplexNoiseDeps=SimplexNoise.cpp SimplexNoise.h Array2D.h fnv.h
OlsenNoiseDeps=OlsenNoise.cpp SimplexNoise.h fnv.h
mainDeps=main.cpp strings.h input.h fnv.h Array2D.h SimplexNoise.h version.h map.h
mapDeps=map.cpp map.h cmap.h FRC.h fnv.h SimplexNoise.h

mapGen: $(objects)
	$(CXX) $(CXXFLAGS) -o mapGen $(objects) $(LDFLAGS) -flto

main.o: $(mainDeps)

fnv.o: $(fnvDeps)

input.o: $(inputDeps)

SimplexNoise.o: $(SimplexNoiseDeps)

OlsenNoise.o: $(OlsenNoiseDeps)

map.o: $(mapDeps)


optsD=-g --std=c++14 -Wall -Wno-missing-braces -msse3 -I ./boost_1_64_0

objectsD=mainD.o fnvD.o inputD.o SimplexNoiseD.o OlsenNoiseD.o mapD.o

debug: $(objectsD)
	$(CXX) $(optsD) -o mapGenD $(objectsD) $(LDFLAGS)

mainD.o: $(mainDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o mainD.o main.cpp

fnvD.o: $(fnvDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o fnvD.o fnv.cpp

inputD.o: $(inputDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o inputD.o input.cpp

SimplexNoiseD.o: $(SimplexNoiseDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o SimplexNoiseD.o SimplexNoise.cpp

OlsenNoiseD.o: $(OlsenNoiseDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o OlsenNoiseD.o OlsenNoise.cpp

mapD.o: $(mapDeps)
	$(CXX) $(optsD) -D_DEBUG -c -o mapD.o map.cpp

optsO=--std=c++14 -Wall -Wno-missing-braces -Ofast -msse3 -I ./boost_1_64_0 -flto --target=i686-pc-linux-gnu
objectsO = mainO.o fnvO.o inputO.o SimplexNoiseO.o OlsenNoiseO.o mapO.o

opt: $(objectsO)
	$(CXX) $(optsO) -o mapGenO $(objectsO) $(LDFLAGS) -flto

mainO.o: $(mainDeps)
	$(CXX) $(optsO) -c -o mainO.o main.cpp

fnvO.o: $(fnvDeps)
	$(CXX) $(optsO) -c -o fnvO.o fnv.cpp

inputO.o: $(inputDeps)
	$(CXX) $(optsO) -c -o inputO.o input.cpp

SimplexNoiseO.o: $(SimplexNoiseDeps)
	$(CXX) $(optsO) -c -o SimplexNoiseO.o SimplexNoise.cpp

OlsenNoiseO.o: $(OlsenNoiseDeps)
	$(CXX) $(optsO) -c -o OlsenNoiseO.o OlsenNoise.cpp

mapO.o: $(mapDeps)
	$(CXX) $(optsO) -c -o mapO.o map.cpp

.PHONY: clean cleanopt cleandbg cleanfull
clean: 
	rm -f mapGen $(objects)

cleanopt: 
	rm -f *O.o mapGenO

cleandbg:
	rm -f *D.o mapGenD

cleanfull:
	rm -f mapGen $(objects) mapGenO $(objectsO)  mapGenD $(objectsD)
