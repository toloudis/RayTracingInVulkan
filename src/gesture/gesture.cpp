#include "gesture.h"

#include "Vulkan/Vulkan.hpp"

// Verify loose coupling between Gesture and GLFW
static_assert(Gesture::Input::kButtonLeft   == GLFW_MOUSE_BUTTON_LEFT  , "Verify pointer buttons mapping");
static_assert(Gesture::Input::kButtonRight  == GLFW_MOUSE_BUTTON_RIGHT , "Verify pointer buttons mapping");
static_assert(Gesture::Input::kButtonMiddle == GLFW_MOUSE_BUTTON_MIDDLE, "Verify pointer buttons mapping");

static_assert(Gesture::Input::kShift == GLFW_MOD_SHIFT  , "Verify pointer modifiers mapping");
static_assert(Gesture::Input::kCtrl  == GLFW_MOD_CONTROL, "Verify pointer modifiers mapping");
static_assert(Gesture::Input::kAlt   == GLFW_MOD_ALT    , "Verify pointer modifiers mapping");
static_assert(Gesture::Input::kSuper == GLFW_MOD_SUPER  , "Verify pointer modifiers mapping");

// During app initialization query the OS accessibility settings how the user configured the
// double-click duration. Developer: never hardcode this time to something that feels
// right to you.
double Gesture::Input::s_doubleClickTime = GetDoubleClickTime();

// Update the current action for one of the button of the pointer device
void Gesture::Input::setButtonEvent(uint32_t mbIndex, Action action, int mods, Vec2f position, double time)
{
    if (mbIndex >= kButtonsCount)
        return;

    Button& button = mbs[mbIndex];
    if (action == Input::kPress)
    {
        // If the the button is pressed and was previously in a neutral state, it could be a click or a
        // double-click.
        if (button.action == Gesture::Input::kNone)
        {
            // A double-click event is recorded if the new click follows a previous click of the
            // same button within some time interval.
            button.doubleClick = (time - button.triggerTime) < Input::s_doubleClickTime;
            button.triggerTime = time;

            // Record position of the pointer during the press event, we are going to use it to
            // determine drag operations
            button.pressedPosition = position;

            // The state of modifiers are recorded when buttons are initially pressed. The state
            // is retained for the duration of the click/drag. We do not update this state until the
            // next button press event.
            button.modifier = mods;
        }

        button.action = Gesture::Input::kPress;
    }
    else if (action == Input::kRelease)
    {
        // When button is released, record any drag distance
        if (button.action != Gesture::Input::kNone)
        {
            Vec2f origin = button.pressedPosition;
            button.drag = position - origin;
        }

        button.action = Gesture::Input::kRelease;
    }
}

void Gesture::Input::setPointerPosition(glm::vec2 position)
{
    cursorPos = position;

    // Update each button action. Each button holding a kPress event becomes a
    // kDrag event.
    for (int mbIndex=0; mbIndex<Input::kButtonsCount; ++mbIndex)
    {
        Button& button = mbs[mbIndex];
        if (button.action == Input::kNone ||
            button.action == Input::kRelease) continue;

        Vec2f origin = button.pressedPosition;
        Vec2f drag = position - origin;
        button.drag = drag;
        bool anyMotion = drag != Vec2f(0);

        if (button.action == Gesture::Input::kPress && anyMotion)
        {
            button.action = Gesture::Input::kDrag;

            // If we hold the shift modifier we record if the initial drag is mostly
            // horizontal or vertical. This information may be used by some tools.
            if (button.modifier & Gesture::Input::kShift)
            {
                button.dragConstraint = (abs(drag.x) > abs(drag.y) ?
                                         Gesture::Input::kHorizontal :
                                         Gesture::Input::kVertical);
            }
            else
            {
                button.dragConstraint = Gesture::Input::kUnconstrained;
            }
        }
    }
}