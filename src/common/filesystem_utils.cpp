#include "filesystem_utils.h"
#include <sys/stat.h>

namespace fs_utils {
namespace dir {
bool is_exists(const char* path)
{
    struct stat st;
    if (stat(path, &st) == -1) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool create(const char* path)
{
    return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}
}
}
