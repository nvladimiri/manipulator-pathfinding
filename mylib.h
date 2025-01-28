
#include <stdlib.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <complex>
#include <unordered_set>
#include <map>
#include <set>
#include <algorithm>


using my_float_t = double;

/*
Точка на плоскости
*/
struct point_t{
    point_t(): x(0), y(0) {}
    point_t(my_float_t x, my_float_t y): x(x), y(y) {}
    point_t(std::complex<my_float_t> c): x(c.real()), y(c.imag()) {}
    my_float_t x;
    my_float_t y;

    bool operator==(const point_t& other) const{
        return x == other.x && y == other.y;
    }
    bool operator!=(const point_t& other) const{
        return !operator==(other);
    }

    point_t operator-() const{
        return point_t(-x, -y);
    }
    point_t operator-(const point_t& other) const{
        return point_t(x-other.x, y-other.y);
    }
    point_t operator+(const point_t& other) const{
        return point_t(x+other.x, y+other.y);
    }
    point_t operator*(my_float_t l) const{
        return point_t(x*l, y*l);
    }
    point_t operator/(my_float_t l) const{
        return point_t(x/l, y/l);
    }
    my_float_t abs() const{
        return std::abs(to_complex());
    }
    my_float_t arg() const{
        return std::arg(to_complex());
    }
    point_t rotate(my_float_t angle) const{
        my_float_t r = abs();
        my_float_t a = arg();
        my_float_t new_a = a + angle;
        return point_t(r*cos(new_a), r*sin(new_a));
    }
    my_float_t angle_to(const point_t& other){
        return std::arg(other.to_complex() / to_complex());
    }
    std::complex<my_float_t> to_complex() const{
        return std::complex<my_float_t>(x, y);
    }

    static point_t from_arg(my_float_t arg, my_float_t r){
        return point_t(r*cos(arg), r*sin(arg));
    }

    point_t rev() const{
        return point_t(y, x);
    }
};

inline point_t operator*(my_float_t l, point_t p){
    return point_t(l*p.x, l*p.y);
}

inline my_float_t dot(point_t a, point_t b){
    return a.x*b.x + a.y*b.y;
}


/*
Ячейка на плоскости
Имеет 2 координаты x и y
Означает квадрат [x;x+1]*[y;y+1]
*/
struct cell_t{
    cell_t(int x, int y): x(x), y(y) {}
    cell_t(point_t P): x(std::floor(P.x)), y(std::floor(P.y)) {}
    bool operator==(const cell_t& other) const {
        return x == other.x && y == other.y;
    };
    bool operator!=(const cell_t& other) const {
        return !operator==(other);
    };
    int x;
    int y;

    cell_t rev() const{
        return cell_t(y, x);
    }

    cell_t rotate_left(){
        return cell_t(-y, x);
    }

    cell_t rotate_right(){
        return cell_t(y, -x);
    }

    point_t to_point(){
        return point_t(x, y);
    }

    void operator+=(const cell_t& other){
        x += other.x;
        y += other.y;
    }
};


struct CellHash{
    std::size_t operator()(const cell_t& p) const {
        // Combine the hash of x and y using a simple hash combination technique
        std::size_t h1 = std::hash<int>{}(p.x);
        std::size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1); // Hash combine
    }
};

using cell_set_t = std::unordered_set<cell_t, CellHash>;
using cell_vec_t = std::vector<cell_t>;
using point_vec_t = std::vector<point_t>;



/*
Возвращает множество клеток, занятых отрезком [A, B]
*/
void get_line_cells(point_t A, point_t B, cell_set_t& S);


/*
Возвращает множество клеток, пересекаемых дугой с центром O, 
начальной точкой A и углом поворота delta_arg (точки A вокруг O)
*/
void get_arc_cells(point_t O, point_t A, my_float_t delta_arg, cell_set_t& S);



/*
S - некоторое связное множество клеток на плоскости
(у каждой клетки 8 соседей: по бокам и диагоналям)
Эта функция заполняет все дыры в этом множестве
*/
void fill(cell_set_t& S);


/*
Возвращает клетки на границе движения манипулятора
*/
cell_set_t get_motion_border(
    const point_vec_t& points,
    size_t k, my_float_t delta_arg
);






