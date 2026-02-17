#ifndef UTIL_HPP
#define UTIL_HPP

#include <Arduino.h>

size_t customStringLen(String str);

size_t customStringLen(const __FlashStringHelper *str);

size_t customStringLen(const char *str);

#endif // UTIL_HPP