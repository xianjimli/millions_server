all:
	mkdir build; cd build && cmake ../ && make

clean:
	rm -rf build bin vs2015 vs2012 vs2008

