#include "log.hpp"

#include <fstream>
#include <unordered_map>

using namespace std;


static shared_ptr<Log> l;

shared_ptr<Log> Log::instance(ostream* os) noexcept
{
	if (!l)
		l = make_unique<Log>();
	
	if ((!l->w) && (os)) {
		l->w = unique_ptr<ostream>(os);
		return l;
	}

	if ((!l->w) && (!os)) {
		l->w = make_unique<fstream>();
		return l;
	}

	if ((l->w) && (os))
		l->w.reset(os);
	
	return l;
}

shared_ptr<Log> Log::instance(const std::string& name) noexcept
{
	fstream *file = nullptr;
	if (!name.empty())
		file = new fstream(name, fstream::out | fstream::app);
	
	return Log::instance(file);
}

void Log::redirect(ostream *os) noexcept
{
    if (!l || !l->w) return;

	l->w.reset(os);
}

void Log::error(const string& message) const noexcept
{
    if (!l || !l->w) return;

    auto prefix = m_prefix.at(level::error);
    *(l->w) << "|" + prefix+ "|"
            << " - "
            << message
            <<'\n';
}

void Log::warning(const string& message) const noexcept
{
    if (!l || !l->w) return;

    auto prefix = m_prefix.at(level::warning);
	*(l->w) << "|" + prefix + "|"
		<< " - "
		<< message
		<< '\n';
}

void Log::info(const string& message) const noexcept
{
    if (!l || !l->w) return;

    auto prefix = m_prefix.at(level::info);
    *(l->w) << "|" + prefix + "|"
		<< " - "
		<< message
		<< '\n';
}
