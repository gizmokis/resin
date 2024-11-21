#external_definition EXTERNAL_MAIN

#include "shader_a.glsl"
#include "shader_c.glsl"

int main() {
  int a = EXTERNAL_A;
  my_func_a();
  my_func_b();
  my_func_c();
  return 0;
}
