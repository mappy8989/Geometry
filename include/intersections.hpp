#pragma once
#include "geometry.hpp"
#include "queries.hpp"
#include <cmath>
#include <optional>
#include <variant>

namespace geometry::intersections {

/*
 * Класс для поиска пересечений между двумя фигурами
 *
 * Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
 *    - Line   & Line
 *    - Line   & Circle
 *    - Circle & Circle
 *
 * Для всех остальных требуется выбросить исключение std::logic_error
 */
class IntersectionVisitor {
public:
    auto GetIntersections(const Shape &figure1, const Shape &figure2) {
        std::visit(
            geometry::queries::Multilambda{
                [&](geometry::Line &line1, geometry::Line &line2) { return IsLinesIntersect(line1, line2); },
                [&](geometry::Line &line, geometry::Circle &circle) { return IsLineAndCircleIntersect(line, circle); },
                [&](geometry::Circle &circle1, geometry::Circle &circle2) {
                    return IsCirclesIntersect(circle1, circle2);
                },
                [](auto &, auto &) {
                    // Fallback for unsupported types (should not be needed here, but good for
                    // extensibility)
                    throw std::logic_error("Unsupported intersection types");
                    return false;
                }},
            figure1, figure2);

        return false;
    }

private:
    bool IsLineAndCircleIntersect(geometry::Line &line, geometry::Circle &circle) {
        double dx = line.end.x - line.start.x;
        double dy = line.end.y - line.start.y;
        double fx = circle.Center().x - line.start.x;
        double fy = circle.Center().y - line.start.y;
        double len_sq = dx * dx + dy * dy;
        double t = (fx * dx + fy * dy) / len_sq;

        // Ограничиваем t от 0 до 1, чтобы получить ближайшую точку на отрезке
        t = std::max(0.0, std::min(1.0, t));

        double closest_x = line.start.x + t * dx;
        double closest_y = line.start.y + t * dy;

        double dist_sq = (circle.Center().x - closest_x) * (circle.Center().x - closest_x) +
                         (circle.Center().y - closest_y) * (circle.Center().y - closest_y);

        return dist_sq <= circle.radius * circle.radius;
    }

    bool IsCirclesIntersect(geometry::Circle &circle1, geometry::Circle &circle2) {
        double dx = circle1.Center().x - circle2.Center().x;
        double dy = circle1.Center().y - circle2.Center().y;
        double d = std::sqrt(dx * dx + dy * dy);

        // circles are equal
        if (d == 0 && circle1.radius == circle2.radius)
            return true;

        // Нет пересечения: одна внутри другой или слишком далеко
        if (d > circle1.radius + circle2.radius || d < std::abs(circle1.radius - circle2.radius))
            return false;

        // В остальных случаях — есть хотя бы одна общая точка
        return true;
    }

    bool IsLinesIntersect(geometry::Line &line1, geometry::Line &line2) {
        auto ccw = [](double x1, double y1, double x2, double y2, double x3, double y3) {
            return (y3 - y1) * (x2 - x1) > (y2 - y1) * (x3 - x1);
        };

        return (ccw(line1.start.x, line1.start.y, line2.start.x, line2.start.y, line2.end.x, line2.end.y) !=
                ccw(line1.end.x, line1.end.y, line2.start.x, line2.start.y, line2.end.x, line2.end.y)) &&
               (ccw(line1.start.x, line1.start.y, line1.end.x, line1.end.y, line2.start.x, line2.start.y) !=
                ccw(line1.start.x, line1.start.y, line1.end.x, line1.end.y, line2.end.x, line2.end.y));
    }
};

inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) { return std::nullopt; }

}  // namespace geometry::intersections