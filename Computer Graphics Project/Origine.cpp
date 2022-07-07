#pragma once

#include "MyProject.hpp"

namespace wow {

	// -------------------- start Player --------------------

struct Camera {
	void init(glm::vec3 angles, glm::vec3 position, float near, float far, float fov, float aspectRatio) {
		this->angles = angles;
		this->position = position;
		this->near = near;
		this->far = far;
		this->fov = fov;

		projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
		projection[1][1] *= -1;
	}

	void rotate(float dy, float dx, float dz) {
		camera = glm::mat3(glm::rotate(glm::mat4(1.0f), angles.y + dy, glm::vec3(0.0f, 1.0f, 0.0f))) *
			glm::mat3(glm::rotate(glm::mat4(1.0f), angles.x + dx, glm::vec3(1.0f, 0.0f, 0.0f))) *
			glm::mat3(glm::rotate(glm::mat4(1.0f), angles.z + dz, glm::vec3(0.0f, 0.0f, 1.0f)));
	}

	void move(glm::vec3 movement) {
		position += movement;
	}

	glm::mat4 getCameraMatrix() {
		return glm::translate(glm::transpose(glm::mat4(camera)), -position);
	}

	glm::mat4 getProjectionMatrix() {
		return projection;
	}

	glm::vec3 getViewDirection() {
		glm::vec3 view = glm::vec3(1.0, 0.0, 0.0);
	}

private:
	glm::vec3 angles;
	glm::vec3 position;
	float near;
	float far;
	float fov;
	glm::mat3 camera;
	glm::mat4 projection;
};

struct CollisionSphere {
	void init(glm::vec3 center, float radius) {
		this->center = center;
		this->radius = radius;
	}

	SphereCollisionEvent planeCollision(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		const glm::vec3 planeCenter = -((v1 + v2 + v3) / 3.0f);
		const glm::vec3 direction = planeCenter - center;

		// plane coeff: ax + by + cz + d = 0 | a, b, c obtained by the normal vector
		const glm::vec3 norm = glm::cross(v1, v2);
		const float d = glm::dot(planeCenter, norm);

		//       d + a*px + b*py + c*pz
		// t = - ----------------------
		//           dx + dy + dz
		const float distance = -((d + glm::dot(norm, center)) /
			           // ---------------------------------------------
			               (direction.x + direction.y + direction.z));

		if (distance <= radius)
			return { (center + direction * radius), (center + direction * distance) };

		return { glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) };
	}

	void translate(glm::vec3 direction) {
		this->center += direction;
	}

	static CollisionSphere create(glm::vec3 center, float radius) {
		CollisionSphere build{};
		build.center = center;
		build.radius = radius;
		return build;
	}

private:
	glm::vec3 center;
	float radius;

};

struct Player {
	std::vector<CollisionSphere> collisionBody;
	Model collisionMesh;
	Camera camera;

	const float movigVelocity;
	const float jumpingVelocity;
	const glm::vec3 gravity = glm::vec3(0.0, 1.0, 0.0);

	void init(float aspectRatio) {
		force = glm::vec3(0, 0, 0);
		velocity = glm::vec3(0, 0, 0);
		position = glm::vec3(-0.2f, 0.95f, 14.5f);

		collisionBody = {
			CollisionSphere::create(position, 0.5),
			CollisionSphere::create(position - glm::vec3(0, 0.25, 0), 0.5)
		};

		camera.init(glm::vec3(0, 0, 0), position, 0.1f, 100.0f, 70.0f, aspectRatio);
	}

	void forward() {
		velocity += movigVelocity * camera.getViewDirection;
	}

	void backward() {
		velocity -= movigVelocity * camera.getViewDirection;
	}

	void left() {
		velocity += movigVelocity * (glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0)) * camera.getViewDirection);
	}

	void right() {
		velocity -= movigVelocity * (glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0)) * camera.getViewDirection);
	}

	void jump() {
		velocity += jumpingVelocity * glm::vec3(0, 1, 0);
	}

	void moveHead(float dy, float dx, float dz) {
		camera.rotate(dy, dx, dz);
	}

	void update(float deltaTime) {
		force += mass * gravity;
		velocity += (force / mass) * deltaTime;

		auto movement = velocity * deltaTime;
		position += movement;
		camera.move(movement);

		force = glm::vec3(0, 0, 0);
	}

	void addVelocity(glm::vec3 vel) {
		velocity += vel;
	}

	void addAcceleration(glm::vec3 newforce) {
		force += newforce;
	}

	void teleport(glm::vec3 newPosition) {
		this->position = newPosition;
	}

	void move(glm::vec3 direction) {
		this->position += direction;
	}

	void handleCollision(SphereCollisionEvent collision) {
		glm::vec3 correction = collision.planeInsideSphere - collision.sphereInsidePlane;
		//glm::vec3 overflow = -correction;
		position += correction;

		//                  dot(u,v)
		// proj_v(u) = ------------------- * v -> the projection of u on v
		//               v.x*v.x + v.y*v.y     -> it should be the squared modulo of the vect

		// proj of velocity on correction
		velocity +=      (glm::dot(velocity, correction)) * correction /
		//  -----------------------------------------------------------------------------------
			(correction.x*correction.x + correction.y*correction.y + correction.z*correction.z);
	}

private:
	glm::vec3 force;
	glm::vec3 velocity;
	glm::vec3 position;
	const float mass = 1.0f;

};

struct SphereCollisionEvent {
	glm::vec3 sphereInsidePlane; // farest sphere point inside plane
	glm::vec3 planeInsideSphere; // farest plane point inside sphere
};

// -------------------- end Player --------------------

}