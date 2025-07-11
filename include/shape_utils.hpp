#pragma once
#include "geometry.hpp"
#include "queries.hpp"
#include <algorithm>
#include <iterator>
#include <print>
#include <random>
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/filter.hpp>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

namespace geometry::utils {

class ShapeGenerator {
public:
    ShapeGenerator(double min_coord = -100.0, double max_coord = 100.0, double min_size = 1.0, double max_size = 20.0)
        : gen(20), coord_dist(min_coord, max_coord), size_dist(min_size, max_size), sides_dist(3, 12), type_dist(0, 4) {
    }

    Shape GenerateRandomShape() {
        Point2D center{coord_dist(gen), coord_dist(gen)};
        double size = size_dist(gen);

        switch (type_dist(gen)) {
        case 0: {
            Point2D end{center.x + size, center.y + size};
            return Line{center, end};
        }
        case 1: {
            Point2D a{center.x, center.y};
            Point2D b{center.x + size, center.y};
            Point2D c{center.x + size / 2, center.y + size};
            return Triangle{a, b, c};
        }
        case 2: {
            return Rectangle{center, size, size * 0.8};
        }
        case 3: {
            int sides = sides_dist(gen);
            return RegularPolygon{center, size, sides};
        }
        case 4: {
            return Circle{center, size};
        }
        }
        return Circle{center, size};
    }

    std::vector<Shape> GenerateShapes(size_t count) {
        std::vector<Shape> shapes;
        shapes.reserve(count);

        for (auto _ : std::views::iota(0u, count)) {
            shapes.push_back(GenerateRandomShape());
        }

        return shapes;
    }

private:
    std::mt19937 gen;
    std::uniform_real_distribution<double> coord_dist;
    std::uniform_real_distribution<double> size_dist;
    std::uniform_int_distribution<int> sides_dist;
    std::uniform_int_distribution<int> type_dist;
};

inline std::vector<std::optional<std::pair<Point2D, Point2D>>> FindAllCollisions(DummyClass shapes) {
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
               return geometry::queries::BoundingBoxesOverlap(shape1.second, shape2.second);
           }) |
           std::ranges::to<std::vector>();
}

inline std::optional<size_t> FindHighestShape(DummyClass shapes) {

    /*
     * Используйте библиотеку ranges, чтобы найти самую высокую фигуру
     *
     * Важно: использование ручной итерации по фигурам не разрешается
     */

    auto get_height = [](const Shape &shape) { return std::visit([&](const auto &el) { return el.Height(); }, shape); };
    auto res = std::ranges::max_element(shapes.shapes_, {}, get_height);
    if (res == shapes.shapes_.end()) {
        return std::nullopt;
    }

    return get_height(*res);
}

}  // namespace geometry::utils