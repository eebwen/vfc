
windir = /mnt/osslive_rescale_ts

LIBS = -lavformat -lavcodec -lavutil
SROUCE = vfc.cpp zlog.cpp
OBJ = vfc

all: 
	g++ -o $(OBJ) $(SROUCE) $(LIBS) -g -std=gnu++0x

release:
	g++ -o $(OBJ) $(SROUCE) $(LIBS) -O2 -std=gnu++0x

clean:
	rm -f $(OBJ)
	
co:
	cp $(windir)/*.cpp . -f
	cp $(windir)/*.h . -f
	
