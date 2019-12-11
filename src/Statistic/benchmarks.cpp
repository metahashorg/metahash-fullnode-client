#include "benchmarks.h"

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <array>
#include <vector>

#include <openssl/sha.h>

#include "duration.h"

static long get_openssl_test() {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> sha_1;
    common::Timer tt;
    for (size_t i = 0; i < 150000; i++) {
        std::string data = std::string(1500, '1') + "data " + std::to_string(i);
        SHA256((const unsigned char*)data.data(), data.size(), sha_1.data());
    }
    tt.stop();
    return tt.countMs();
}

static long get_mem_total() {
    struct sysinfo info;
    const int res = sysinfo(&info);
    if (res != 0) {
        return 0;
    }
    return info.totalram;
}

static long checkIO() {
    const char *fileName = "./tmp";
    const size_t countRepeat = 1000;
    
    std::array<char, 1024> buffer;
    for (size_t i = 0; i < buffer.size(); i++) {
        buffer[i] = rand() % 256;
    }

    int fd = ::open(fileName, O_SYNC | O_CREAT | O_WRONLY, 0600);
    common::Timer tt;
    
    for (size_t i = 0; i < countRepeat; i++) {
        auto res = write(fd, buffer.data(), buffer.size());
        if (res == 0) {
            return 0;
        }
    }
    close(fd);
    
    int fd2 = ::open(fileName, O_SYNC | O_RDONLY);
    for (size_t i = 0; i < countRepeat; i++) {
        auto res = read(fd, buffer.data(), buffer.size());
        if (res == 0) {
            return 0;
        }
    }
    close(fd2);
        
    tt.stop();
    
    remove(fileName);
    
    return tt.countMs();
}

static long checkIO2() {
    const char *fileName = "./tmp";
    
    std::vector<char> buffer(1024*1024*300);
    const size_t fillTo = buffer.size() / 100;
    for (size_t i = 0; i < buffer.size() / 100; i++) {
        buffer[i] = rand() % 256;
    }
    for (size_t i = fillTo; i < buffer.size(); i++) {
        buffer[i] = buffer[i % fillTo];
    }
    
    int fd = ::open(fileName, O_SYNC | O_CREAT | O_WRONLY, 0600);
    common::Timer tt;
    
    auto res = write(fd, buffer.data(), buffer.size());
    close(fd);
    if (res == 0) {
        return 0;
    }
    
    int fd2 = ::open(fileName, O_SYNC | O_RDONLY);
    auto res2 = read(fd, buffer.data(), buffer.size());
    close(fd2);
    if (res2 == 0) {
        return 0;
    }
    
    tt.stop();
    
    remove(fileName);
    
    return tt.countMs();
}

static BenchmarkInfo getBenchImpl() {
    BenchmarkInfo info;
    info.memory = get_mem_total();
    info.opensslTest = get_openssl_test();
    info.ioTest = checkIO();
    info.ioTest2 = checkIO2();
    return info;
}

BenchmarkInfo getBench() {
    const static BenchmarkInfo info = getBenchImpl();
    return info;
}
