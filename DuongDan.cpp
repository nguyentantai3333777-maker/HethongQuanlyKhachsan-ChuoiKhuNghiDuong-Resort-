// DuongDan.cpp
#include "DuongDan.h"
#include <cstdlib>
#include <filesystem>
#include <system_error>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#include <climits>
#endif

namespace fs = std::filesystem;
using std::string;
using std::vector;

namespace
{
    bool coDuLieu(const fs::path &d)
    {
        std::error_code ec;
        return fs::exists(d / "chinhanh.csv", ec) || fs::exists(d / "chinhanh.txt", ec);
    }

    string chuanHoa(const fs::path &p)
    {
        std::error_code ec;
        fs::path r = fs::weakly_canonical(p, ec);
        return (ec ? fs::absolute(p) : r).string();
    }

    fs::path duongDanExe(const string &argv0)
    {
        std::error_code ec;
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (n > 0 && n < MAX_PATH)
            return fs::path(string(buf, n));
#elif defined(__APPLE__)
        char buf[PATH_MAX];
        uint32_t sz = sizeof(buf);
        if (_NSGetExecutablePath(buf, &sz) == 0)
            return fs::path(buf);
#else
        char buf[PATH_MAX];
        ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (n > 0)
            return fs::path(string(buf, (size_t)n));
#endif
        if (!argv0.empty())
        {
            fs::path p = fs::absolute(fs::path(argv0), ec);
            if (!ec)
                return p;
        }
        return fs::path();
    }

    // Duyet tu goc di len toi 6 cap cha, tim data/ hoac backend/data/
    bool timTuGoc(fs::path goc, string &kq, vector<string> *daTim)
    {
        if (goc.empty())
            return false;
        for (int cap = 0; cap < 7; ++cap)
        {
            for (const char *rel : {"data", "backend/data", "hotel-resort-management/backend/data"})
            {
                fs::path d = goc / rel;
                if (daTim)
                    daTim->push_back(chuanHoa(d));
                if (coDuLieu(d))
                {
                    kq = chuanHoa(d);
                    return true;
                }
            }
            if (!goc.has_parent_path() || goc.parent_path() == goc)
                break;
            goc = goc.parent_path();
        }
        return false;
    }
}

namespace DuongDan
{
    string thuMucChayChuongTrinh(const string &argv0)
    {
        fs::path e = duongDanExe(argv0);
        return e.empty() ? string() : chuanHoa(e.parent_path());
    }

    string timThuMucDuLieu(const string &argv0, const string &chiDinh, vector<string> *daTim)
    {
        if (!chiDinh.empty())
            return chuanHoa(fs::path(chiDinh));
        if (const char *env = getenv("OCEANPEARL_DATA"))
            if (*env)
                return chuanHoa(fs::path(env));

        const string exeDir = thuMucChayChuongTrinh(argv0);
        string kq;
        if (!exeDir.empty() && timTuGoc(fs::path(exeDir), kq, daTim))
            return kq;
        std::error_code ec;
        if (timTuGoc(fs::current_path(ec), kq, daTim))
            return kq;
        return exeDir.empty() ? chuanHoa(fs::path("data")) : chuanHoa(fs::path(exeDir) / "data");
    }
}
