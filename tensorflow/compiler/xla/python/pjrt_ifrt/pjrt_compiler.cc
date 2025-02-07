/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/compiler/xla/python/pjrt_ifrt/pjrt_compiler.h"

#include <memory>
#include <optional>
#include <utility>

#include "absl/status/status.h"
#include "llvm/Support/Casting.h"
#include "tensorflow/compiler/xla/python/pjrt_ifrt/pjrt_client.h"
#include "tensorflow/compiler/xla/python/pjrt_ifrt/pjrt_executable.h"
#include "tensorflow/compiler/xla/python/pjrt_ifrt/xla_compiler.h"
#include "tensorflow/tsl/platform/statusor.h"

namespace xla {
namespace ifrt {

char PjRtCompiler::ID = 0;

StatusOr<std::unique_ptr<LoadedExecutable>> PjRtCompiler::Compile(
    const Program& program, std::unique_ptr<CompileOptions> options) {
  DCHECK(this);
  const auto* xla_program = llvm::dyn_cast<XlaProgram>(&program);
  if (xla_program == nullptr) {
    return absl::InvalidArgumentError("PjRtCompiler requires an XlaProgram");
  }
  TF_ASSIGN_OR_RETURN(auto xla_compile_options,
                      GetXlaCompileOptions(std::move(options)));
  return PjRtLoadedExecutable::Create(
      client_, xla_program->mlir_module,
      std::move(xla_compile_options->compile_options),
      std::move(xla_compile_options->loaded_host_callbacks));
}

StatusOr<std::unique_ptr<LoadedExecutable>>
PjRtCompiler::DeserializeLoadedExecutable(
    absl::string_view serialized,
    std::unique_ptr<DeserializeExecutableOptions> options) {
  DCHECK(this);
  TF_ASSIGN_OR_RETURN(auto xla_deserialize_options,
                      GetXlaDeserializeExecutableOptions(std::move(options)));
  TF_ASSIGN_OR_RETURN(
      auto pjrt_loaded_executble,
      client_->pjrt_client()->DeserializeExecutable(
          serialized, std::move(xla_deserialize_options->compile_options)));
  return PjRtLoadedExecutable::Create(
      client_, std::move(pjrt_loaded_executble),
      std::move(xla_deserialize_options->loaded_host_callbacks));
}

}  // namespace ifrt
}  // namespace xla
