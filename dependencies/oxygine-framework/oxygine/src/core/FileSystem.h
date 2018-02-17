#pragma once
#include "oxygine-include.h"
#include "Object.h"

namespace oxygine
{
    namespace file
    {
        class buffer;
        class FileSystem;

        class fileHandle: public ObjectBase
        {
        public:
            fileHandle(FileSystem* fs): _fs(fs) {}
            virtual void release() = 0;
            virtual unsigned int read(void* dest, unsigned int size) = 0;
            virtual unsigned int write(const void* src, unsigned int size) = 0;
            virtual unsigned int getSize() const = 0;
            virtual int          seek(unsigned int offset, int whence) = 0;
            virtual unsigned int tell() const = 0;

            FileSystem* _fs;
        };


        class FileSystem
        {
        public:
            enum status
            {
                //don't add new items (isExists will be broken)
                status_error,
                status_ok,
            };

            FileSystem(bool readonly);
            virtual ~FileSystem();

            void setPrefix(const char* str);

            bool isExists(const char* file);
            bool isExistsHere(const char* file);

            status deleteFile(const char* file);
            status renameFile(const char* src, const char* dest);
            status makeDirectory(const char* path);
            status deleteDirectory(const char* path);

            status open(const char* file, const char* mode, error_policy ep, fileHandle*& fh);
            status read(const char* file, file::buffer& bf, error_policy ep);

            void mount(FileSystem* fs);
            void unmount(FileSystem* fs);

        protected:
            virtual status _open(const char* file, const char* mode, error_policy ep, fileHandle*&) = 0;
            virtual status _read(const char* file, file::buffer&, error_policy ep) = 0;
            virtual bool _isExists(const char* file);
            virtual status _deleteFile(const char* file) = 0;
            virtual status _makeDirectory(const char* path) = 0;
            virtual status _deleteDirectory(const char* path) = 0;
            virtual status _renameFile(const char* src, const char* dest) = 0;

            char _prefix[32];
            bool _readonly;

            typedef std::vector<FileSystem*> filesystems;
            filesystems _filesystems;
        };

        void mount(FileSystem* fs);
        void unmount(FileSystem* fs);
    }
}