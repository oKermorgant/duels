#ifndef DUELS_STREAM_OVERLOADS_H
#define DUELS_STREAM_OVERLOADS_H

#include <ostream>
#include <vector>
#include <array>
#include <yaml-cpp/yaml.h>

// streaming vectors and arrays
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &data)
{
  os << "[";
  const auto size(data.size());
  auto sep{""};
  for(auto &val: data)
  {
    os << sep << val;
    sep = ",";
  }
  return os << "]";
}

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T,N> &data)
{
  os << '[';
  for(size_t i = 0; i < N; ++i)
    os << (i ? ',' : ' ') << data[i];
  return os << ']';
}


// how to stream and load Enums

template<typename EnumValue>
std::ostream& operator<<(typename std::enable_if_t<std::is_enum_v<EnumValue>, std::ostream>& ss, const EnumValue& e)
{
  return ss << static_cast<typename std::underlying_type_t<EnumValue>>(e);
}

namespace YAML
{
template<typename EnumValue>
struct convert
{
  static_assert (std::is_enum_v<EnumValue>, "This specialization is for enums only");
  static bool decode(Node const& node, EnumValue & rhs)
  {
    rhs = static_cast<EnumValue>(node.template as<typename std::underlying_type_t<EnumValue> >());
    return true;
  }
};
}


#endif // DUELS_STREAM_OVERLOADS_H
