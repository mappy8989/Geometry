#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <iterator>
#include <print>
#include <ranges>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;

void PrintAllIntersections(const Shape &shape, std::vector<Shape> others) {
    std::println("\n=== Intersections ===");
    if (std::holds_alternative<geometry::Line>(shape) || std::holds_alternative<geometry::Circle>(shape)) {
        return;
    }

    auto get_name = [](const Shape &shape) { return std::visit([](auto &elem) { return elem.GetName(); }, shape); };
    const std::string_view shape_name = get_name(shape);

    others | std::views::filter([](const auto &shape) {
        return std::holds_alternative<geometry::Line>(shape) || std::holds_alternative<geometry::Circle>(shape);
    }) | views::transform([&](auto &&elem) {
        auto is_intersect = intersections::GetIntersectPoint(shape, elem);
        if (std::holds_alternative<geometry::Line>(elem)) {
        }
        std::println("{} {} {}", shape_name, get_name(elem),
                     is_intersect.has_value() ? " intersects" : " not intersect");
        return is_intersect;
    });
    /*
     * Используйте ranges чтобы оставить только фигуры,
     * поддерживающие возможность находить пересечения между собой
     *
     * Затем примените монадический интерфейс для обработки результатов:
     *     - Пересечение найдено в точке A между фигурами B и C
     *     - Фигуры B и C не пересекаются
     */
}

void PrintDistancesFromPointToShapes(Point2D p, std::vector<Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");
    std::println("Testing point: {:} ", p);

    if (shapes.size() < 5) {
        return;
    }

    auto get_name = [](const Shape &shape) { return std::visit([](auto &elem) { return elem.GetName(); }, shape); };

    auto first_shapes = shapes | std::views::take(5);
    rng::for_each(first_shapes, [&](auto &&elem) {
        std::println("Distance from point P to the center of {} is {}", get_name(elem),
                     geometry::queries::PointToShapeDistanceVisitor{}(p, elem));
    });

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */
}

void PerformShapeAnalysis(std::vector<Shape> shapes) {
    std::println("\n=== Shape Analysis ===");
    auto collisions = geometry::utils::FindAllCollisions(shapes);

    rng::for_each(collisions, [](auto &&box) {
        if (box.has_value()) {
            std::println("{} {}", box.value().first, box.value().second);
        }
    });

    std::println("Max heigth is {}", geometry::utils::FindHighestShape(shapes).value());
    auto distances = geometry::queries::GetDistancesBetweenShapes(shapes);

    std::println("Distances:");
    rng::for_each(distances, [](const auto &elem) {
        if (elem.has_value()) {
            std::println("Distance = {}", *elem);
        } else {
            std::println("Unsupported figures");
        }
    });
    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    auto get_height = [](auto &elem) { return std::visit([](auto &shape) { return shape.Height(); }, elem); };
    auto res = shapes | views::filter([&](auto &shape) { return get_height(shape) > 50.0; }) | views::take(3);
    /*rng::for_each(shapes, [&](auto &elem) {
        std::visit(
            [&](auto &el) {
                using T = std::decay_t<decltype(el)>;
                std::println("{}", static_cast<const T>(el));  // Приводим к value/
            },
            elem);
    }); */
    (void)res;
    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    std::vector<Shape> shapes = generator.GenerateShapes(15);

    std::println("Generated {} random shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту
    int index = 0;
    rng::for_each(shapes, [&](auto &shape) {
        auto height = std::visit([&](const auto &elem) { return elem.Height(); }, shape);
        std::println("[{}] {}", index++, height);
    });
    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(shapes[0], shapes);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(shapes);

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;

    /* ваш код здесь */
    auto get_vertices = [](auto &shape) { return std::visit([](auto &elem) { return elem.Vertices(); }, shape); };
    rng::for_each(shapes, [&](auto &shape) {
        points.insert(points.end(), get_vertices(shape).begin(), get_vertices(shape).end());
    });
    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //
    auto graham_points = geometry::convex_hull::GrahamScan(points);
    if (graham_points.has_value()) {
        shapes.emplace_back(geometry::Polygon{graham_points.value()});
        geometry::visualization::Draw(shapes);
    }
    /* ваш код здесь */

    //
    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    //

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};
        auto triangulation = geometry::triangulation::DelaunayTriangulation(points);
        if (triangulation.has_value()) {
            geometry::visualization::Draw(triangulation.value());
        }
        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
    }
    return 0;
}