#include <iostream>
#include <string>
#include <memory>

// Abstract Product A
class Button {
public:
    virtual ~Button() = default;
    virtual std::string render() const = 0;
};

// Concrete Product A1
class WinButton : public Button {
public:
    std::string render() const override {
        return "Rendering a Windows-style button.";
    }
};

// Concrete Product A2
class MacButton : public Button {
public:
    std::string render() const override {
        return "Rendering a macOS-style button.";
    }
};

// Abstract Product B
class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual std::string render() const = 0;
};

// Concrete Product B1
class WinCheckbox : public Checkbox {
public:
    std::string render() const override {
        return "Rendering a Windows-style checkbox.";
    }
};

// Concrete Product B2
class MacCheckbox : public Checkbox {
public:
    std::string render() const override {
        return "Rendering a macOS-style checkbox.";
    }
};

// Abstract Factory
class GUIFactory {
public:
    virtual ~GUIFactory() = default;
    virtual std::unique_ptr<Button> createButton() const = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() const = 0;
};

// Concrete Factory 1
class WinFactory : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<WinButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<WinCheckbox>();
    }
};

// Concrete Factory 2
class MacFactory : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<MacButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<MacCheckbox>();
    }
};

// Client code
class Application {
private:
    std::unique_ptr<Button> button;
    std::unique_ptr<Checkbox> checkbox;

public:
    Application(std::unique_ptr<GUIFactory> factory) {
        button = factory->createButton();
        checkbox = factory->createCheckbox();
    }

    void paint() const {
        std::cout << "Painting UI elements:\n";
        std::cout << "- Button: " << button->render() << "\n";
        std::cout << "- Checkbox: " << checkbox->render() << "\n";
    }
};

// Function to decide which factory to use
std::unique_ptr<GUIFactory> createOsSpecificFactory() {
    // In a real application, you would determine the OS here
    std::string os = "mac"; // Simulate macOS

    if (os == "windows") {
        return std::make_unique<WinFactory>();
    } else if (os == "mac") {
        return std::make_unique<MacFactory>();
    } else {
        throw std::runtime_error("Unsupported operating system.");
    }
}

int main() {
    try {
        std::unique_ptr<GUIFactory> factory = createOsSpecificFactory();
        Application app(std::move(factory));
        app.paint();
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
