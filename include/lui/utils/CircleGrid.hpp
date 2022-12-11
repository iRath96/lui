#pragma once

#include <lore/math.h>

struct CircleGrid {
    struct iterator {
        iterator(const CircleGrid &cg, bool) : done(true), cg(cg) {}
        iterator(const CircleGrid &cg)
            : done(false), x(-1), y(0), cg(cg) {
            advance();
        }

        const lore::Vector2<float> &operator*() const { return value; }
        iterator &operator++() {
            advance();
            return *this;
        }

        bool operator!=(const iterator &other) const {
            return done != other.done;
        }

    private:
        lore::Vector2<float> value;

        bool done;
        int x, y;
        const CircleGrid &cg;

        void advance() {
            while (!done) {
                if (++x >= cg.width) {
                    x = 0;
                    if (++y >= cg.height) {
                        done = true;
                        y = 0;
                    }
                }

                value.x() = 2 * float(x) / float(cg.width - 1) - 1;
                value.y() = 2 * float(y) / float(cg.height - 1) - 1;

                value /= 1.002f; // simulate what OSLO does (???)
                if (value.lengthSquared() <= 1) {
                    break;
                }
            }
        }
    };

    CircleGrid(int width, int height)
        : width(width), height(height) {}

    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(*this, true); }

private:
    int width, height;
};
