#include "log.hpp"

#include <fstream>
#include <unordered_map>

using namespace std;

using unique_writer = unique_ptr<ostream>;

static unique_writer w = nullptr;

Log::Log(const string& ctx, ostream *os)
    : context(ctx)
{
    if ((!w) && (os)) {
        //TODO(hoenir): Find a way to remove this extra alloc/free
        w = make_unique<fstream>("");
        w.reset(os);
        return;
    }
         
    if ((!w) && (!os)) {
        w = make_unique<fstream>("log.txt");
        return;
    }
    
    if ((w) && (os))
        w.reset(os);
    
}

void Log::error(const string& message) const noexcept
{
    if (!w) return;

    auto prefix = m_prefix.at(level::error);
    (*w) << "|" + this->context + "|"
         << " - " 
         << "|" + prefix+ "|"
         << " - "
         << message
         <<'\n';
}

void Log::warning(const string& message) const noexcept
{
    if (!w) return;

    auto prefix = m_prefix.at(level::warning);
    (*w) << "|" + this->context + "|"
         << " - " 
         << "|" + prefix+ "|"
         << " - "
         << message
         <<'\n';
}

void Log::info(const string& message) const noexcept
{
    if (!w) return;
    auto prefix = m_prefix.at(level::info);
    (*w) << "|" + this->context + "|"
         << " - " 
         << "|" + prefix+ "|"
         << " - "
         << message
         <<'\n';
}
