#include "convex_hull.hpp"
#include "geometry.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <stack>
#include <stdexcept>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

GeometryResult<std::vector<Point2D>> GrahamScan(std::vector<Point2D> points) {
    if (points.size() < 3)
        return std::unexpected{GeometryError::InsufficientPoints};

    // 1. Найти точку с минимальным y (и x при равенстве)
    auto it = std::min_element(points.begin(), points.end(), [](const Point2D &a, const Point2D &b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    });
    Point2D P0 = *it;
    std::swap(points[0], *it);

    // 2. Сортировка по полярному углу относительно P0
    auto polar_cmp = [P0](const Point2D &a, const Point2D &b) {
        Point2D va = a - P0, vb = b - P0;
        double cross = va.Cross(vb);
        if (std::abs(cross) < 1e-9)  // почти коллинеарны
            return va.Length() < vb.Length();
        return cross > 0;
    };
    std::sort(points.begin() + 1, points.end(), polar_cmp);

    // 3. Удаление ближних точек с одинаковым углом (оставляем только дальние)
    std::vector<Point2D> unique_points = {P0};
    for (size_t i = 1; i < points.size(); ++i) {
        while (i + 1 < points.size() && std::abs((points[i] - P0).Cross(points[i + 1] - P0)) < 1e-9)
            ++i;
        unique_points.push_back(points[i]);
    }

    // 4. Построение оболочки
    std::vector<Point2D> hull;
    hull.push_back(unique_points[0]);
    hull.push_back(unique_points[1]);
    hull.push_back(unique_points[2]);
    for (size_t i = 3; i < unique_points.size(); ++i) {
        while (hull.size() > 1 && ((hull.back() - hull[hull.size() - 2]).Cross(unique_points[i] - hull.back()) <= 0))
            hull.pop_back();
        hull.push_back(unique_points[i]);
    }
    return hull;
}

}  // namespace geometry::convex_hull