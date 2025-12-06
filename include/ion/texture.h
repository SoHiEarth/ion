#pragma once
#include <memory>
#include <string>
#include <filesystem>

struct Texture {
  std::string id;
public:
  unsigned int texture = 0;
	Texture(std::string_view new_id) : id(new_id) {}
	std::string GetID() const { return id; }
  void Use();
};