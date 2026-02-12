#pragma once

#include "../shader.h"
#include "shader_types.h"

namespace core {
class PhongShader final : public IShader {
public:
  VertexOut vertex(const VertexIn &in,
                   const ShaderUniforms &uniforms) const override;
  FragmentOut fragment(const FragmentIn &in,
                       const ShaderUniforms &uniforms) const override;
};
} // namespace core
