
#include "mylib.h"


constexpr my_float_t PI = 3.1415926535;


/*
Добавляет в множество S клетки занятые отрезком
*/
void get_line_cells(
    point_t A,
    point_t B,
    cell_set_t& S
){
    int x_min = std::ceil(std::min(A.x, B.x));
    int x_max = std::floor(std::max(A.x, B.x));
    int y_min = std::ceil(std::min(A.y, B.y));
    int y_max = std::floor(std::max(A.y, B.y));

    if(A.x == B.x)  x_min += 1;
    if(A.y == B.y)  y_min += 1;

    for(int x = x_min; x <= x_max; ++x){
        my_float_t a = (x-A.x)/(B.x-A.x);
        assert(0 <= a && a <= 1);

        int y = std::floor(A.y + (B.y - A.y)*a);
        S.insert(cell_t(x-1, y));
        S.insert(cell_t(x, y));
    }

    for(int y = y_min; y <= y_max; ++y){
        my_float_t a = (y-A.y)/(B.y-A.y);
        assert(0 <= a && a <= 1);

        int x = std::floor(A.x + (B.x - A.x)*a);
        S.insert(cell_t(x, y-1));
        S.insert(cell_t(x, y));
    }
}


/*
Проверяет, правда ли угол angle будет задет
при повороте от угла start на угол delta

Все углы измеряются в радианах; допустимы любые вещественные значения
(углы с разницей 2pi считаются одинаковыми, однако это не касается угла поворота - 
для него разница 2pi означает один дополнительный поворот)
*/
bool is_in_rotation(my_float_t start, my_float_t delta, my_float_t angle){
    start /= 2*PI; delta /= 2*PI; angle /= 2*PI;
    start -= std::floor(start); angle -= std::floor(angle);
    my_float_t end = start + delta;
    if(end < start)
        std::swap(start, end);
    return (start <= angle && angle <= end) || 
        (start <= angle-1 && angle-1 <= end) || 
        (start <= angle+1 && angle+1 <= end);
}



cell_set_t get_arc_x_cells(
    point_t O, 
    point_t A,
    my_float_t delta_arg
){ 
    point_t rA = (A - O);
    point_t rB = rA.rotate(delta_arg);
    point_t B = O + rB;

    my_float_t r = rA.abs();
    my_float_t arg_a = rA.arg();

    int x_min = std::min(std::ceil(A.x), std::ceil(B.x));
    int x_max = std::max(std::floor(A.x), std::floor(B.x));

    auto is_in_our_rotation = [&](my_float_t arg){
        return is_in_rotation(arg_a, delta_arg, arg);
    };

    if(is_in_our_rotation(0)){
        x_max = std::floor(O.x + r);
    }

    if(is_in_our_rotation(PI)){
        x_min = std::ceil(O.x - r);
    }

    cell_set_t res;
    size_t count_x = x_max - x_min + 1;
    res.reserve(count_x);

    for(int x = x_min; x <= x_max; ++x){
        my_float_t a = std::acos((x-O.x)/r);
        if(is_in_our_rotation(a)){
            point_t P = point_t::from_arg(a, r) + O;
            assert(abs(P.x - x) < 1e-5);
            int y = std::floor(P.y);
            res.insert(cell_t(x-1, y));
            res.insert(cell_t(x, y));
        }
        if(is_in_our_rotation(-a)){
            point_t P = point_t::from_arg(-a, r) + O;
            assert(abs(P.x - x) < 1e-5);
            int y = std::floor(P.y);
            res.insert(cell_t(x-1, y));
            res.insert(cell_t(x, y));
        }
    }
    return res;
}


/*
Добавляет в множество S клетки занятые дугой
*/
void get_arc_cells(
    point_t O, 
    point_t A,
    my_float_t delta_arg,
    cell_set_t& S
){
    cell_set_t x_cells = get_arc_x_cells(O, A, delta_arg);
    cell_set_t y_rev_cells = get_arc_x_cells(O.rev(), A.rev(), -delta_arg);

    for(auto cell: x_cells){
        S.insert(cell);
    }

    for(auto cell: y_rev_cells){
        S.insert(cell.rev());
    }
}


/*
S - некоторое связное множество клеток на плоскости
(у каждой клетки 8 соседей: по бокам и диагоналям)
Эта функция заполняет все дыры в этом множестве
*/
void fill(cell_set_t& S){
    /*
    1) Находит один участок границы
    2) Движется вдоль него, получая полную границу множества
    3) По этой границе заполняет все дыры внутри
    */

    // 1) Находим произвольный отрезок, с одной стороны от которого занятая область, а с другой - нет
    if(S.empty())   return;

    cell_t random_cell = *S.begin();
    int x0 = random_cell.x;
    int y0 = random_cell.y;

    for(cell_t cell: S){
        if(cell.x == x0){
            y0 = std::min(y0, cell.y);
        }
    }
    // этот отрезок - от точки (x, y) в сторону (1, 0), препятствие сверху

    cell_t pos(x0+1, y0); // точка на границе где мы сейчас находимся
    cell_t dir(1, 0);   // направление откуда пришли
    // слева от нас всегда должно быть препятствие


    std::unordered_map<int, std::vector<int>> x_to_ys;
    x_to_ys[x0].push_back(y0);


    // до тех пор пока не пришли в точку где уже были шагаем вперёд
    // всегда держим стенку слева от себя
    // поворачиваем налево если возможно, идём прямо если возможно, иначе направо
    while(pos != cell_t(x0, y0)){
        point_t fpos = pos.to_point();
        point_t fdir = dir.to_point() / 2;

        cell_t cell_left = cell_t(fpos + fdir + fdir.rotate(PI/2));    // клетка слева спереди
        cell_t cell_right = cell_t(fpos + fdir + fdir.rotate(-PI/2));  // клетка справа спереди

        bool l = S.count(cell_left);    // занята ли клетка слева спереди
        bool r = S.count(cell_right);   // занята ли клетка справа спереди

        if(r)   // правая занята
            dir = dir.rotate_right();
        
        if(!l && !r) // обе свободны
            dir = dir.rotate_left();

        if(dir == cell_t(1, 0))
            x_to_ys[pos.x].push_back(pos.y);

        if(dir == cell_t(-1, 0))
            x_to_ys[pos.x-1].push_back(pos.y);

        pos += dir;
    }


    for(auto& pair: x_to_ys){
        int x = pair.first;
        std::vector<int>& ys = pair.second;
        assert(ys.size() % 2 == 0);

        std::sort(ys.begin(), ys.end());
        for(size_t i = 0; i < ys.size(); i+=2){
            int y_min = ys[i];
            int y_max = ys[i+1];
            assert(y_min <= y_max);

            for(int y = y_min; y < y_max; ++y){
                S.insert(cell_t(x, y));
            }
        }
    }
}



/*
Возвращает клетки на границе движения манипулятора
*/
cell_set_t get_motion_border(
    const point_vec_t& points,
    size_t k, my_float_t delta_arg
){
    cell_set_t S;
    if(points.empty())
        return S;

    size_t K = points.size() - 1;
    for(size_t i = 0; i < k; ++i){
        get_line_cells(points[i], points[i+1], S);
    }

    point_t O = points[k];
    point_vec_t rotated_points;
    rotated_points.push_back(O);
    for(size_t i = k; i < K; ++i){
        point_t A = points[i];
        point_t B = points[i+1];

        point_t AB = B - A;
        point_t AO = O - A;

        my_float_t t = dot(AB, AO) / dot(AB, AB);
        if(0 < t && t < 1){     // проекция лежит на отрезке [A; B]
            point_t H = A + t * AB;
            rotated_points.push_back(H);
        }
        rotated_points.push_back(B);
    }
    for(size_t i = 0; i < rotated_points.size() - 1; ++i){
        point_t A = rotated_points[i];
        point_t B = rotated_points[i+1];

        point_t A1 = O + (A - O).rotate(delta_arg);
        point_t B1 = O + (B - O).rotate(delta_arg);

        get_line_cells(A, B, S);
        get_line_cells(A1, B1, S);
        get_arc_cells(O, A, delta_arg, S);
        get_arc_cells(O, B, delta_arg, S);
    }

    return S;
}


extern "C" size_t get_state_cells(
    size_t K, const point_t* points,
    cell_t** cells
){
    cell_set_t S;
    for(size_t i = 0; i < K; ++i){
        get_line_cells(points[i], points[i+1], S);
    }

    size_t N = S.size();
    *cells = (cell_t*)malloc(N * sizeof(cell_t));

    size_t i = 0;
    for(cell_t cell: S){
        (*cells)[i] = cell;
        i += 1;
    }
    return N;
}



extern "C" size_t get_motion_cells(
    size_t K, const point_t* points,
    size_t k, double delta_arg,
    cell_t** cells
){
    point_vec_t vec_points;
    for(size_t i = 0; i <= K; ++i){
        vec_points.push_back(points[i]);
    }

    cell_set_t S = get_motion_border(vec_points, k, delta_arg);
    fill(S);

    size_t N = S.size();
    *cells = (cell_t*)malloc(N * sizeof(cell_t));

    size_t i = 0;
    for(cell_t cell: S){
        (*cells)[i] = cell;
        i += 1;
    }
    return N;
}