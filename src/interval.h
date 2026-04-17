#if !defined(INTERVAL_H)
#define INTERVAL_H

#include "rtweekend.h"

class interval {
    public:
        double min;
        double max;

        interval() : min(+infinity), max(-infinity) {}
        interval(double min, double max) : min(min), max(max) {}

        double size() {
            return max - min;
        }

        bool contains(double t) {
            return min <= t && t <= max;
        }

        bool surrounds(double t) {
            return min < t && t < max;
        }

        double clamp(double t) const {
            if (t < min) return min;
            if (t > max) return max;
            return t;
        }

        static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif // INTERVAL_H
