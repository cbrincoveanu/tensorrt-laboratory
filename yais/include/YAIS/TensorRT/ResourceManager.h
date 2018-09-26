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
#ifndef _YAIS_TENSORRT_RESOURCEMANAGER_H_
#define _YAIS_TENSORRT_RESOURCEMANAGER_H_

#include <map>

#include <NvInfer.h>

#include "YAIS/Pool.h"
#include "YAIS/Resources.h"
#include "YAIS/TensorRT/Common.h"
#include "YAIS/TensorRT/Model.h"
#include "YAIS/TensorRT/Buffers.h"
#include "YAIS/TensorRT/ExecutionContext.h"

namespace yais
{
namespace TensorRT
{

/**
 * @brief TensorRT Resource Manager
 */
class ResourceManager : public ::yais::Resources
{
  public:
    ResourceManager(int max_executions, int max_buffers);
    virtual ~ResourceManager();

    void RegisterModel(std::string name, std::shared_ptr<Model> model);
    void RegisterModel(std::string name, std::shared_ptr<Model> model, uint32_t max_concurrency);

    void AllocateResources();

    auto GetBuffers() -> std::shared_ptr<Buffers>;
    auto GetModel(std::string model_name) -> std::shared_ptr<Model>;
    auto GetExecutionContext(const Model *model) -> std::shared_ptr<ExecutionContext>;
    auto GetExecutionContext(const std::shared_ptr<Model> &model) -> std::shared_ptr<ExecutionContext>;

  private:
    int m_MaxExecutions;
    int m_MaxBuffers;
    size_t m_MinHostStack;
    size_t m_MinDeviceStack;
    size_t m_MinActivations;
    std::shared_ptr<Pool<Buffers>> m_Buffers;
    std::shared_ptr<Pool<ExecutionContext>> m_ExecutionContexts;
    std::map<std::string, std::shared_ptr<Model>> m_Models;
    std::map<const Model *, std::shared_ptr<Pool<::nvinfer1::IExecutionContext>>> m_ModelExecutionContexts;

    std::size_t Align(std::size_t size, std::size_t alignment)
    {
        std::size_t remainder = size % alignment;
        size = (remainder == 0) ? size : size + alignment - remainder;
        return size;
    }
};

} // namespace TensorRT
} // namespace yais

#endif // _YAIS_TENSORRT_RESOURCEMANAGER_H_