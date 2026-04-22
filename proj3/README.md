This is Micah Hicks and The build instructions are to compile the prgoram with the provided wrapper implementation. There are 3 different ussage examples: create,
insert, and append. using these commands:
./mmap_util create <path> <fill_char> <size> 
./mmap_util insert <path> <offset> <bytes_incoming> <input>
./mmap_util append <path> <bytes_incoming> <input>
Create makes a new file or truncates an exisiting one and fills it with a given byte
Insert inserts bytes from standard input at the requested offset
Append appends bytes from standard input to the end of the file. 
Main.cc contains the full command line implementation for create insert and append.
mmap.cc provides the wrapper implementation for file operations
Mmap.h provides the wrapper declarations used by main.cc