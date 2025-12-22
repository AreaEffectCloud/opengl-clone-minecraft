#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace util {
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };

    constexpr float CAMERA_YAW         = -90.0f;
    constexpr float CAMERA_PITCH       =  0.0f;
    constexpr float CAMERA_SPEED       =  2.5f;
    constexpr float CAMERA_SENSITIVITY =  0.1f;
    constexpr float CAMERA_ZOOM        =  45.0f;
    
    class Camera {
        public:
            glm::vec3 Position;
            glm::vec3 Front;
            glm::vec3 Up;
            glm::vec3 Right;
            glm::vec3 WorldUp;
    
            float Yaw;
            float Pitch;
            float MovementSpeed;
            float MouseSensitivity;
            float Zoom;
    
            Camera(
                glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                float yaw = CAMERA_YAW,
                float Pitch = CAMERA_PITCH);
            
            glm::mat4 GetViewMatrix() const;
    
            // キー入力に基づいてカメラの位置を更新
            void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    
            // マウス入力に基づいてオイラー角と方向ベクトルを更新
            // xoffset, yoffset は生のマウス移動量（通常 glfw のコールバックの値）を渡す
            void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    
            // マウスホイール（スクロール）でズーム（FOV）を変更
            void ProcessMouseScroll(float yoffset);
    
            void updateCameraVectors();
        private:
            // Front/Right/Up の更新
    };
} // namespace util
