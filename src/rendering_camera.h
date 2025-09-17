#pragma once

struct camera {
    b32 Orthographic;
    f32 FOV; // in radians
    f32 Near;
    f32 Far;
    f32 Aspect;
    vec3 Position;
    vec3 Target;
    vec3 Up;
};

// can add other stuff like camera motions
// or maybe different kinds of cameras etc
