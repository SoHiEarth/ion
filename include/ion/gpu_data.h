#pragma once
#include "exports.h"
#include <cstdint>
#include <string>
#include <vector>

enum class DataType : std::uint8_t { INT, UNSIGNED_INT, FLOAT };
struct ION_API AttributePointer {
  int size = 2;
  DataType type = DataType::FLOAT;
  bool normalized = false;
  size_t stride = 0;
  const void *pointer = 0;
};

struct ION_API DataDescriptor {
  std::vector<AttributePointer> pointers;
  bool element_enabled = false;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

struct ION_API GPUData {
private:
  const DataDescriptor descriptor;
  std::string id;

public:
  unsigned int vertex_attrib = 0, vertex_buffer = 0;
  bool element_enabled = false;
  unsigned int element_buffer = 0;
  const std::string &GetID() const { return id; }
  const DataDescriptor &GetDescriptor() const { return descriptor; }
  GPUData(const DataDescriptor &desc, std::string_view new_id)
      : descriptor(desc), id(new_id) {}
};