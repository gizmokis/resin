#include <gtest/gtest.h>

#include <filesystem>
#include <libresin/core/resources/shader_resource.hpp>
#include <tests/files_helper.hpp>
#include <tests/libresin/test_consts.hpp>

class ShaderResourceTest : public testing::Test {
 protected:
  const std::filesystem::path data_path      = RESIN_TESTS_DATA_PATH;
  const std::filesystem::path resources_path = data_path / "core" / "resources";
};

TEST_F(ShaderResourceTest, ShaderWithDepsIsCorrectlyGenerated) {
  resin::ShaderResourceManager sh_resman;
  auto res = sh_resman.get_res(resources_path / "regular_load" / "main.frag");

  EXPECT_EQ(res.has_value(), true);

  resin::ShaderResource cpy(*res.value());
  cpy.set_ext_defi("EXTERNAL_MAIN", "int func() { return 5; }");
  cpy.set_ext_defi("EXTERNAL_A", "100");
  cpy.set_ext_defi("EXTERNAL_B", "50");

  EXPECT_FILE_CONTENT_EQ(resources_path / "regular_load" / "expected_raw.frag", cpy.get_raw());
  EXPECT_FILE_CONTENT_EQ(resources_path / "regular_load" / "expected_glsl.frag", cpy.get_glsl());
}

TEST_F(ShaderResourceTest, ShaderDepsCycleIsDetected) {
  resin::ShaderResourceManager sh_resman;
  auto res = sh_resman.get_res(resources_path / "deps_cycle" / "main.vert");

  EXPECT_EQ(res.has_value(), false);
}
