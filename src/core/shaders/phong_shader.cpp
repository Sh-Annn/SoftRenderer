#include "phong_shader.h"

#include "../../color.h"
#include "../texture.h"
#include "shader_types.h"
#include "types.h"

namespace core {
namespace {
Vec3 color_to_vec3(Color c) {
  return Vec3(((c >> 16) & 0xFF) / 255.f, ((c >> 8) & 0xFF) / 255.f,
              (c & 0xFF) / 255.f);
}

Vec3 safe_normalize(const Vec3 &v, const Vec3 &fallback) {
  return glm::dot(v, v) > 1e-8f ? glm::normalize(v) : fallback;
}

Vec3 build_fallback_tangent(const Vec3 &n) {
  const Vec3 axis =
      std::abs(n.z) < 0.99f ? Vec3(0.f, 0.f, 1.f) : Vec3(0.f, 1.f, 0.f);
  return safe_normalize(glm::cross(axis, n), Vec3(1.f, 0.f, 0.f));
}
} // namespace

VertexOut PhongShader::vertex(const VertexIn &in,
                              const ShaderUniforms &uniforms) const {
  VertexOut out;
  out.clip_pos = uniforms.mvp * Vec4(in.position, 1.f);
  out.world_pos = Vec3(uniforms.model * Vec4(in.position, 1.f));
  out.normal = glm::normalize(Vec3(uniforms.model * Vec4(in.normal, 0.f)));
  out.tangent = safe_normalize(Vec3(uniforms.model * Vec4(in.tangent, 0.f)),
                               Vec3(1.f, 0.f, 0.f));
  out.bitangent = safe_normalize(Vec3(uniforms.model * Vec4(in.bitangent, 0.f)),
                                 Vec3(0.f, 1.f, 0.f));
  out.uv = in.uv;
  out.inv_w = (out.clip_pos.w != 0.f) ? (1.f / out.clip_pos.w) : 0.f;

  return out;
}

FragmentOut PhongShader::fragment(const FragmentIn &in,
                                  const ShaderUniforms &uniforms) const {
  FragmentOut out;

  const Vec3 ambient = uniforms.light_color * uniforms.ambient_intensity;
  Color color = make_color((u8)(ambient.x * 255.f), (u8)(ambient.y * 255.f),
                           (u8)(ambient.z * 255.f));

  if (uniforms.texture_enabled && uniforms.texture &&
      uniforms.texture->valid()) {
    color = uniforms.texture->sample(in.uv.x, in.uv.y);
  }

  if (!uniforms.light_enabled) {
    out.color = color;
    return out;
  }

  const Vec3 object_color = color_to_vec3(color);
  Vec3 normal = safe_normalize(in.normal, Vec3(0.f, 0.f, 1.f));

  if (uniforms.normal_map_enabled && uniforms.normal_map &&
      uniforms.normal_map->valid()) {
    const Color narmal_color = uniforms.normal_map->sample(in.uv.x, in.uv.y);
    const Vec3 n_ts =
        safe_normalize(color_to_vec3(narmal_color) * 2.f - Vec3(1.f, 1.f, 1.f),
                       Vec3(0.f, 0.f, 1.f));

    Vec3 tangent = in.tangent - normal * glm::dot(normal, in.tangent);
    tangent = safe_normalize(tangent, build_fallback_tangent(normal));

    Vec3 bitangent = in.bitangent - normal * glm::dot(normal, in.bitangent);
    bitangent = bitangent - tangent * glm::dot(tangent, bitangent);

    if (glm::dot(bitangent, bitangent) <= 1e-8f) {
      bitangent = glm::cross(normal, tangent);
    }
    bitangent = safe_normalize(bitangent, Vec3(0.f, 1.f, 0.f));

    normal = safe_normalize(
        tangent * n_ts.x + bitangent * n_ts.y + normal * n_ts.z, normal);
  }

  const Vec3 light_dir_norm = glm::normalize(uniforms.light_pos - in.world_pos);
  const Vec3 view_dir_norm = glm::normalize(uniforms.view_pos - in.world_pos);
  const Vec3 h = glm::normalize(light_dir_norm + view_dir_norm);

  const float r_2 = glm::dot(light_dir_norm, light_dir_norm);
  const float intensity = uniforms.light_intensity / r_2;

  Vec3 diffuse(0.f);
  if (uniforms.diff_enabled) {
    const float diff = std::max(glm::dot(normal, light_dir_norm), 0.f);
    diffuse = uniforms.kd * intensity * diff;
  }

  Vec3 specular(0.f);
  if (uniforms.spec_enabled) {
    const float spec =
        std::pow(std::max(0.f, glm::dot(h, normal)), uniforms.shininess);
    specular = uniforms.ks * intensity * spec;
  }

  Vec3 result = (ambient + diffuse + specular) * object_color;
  result = glm::clamp(result, 0.f, 1.f);
  out.color = make_color((u8)(result.x * 255.f), (u8)(result.y * 255.f),
                         (u8)(result.z * 255.f));

  return out;
}
} // namespace core
