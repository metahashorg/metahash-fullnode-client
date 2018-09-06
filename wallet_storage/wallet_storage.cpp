#include "wallet_storage.h"
#include <boost/filesystem.hpp>

namespace storage
{
    std::mutex keys::_locker;
    std::map<std::string, crypt_keys> keys::_storage;

    bool keys::peek(const std::string& address, crypt_keys& result)
    {
        std::lock_guard<std::mutex> guard(_locker);
        auto it = _storage.find(address);
        if (it == _storage.end())
        {
            crypt_keys tmp;
            if (!read_from_file(address + ".raw.prv", tmp.prv_key))
                return false;
            if (!read_from_file(address + ".raw.pub", tmp.pub_key))
                return false;
            _storage[address] = tmp;
            result = tmp;
            return true;
        }
        result = it->second;
        return true;
    }

    bool keys::store(const std::string& address, const crypt_keys& data)
    {
        std::lock_guard<std::mutex> guard(_locker);
        if (_storage.find(address) != _storage.end())
                return false;
        if (!write_to_file(address + ".raw.prv", data.prv_key))
            return false;
        if (!write_to_file(address + ".raw.pub", data.pub_key))
            return false;
        return true;
    }

    bool keys::read_from_file(const std::string& file, std::string& result)
    {
        std::string path = settings::system::wallet_stotage;
        if (path.empty())
            return false;
        if (path[path.size() - 1] != '/')
            path.append("/");
        path.append(file);

        std::ifstream stream (path, std::ifstream::binary);
        if (!stream)
            return false;

        stream.seekg (0, stream.end);
        long length = stream.tellg();
        stream.seekg (0, stream.beg);
        result.clear();
        result.resize(static_cast<size_t>(length));
        stream.read(&result[0], length);
        stream.close();

        return true;
    }

    bool keys::write_to_file(const std::string& file, const std::string& data)
    {
        std::string path = settings::system::wallet_stotage;
        if (path.empty())
            return false;
        if (path[path.size() - 1] != '/')
            path.append("/");

        if (!boost::filesystem::exists(path))
            boost::filesystem::create_directory(path);

        path.append(file);

        std::ofstream stream (path, std::ifstream::binary);
        if (!stream)
            return false;

        stream.write(&data[0], data.size());
        stream.flush();
        stream.close();

        return true;
    }
}
