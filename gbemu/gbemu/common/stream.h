//Taken from ScummVM's Common::Stream

#include "endianness.h"

namespace Common {
	class Stream {
        virtual ~Stream() {}

        // Below functions throw error and reset the error flag
        // Similar to ferror() and clearerr()

        virtual bool err() const { return false; }
        virtual void clearError() {}
	};


}