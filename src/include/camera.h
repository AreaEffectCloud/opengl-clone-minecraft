#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float YAW = -90.0f; // ヨー (左右の回転)
const float PITCH =  0.0f;  // ピッチ (上下の回転)
const float SPEED =  2.5f;  // 移動速度
const float SENSITIVITY =  0.1f;  // マウス感度
const float ZOOM =  45.0f; // 視野角 (FOV)

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

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
        glm::mat4 GetViewMatrix();

        // キー入力に基づいてカメラの位置を更新
        void ProcessKeyboard(Camera_Movement direction, float deltaTime);
        // マウス入力に基づいてオイラー角と方向ベクトルを更新
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    private:
        void updateCameraVectors();
    };

#endif