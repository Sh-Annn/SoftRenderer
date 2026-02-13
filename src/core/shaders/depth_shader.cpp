#include "depth_shader.h"

#include "../../color.h"
#include <algorithm>

namespace core {
VertexOut DepthShader::vertex(const VertexIn &in,
                              const ShaderUniforms &uniforms) const {
  VertexOut out;

  out.clip_pos = uniforms.mvp * Vec4(in.position, 1.f);
  out.world_pos = Vec3(uniforms.model * Vec4(in.position, 1.f));
  out.normal = glm::normalize(Vec3(uniforms.model * Vec4(in.normal, 0.f)));
  out.uv = in.uv;
  out.inv_w = (out.clip_pos.w != 0.f) ? (1.f / out.clip_pos.w) : 0.f;

  return out;
}

FragmentOut DepthShader::fragment(const FragmentIn &in,
                                  const ShaderUniforms &uniforms) const {
  (void)uniforms;
  FragmentOut out;

  const float depth = std::clamp(in.depth, 0.f, 1.f);
  const float gray = 1.f - depth;
  const u8 c = static_cast<u8>(gray * 255.f);
  out.color = make_color(c, c, c);

  return out;
}
} // namespace core
