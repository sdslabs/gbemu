//Taken from ScummVM's Common::Stream

#include "endianness.h"

namespace Common {
	class Stream {
        public:
            virtual ~Stream() {}

            // Below functions throw error and reset the error flag
            // Similar to ferror() and clearerr()

            virtual bool err() const { return false; }
            virtual void clearError() {}
	};

    class ReadStream : virtual public Stream {
        public:
	        ReadStream() {}

            // End of stream interface
            virtual bool eos() const = 0;

            // Read interface
            virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;
    };

}