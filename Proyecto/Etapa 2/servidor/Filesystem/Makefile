all: test_disk.out fs_cli

test_disk.out: main.cc filesystem.cc disk.cc
	g++ -g main.cc filesystem.cc disk.cc -o test_disk.out

fs_cli: fs_cli.cc filesystem.cc disk.cc
	g++ -g fs_cli.cc filesystem.cc disk.cc -o fs_cli

run: test_disk.out
	./test_disk.out

clean:
	rm -f *.out *.bin fs_cli