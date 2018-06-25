#include "StrConfig.h"

#include <sstream>

string str_utility::IntToStr(int val)
{
	ostringstream tmp_str_stream;
	tmp_str_stream << val;
	return tmp_str_stream.str();
}
