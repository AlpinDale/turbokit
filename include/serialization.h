#pragma once

#include "buffer.h"
#include "hash_map.h"
#include "simple_vector.h"
#include "vector.h"

#include <cstddef>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace turbokit {

template <typename Context> struct serialize_detector {
  template <typename Type> static std::false_type detect_serialize(...);
  template <typename Type, typename = decltype(std::declval<Type>().serialize(
                               std::declval<Context &>()))>
  static std::true_type detect_serialize(int);
  template <typename Type>
  static const bool has_serialize = decltype(detect_serialize<Type>(0))::value;
};

struct DataFormatError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

template <typename Context, typename First, typename Second>
void serialize(Context &context, const std::pair<First, Second> &pair) {
  context(pair.first, pair.second);
}

template <typename Context, typename First, typename Second>
void serialize(Context &context, std::pair<First, Second> &pair) {
  context(pair.first, pair.second);
}

template <typename Context, typename Type>
void serialize(Context &context, const std::optional<Type> &optional) {
  context(optional.has_value());
  if (optional.has_value()) {
    context(optional.value());
  }
}

template <typename Context, typename Type>
void serialize(Context &context, std::optional<Type> &optional) {
  if (context.template read<bool>()) {
    optional.emplace();
    context(optional.value());
  } else {
    optional.reset();
  }
}

template <typename Context, typename... Types>
void serialize(Context &context, const std::variant<Types...> &variant) {
  context(variant.index());
  std::visit([&](auto &value) { context(value); }, variant);
}

template <size_t Index, typename Context, typename VariantType, typename First,
          typename... Rest>
void deserialize_variant_helper(size_t variant_index, Context &context,
                                VariantType &variant) {
  if (variant_index == Index) {
    context(variant.template emplace<Index>());
  }
  if constexpr (Index + 1 != std::variant_size_v<VariantType>) {
    deserialize_variant_helper<Index + 1, Context, VariantType, Rest...>(
        variant_index, context, variant);
  }
}

template <typename Context, typename... Types>
void serialize(Context &context, std::variant<Types...> &variant) {
  size_t variant_index = context.template read<size_t>();
  deserialize_variant_helper<0, Context, std::variant<Types...>, Types...>(
      variant_index, context, variant);
}

template <typename Context, typename... Types>
void serialize(Context &context, const std::tuple<Types...> &tuple) {
  std::apply(
      [&context](const std::decay_t<Types> &...values) { context(values...); },
      tuple);
}

template <typename Context, typename... Types>
void serialize(Context &context, std::tuple<Types...> &tuple) {
  std::apply([&context](std::decay_t<Types> &...values) { context(values...); },
             tuple);
}

template <typename Context, typename Container>
void serialize_container(Context &context, const Container &container) {
  context(container.size());
  for (auto &item : container) {
    context(item);
  }
}

template <typename Context, typename Container>
void serialize_container(Context &context, Container &container) {
  using ElementType = typename Container::value_type;
  if constexpr (std::is_trivial_v<ElementType> &&
                !serialize_detector<Context>::template has_serialize<
                    ElementType>) {
    std::basic_string_view<ElementType> view;
    context(view);
    container.resize(view.size());
    std::memcpy(container.data(), view.data(),
                sizeof(ElementType) * view.size());
  } else {
    size_t count = context.template read<size_t>();
    container.resize(count);
    for (size_t i = 0; i != count; ++i) {
      context(container[i]);
    }
  }
}

template <typename Context, typename Type>
void serialize(Context &context, const std::vector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename Type>
void serialize(Context &context, std::vector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename Type>
void serialize(Context &context, const Vector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename Type>
void serialize(Context &context, Vector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename Type>
void serialize(Context &context, const SimpleVector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename Type>
void serialize(Context &context, SimpleVector<Type> &vector) {
  serialize_container(context, vector);
}

template <typename Context, typename MapType>
void serialize_map(Context &context, const MapType &map) {
  context(map.size());
  for (auto &entry : map) {
    context(entry.first, entry.second);
  }
}

template <typename Context, typename MapType>
void serialize_map(Context &context, MapType &map) {
  map.clear();
  size_t count = context.template read<size_t>();
  for (; count; --count) {
    auto key = context.template read<typename MapType::key_type>();
    map.emplace(std::move(key),
                context.template read<typename MapType::mapped_type>());
  }
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context, const std::map<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context, std::map<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context,
               const std::unordered_map<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context, std::unordered_map<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context, const HashMap<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

template <typename Context, typename KeyType, typename ValueType>
void serialize(Context &context, HashMap<KeyType, ValueType> &map) {
  serialize_map(context, map);
}

struct SizeOperation {};
struct WriteOperation {};
struct ReadOperation {};

// This is not a cross platform serializer
struct DataWriter {
  std::byte *write(SizeOperation, std::byte *destination,
                   [[maybe_unused]] const void *source, size_t length) {
    return (std::byte *)((uintptr_t)destination + length);
  }
  std::byte *write(WriteOperation, std::byte *destination, const void *source,
                   size_t length) {
    if (source) {
      std::memcpy(destination, source, length);
    }
    return destination + length;
  }
  template <typename Operation, typename Type,
            std::enable_if_t<std::is_trivial_v<Type>> * = nullptr>
  std::byte *write(Operation, std::byte *destination, Type value) {
    destination =
        write(Operation{}, destination, (void *)&value, sizeof(value));
    return destination;
  }
  template <typename Operation>
  std::byte *write(Operation, std::byte *destination, std::string_view string) {
    destination = write(Operation{}, destination, string.size());
    destination = write(Operation{}, destination, string.data(), string.size());
    return destination;
  }
  template <typename Operation, typename Type>
  std::byte *write(Operation, std::byte *destination,
                   std::basic_string_view<Type> string) {
    destination = write(Operation{}, destination, string.size());
    destination = write(Operation{}, destination, string.data(),
                        sizeof(Type) * string.size());
    return destination;
  }
};

struct DataReader {
  std::string_view buffer;
  DataReader() = default;
  DataReader(std::string_view buffer) : buffer(buffer) {}
  DataReader(const void *data, size_t length)
      : buffer((const char *)data, length) {}
  [[noreturn]] void end_of_data() {
    throw DataFormatError("DataReader: reached end of data");
  }
  void advance(size_t length) {
    buffer = {buffer.data() + length, buffer.size() - length};
  }
  template <typename Type> std::basic_string_view<Type> read_string_view() {
    size_t length = read<size_t>();
    if (buffer.size() < sizeof(Type) * length) {
      end_of_data();
    }
    Type *data = (Type *)buffer.data();
    advance(sizeof(Type) * length);
    return {data, length};
  }
  std::string_view read_string() {
    size_t length = read<size_t>();
    if (buffer.size() < length) {
      end_of_data();
    }
    const char *data = buffer.data();
    advance(length);
    return {data, length};
  }
  template <typename Type,
            std::enable_if_t<std::is_trivial_v<Type>> * = nullptr>
  void read(Type &result) {
    if (buffer.size() < sizeof(Type)) {
      end_of_data();
    }
    std::memcpy(&result, buffer.data(), sizeof(Type));
    advance(sizeof(Type));
  }
  void read(std::string_view &result) { result = read_string(); }
  void read(std::string &result) { result = read_string(); }
  template <typename Type> void read(std::basic_string_view<Type> &result) {
    result = read_string_view<Type>();
  }

  template <typename Type> Type read() {
    Type result;
    read(result);
    return result;
  }
  std::string_view read() { return read_string(); }

  bool empty() { return buffer.empty(); }
};

template <typename Operation> struct SerializeContext {
  std::byte *start = nullptr;
  std::byte *current = nullptr;
  template <typename Type> static std::false_type detect_serialize_f(...);
  template <typename Type, typename = decltype(std::declval<Type>().serialize(
                               std::declval<SerializeContext &>()))>
  static std::true_type detect_serialize_f(int);
  template <typename Type>
  static const bool has_serialize =
      decltype(detect_serialize_f<Type>(0))::value;
  template <typename Type> static std::false_type detect_builtin_write_f(...);
  template <typename Type,
            typename = decltype(std::declval<DataWriter>().write(
                WriteOperation{}, (std::byte *)nullptr, std::declval<Type>()))>
  static std::true_type detect_builtin_write_f(int);
  template <typename Type>
  static const bool has_builtin_write =
      decltype(detect_builtin_write_f<Type>(0))::value;
  template <typename Type> void operator()(const Type &value) {
    if constexpr (has_serialize<const Type>) {
      value.serialize(*this);
    } else if constexpr (has_serialize<Type>) {
      const_cast<Type &>(value).serialize(*this);
    } else if constexpr (has_builtin_write<const Type>) {
      current = DataWriter{}.write(Operation{}, current, value);
    } else {
      serialize(*this, value);
    }
  }
  template <typename... Types> void operator()(const Types &...values) {
    ((*this)(std::forward<const Types &>(values)), ...);
  }

  void write(const void *data, size_t length) {
    current =
        DataWriter{}.write(Operation{}, current, (std::byte *)data, length);
  }

  size_t tell() const { return current - start; }
};

struct DeserializeContext {
  DeserializeContext(DataReader &reader) : reader(reader) {}
  DataReader &reader;

  template <typename Type> static std::false_type detect_serialize_f(...);
  template <typename Type, typename = decltype(std::declval<Type>().serialize(
                               std::declval<DeserializeContext &>()))>
  static std::true_type detect_serialize_f(int);
  template <typename Type>
  static const bool has_serialize =
      decltype(detect_serialize_f<Type>(0))::value;
  template <typename Type> static std::false_type detect_builtin_read_f(...);
  template <typename Type, typename = decltype(std::declval<DataReader>().read(
                               std::declval<Type &>()))>
  static std::true_type detect_builtin_read_f(int);
  template <typename Type>
  static const bool has_builtin_read =
      decltype(detect_builtin_read_f<Type>(0))::value;
  template <typename Type> void operator()(Type &value) {
    if constexpr (has_serialize<Type>) {
      value.serialize(*this);
    } else if constexpr (has_builtin_read<Type>) {
      reader.read(value);
    } else {
      serialize(*this, value);
    }
  }

  template <typename... Types> void operator()(Types &...values) {
    ((*this)(values), ...);
  }

  const char *consume(size_t count) {
    const char *result = reader.buffer.data();
    reader.advance(count);
    return result;
  }

  template <typename Type> Type read() {
    if constexpr (has_serialize<Type>) {
      Type result;
      result.serialize(*this);
      return result;
    } else if constexpr (has_builtin_read<Type>) {
      return reader.read<Type>();
    } else {
      Type result;
      serialize(*this, result);
      return result;
    }
  }
};

template <typename Output, typename... Types>
void serialize_to(Output &output, const Types &...values) {
  static_assert(sizeof(*output.data()) == sizeof(std::byte));
  SerializeContext<SizeOperation> context{};
  (context(values), ...);
  size_t size = context.current - (std::byte *)nullptr;
  output.resize(size);
  std::byte *destination = (std::byte *)output.data();
  SerializeContext<WriteOperation> context2{destination, destination};
  (context2(values), ...);
}

template <typename... Types>
[[gnu::warn_unused_result]] BufferHandle
serialize_to_buffer(const Types &...values) {
  SerializeContext<SizeOperation> context{};
  (context(values), ...);
  size_t size = context.current - (std::byte *)nullptr;
  BufferHandle handle = makeBuffer(size);
  std::byte *destination = handle->get_data();
  SerializeContext<WriteOperation> context2{destination, destination};
  (context2(values), ...);
  return handle;
}

template <typename... Types>
void serialize_to_string_view(std::string_view buffer, const Types &...values) {
  SerializeContext<SizeOperation> context{};
  (context(values), ...);
  size_t size = context.current - (std::byte *)nullptr;
  if (buffer.size() < size) {
    throw DataFormatError("Data does not fit in target buffer");
  }
  std::byte *destination = (std::byte *)buffer.data();
  SerializeContext<WriteOperation> context2{destination, destination};
  (context2(values), ...);
}

template <typename... Types>
size_t serialize_to_unchecked(void *pointer, const Types &...values) {
  std::byte *destination = (std::byte *)pointer;
  SerializeContext<WriteOperation> context{destination, destination};
  (context(values), ...);
  return context.current - (std::byte *)pointer;
}

template <typename... Types>
std::string_view deserialize_buffer_part(const void *pointer, size_t length,
                                         Types &...results) {
  DataReader reader(std::string_view{(const char *)pointer, length});
  DeserializeContext context(reader);
  context(results...);
  return reader.buffer;
}

template <typename... Types>
std::string_view deserialize_buffer_part(std::string_view data,
                                         Types &...results) {
  return deserialize_buffer_part(data.data(), data.size(), results...);
}

template <typename... Types>
void deserialize_buffer(const void *pointer, size_t length, Types &...results) {
  DataReader reader(std::string_view{(const char *)pointer, length});
  DeserializeContext context(reader);
  context(results...);
  if (reader.buffer.size() != 0) {
    throw DataFormatError(
        "deserialize_buffer: " + std::to_string(reader.buffer.size()) +
        " trailing bytes");
  }
}
template <typename... Types>
void deserialize_buffer(std::string_view data, Types &...results) {
  deserialize_buffer(data.data(), data.size(), results...);
}
template <typename... Types>
void deserialize_buffer(Buffer *buffer, Types &...results) {
  DataReader reader(
      std::string_view{(const char *)buffer->get_data(), buffer->get_size()});
  DeserializeContext context(reader);
  context(results...);
  if (reader.buffer.size() != 0) {
    throw DataFormatError(
        "deserialize_buffer: " + std::to_string(reader.buffer.size()) +
        " trailing bytes");
  }
}

template <typename... Types>
std::string_view deserialize_buffer_part(Buffer *buffer, Types &...results) {
  DataReader reader(
      std::string_view{(const char *)buffer->get_data(), buffer->get_size()});
  DeserializeContext context(reader);
  context(results...);
  return reader.buffer;
}

template <typename Type> struct SerializeFunction {
  const Type &function;
  SerializeFunction(const Type &function) : function(function) {}
  template <typename Context> void serialize(Context &context) {
    function(context);
  }
};

using SerializationError = DataFormatError;
using Serializer = DataWriter;
using Deserializer = DataReader;
using Serialize = SerializeContext<WriteOperation>;
using Deserialize = DeserializeContext;
template <typename Output, typename... Types>
void serializeTo(Output &output, const Types &...values) {
  serialize_to(output, values...);
}
template <typename... Types>
BufferHandle serializeToBuffer(const Types &...values) {
  return serialize_to_buffer(values...);
}
template <typename... Types>
void serializeToStringView(std::string_view buffer, const Types &...values) {
  serialize_to_string_view(buffer, values...);
}
template <typename... Types>
size_t serializeToUnchecked(void *pointer, const Types &...values) {
  return serialize_to_unchecked(pointer, values...);
}
template <typename... Types>
std::string_view deserializeBufferPart(const void *pointer, size_t length,
                                       Types &...results) {
  return deserialize_buffer_part(pointer, length, results...);
}
template <typename... Types>
std::string_view deserializeBufferPart(std::string_view data,
                                       Types &...results) {
  return deserialize_buffer_part(data, results...);
}
template <typename... Types>
void deserializeBuffer(const void *pointer, size_t length, Types &...results) {
  deserialize_buffer(pointer, length, results...);
}
template <typename... Types>
void deserializeBuffer(std::string_view data, Types &...results) {
  deserialize_buffer(data, results...);
}
template <typename... Types>
void deserializeBuffer(Buffer *buffer, Types &...results) {
  deserialize_buffer(buffer, results...);
}
template <typename... Types>
std::string_view deserializeBufferPart(Buffer *buffer, Types &...results) {
  return deserialize_buffer_part(buffer, results...);
}

} // namespace turbokit
