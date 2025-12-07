#pragma once
#include <memory>
#include <string>
#include <filesystem>

struct Texture {
  std::string id;
  std::filesystem::path path;
public:
  unsigned int texture = 0;
	Texture(std::filesystem::path new_path, std::string_view new_id) : path(new_path), id(new_id) {}
	std::filesystem::path GetPath() const { return path; }
	std::string GetID() const { return id; }
  void Use();
};