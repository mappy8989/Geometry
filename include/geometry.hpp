#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <format>
#include <numbers>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <variant>
#include <vector>

namespace geometry {

/*
 * Добавьте к методам класса Point2D и Lines2DDyn все необходимые аттрибуты и спецификаторы
 * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
 */
struct Point2D {
    double x, y;

    constexpr Point2D() : x(0), y(0) {}
    constexpr Point2D(double x, double y) : x(x), y(y) {}

    // Comparison
    bool operator<(const Point2D &other) { return x < other.x && y < other.y; }
    bool operator==(const Point2D &other) { return x == other.x && y == other.y; }

    // Binary math operators
    [[nodiscard]] constexpr Point2D operator+(const Point2D &other) const noexcept {
        return {x + other.x, y + other.y};
    }
    [[nodiscard]] constexpr Point2D operator-(const Point2D &other) const noexcept {
        return {x - other.x, y - other.y};
    }
    [[nodiscard]] constexpr Point2D operator*(double value) const noexcept { return {x * value, y * value}; }
    [[nodiscard]] constexpr Point2D operator/(double value) const noexcept { return {x / value, y / value}; }

    // Binary geometry operations
    [[nodiscard]] constexpr double Dot(const Point2D &other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr double Cross(const Point2D &other) const noexcept { return x * other.y - y * other.x; }
    [[nodiscard]] constexpr double Length() const noexcept { return std::sqrt(x * x + y * y); }
    [[nodiscard]] constexpr double DistanceTo(const Point2D &other) const noexcept { return (*this - other).Length(); }

    Point2D Normalize() {
        const double len = Length();
        return len > 0 ? Point2D{x / len, y / len} : Point2D{0, 0};
    }
};

template <size_t N>
struct Lines2D {
    std::array<double, N> x;
    std::array<double, N> y;
};

struct Lines2DDyn {
    std::vector<double> x;
    std::vector<double> y;

    void Reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }
    void PushBack(Point2D p) {
        x.push_back(p.x);
        y.push_back(p.y);
    }
    void PushBack(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }
    Point2D Front() { return {x.front(), y.front()}; }
};

struct BoundingBox {
    double min_x, min_y, max_x, max_y;

    /* ваш код здесь */
};

struct Line {
    Point2D start, end;
    std::string_view GetName() const { return "Line"; }
    std::vector<Point2D> Vertices() { return {start, end}; }

    double Length(void) const { return start.DistanceTo(end); }
    Point2D Center(void) const { return {(end.x + start.x) / 2, (end.y + start.y) / 2}; }

    Lines2D<2> Lines() const { return {{start.x, end.x}, {start.y, end.y}}; }
    double Height() const { return std::max(start.y, end.y); }

    BoundingBox BoundBox() const {
        return {std::min(start.x, end.x), std::min(start.y, end.y), std::max(start.x, end.x), std::max(start.y, end.y)};
    }
};

struct Triangle {
    Point2D a, b, c;
    std::string_view GetName() const { return "Triangle"; }
    std::vector<Point2D> Vertices() { return {a, b, c}; }

    Point2D Center(void) { return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3}; }

    Lines2D<4> Lines() const { return {{a.x, b.x, c.x, a.x}, {a.y, b.y, c.y, a.y}}; }
    double Height() const { return std::max(a.y, std::max(b.y, c.y)); }

    BoundingBox BoundBox() const {
        return {std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y}), std::max({a.x, b.x, c.x}),
                std::max({a.y, b.y, c.y})};
    }
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;
    std::string_view GetName() const { return "Rectangle"; }
    std::vector<Point2D> Vertices() {
        return {bottom_left, Point2D{bottom_left.x + width, bottom_left.y},
                Point2D{bottom_left.x, bottom_left.y + height}, Point2D{bottom_left.x + width, bottom_left.y + height}};
    }

    Point2D Center(void) { return {bottom_left.x + (width / 2), bottom_left.y + (height / 2)}; }

    Lines2D<4> Lines() const {
        return {{bottom_left.x, bottom_left.x, bottom_left.x + width, bottom_left.x + width},
                {bottom_left.y, bottom_left.y + height, bottom_left.y + height, bottom_left.y}};
    }

    double Height() const { return (bottom_left.y + height); }

    BoundingBox BoundBox() const {
        return {bottom_left.x, bottom_left.y, bottom_left.x + width, bottom_left.y + height};
    }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    std::string_view GetName() const { return "RegularPolygon"; }

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {}

    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (int i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }
        return points;
    }

    Point2D Center(void) { return center_p; }

    Lines2DDyn Lines() const {
        Lines2DDyn lines;
        lines.Reserve(sides);

        for (int i = 0; i < sides; i++) {
            const double angle = 2 * std::numbers::pi * i / sides;
            lines.PushBack(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }

        return lines;
    }

    double Height() const {
        auto points = Vertices();
        auto max = std::ranges::max_element(points, {}, &Point2D::y);

        return max->y;
    }

    BoundingBox BoundBox() const {
        auto vertices = Vertices();

        double min_x = 0;
        double min_y = 0;
        double max_x = 0;
        double max_y = 0;
        std::ranges::for_each(vertices, [&](auto &point) {
            if (max_x < point.x)
                max_x = point.x;
            else if (min_x > point.x)
                min_x = point.x;

            if (max_y < point.y)
                max_y = point.y;
            else if (min_y > point.y)
                min_y = point.y;
        });

        return {min_x, min_y, max_x, max_y};
    }
};

struct Circle {
    Point2D center_p;
    double radius;
    std::string_view GetName() const { return "Circle"; }

    constexpr Circle(Point2D center, double radius) : center_p(center), radius(radius) {}

    BoundingBox BoundBox() const {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }
    double Height() const { return center_p.y + radius; }
    Point2D Center() const { return center_p; }

    //
    // Должны быть сделана по аналогии с RegularPolygon::Vertices
    //
    std::vector<Point2D> Vertices(size_t N = 30) {
        std::vector<Point2D> points;
        points.reserve(N);

        for (int i = 0; i < (int)N; ++i) {
            const double angle = 2 * std::numbers::pi * i / N;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }
        return points;
    }
    Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn lines;
        lines.Reserve(N);

        for (int i = 0; i < (int)N; i++) {
            const double angle = 2 * std::numbers::pi * i / N;
            lines.PushBack(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }

        return lines;
    }
};

class Polygon {
public:
    std::string_view GetName() const { return "Polygon"; }
    /* ваш код здесь */
    Polygon(std::vector<Point2D> points) : points_(std::move(points)) {}

    Point2D Center(void) const {
        return {std::accumulate(points_.begin(), points_.end(), 0.0,
                                [](double val, const auto &elem) { return val + elem.x; }) /
                    points_.size(),
                std::accumulate(points_.begin(), points_.end(), 0.0, [](double val, const auto &elem) {
                    return val + elem.y;
                }) / points_.size()};
    }

    const std::vector<Point2D> &Vertices() const { return points_; }

    Lines2DDyn Lines(void) const {
        Lines2DDyn lines;
        lines.Reserve(points_.size());
        for (const auto &point : points_) {
            lines.PushBack(point);
        }
        return lines;
    }

    double Height() const {
        auto max = std::ranges::max_element(points_, {}, &Point2D::y);
        return max->y;
    }

    BoundingBox BoundBox() const {

        double min_x = 0;
        double min_y = 0;
        double max_x = 0;
        double max_y = 0;
        std::ranges::for_each(points_, [&](auto &point) {
            if (max_x < point.x)
                max_x = point.x;
            else if (min_x > point.x)
                min_x = point.x;

            if (max_y < point.y)
                max_y = point.y;
            else if (min_y > point.y)
                min_y = point.y;
        });

        return {min_x, min_y, max_x, max_y};
    }

private:
    std::vector<Point2D> points_;
};

using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;

/*
 * В коде везде используется DummyClass. Ваша задача - выбрать наиболее подходящий тип для решения задачи
 */
enum class GeometryError { Unsupported, NoIntersection, InvalidInput, DegenrateCase, InsufficientPoints };

template <typename T>
using GeometryResult = std::expected<T, GeometryError>;

}  // namespace geometry

template <>
struct std::formatter<geometry::Point2D> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Point2D &p, FormatContext &ctx) const {
        return format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
    }
};
template <>
struct std::formatter<std::vector<geometry::Point2D>> {
    bool use_new_line = false;

    constexpr auto parse(std::format_parse_context &ctx) {
        auto it = ctx.begin();

        if (it != ctx.end() && *it == 'n') {
            std::string_view remaining{it, ctx.end()};
            std::string_view spec{"new_line"};

            if (remaining.starts_with(spec)) {
                use_new_line = true;
                return it + spec.length();
            }
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) {
        auto out = ctx.out();

        bool first = true;
        for (const auto &elem : v) {
            if (!first) {
                out = std::format_to(out, use_new_line ? "\t" : " ");
            }
            first = false;
            // Format each element using its own formatter
            out = std::format_to(out, "{}", elem);
        }
        return out;
    }
};

template <>
struct std::formatter<geometry::Line> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Line &l, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Line({}, {})", l.start, l.end);
    }
};

template <>
struct std::formatter<geometry::Circle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Circle &c, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.center_p, c.radius);
    }
};

template <>
struct std::formatter<geometry::Rectangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Rectangle &r, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.bottom_left, r.width,
                              r.height);
    }
};

template <>
struct std::formatter<geometry::RegularPolygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::RegularPolygon &p, FormatContext &ctx) {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.center_p, p.radius,
                              p.sides);
    }
};
template <>
struct std::formatter<geometry::Triangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Triangle &t, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.a, t.b, t.c);
    }
};
template <>
struct std::formatter<geometry::Polygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Polygon &poly, FormatContext &ctx) {
        auto out = ctx.out();
        out = std::format_to(out, "Polygon[{} points]: [", poly.Vertices().size());

        for (const auto &p : poly.Vertices()) {
            out = std::format_to(out, "{} ", p);
        }

        return std::format_to(out, "]");
    }
};

namespace std {
template <>
struct formatter<const geometry::Line> : formatter<geometry::Line> {};
template <>
struct formatter<const geometry::Triangle> : formatter<geometry::Triangle> {};
template <>
struct formatter<const geometry::Rectangle> : formatter<geometry::Rectangle> {};
template <>
struct formatter<const geometry::RegularPolygon> : formatter<geometry::RegularPolygon> {};
template <>
struct formatter<const geometry::Circle> : formatter<geometry::Circle> {};
template <>
struct formatter<const geometry::Polygon> : formatter<geometry::Polygon> {};
}  // namespace std