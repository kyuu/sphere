#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "../Log.hpp"


namespace script {

    namespace internal {

        bool InitScript(const Log& log);
        void DeinitScript(const Log& log);

    } // namespace internal

} // namespace script


#endif
