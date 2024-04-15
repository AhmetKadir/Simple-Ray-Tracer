#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

class Ray {
    public:
        Ray() {}

        Ray(Vector3 origin, Vector3 direction) {
            this->origin = origin;
            this->direction = direction;
        }

        const Vector3& getOrigin() const {
            return origin;
        }

        const Vector3& getDirection() const {
            return direction;
        }

        Vector3 at(double t) {
            return origin + t * direction;
        }

    private:
        Vector3 origin;
        Vector3 direction;
};

#endif // RAY_H