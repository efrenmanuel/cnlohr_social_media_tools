all : streambuddy

TCCENGINE:=../cntools/tccengine/tccengine.c ../cntools/tccengine/tccexports.c
RAWDRAW:=../rawdraw/CNFGFunctions.c ../rawdraw/CNFGXDriver.c
CFLAGS:=-I../rawdraw -I../cntools/tccengine -DHAS_XSHAPE -DHAS_XINERAMA -DALLOW_ALPHA -DCNFGOGL
LDFLAGS:=-lm -lX11 -lXext -lXinerama -lpthread ../cntools/tccengine/tcc/libtcc_$(shell uname -m).a -ldl -lrt -lGL

streambuddy : streambuddy.c $(TCCENGINE) $(RAWDRAW)
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean :
	rm -rf *.o *~ streambuddy

