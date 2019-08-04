/***
 * millipede: OSSWrapper.h
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_OSSWRAPPER
#define H_MILLIPEDE_OSSWRAPPER

#include <sstream>
#include <string>

namespace mp {

class OSSWrapper
{
	//#################### PRIVATE VARIABLES ####################
private:
	std::ostringstream m_os;

	//#################### PUBLIC OPERATORS ####################
public:
	operator std::string() const
	{
		return m_os.str();
	}

	template <typename T>
	OSSWrapper& operator<<(const T& rhs)
	{
		m_os << rhs;
		return *this;
	}

	OSSWrapper& operator<<(const char *rhs)
	{
		m_os << rhs;
		return *this;
	}

	OSSWrapper& operator<<(std::ostream& (*manip)(std::ostream&))
	{
		m_os << manip;
		return *this;
	}

	OSSWrapper& operator<<(std::ios& (*manip)(std::ios&))
	{
		m_os << manip;
		return *this;
	}
};

}

#endif
