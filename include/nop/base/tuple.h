#ifndef LIBNOP_INCLUDE_NOP_BASE_TUPLE_H_
#define LIBNOP_INCLUDE_NOP_BASE_TUPLE_H_

#include <tuple>

#include <nop/base/encoding.h>
#include <nop/base/utility.h>

namespace nop {

//
// std::tuple<Ts...> encoding format:
// +-----+---------+-----//-----+
// | ARY | INT64:N | N ELEMENTS |
// +-----+---------+-----//-----+
//
// Elements are expected to be valid encodings of each corresponding type in Ts.
//

template <typename... Types>
struct Encoding<std::tuple<Types...>> : EncodingIO<std::tuple<Types...>> {
  using Type = std::tuple<Types...>;

  template <std::size_t Index>
  using ElementType = typename std::tuple_element<Index, Type>::type;

  static constexpr EncodingByte Prefix(const Type& value) {
    return EncodingByte::Array;
  }

  static constexpr std::size_t Size(const Type& value) {
    return BaseEncodingSize(Prefix(value)) +
           Encoding<std::uint64_t>::Size(sizeof...(Types)) +
           Size(value, Index<sizeof...(Types)>{});
  }

  static constexpr bool Match(EncodingByte prefix) {
    return prefix == EncodingByte::Array;
  }

  template <typename Writer>
  static Status<void> WritePayload(EncodingByte /*prefix*/, const Type& value,
                                   Writer* writer) {
    auto status = Encoding<std::uint64_t>::Write(sizeof...(Types), writer);
    if (!status)
      return status;
    else
      return WriteElements(value, writer, Index<sizeof...(Types)>{});
  }

  template <typename Reader>
  static Status<void> ReadPayload(EncodingByte prefix, Type* value,
                                  Reader* reader) {
    std::uint64_t size;
    auto status = Encoding<std::uint64_t>::Read(&size, reader);
    if (!status)
      return status;
    else if (size != sizeof...(Types))
      return ErrorStatus(EIO);
    else
      return ReadElements(value, reader, Index<sizeof...(Types)>{});
  }

 private:
  // Terminates template recursion.
  static constexpr std::size_t Size(const Type& value, Index<0>) { return 0; }

  // Recursively determines the size of all tuple elements.
  template <std::size_t index>
  static constexpr std::size_t Size(const Type& value, Index<index>) {
    return Size(value, Index<index - 1>{}) +
           Encoding<ElementType<index - 1>>::Size(std::get<index - 1>(value));
  }

  // Terminates template recursion.
  template <typename Writer>
  static Status<void> WriteElements(const Type& value, Writer*, Index<0>) {
    return {};
  }

  // Recursively writes tuple elements to the writer.
  template <std::size_t index, typename Writer>
  static Status<void> WriteElements(const Type& value, Writer* writer,
                                    Index<index>) {
    auto status = WriteElements(value, writer, Index<index - 1>{});
    if (!status)
      return status;

    return Encoding<ElementType<index - 1>>::Write(std::get<index - 1>(value),
                                                   writer);
  }

  template <typename Reader>
  static Status<void> ReadElements(Type* value, Reader* reader, Index<0>) {
    return {};
  }

  template <std::size_t index, typename Reader>
  static Status<void> ReadElements(Type* value, Reader* reader, Index<index>) {
    auto status = ReadElements(value, reader, Index<index - 1>{});
    if (!status)
      return status;

    return Encoding<ElementType<index - 1>>::Read(&std::get<index - 1>(*value),
                                                  reader);
  }
};

}  // namespace nop

#endif  //  LIBNOP_INCLUDE_NOP_BASE_TUPLE_H_