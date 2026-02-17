#include "util.hpp"

size_t customStringLen(String str) {
    return str.length();
}

size_t customStringLen(const __FlashStringHelper *str) {
    return strlen_P(reinterpret_cast<PGM_P>(str));
}

size_t customStringLen(const char *str) {
    return strlen(str);
}