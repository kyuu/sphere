#include <stdexcept>
#include "error.hpp"


//-----------------------------------------------------------------
void ReportOutOfMemory()
{
    throw std::bad_alloc("Out of memory");
}
