
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nop/serializer.h>
#include <nop/structure.h>
#include <nop/table.h>
#include <nop/value.h>
#include <nop/utility/buffer_writer.h>

namespace {

struct Node {
  uint64_t name;
  uint16_t parent;
  NOP_STRUCTURE(Node, name, parent);
};

template <typename T, size_t Size>
struct Array {
    constexpr Array(T value, size_t size)
        : Array{value, size, std::make_index_sequence<Size>{}} {}
    template<size_t... Is>
    constexpr Array(T value, size_t size, std::index_sequence<Is...>)
        : data_{ T(value + 0 * Is) ... }, size_{size} {}

    T data_[Size];
    size_t size_;

    constexpr T& operator[] (size_t index) { return data_[index]; }
    constexpr T* data() { return data_; }
    constexpr const T* data() const { return data_; }
    constexpr size_t size() const { return size_; }
};

template <typename T>
constexpr size_t EncodingSize(const T& value) {
    return nop::Encoding<T>::Size(value);
}

template <std::size_t Size, typename T>
constexpr auto Serialize(const T& value) {
    Array<std::uint8_t, Size> bytes{0, Size};
    nop::Serializer<nop::BufferWriter> serializer{bytes.data(), bytes.size()};
    serializer.Write(value);
    return bytes;
}

template <std::size_t Size>
constexpr auto GenerateTopology() {
    uint16_t index = 0, big_cluster = 0, little_cluster = 0;
    std::array<Node, Size> nodes{{}};
    //NodeArray<Node, Size> nodes{{.name=0, .parent=0}, Size};
    nodes[big_cluster = index++] = {
        .name = 0xB16,
        .parent = 0,
    };
    nodes[index++] = {
        .name = 0xC1,
        .parent = big_cluster,
    };
    nodes[index++] = {
        .name = 0xC2,
        .parent = big_cluster,
    };
    nodes[index++] = {
        .name = 0xC3,
        .parent = big_cluster,
    };
    nodes[index++] = {
        .name = 0xC4,
        .parent = big_cluster,
    };
    nodes[little_cluster = index++] = {
        .name = 0x11771E,
        .parent = 0,
    };
    nodes[index++] = {
        .name = 0xC5,
        .parent = little_cluster,
    };
    nodes[index++] = {
        .name = 0xC6,
        .parent = little_cluster,
    };
    nodes[index++] = {
        .name = 0xC7,
        .parent = little_cluster,
    };
    nodes[index++] = {
        .name = 0xC8,
        .parent = little_cluster,
    };

    return nodes;
}

constexpr auto SerializeTopology() {
  constexpr auto nodes = GenerateTopology<10>();
  return Serialize<EncodingSize(nodes)>(nodes);
}

template <std::size_t Size>
constexpr auto GeneratePrimArray() {
    uint16_t index = 0, big_cluster = 0, little_cluster = 0;
    std::array<uint16_t, Size> nodes{{}};
    //NodeArray<Node, Size> nodes{{.name=0, .parent=0}, Size};
    nodes[big_cluster = index++] = 1000;
    nodes[index++] = big_cluster;
    nodes[index++] = big_cluster;
    nodes[index++] = big_cluster;
    nodes[index++] = big_cluster;
    nodes[little_cluster = index++] = 1000;
    nodes[index++] = little_cluster;
    nodes[index++] = little_cluster;
    nodes[index++] = little_cluster;
    nodes[index++] = little_cluster;
    return nodes;
}

constexpr auto SerializePrimArray() {
  constexpr auto nodes = GeneratePrimArray<10>();
  return Serialize<EncodingSize(nodes)>(nodes);
}

} // namespace

TEST(ConstExpr, SerializeStructArray) {
  constexpr auto serialized = SerializeTopology();
  printf("Topo: %lX \n", *(uint64_t*)serialized.data());
}

TEST(ConstExpr, SerializePrimativeArray) {
  constexpr auto serialized = SerializePrimArray();
  printf("Array: %lX \n", *(uint64_t*)serialized.data());
}

