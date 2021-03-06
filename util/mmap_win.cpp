// mmap_win.cpp

/*    Copyright 2009 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "pch.h"
#include "mmap.h"
#include "text.h"
#include <windows.h>

namespace mongo {

    MemoryMappedFile::MemoryMappedFile() {
        fd = 0;
        maphandle = 0;
        view = 0;
        len = 0;
        created();
    }

    void MemoryMappedFile::close() {
        if ( view )
            UnmapViewOfFile(view);
        view = 0;
        if ( maphandle )
            CloseHandle(maphandle);
        maphandle = 0;
        if ( fd )
            CloseHandle(fd);
        fd = 0;
    }
    
    unsigned mapped = 0;

    void* MemoryMappedFile::map(const char *filenameIn, long &length, int options) {
        _filename = filenameIn;
        /* big hack here: Babble uses db names with colons.  doesn't seem to work on windows.  temporary perhaps. */
        char filename[256];
        strncpy(filename, filenameIn, 255);
        filename[255] = 0;
        { 
            size_t len = strlen( filename );
            for ( size_t i=len-1; i>=0; i-- ){
                if ( filename[i] == '/' ||
                     filename[i] == '\\' )
                    break;
                
                if ( filename[i] == ':' )
                    filename[i] = '_';
            }
        }

        updateLength( filename, length );

        DWORD createOptions = FILE_ATTRIBUTE_NORMAL;
        if ( options & SEQUENTIAL )
            createOptions |= FILE_FLAG_SEQUENTIAL_SCAN;

        fd = CreateFile(
                 toNativeString(filename).c_str(),
                 GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ,
                 NULL, OPEN_ALWAYS, createOptions , NULL);
        if ( fd == INVALID_HANDLE_VALUE ) {
            out() << "Create/OpenFile failed " << filename << ' ' << GetLastError() << endl;
            return 0;
        }

        mapped += length;

        maphandle = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, length, NULL);
        if ( maphandle == NULL ) {
            out() << "CreateFileMapping failed " << filename << ' ' << GetLastError() << endl;
            return 0;
        }

        view = MapViewOfFile(maphandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if ( view == 0 ) {
            out() << "MapViewOfFile failed " << filename << " " << errnoWithDescription() << " ";
            out() << GetLastError();
            out() << endl;
        }
        len = length;
        return view;
    }

    void MemoryMappedFile::flush(bool sync) {
        uassert(13056, "Async flushing not supported on windows", sync);

        if (!view || !fd) return;

        bool success = FlushViewOfFile(view, 0); // 0 means whole mapping
        if (!success){
            int err = GetLastError();
            out() << "FlushViewOfFile failed " << err << " file: " << _filename << endl;
        }

        success = FlushFileBuffers(fd);
        if (!success){
            int err = GetLastError();
            out() << "FlushFileBuffers failed " << err << " file: " << _filename << endl;
        }
    }

    void MemoryMappedFile::_lock() {}
    void MemoryMappedFile::_unlock() {}

} 
