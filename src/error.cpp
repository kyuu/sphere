#include "error.hpp"


//-----------------------------------------------------------------
void ReportOutOfMemory()
{
    throw std::runtime_error("Out of memory");
}
