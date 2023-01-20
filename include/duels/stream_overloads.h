#ifndef DUELS_STREAM_OVERLOADS_H
#define DUELS_STREAM_OVERLOADS_H

#include <ostream>
#include <vector>
#include <array>
#include <yaml-cpp/yaml.h>

namespace duels
{
template <typename T>
struct is_array_or_vector {};

template <typename T>
struct is_array_or_vector<std::vector<T>> {T type;};

template <typename T, std::size_t N>
struct is_array_or_vector<std::array<T, N>> {T type;};
}

template <class Container,
          typename ValueType = decltype(duels::is_array_or_vector<Container>::type)>
inline std::ostream& operator<<(std::ostream& ss, const Container &data)
{
    ss << '[';
    const auto size(data.size());
    for(size_t i = 0; i < size; ++i)
        ss << (i ? ',' : ' ') << data[i];
    return ss << ']';
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
