#include "proj3/lib/include/mmap.h"
#include <algorithm>
#include <iostream>
#include <string>

using String = std::string;

int ftruncateCheck(int ftruncate_result, int fd){
    if(ftruncate_result == -1){
        proj3::close(fd);
        return 1;
    }
    return 0;
}


int mmapCheck(void* addr, int fd, off_t file_size){
    if(addr == reinterpret_cast<void*>(-1)){
        proj3::ftruncate(fd, file_size);
        proj3::close(fd);
        return 1;
    }
    return 0;
}

int msyncCheck(int msync_result, void* addr, int fd, off_t original_file_size, size_t sync_size){
    if(msync_result == -1){
        proj3::munmap(addr, sync_size);
        proj3::ftruncate(fd, original_file_size);
        proj3::close(fd);
        return 1;
    }
    return 0;
}

int munmapCheck(int munmap_result, int fd, off_t file_size){
    if(munmap_result == -1){
        proj3::ftruncate(fd, file_size);
        proj3::close(fd);
        return 1;
    }
    return 0;
}



int create(int argc, char* argv[]){
    if(argc < 4){
        std::cout <<"Not enough arguments provided. Usage: create <file_path> <fill_char> <file_size>" << std::endl;
        return 1;
    }   
    String file_path = argv[2];
    char fill_char = argv[3][0];
    off_t file_size = std::atoi(argv[4]);
    int fd = proj3::open(file_path.c_str(), proj3::O_RDWR | proj3::O_CREAT | proj3::O_TRUNC, 0666);
    if(fd == -1){
        return 1;
    }
    int ft = proj3::ftruncate(fd, file_size);
    if(ftruncateCheck(ft, fd) == 1){
        return 1;
    }
    size_t length = std::atoi(argv[4]);
    void* addr = proj3::mmap(nullptr, length, proj3::PROT_READ | proj3::PROT_WRITE, proj3::MAP_SHARED, fd, 0);

    if(mmapCheck(addr, fd, file_size) == 1){
        return 1;
    }

    char* mapRegion = static_cast<char*>(addr);
    for(size_t i = 0; i < static_cast<size_t>(file_size); i++){
        mapRegion[i] = fill_char;
    }

    int msync_result = proj3::msync(addr, length, proj3::MS_SYNC);
    
    if(msyncCheck(msync_result, addr, fd, static_cast<size_t>(file_size)) == 1){
        return 1;
    }
    
    int munmap_result = proj3::munmap(addr, length);
    if(munmapCheck(munmap_result, fd, static_cast<off_t>(file_size)) == 1){
        return 1;
    }
    proj3::close(fd);

    return 0;
}

int insert(int argc, char* argv[]){
    if(argc < 5){
        std::cout <<"Not enough arguments provided. Usage: insert <file_path> <offset> <bytes_incoming>" << std::endl;
        return 1;
    }
    String file_path = argv[2];
    size_t offset = std::atoi(argv[3]);
    size_t bytes_incoming = std::atoi(argv[4]);

    int fd = proj3::open(file_path.c_str(), proj3::O_RDWR);

    if(fd == -1){
        return 1;
    }
    struct stat fStat;
    proj3::fstat(fd, &fStat);
    size_t file_size = static_cast<size_t>(fStat.st_size);

    if(offset > file_size){
        proj3::close(fd);
        return 1;
    }

    size_t new_file_size = file_size + bytes_incoming;
    off_t new_file_size_off_t = static_cast<off_t>(new_file_size);
    int ft = proj3::ftruncate(fd,new_file_size_off_t);
    if(ftruncateCheck(ft, fd) == 1){
        return 1;
    }

    void* addr = proj3::mmap(nullptr, new_file_size, proj3::PROT_READ | proj3::PROT_WRITE, proj3::MAP_SHARED, fd, 0);
    if(mmapCheck(addr, fd, new_file_size) == 1){
        return 1;
    }

    char* mapRegion = static_cast<char*>(addr);
    for(size_t i = file_size - 1; i >= offset; i--){
        mapRegion[i + bytes_incoming] = mapRegion[i];
    }

    for(size_t i = offset; i < offset + bytes_incoming; i++){
        int character = std::cin.get();
        if(character == EOF){
            proj3::munmap(addr, new_file_size);
            proj3::ftruncate(fd, static_cast<off_t>(file_size));
            proj3::close(fd);
            return 1;
        }
        mapRegion[i] = static_cast<char>(character);
    }

    size_t sync_size = static_cast<size_t>(new_file_size);
    int msync_result = proj3::msync(addr, sync_size, proj3::MS_SYNC);
    
    if(msyncCheck(msync_result, addr, fd, static_cast<off_t>(file_size) ,new_file_size) == 1){
        return 1;
    }
    
    int munmap_result = proj3::munmap(addr, sync_size);
    if(munmapCheck(munmap_result, fd, new_file_size) == 1){
        return 1;
    }
    proj3::close(fd);

    return 0;

}

int append(int argc, char* argv[]){
    if(argc < 4){
        std::cout <<"Not enough arguments provided. Usage: append <file_path> <bytes_incoming>" << std::endl;
        return 1;
    }
    String file_path = argv[2];
    size_t bytes_incoming = std::atoi(argv[3]);

    int fd = proj3::open(file_path.c_str(), proj3::O_RDWR);
    if(fd == -1){
        return 1;
    }
    struct stat fStat;
    int fStat_result = proj3::fstat(fd, &fStat);
    if(fStat_result == -1){
        proj3::close(fd);
        return 1;
    }
    size_t file_size = static_cast<size_t>(fStat.st_size);
    
    size_t current_file_size = file_size;
    size_t current_bytes_incoming = bytes_incoming;

    if(current_file_size == 0 && current_bytes_incoming > 0 ){
        int ftruncate_result = proj3::ftruncate(fd, 1);
        if(ftruncateCheck(ftruncate_result, fd) == 1){
            return 1;
        }
        void* addr = proj3::mmap(nullptr, 1, proj3::PROT_READ | proj3::PROT_WRITE, proj3::MAP_SHARED, fd, 0);
        if(mmapCheck(addr, fd, file_size) == 1){
            return 1;
        }
        char* mapRegion = static_cast<char*>(addr);
        
        int charcter = std::cin.get();
        if(charcter == EOF){
            proj3::munmap(addr, 1);
            proj3::ftruncate(fd, 0);
            proj3::close(fd);
            return 1;
        }
        mapRegion[0] = static_cast<char>(charcter);
        
        size_t sync_size = 1;
        int msync_result = proj3::msync(addr, sync_size, proj3::MS_SYNC);
        if(msyncCheck(msync_result, addr, fd, file_size, sync_size) == 1){
            return 1;
        }
        int munmap_result = proj3::munmap(addr, sync_size);
        if(munmapCheck(munmap_result, fd, file_size) == 1){
            return 1;
        }
        current_file_size = 1;
        current_bytes_incoming --;
    }

    while(current_bytes_incoming > 0 ){
        size_t max_append_size = std::min(current_file_size, current_bytes_incoming);
        size_t max_file_size = current_file_size + max_append_size;
        int ftruncate_result = proj3::ftruncate(fd, max_file_size);
        if( ftruncate_result == -1){
            proj3::ftruncate(fd, current_file_size);
                proj3::close(fd);
                return 1;
            }
            void* addr = proj3::mmap(nullptr, max_file_size, proj3::PROT_READ | proj3::PROT_WRITE, proj3::MAP_SHARED, fd, 0);
            if(mmapCheck(addr, fd, max_file_size) == 1){
                return 1;
            }
            char* mapRegion = static_cast<char*>(addr);
            for(size_t i = current_file_size; i < max_file_size; i++){
                int character = std::cin.get();
                if(character == EOF){
                    proj3::munmap(addr, max_file_size);
                    proj3::ftruncate(fd, current_file_size);
                    proj3::close(fd);
                    return 1;
                }
                mapRegion[i] = static_cast<char>(character);
            }
            size_t sync_size = max_file_size;
            int msync_result = proj3::msync(addr, sync_size, proj3::MS_SYNC);
            if(msyncCheck(msync_result, addr, fd, file_size, sync_size) == 1){
                return 1;
            }
            int munmap_result = proj3::munmap(addr, sync_size);
            if(munmapCheck(munmap_result, fd, file_size) == 1){
                return 1;
            }
            current_file_size = max_file_size;
            current_bytes_incoming -= max_append_size;

        }
        proj3::close(fd);
        return 0;
    }

int main(int argc, char* argv[]){
    String command = argv[1];
    if (command == "create"){
        return create(argc, argv);
    }
    else if(command == "insert"){
        return insert(argc, argv);
    }
    else if(command == "append"){
        return append(argc, argv);
    }
    else{
        std::cerr << "Invalid command. Use 'create', 'insert', or 'append'." << std::endl;
        return 1;
    }
    return 0;

}
