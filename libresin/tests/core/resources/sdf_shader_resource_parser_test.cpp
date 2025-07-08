#include <gtest/gtest.h>

#include <filesystem>
#include <libresin/core/resources/sdf_shader_resource_parser.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/utils/exceptions.hpp>
#include <tests/files_helper.hpp>
#include <tests/libresin/test_consts.hpp>

class SDFShaderResourceParserTest : public testing::Test {
 protected:
  const std::filesystem::path data_path_      = RESIN_TESTS_DATA_PATH;
  const std::filesystem::path resources_path_ = data_path_ / "core" / "resources";
};

TEST_F(SDFShaderResourceParserTest, SphereShaderIsProperlyParsed) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::ShaderResource test = *sh_resman.get_res(resources_path_ / "sdf_func" / "sphere.sdf");

  // when
  auto result = resin::SDFShaderResourceParser::parse(test);

  // then
  EXPECT_EQ(result.sdf_args_count, 1);
  EXPECT_EQ(result.sdf_args[0], "radius");
  EXPECT_EQ(result.glsl_primitive_function_name, "Sphere_Primitive");
  EXPECT_EQ(result.glsl_sdf_name, "Sphere_SDF");
  EXPECT_TRUE(result.content.find("res.dist = Sphere_SDF(pos,u_sdf_primitives[primitive_id].size.x)") !=
              std::string::npos);
  EXPECT_TRUE(result.content.find("float Sphere_SDF") != std::string::npos);
  EXPECT_TRUE(result.content.find("sdf_result Sphere_Primitive") != std::string::npos);
  std::print("{}", result.content);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenNoSDFIsDefined) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::ShaderResource test1 = *sh_resman.get_res(resources_path_ / "sdf_func" / "sphere_signature_only.sdf");
  resin::ShaderResource test2 = *sh_resman.get_res(resources_path_ / "sdf_func" / "no_func.sdf");

  // when/then
  EXPECT_THROW(resin::SDFShaderResourceParser::parse(test1), resin::SDFShaderNoFunctionBodyFound);
  EXPECT_THROW(resin::SDFShaderResourceParser::parse(test2), resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenMoreThanSDFIsDefined) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::ShaderResource test = *sh_resman.get_res(resources_path_ / "sdf_func" / "two_funcs.sdf");

  // when/then
  EXPECT_THROW(resin::SDFShaderResourceParser::parse(test), resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenUnsupportedParamTypeIsUsed) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::ShaderResource test = *sh_resman.get_res(resources_path_ / "sdf_func" / "invalid_param_type.sdf");

  // when/then
  EXPECT_THROW(resin::SDFShaderResourceParser::parse(test), resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenTooManyParamsAreProvided) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::ShaderResource test = *sh_resman.get_res(resources_path_ / "sdf_func" / "invalid_param_type.sdf");

  // when/then
  EXPECT_THROW(resin::SDFShaderResourceParser::parse(test), resin::SDFShaderInvalidFunctionSignature);
}
