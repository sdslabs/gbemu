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

            byte readByte() {
                byte b = 0;
                read(&b, 1);
                return b;
            }

            uint16 readUint16LE() {
		        uint16 val;
		        read(&val, 2);
		        return FROM_LE_16(val);
	        }

            uint32 readUint32LE() {
	        	uint32 val;
	        	read(&val, 4);
	        	return FROM_LE_32(val);
	        }

            uint64 readUint64LE() {
	        	uint64 val;
	        	read(&val, 8);
	        	return FROM_LE_64(val);
	        }

            uint16 readUint16BE() {
	        	uint16 val;
	        	read(&val, 2);
	        	return FROM_BE_16(val);
	        }

            uint32 readUint32BE() {
	        	uint32 val;
	        	read(&val, 4);
	        	return FROM_BE_32(val);
	        }

            uint64 readUint64BE() {
	        	uint64 val;
	        	read(&val, 8);
	        	return FROM_BE_64(val);
	        }

            inline byte readSByte() {
                return (int8) readByte();
            }

            inline int16 readSint16LE() {
		        return (int16) readUint16LE();
	        }

            inline int32 readSint32LE() {
	        	return (int32) readUint32LE();
	        }

            inline int64 readSint64LE() {
	        	return (int64) readUint64LE();
	        }

            inline int16 readSint16BE() {
	        	return (int16) readUint16BE();
	        }

            inline int32 readSint32BE() {
	        	return (int32) readUint32BE();
	        }

            inline int64 readSint64BE() {
	        	return (int64) readUint64BE();
	        }
    };

    class SeekableReadStream : virtual public ReadStream {
        public:
            // Position of cursor
            virtual int64 pos() const = 0;

            // Size of stream
            virtual int64 size() const = 0;

            // Set the cursor to a specific place in stream
            // wrapper identical to fseek()
            virtual bool seek(int64 offset, int whence = SEEK_SET) = 0;

            // Skip given bytes in stream
            virtual bool skip(uint32 offset) { return seek(offset, SEEK_CUR); }
    };
}