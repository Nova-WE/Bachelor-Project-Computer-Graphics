#if !defined(INTERVAL_H)
#define INTERVAL_H

class interval {
public:
    double min;
    double max;

    interval() : min(+infinity), max(-infinity) {
    }

    interval(double min, double max) : min(min), max(max) {
    }

    [[nodiscard]] double size() const {
        return max - min;
    }

    [[nodiscard]] bool contains(double t) const {
        return min <= t && t <= max;
    }

    [[nodiscard]] bool surrounds(double t) const {
        return min < t && t < max;
    }

    [[nodiscard]] double clamp(double t) const {
        if (t < min) return min;
        if (t > max) return max;
        return t;
    }

    static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif // INTERVAL_H
