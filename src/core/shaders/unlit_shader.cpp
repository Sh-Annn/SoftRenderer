#include "unlit_shader.h"

#include "../../color.h"
#include "../texture.h"

namespace core {
VertexOut UnlitShader::vertex(const VertexIn &in,
                              const ShaderUniforms &uniforms) const {
  VertexOut out;

  out.clip_pos = uniforms.mvp * Vec4(in.position, 1.f);
  out.world_pos = Vec3(uniforms.model * Vec4(in.position, 1.f));
  out.normal = glm::normalize(Vec3(uniforms.model * Vec4(in.normal, 0.f)));
  out.uv = in.uv;
  out.inv_w = (out.clip_pos.w != 0.f) ? (1.f / out.clip_pos.w) : 0.f;

  return out;
}

FragmentOut UnlitShader::fragment(const FragmentIn &in,
                                  const ShaderUniforms &uniforms) const {
  FragmentOut out;

  if (uniforms.texture_enabled && uniforms.texture &&
      uniforms.texture->valid()) {
    out.color = uniforms.texture->sample(in.uv.x, in.uv.y);
  } else {
    out.color = colors::White;
  }

  return out;
}
} // namespace core
