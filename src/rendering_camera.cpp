#pragma once

static inline mat3 GetWorldToCameraRotation(camera *Camera) {
    vec3 Right = Cross(Camera->Target, Camera->Up);
    mat3 Rotation = {
         Right.x, Camera->Up.x, Camera->Target.x,
         Right.y, Camera->Up.y, Camera->Target.y,
         Right.z, Camera->Up.z, Camera->Target.z
    };
    return Rotation;
}

static inline mat3 GetCameraToWorldRotation(camera *Camera) {
    vec3 Right = Cross(Camera->Target, Camera->Up);
    mat3 Rotation = {
        Right.x, Right.y, Right.z, 
        Camera->Up.x, Camera->Up.y, Camera->Up.z, 
        Camera->Target.x, Camera->Target.y, Camera->Target.z
    };
    return Rotation;
}

static mat4 CalculateCameraView(camera *Camera) {
    vec3 P = Camera->Position;
    vec3 T = Camera->Target;
    vec3 U = Camera->Up;
    vec3 R = Cross(T, U);

    f32 x = R.x*P.x + U.x*P.y + T.x*P.z;
    f32 y = R.y*P.x + U.y*P.y + T.y*P.z;
    f32 z = R.z*P.x + U.z*P.y + T.z*P.z;
    mat4 View = {
        R.x, U.x, T.x, -x,
        R.y, U.y, T.y, -y,
        R.z, U.z, T.z, -z,
        0.f, 0.f, 0.f, 1.f
    };
    return View;
}

static inline vec3 WorldToCamera(camera *Camera, vec3 P) {
    mat3 Rotation = GetWorldToCameraRotation(Camera);
    return Rotation*(P - Camera->Position);
}

static inline vec3 CameraToWorld(camera *Camera, vec3 P) {
    mat3 InverseRotation = GetCameraToWorldRotation(Camera);
    return InverseRotation*P + Camera->Position;
}

static mat4 CalculateCameraProjectionView(camera *Camera, f32 Aspect) {
    mat4 Projection = Mat4Identity;
    if (Camera->Orthographic) {
    }
    else {
        // half the height of the near plane
        f32 h = Camera->Near*tanf(Camera->FOV/2.f);
        f32 w = Aspect*h;

        f32 a = -1.f/w;
        f32 b = -1.f/h;
        f32 c = 2.f/(Camera->Near - Camera->Far);
        f32 d = (Camera->Near + Camera->Far)/(Camera->Near - Camera->Far);
        f32 e = 1.f; // This should be taking care of the z-flip
        Projection = {
              a, 0.f, 0.f, 0.f,
            0.f,   b, 0.f, 0.f,
            0.f, 0.f,   c,   d,
            0.f, 0.f,   e, 0.f
        };
    }
    mat4 View = CalculateCameraView(Camera);
    return Projection*View;
}

static inline void LookAt(camera *Camera, vec3 Target, vec3 WorldUp) {
    // TODO: check against if Camera->Position = Target
    // check against Camera->Target is parallel to WorldUp
    Camera->Target = Normalized(Target - Camera->Position);
    vec3 Right = Cross(Camera->Target, WorldUp);
    Camera->Up = Normalized(Cross(Right, Camera->Target));
}

// Camera Target and Up are in World space, so V must be in World Space.
static inline void RotateCamera(camera *Camera, f32 Amount, vec3 V) {
    quaternion Q = quaternion(Amount, V);
    Camera->Target = Normalized(Rotate(Camera->Target, Q));
    Camera->Up = Normalized(Rotate(Camera->Up, Q));
}
