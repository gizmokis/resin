#version 150
#define EXTERNAL_B 50
#define EXTERNAL_A 100
#define EXTERNAL_MAIN int func() { return 5; }
#ifndef SHADER_A
#define SHADER_A
#ifndef SHADER_B
#define SHADER_B
#ifndef SHADER_C
#define SHADER_C
int my_func_c() {
  return 1;
}
#endif
int my_func_b() {
  return 1;
}
#endif
int my_func_a() {
  return 1;
}
#endif
#ifndef SHADER_C
#define SHADER_C
int my_func_c() {
  return 1;
}
#endif
int main() {
  int a = EXTERNAL_A;
  my_func_a();
  my_func_b();
  my_func_c();
  return 0;
}
