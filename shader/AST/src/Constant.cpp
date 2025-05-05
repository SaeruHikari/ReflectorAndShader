#include "SSL/Constant.hpp"

namespace skr::SSL {

IntValue::IntValue(int8_t v) : _bitwidth(8), _signed(true) { new (&_storage) int64_t(v); }
IntValue::IntValue(int16_t v) : _bitwidth(16), _signed(true) { new (&_storage) int64_t(v); }
IntValue::IntValue(int32_t v) : _bitwidth(32), _signed(true) { new (&_storage) int64_t(v); }
IntValue::IntValue(int64_t v) : _bitwidth(64), _signed(true) { new (&_storage) int64_t(v); }

IntValue::IntValue(uint8_t v) : _bitwidth(8), _signed(false) { new (&_storage) uint64_t(v); }
IntValue::IntValue(uint16_t v) : _bitwidth(16), _signed(false) { new (&_storage) uint64_t(v); }
IntValue::IntValue(uint32_t v) : _bitwidth(32), _signed(false) { new (&_storage) uint64_t(v); }
IntValue::IntValue(uint64_t v) : _bitwidth(64), _signed(false) { new (&_storage) uint64_t(v); }

FloatValue::FloatValue(float v)
    : _bitwidth(32), _ieee(v)
{

}

} // namespace skr::SSL