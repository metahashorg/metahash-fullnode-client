#pragma once

#include <string>
#include <map>
#include <mutex>
#include <fstream>

namespace storage
{
    struct crypt_keys
    {
        std::string pub_key;
        std::string prv_key;
    };

    class keys
    {
    public:
        static bool peek(const std::string& address, crypt_keys& result);
        static bool store(const std::string& address, const crypt_keys& data);

    protected:
        static bool read_from_file(const std::string& file, std::string& result);
        static bool write_to_file(const std::string& file, const std::string& result);

    protected:
        static std::mutex _locker;
        static std::map<std::string, crypt_keys> _storage;
    };
}
