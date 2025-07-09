#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <variant>

namespace geometry::queries {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

/*
 * Класс для поиска расстояния от точки до фигуры
 *
 * Требуется организовать возможность нахождения расстояния для всех возможных фигур типа-суммы Shape
 */
struct PointToShapeDistanceVisitor {
    Point2D point;

    explicit PointToShapeDistanceVisitor(const Point2D &p) : point(p) {}

    /* ваш код здесь */
};

/*
 * Класс для поиска расстояния между двумя фигурами
 *
 * Требуется организовать возможность нахождения расстояния только для следующих комбинаций фигур:
 *    - Any    & Point
 *    - Line   & Line
 *    - Circle & Circle
 *
 * Для всех остальных требуется вернуть пустое значение
 */
struct ShapeToShapeDistanceVisitor {

    /* ваш код здесь */
};

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {
    auto dist = [](const Point2D &a, const Point2D &b) {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    };

    auto res = std::visit(Multilambda{[](const Line &line) { return 0.0; },
                                      [](const auto &fig) {
                                          throw std::runtime_error("Unrecognised figure");
                                          return 0.0;
                                      }

                          }

                          ,
                          shape);
    /* ваш код здесь */
    return res;
}

inline BoundingBox GetBoundBox(const Shape &shape) {
    return std::visit([](const auto &s) { return s.BoundBox(); }, shape);
}

inline double GetHeight(const Shape &shape) {

    /* ваш код здесь */
    return std::visit([](const auto &s) { return s.Height(); }, shape);
}

inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {

    /* ваш код здесь */
    return false;
}

std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {

    /* ваш код с ShapeToShapeDistanceVisitor здесь*/
    return std::nullopt;
}

}  // namespace geometry::queries