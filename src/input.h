#pragma once

enum button_type {
    BUTTON_KEY_SPACE,
    BUTTON_KEY_ESCAPE,
    BUTTON_KEY_SHIFT,

    BUTTON_KEY_W,
    BUTTON_KEY_A,
    BUTTON_KEY_S,
    BUTTON_KEY_D,

    BUTTON_KEY_LEFT,
    BUTTON_KEY_RIGHT,
    BUTTON_KEY_UP,
    BUTTON_KEY_DOWN,

    BUTTON_MOUSE_LEFT,
    BUTTON_MOUSE_RIGHT,

    BUTTON_TYPE_MAX_COUNT
};

// TODO: pack these?
struct button_state {
    b8 Pressed;
    b8 Released;
    b8 Down;
};

struct program_input {
    vec2i MousePosition;
    vec2i MouseDelta;
    button_state Buttons[BUTTON_TYPE_MAX_COUNT];

    f64 Frametime;
    u32 ClientWidth;
    u32 ClientHeight;
};

static inline void UpdateButton(button_type Type, program_input *NewInput, b8 IsUp) {
    NewInput->Buttons[Type].Pressed = !IsUp;
    NewInput->Buttons[Type].Down = !IsUp;
    NewInput->Buttons[Type].Released = IsUp;
}

static inline b32 ButtonPressed(program_input *Input, button_type Type) {
    return Input->Buttons[Type].Pressed;
}

static inline b32 ButtonDown(program_input *Input, button_type Type) {
    return Input->Buttons[Type].Down;
}

static inline b32 ButtonUp(program_input *Input, button_type Type) {
    return !Input->Buttons[Type].Down;
}

static inline b32 ButtonReleased(program_input *Input, button_type Type) {
    return Input->Buttons[Type].Released;
}
