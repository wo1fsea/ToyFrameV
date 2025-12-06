#include "ToyFrameV/InputSystem.h"
#include "ToyFrameV/WindowSystem.h"
#include "ToyFrameV/App.h"

namespace ToyFrameV {

std::vector<const std::type_info*> InputSystem::GetDependencies() const {
    // InputSystem depends on WindowSystem for receiving events
    return { &typeid(WindowSystem) };
}

bool InputSystem::Initialize(App* app) {
    System::Initialize(app);
    
    // Reset all input state
    Input::_UpdatePreviousState();
    Input::_ResetScrollDelta();
    
    return true;
}

void InputSystem::PreUpdate() {
    // Save current state as previous state
    // This must happen before WindowSystem processes new events
    Input::_UpdatePreviousState();
}

void InputSystem::PostUpdate() {
    // Reset per-frame delta values
    Input::_ResetScrollDelta();
}

void InputSystem::Shutdown() {
    // Nothing to clean up
}

} // namespace ToyFrameV
