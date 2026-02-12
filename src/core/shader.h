#pragma once

#include "shader_types.h"

namespace core {
class IShader {
public:
  virtual ~IShader() = default;
  virtual VertexOut vertex(const VertexIn &in,
                           const ShaderUniforms &uniforms) const = 0;
  virtual FragmentOut fragment(const FragmentIn &in,
                               const ShaderUniforms &uniforms) const = 0;
};
} // namespace core
