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
    std::optional<std::vector<Point2D>> GetIntersections(const Shape &figure1, const Shape &figure2) {
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
                    return std::nullopt;
                }},
            figure1, figure2);

        return std::nullopt;
    }

private:
    std::optional<std::vector<Point2D>> IsLineAndCircleIntersect(geometry::Line &line, geometry::Circle &circle) {
        Point2D d = line.end - line.start;
        Point2D f = line.start - circle.Center();

        double a = d.Dot(d);
        double b = 2 * f.Dot(d);
        double c = f.Dot(f) - circle.radius * circle.radius;

        double discriminant = b * b - 4 * a * c;
        if (discriminant < 0)
            return std::nullopt;  // нет пересечений

        double sqrtD = std::sqrt(discriminant);
        std::vector<Point2D> points;
        for (int sign : {-1, 1}) {
            double t = (-b + sign * sqrtD) / (2 * a);
            if (0.0 <= t && t <= 1.0)
                points.emplace_back(line.start + d * t);
        }
        if (!points.empty())
            return points;
        return std::nullopt;
    }

    std::optional<std::vector<Point2D>> IsCirclesIntersect(geometry::Circle &c1, geometry::Circle &c2) {
        Point2D d = c2.center_p - c1.center_p;
        double dist = d.Length();

        if (dist > c1.radius + c2.radius || dist < std::abs(c1.radius - c2.radius))
            return std::nullopt;  // нет пересечений

        double a = (c1.radius * c1.radius - c2.radius * c2.radius + dist * dist) / (2 * dist);
        double h = std::sqrt(std::max(0.0, c1.radius * c1.radius - a * a));
        Point2D P = c1.center_p + d * (a / dist);

        if (h < 1e-12)  // одно касание
            return std::vector<Point2D>{P};
        // две точки
        double rx = -d.y * (h / dist);
        double ry = d.x * (h / dist);
        return std::vector<Point2D>{{P.x + rx, P.y + ry}, {P.x - rx, P.y - ry}};
    }

    std::optional<std::vector<Point2D>> IsLinesIntersect(geometry::Line &l1, geometry::Line &l2) {
        double x1 = l1.start.x, y1 = l1.start.y, x2 = l1.end.x, y2 = l1.end.y;
        double x3 = l2.start.x, y3 = l2.start.y, x4 = l2.end.x, y4 = l2.end.y;

        double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (std::abs(denom) < 1e-12)
            return std::nullopt;  // Параллельно или совпадает

        double px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
        double py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

        // Проверяем, принадлежит ли точка обоим отрезкам
        auto between = [](double a, double b, double c) { return (a <= b && b <= c) || (c <= b && b <= a); };
        if (between(x1, px, x2) && between(y1, py, y2) && between(x3, px, x4) && between(y3, py, y4))
            return std::vector<Point2D>{{px, py}};
        return std::nullopt;
    }
};

inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) { return std::nullopt; }

}  // namespace geometry::intersections