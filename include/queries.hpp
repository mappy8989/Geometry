#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <variant>

namespace geometry::queries {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

inline double DistFromPointToLine(const Point2D &point, const Line &line) {
    double dx = line.end.x - line.start.x;
    double dy = line.end.y - line.start.y;
    if (line.Length() == 0.0)  // A and B are the same point
        return std::hypot(point.x - line.start.x, point.y - line.start.y);

    // Project AP onto AB, computing parameterized position t
    double t = ((point.x - line.start.x) * dx + (point.y - line.start.y) * dy) / line.Length();
    t = std::max(0.0, std::min(1.0, t));  // Clamp to segment

    // Find closest point on segment
    double cx = line.start.x + t * dx;
    double cy = line.start.y + t * dy;

    // Distance from P to closest point
    return std::hypot(point.x - cx, point.y - cy);
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
    double operator()(const Shape &shape1, const Shape &shape2) {
        return std::visit(
            Multilambda{[](const auto &shape, const Point2D &point) { return DistanceToPoint(shape, point); },
                        [](const Line &lin1, const Line &lin2) {
                            double d1 = DistFromPointToLine(lin1.start, Line{lin2.start, lin2.end});
                            double d2 = DistFromPointToLine(lin1.end, Line{lin2.start, lin2.end});
                            double d3 = DistFromPointToLine(lin2.start, Line{lin1.start, lin1.end});
                            double d4 = DistFromPointToLine(lin2.end, Line{lin1.start, lin1.end});
                            return std::min({d1, d2, d3, d4});
                        },
                        [](const Circle &circ1, const Circle &circ2) {
                            double dx = circ1.center_p.x - circ2.center_p.x;
                            double dy = circ1.center_p.y - circ2.center_p.y;
                            double d = std::hypot(dx, dy);
                            double s = d - circ1.radius - circ2.radius;
                            // Если одна окружность вложена в другую, расстояние отрицательно
                            if (d < fabs(circ1.radius - circ2.radius)) {
                                s = fabs(circ1.radius - circ2.radius) - d;
                            }
                            return s;
                        },
                        [](const auto &fig1, const auto &fig2) {
                            throw std::logic_error("Unsupported figure");
                            return 0.0;
                        }},
            shape1, shape2);
    }
    /* ваш код здесь */
};
/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {
    auto sign = [](const Point2D &p1, const Point2D &p2, const Point2D &p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };

    auto createLines = [](const std::vector<Point2D> &points) -> std::vector<Line> {
        std::vector<Line> lines;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            lines.push_back(Line{points[i], points[i + 1]});
        }
        lines.push_back(Line{points[points.size() - 1], points[0]});
        return lines;
    };

    auto GetMinDistanceForLines = [&](const auto &figure) {
        auto points =
            std::views::zip(figure.Lines().x, figure.Lines().y) |
            std::views::transform([](const auto &&pair) { return Point2D{std::get<0>(pair), std::get<1>(pair)}; }) |
            std::ranges::to<std::vector>();

        auto lines = createLines(points);

        auto distances = std::views::transform(lines, [&](auto &&elem) { return DistFromPointToLine(point, elem); }) |
                         std::ranges::to<std::vector>();
        auto res = std::ranges::min_element(distances);
        return *res;
    };

    auto res =
        std::visit(Multilambda{[&](const Line &line) { return DistFromPointToLine(point, line); },
                               [&](const Triangle &triangle) {
                                   if (sign(point, triangle.a, triangle.b) == sign(point, triangle.b, triangle.c) &&
                                       sign(point, triangle.b, triangle.c) == sign(point, triangle.c, triangle.a)) {
                                       // point located inside triangle
                                       return 0.0;
                                   }
                                   return GetMinDistanceForLines(triangle);
                               },
                               [&](const Rectangle &rect) {
                                   if (point.x >= rect.bottom_left.x && point.x <= (rect.bottom_left.x + rect.width) &&
                                       point.y >= rect.bottom_left.y && point.y <= (rect.bottom_left.y + rect.height)) {
                                       // point located inside triangle
                                       return 0.0;
                                   }
                                   return GetMinDistanceForLines(rect);
                               },
                               [&](const Circle &circle) {
                                   double distance = circle.center_p.DistanceTo(point);
                                   if (distance < circle.radius) {
                                       return 0.0;
                                   }
                                   return distance;
                               },
                               [&](const RegularPolygon &regpoly) { return GetMinDistanceForLines(regpoly); },
                               [&](const Polygon &poly) { return GetMinDistanceForLines(poly); },

                               [](const auto &fig) {
                                   throw std::logic_error("Unsupported figure");
                                   return 0.0;
                               }

                   }  // namespace geometry::queries

                   ,
                   shape);
    /* ваш код здесь */
    return res;
}

/*
 * Класс для поиска расстояния от точки до фигуры
 *
 * Требуется организовать возможность нахождения расстояния для всех возможных фигур типа-суммы Shape
 */
struct PointToShapeDistanceVisitor {
    double operator()(const Point2D &p, const Shape &shape) { return DistanceToPoint(shape, p); }

    /* ваш код здесь */
};

inline BoundingBox GetBoundBox(const Shape &shape) {
    return std::visit([](const auto &s) { return s.BoundBox(); }, shape);
}

inline double GetHeight(const Shape &shape) {

    /* ваш код здесь */
    return std::visit([](const auto &s) { return s.Height(); }, shape);
}

inline std::optional<std::pair<Point2D, Point2D>> BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {
    BoundingBox Bbox1 = GetBoundBox(shape1);
    BoundingBox Bbox2 = GetBoundBox(shape2);

    double x_left = std::max(Bbox1.min_x, Bbox2.min_x);
    double y_bottom = std::max(Bbox1.min_y, Bbox2.min_y);
    double x_right = std::min(Bbox1.max_x, Bbox2.max_x);
    double y_top = std::min(Bbox1.max_y, Bbox2.max_y);
    /* ваш код здесь */
    if (x_left < x_right && y_bottom < y_top) {
        return {std::make_pair(Point2D{x_left, y_bottom}, Point2D{x_right, y_top})};
    }

    return std::nullopt;
}

std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {
    double distance = 0.0;
    try {
        distance = ShapeToShapeDistanceVisitor{}(shape1, shape2);
    } catch (...) {
        return std::nullopt;
    }

    return distance;
}

std::vector<std::optional<double>> GetDistancesBetweenShapes(DummyClass shapes) {
    std::vector<std::pair<int, Shape>> enum_shapes;
    enum_shapes.reserve(shapes.shapes_.size());
    std::ranges::transform(shapes.shapes_ | std::views::enumerate, std::back_inserter(enum_shapes), [](auto &&pair) {
        auto [idx, shape] = pair;
        return std::make_pair(idx, shape);
    });

    return std::ranges::views::cartesian_product(enum_shapes, enum_shapes) |
           std::ranges::views::filter([](auto &&pair) {
               auto &[x, y] = pair;
               return x.first < y.first;
           }) |
           std::views::transform([](auto &&tuple) {
               auto &[shape1, shape2] = tuple;
               return geometry::queries::DistanceBetweenShapes(shape1.second, shape2.second);
           }) |
           std::ranges::to<std::vector>();
}

}  // namespace geometry::queries