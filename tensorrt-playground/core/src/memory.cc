/* Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "tensorrt/playground/core/memory.h"

#include <algorithm>
#include <cstring>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include <glog/logging.h>

namespace yais
{
// HostMemory

size_t HostMemory::DefaultAlignment() const
{
    return 64;
}

void HostMemory::Fill(char fill_value)
{
    std::memset(Data(), 0, Size());
}

const std::string& HostMemory::Type() const
{
    static std::string type = "HostMemory";
    return type;
}

/*
std::shared_ptr<HostMemory> HostMemory::UnsafeWrapRawPointer(
    void* ptr, size_t size, std::function<void(HostMemory*)> deleter)
{
    return std::shared_ptr<HostMemory>(new HostMemory(ptr, size), deleter);
}
*/

// SystemMallocMemory

void* SystemMallocMemory::Allocate(size_t size)
{
    void* ptr = malloc(size);
    CHECK(ptr) << "malloc(" << size << ") failed";
    return ptr;
}

void SystemMallocMemory::Free()
{
    free(Data());
}

const std::string& SystemMallocMemory::Type() const
{
    static std::string type = "SystemMallocMemory";
    return type;
}

// SystemV

const std::string& SystemV::Type() const
{
    static std::string type = "SystemV";
    return type;
}

void* SystemV::Attach(int shm_id)
{
    auto ptr = shmat(shm_id, 0, 0);
    CHECK(ptr);
    return ptr;
}

size_t SystemV::SegSize(int shm_id)
{
    struct shmid_ds stats;
    CHECK_EQ(shmctl(shm_id, IPC_STAT, &stats), 0);
    auto size = stats.shm_segsz;
    return size;
}

SystemV::~SystemV()
{
    if(Data() && Size())
    {
       DLOG(INFO) << "SystemV dtor detaching from sysv shmem";
       CHECK_EQ(shmdt(Data()), 0);
    }
}

void* SystemV::Allocate(size_t size)
{
    m_ShmID = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    m_Attachable = (bool)(m_ShmID != -1);
    CHECK(m_Attachable);
    return Attach(m_ShmID);
}

void SystemV::Free()
{
    DisableAttachment();
    DLOG(INFO) << "SystemV::Free deleting sysv shmem segment";
    // CHECK_EQ(shmdt(Data()), 0);
}

void SystemV::DisableAttachment()
{
    if(m_Attachable)
    {
        struct shmid_ds buff;
        CHECK_EQ(shmctl(m_ShmID, IPC_RMID, &buff), 0);
    }
    else
    {
        DLOG(WARNING) << "Attempting to disable attachment from a SystemV object that is either "
                      << "already detached, or was not the originating creator.";
    }
}

int SystemV::ShmID() const
{
    return m_ShmID;
}

bool SystemV::Attachable() const
{
    return m_Attachable;
}

} // namespace yais
