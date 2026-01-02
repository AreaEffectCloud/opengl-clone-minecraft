#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace util {
    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Position(position), 
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
        Up(glm::vec3(0.0f, 1.0f, 0.0f)), 
        Right(glm::vec3(1.0f, 0.0f, 0.0f)), 
        WorldUp(up),
        Yaw(yaw),
        Pitch(pitch),
        MovementSpeed(CAMERA_SPEED), 
        MouseSensitivity(CAMERA_SENSITIVITY), 
        Zoom(CAMERA_ZOOM)
    {
        updateCameraVectors();
    }
    
    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }
    
    void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime * 4.0f;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == TOP)
            Position += WorldUp * velocity;
        if (direction == BOTTOM)
            Position -= WorldUp * velocity;
    }
    
    void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw   += xoffset;
        Pitch += yoffset;
    
        // ピッチ（上下の回転）を制限し、カメラがひっくり返るのを防ぐ
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }
    
    void Camera::ProcessMouseScroll(float yoffset) {
        // Zoom は FOV（度）として扱う。適当な範囲にクランプする
        Zoom -= yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 130.0f) Zoom = 130.0f;
    }
    
    void Camera::updateCameraVectors() {
        glm::vec3 front;
        float yawRad = glm::radians(Yaw);
        float pitchRad = glm::radians(Pitch);
    
        front.x = std::cos(yawRad) * std::cos(pitchRad);
        front.y = std::sin(pitchRad);
        front.z = std::sin(yawRad) * std::cos(pitchRad);
        Front = glm::normalize(front);
        
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
} // namespace util