#include <gtest/gtest.h>

#include <filesystem>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/resources/shader_type.hpp>
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

  // when
  resin::ShaderResource res = *sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere.sdf");
  const auto& type          = std::get<resin::SDFShaderType>(res.type());

  // then
  EXPECT_EQ(type.args.size(), 1);
  EXPECT_EQ(type.args[0], "radius");
  EXPECT_EQ(res.name(), "Sphere");
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenNoSDFIsDefined) {
  // given
  resin::ShaderResourceManager sh_resman;

  // when/then
  EXPECT_THROW(*sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere_signature_only.sdf"),
               resin::SDFShaderNoFunctionBodyFound);
  EXPECT_THROW(*sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "no_func.sdf"),
               resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenMoreThanSDFIsDefined) {
  // given
  resin::ShaderResourceManager sh_resman;

  // when/then
  EXPECT_THROW(*sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "two_funcs.sdf"),
               resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenUnsupportedParamTypeIsUsed) {
  // given
  resin::ShaderResourceManager sh_resman;

  // when/then
  EXPECT_THROW(*sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "invalid_param_type.sdf"),
               resin::SDFShaderInvalidFunctionSignature);
}

TEST_F(SDFShaderResourceParserTest, ParserThrowsWhenTooManyParamsAreProvided) {
  // given
  resin::ShaderResourceManager sh_resman;

  // when/then
  EXPECT_THROW(*sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "invalid_param_type.sdf"),
               resin::SDFShaderInvalidFunctionSignature);
}
