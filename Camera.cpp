#include "Camera.hpp"

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition); // Calculate front direction
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection)); // Right vector
        this->cameraUpDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraRightDirection)); // Up vector
    }

    // Return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    // Update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(cameraFrontDirection.x, 0.0f, cameraFrontDirection.z));

        if (direction == gps::MOVE_FORWARD) {
            cameraPosition += speed * horizontalFront;
        }
        if (direction == gps::MOVE_BACKWARD) {
            cameraPosition -= speed * horizontalFront;
        }
        if (direction == gps::MOVE_LEFT) {
            cameraPosition -= speed * cameraRightDirection;
        }
        if (direction == gps::MOVE_RIGHT) {
            cameraPosition += speed * cameraRightDirection;
        }
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }


    // Update the camera internal parameters following a camera rotate event
    // yaw - camera rotation around the Y-axis
    // pitch - camera rotation around the X-axis
    void Camera::rotate(float pitch, float yaw) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // Recalculate the front direction
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);

        // Recalculate right and up vectors
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f))); // Global up is used
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
    void Camera::setCameraPosition(glm::vec3 newPosition) {
        this->cameraPosition = newPosition;
        // Optional: If you want to maintain the camera's orientation
        // (e.g., front direction stays consistent), you don't need
        // to recalculate the directions here. If needed:
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraFrontDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraRightDirection));
    }

    // Setter for camera target
    void Camera::setCameraTarget(const glm::vec3& target) {
        this->cameraTarget = target;
        // Recalculate the front direction
        this->cameraFrontDirection = glm::normalize(target - cameraPosition);

        // Define a global up vector
        glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // Recalculate the right and up vectors
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, globalUp));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }


}
