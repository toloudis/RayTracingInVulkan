#pragma once

#include "Utilities/Glm.hpp"

struct Gesture
{
    struct Input
    {
        // User Input
        static constexpr size_t kButtonsCount = 3;

        static double s_doubleClickTime;  //< Mouse double-click time in seconds.

        enum ButtonId
        {
            kButtonLeft   = 0,
            kButtonRight  = 1,
            kButtonMiddle = 2
        };
        enum Mods
        {
            kShift = 0x0001,
            kCtrl  = 0x0002,
            kAlt   = 0x0004,
            kSuper = 0x0008,
            kCtrlShift = kCtrl | kShift
        };
        enum Action
        {
            kNone = 0,
            kPress = 1,     //< The pointer button had been pressed during this update cycle
            kDrag = 2,      //< The pointer changed its position from where a button was pressed
            kRelease = 3    //< The pointer button got released
        };
        enum DragConstrain
        {
            kUnconstrained = 0,
            kHorizontal = 1,
            kVertical = 2
        };

        // Call to update the action state of a pointer button.
        void setButtonEvent(uint32_t buttonIndex, Action action, int mods,
            glm::vec2 position, double time);

        // Call to provide a new screen position of the pointer.
        void setPointerPosition(glm::vec2 position);

        void reset(int mbIndex)
        {
            assert(mbIndex < kButtonsCount);
            if (mbIndex >= kButtonsCount) return;

            reset(mbs[mbIndex]);
        }

        void reset()
        {
            for (int mbIndex = 0; mbIndex < kButtonsCount; ++mbIndex)
                reset(mbs[mbIndex]);
        }

        // Call this function at the end of a frame before polling new events.
        void consume()
        {
            // Any button release event that we didn't explicitly use must be consumed, and the button
            // state must be reset.
            for (int mbIndex = 0; mbIndex < kButtonsCount; ++mbIndex)
                if (mbs[mbIndex].action == kRelease) reset(mbs[mbIndex]);
        }

        struct Button
        {
            Action action = kNone;
            // A bitwise combination of Mods keys (such as alt, ctrl, shift, etc...)
            int modifier = 0;
            // The screen coordinates the button was initially pressed, zero if no action
            glm::vec2  pressedPosition = {};
            // The drag vector from pressedPosition, this is still valid if pressed is kRelease and
            // it can be used to differentiate between a click action to be execute on button release
            // and the end of a click-drag gesture.
            glm::vec2  drag = {};
            // 0: unconstrained, 1: horizontal, 2: vertical
            DragConstrain dragConstraint = kUnconstrained;
            bool doubleClick = false;
            // The last time the button was pressed. This is used to detect double-click events.
            double triggerTime = 0;
        };

        Button mbs[kButtonsCount];
        glm::vec2 cursorPos = glm::vec2(0);

        // Reset is typically executed by the command or tool that consumes the button release action.
        static void reset(Button& button)
        {
            button.action = kNone;
            button.modifier = 0;
            button.pressedPosition = glm::vec2(0);
            button.drag = glm::vec2(0);
            button.dragConstraint = kUnconstrained;
            button.doubleClick = false;
            // Do not reset triggerTime here: we need to retain its value to detect double-clicks
        }

    };
};