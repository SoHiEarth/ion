// Code block
#include <glad/glad.h>
// Code block
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  if ((argc - 1) % 2 != 0) {
    std::cout << "Warning: Number of arguments is not even" << std::endl;
  }
  std::vector<std::pair<std::string, std::string>> arguments;
  for (int i = 1; i < argc; i += 2) {
    if (argv[i] != nullptr) {
      if (i + 1 < argc) {
        if (argv[i + 1] != nullptr) {
          arguments.push_back({argv[i], argv[i + 1]});
        }
      }
    }
  }

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  auto window = glfwCreateWindow(800, 600, "ion", NULL, NULL);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glViewport(0, 0, 800, 600);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
