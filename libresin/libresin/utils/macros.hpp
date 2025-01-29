#ifndef RESIN_MACROS_HPP
#define RESIN_MACROS_HPP

#define DISABLE_COPY_AND_MOVE(ClassName)           \
  ClassName(const ClassName&)            = delete; \
  ClassName(ClassName&&)                 = delete; \
  ClassName& operator=(const ClassName&) = delete; \
  ClassName& operator=(ClassName&&)      = delete;

#endif
