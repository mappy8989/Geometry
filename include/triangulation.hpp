#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <vector>

namespace geometry::triangulation {

struct DelaunayTriangle {
    Point2D a, b, c;

    DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

    bool ContainsPoint(const Point2D &p) const {
        Point2D center = Circumcenter();
        double radius = Circumradius();
        return center.DistanceTo(p) <= radius + 1e-10;
    }

    Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < 1e-10) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    double Circumradius() const {
        Point2D center = Circumcenter();
        return center.DistanceTo(a);
    }

    bool SharesEdge(const DelaunayTriangle &other) const {
        std::vector<Point2D> this_points = {a, b, c};
        std::vector<Point2D> other_points = {other.a, other.b, other.c};

        int shared_count = 0;
        for (const Point2D &p1 : this_points) {
            for (const Point2D &p2 : other_points) {
                if (std::abs(p1.x - p2.x) < 1e-10 && std::abs(p1.y - p2.y) < 1e-10) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge &other) const {
        if (std::abs(p1.x - other.p1.x) > 1e-10)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > 1e-10)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > 1e-10)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    bool operator==(const Edge &other) const {
        return std::abs(p1.x - other.p1.x) < 1e-10 && std::abs(p1.y - other.p1.y) < 1e-10 &&
               std::abs(p2.x - other.p2.x) < 1e-10 && std::abs(p2.y - other.p2.y) < 1e-10;
    }
};

inline bool operator==(const DelaunayTriangle &lhs, const DelaunayTriangle &rhs) {
    std::vector<Point2D> lhs_vertices = {lhs.a, lhs.b, lhs.c};
    std::vector<Point2D> rhs_vertices = {rhs.a, rhs.b, rhs.c};

    auto point_equal = [](const Point2D &p1, const Point2D &p2) {
        return std::abs(p1.x - p2.x) < 1e-10 && std::abs(p1.y - p2.y) < 1e-10;
    };

    // Проверяем, что каждое из вершин lhs есть в rhs
    for (const auto &p : lhs_vertices) {
        bool found = std::any_of(rhs_vertices.begin(), rhs_vertices.end(),
                                 [&](const Point2D &rp) { return point_equal(p, rp); });
        if (!found)
            return false;
    }
    return true;
}

inline GeometryResult<std::vector<DelaunayTriangle>> DelaunayTriangulation(std::span<const Point2D> points) {
    if (points.size() < 3) {
        return std::unexpected(GeometryError::InvalidInput);
    }

    // Находим ограничивающий прямоугольник для всех точек
    double minX = points[0].x, minY = points[0].y;
    double maxX = points[0].x, maxY = points[0].y;
    for (const auto &p : points) {
        if (p.x < minX)
            minX = p.x;
        if (p.y < minY)
            minY = p.y;
        if (p.x > maxX)
            maxX = p.x;
        if (p.y > maxY)
            maxY = p.y;
    }

    double dx = maxX - minX;
    double dy = maxY - minY;
    double deltaMax = std::max(dx, dy);
    double midX = (minX + maxX) / 2;
    double midY = (minY + maxY) / 2;

    // Создаём супер-треугольник, который гарантированно содержит все точки
    Point2D super1(midX - 20 * deltaMax, midY - deltaMax);
    Point2D super2(midX, midY + 20 * deltaMax);
    Point2D super3(midX + 20 * deltaMax, midY - deltaMax);

    std::vector<DelaunayTriangle> triangulation;
    triangulation.emplace_back(super1, super2, super3);

    // Основной цикл по всем точкам
    for (const auto &point : points) {
        std::vector<DelaunayTriangle> badTriangles;
        std::vector<Edge> polygon;

        // Находим все треугольники, в окружность которых попадает новая точка
        for (const auto &triangle : triangulation) {
            if (triangle.ContainsPoint(point)) {
                badTriangles.push_back(triangle);
            }
        }

        // Формируем границу дырки (polygonal hole)
        for (const auto &triangle : badTriangles) {
            std::vector<Point2D> verts = triangle.vertices();
            for (int i = 0; i < 3; ++i) {
                Edge edge(verts[i], verts[(i + 1) % 3]);
                bool isShared = false;
                for (const auto &otherTriangle : badTriangles) {
                    if (&triangle == &otherTriangle)
                        continue;
                    std::vector<Point2D> otherVerts = otherTriangle.vertices();
                    for (int j = 0; j < 3; ++j) {
                        Edge otherEdge(otherVerts[j], otherVerts[(j + 1) % 3]);
                        if (edge == otherEdge) {
                            isShared = true;
                            break;
                        }
                    }
                    if (isShared)
                        break;
                }
                if (!isShared) {
                    polygon.push_back(edge);
                }
            }
        }

        // Удаляем плохие треугольники из триангуляции
        triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
                                           [&](const DelaunayTriangle &t) {
                                               return std::find(badTriangles.begin(), badTriangles.end(), t) !=
                                                      badTriangles.end();
                                           }),
                            triangulation.end());

        // Заполняем дырку новыми треугольниками
        for (const auto &edge : polygon) {
            triangulation.emplace_back(edge.p1, edge.p2, point);
        }
    }

    // Удаляем треугольники, содержащие вершины супер-треугольника
    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
                                       [&](const DelaunayTriangle &t) {
                                           return t.a == super1 || t.a == super2 || t.a == super3 || t.b == super1 ||
                                                  t.b == super2 || t.b == super3 || t.c == super1 || t.c == super2 ||
                                                  t.c == super3;
                                       }),
                        triangulation.end());

    return triangulation;
}
}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};