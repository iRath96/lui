#pragma once

struct linspace {
    struct iterator {
        iterator(int i, const linspace &owner)
            : i(i), owner(owner) {}

        float operator*() const {
            return owner.min + (owner.max - owner.min) * float(i) / (owner.n - 1);
        }

        iterator &operator++() {
            i++;
            return *this;
        }

        bool operator!=(const iterator &other) const {
            return i != other.i || &owner != &other.owner;
        }

    private:
        int i;
        const linspace &owner;
    };

    linspace(float min, float max, int n)
        : min(min), max(max), n(n) {}

    iterator begin() { return iterator(0, *this); }

    iterator end() { return iterator(n, *this); }

private:
    float min, max;
    int n;
};
