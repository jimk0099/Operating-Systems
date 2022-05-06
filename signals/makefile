TARGETS = child parent

# Targets to build when running `make`
all: $(TARGETS)

# Phony targets. Executed always.
.PHONY: all clean

# Targets to build `child` binary
child: child.o
	gcc child.o -o child
child.o: child.c
	gcc child.c -c -o child.o

# Targets to build `parentparent` binary
parentparent:parent.o
	gcc parent.o -oparent
parentparent.o:parent.c
	gcc parent.c -c -oparent.o

# Run `make clean` to delete binaries and object files.
clean:
	rm -rf $(TARGETS) *.o
