#include <iostream>
#include <memory>

class Window {
public:
    virtual ~Window() = default;
    virtual void draw() const = 0;
};

class Button {
public:
    virtual ~Button() = default;
    virtual void draw() const = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile() const = 0;
};

class OpenGLWindow : public Window {
public:
    void draw() const override { std::cout << "OpenGL: dibujando ventana con el pipeline OpenGL\n"; }
};

class OpenGLButton : public Button {
public:
    void draw() const override { std::cout << "OpenGL: dibujando boton mediante OpenGL\n"; }
};

class OpenGLShader : public Shader {
public:
    void compile() const override { std::cout << "OpenGL: compilando shader GLSL\n"; }
};

class VulkanWindow : public Window {
public:
    void draw() const override { std::cout << "Vulkan: dibujando ventana con comandos Vulkan\n"; }
};

class VulkanButton : public Button {
public:
    void draw() const override { std::cout << "Vulkan: dibujando boton mediante Vulkan\n"; }
};

class VulkanShader : public Shader {
public:
    void compile() const override { std::cout << "Vulkan: compilando shader para SPIR-V\n"; }
};

class RenderFactory {
public:
    virtual ~RenderFactory() = default;
    virtual std::unique_ptr<Window> createWindow() const = 0;
    virtual std::unique_ptr<Button> createButton() const = 0;
    virtual std::unique_ptr<Shader> createShader() const = 0;
};

class OpenGLFactory : public RenderFactory {
public:
    std::unique_ptr<Window> createWindow() const override { return std::make_unique<OpenGLWindow>(); }
    std::unique_ptr<Button> createButton() const override { return std::make_unique<OpenGLButton>(); }
    std::unique_ptr<Shader> createShader() const override { return std::make_unique<OpenGLShader>(); }
};

class VulkanFactory : public RenderFactory {
public:
    std::unique_ptr<Window> createWindow() const override { return std::make_unique<VulkanWindow>(); }
    std::unique_ptr<Button> createButton() const override { return std::make_unique<VulkanButton>(); }
    std::unique_ptr<Shader> createShader() const override { return std::make_unique<VulkanShader>(); }
};

void renderScene(const RenderFactory& factory) {
    auto window = factory.createWindow();
    auto button = factory.createButton();
    auto shader = factory.createShader();
    window->draw();
    button->draw();
    shader->compile();
}

int main() {
    std::unique_ptr<RenderFactory> factory = std::make_unique<OpenGLFactory>();
    std::cout << "Primera plataforma:\n";
    renderScene(*factory);
    factory = std::make_unique<VulkanFactory>();
    std::cout << "\nSegunda plataforma:\n";
    renderScene(*factory);
    return 0;
}
