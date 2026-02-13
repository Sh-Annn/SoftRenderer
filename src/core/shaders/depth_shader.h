#pragma once

#include "../shader.h"
#include "shader_types.h"

namespace core {
class DepthShader final : public IShader {
  VertexOut vertex(const VertexIn &in,
                   const ShaderUniforms &uniforms) const override;
  FragmentOut fragment(const FragmentIn &in,
                       const ShaderUniforms &uniforms) const override;
};
} // namespace core
