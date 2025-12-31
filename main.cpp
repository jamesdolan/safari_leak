#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <emscripten.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/disassemble.h>

namespace {

const char* kFragmentShader = R"(
#version 450

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 frag_color;

layout(binding = 0) uniform sampler2D u_texture;

void main() {
  vec4 color = texture(u_texture, v_uv);
  frag_color = color;
}
)";

struct GlslangInitializer {
  GlslangInitializer() { glslang::InitializeProcess(); }
  ~GlslangInitializer() { glslang::FinalizeProcess(); }
};

std::vector<uint32_t>  compile_glsl(const std::string& source, EShLanguage stage = EShLangFragment) {
  static GlslangInitializer init;

  glslang::TShader shader(stage);
  
  const char* sources[] = { source.c_str() };
  const int lengths[] = { static_cast<int>(source.length()) };
  shader.setStringsWithLengths(sources, lengths, 1);
  
  shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
  shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
  shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  const TBuiltInResource* resources = GetDefaultResources();
  
  constexpr int default_version = 450;
  constexpr bool forward_compatible = false;
  constexpr EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

  if (!shader.parse(resources, default_version, forward_compatible, messages)) {
    std::fprintf(stderr, "GLSL parsing failed:\n%s\n%s\n", shader.getInfoLog(), shader.getInfoDebugLog());
    return {};
  }

  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    std::fprintf(stderr, "GLSL linking failed:\n%s\n%s\n", program.getInfoLog(), program.getInfoDebugLog());
    return {};
  }

  std::vector<uint32_t> spirv;
  glslang::SpvOptions spv_options;
  spv_options.generateDebugInfo = false;
  spv_options.disableOptimizer = true;
  spv_options.optimizeSize = false;

  glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spv_options);
  
  return spirv;
}

} // namespace

int main() {
  auto frag_spirv = compile_glsl(kFragmentShader, EShLangFragment);
  if (frag_spirv.empty()) {
    std::fprintf(stderr, "Failed to compile fragment shader\n");
    return 1;
  }
  std::printf("Fragment shader SPIRV size: %zu words\n\n", frag_spirv.size());
  std::printf("Done.... watch for Page memory leaks in Safari Timeline\n");
  emscripten_exit_with_live_runtime();
  return 0;
}
