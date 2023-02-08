#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace sf;


//SCOPE - масштаб модели
const int SCOPE = 220;
//Гравитационная постоянная
const double G = 6.6743e-11;
//Скорсоть симуляции
const double TIME_SPEED = 100;


//Нормирование вектора (Вычисление его длины)
double normed(Vector2f vector) {
    return sqrt(pow(vector.x, 2) + pow(vector.y, 2));
}
//Знак числа (Необходимо для вычисления направления вектора)
int sign(double a) {
    if (a < 0) return -1;
    else return 1;
}
//Перегрузка арифметических операторов для вектора
Vector2f operator+(const Vector2f& v, const double& d) {
    return Vector2f(v.x + d, v.y + d);
}
Vector2f operator-(const Vector2f& v, const double& d) {
    return Vector2f(v.x - d, v.y - d);
}
Vector2f operator*(const Vector2f& v, const double& d) {
    return Vector2f(v.x * d, v.y * d);
}
Vector2f operator/(const Vector2f& v, const double& d) {
    return Vector2f(v.x / d, v.y / d);
}
Vector2f operator+(const Vector2f& v1, const Vector2f& v2) {
    return Vector2f(v1.x + v2.x, v1.y + v2.y);
}
Vector2f operator-(const Vector2f& v1, const Vector2f& v2) {
    return Vector2f(v1.x - v2.x, v1.y - v2.y);
}
Vector2f operator*(const Vector2f& v1, const Vector2f& v2) {
    return Vector2f(v1.x * v2.x, v1.y * v2.y);
}
Vector2f operator/(const Vector2f& v1, const Vector2f& v2) {
    return Vector2f(v1.x / v2.x, v1.y / v2.y);
}


//Небесное тело
class Object {
private:
    double mass;   //kg
    double radius;  //m
    Vector2f velocity; //Векторная скорость м/c
    Vector2f acceleration; //Вектороное ускорение м/c^2
    CircleShape body;
    vector<Vertex> trajectory;
    String ID;
public:
    Object(String ID, Vector2f position, double mass = 0, double radius = 0, Vector2f velocity = Vector2f(0, 0), Color color = Color(195, 195, 195)) {
        this->mass = mass;
        this->radius = radius;
        this->acceleration = Vector2f(0, 0);
        this->velocity = velocity * TIME_SPEED;
        this->ID = ID;

        body.setFillColor(color);
        body.setRadius(radius > SCOPE ? (radius / (SCOPE)) : 2);
        body.setOrigin(body.getRadius(), body.getRadius());
        body.setPosition(position);
    }
    ~Object() { }
    CircleShape* getBody() {
        return &body;
    }
    double getMass() {
        return mass;
    }
    double getRadius() {
        return radius;
    }
    
    Vector2f getAcceleration() {
        return acceleration;
    }
    void setAcceleration(Vector2f acceleration) {
        this->acceleration = acceleration;
    }

    Vector2f getVelocity() {
        return velocity;
    }
    void setVelocity(Vector2f velocity) {
        this->velocity = velocity;
    }

    vector<Vertex> getTrajectory() {
        return trajectory;
    }
    void updateTrajectory(Vertex vert) {
        trajectory.push_back(vert);
    }

    //Влияние силы на скорость небесного тела
    void impact(Vector2f F) {
        //Изменение вектора ускорения
        acceleration = F / mass;

        //Изменение вектора скорости через секунду
        velocity = velocity + (acceleration * TIME_SPEED);
        
        //Изменение координат от скорости
        body.setPosition(body.getPosition() + (velocity / (1000 * SCOPE)));
    }
};

//Вычисление вектора расстояния
Vector2f distance(Object a, Object b) {
    Vector2f distance(a.getBody()->getPosition() - b.getBody()->getPosition());
    if (a.getBody()->getRadius() + b.getBody()->getRadius() >= normed(distance)) {
        return Vector2f(0, 0);
    }
    return distance;
}

//Вычисления гравитационного притяжения
Vector2f gravitational_pull(Object a, Object b) {
    //Вычисляем расстояние, переводим из км в м и масштабируем
    Vector2f dist = distance(a, b) * 1000 * SCOPE;
    //Вычисляем угол наклонения вектора относительно небесного тела
    Vector2f F_vector = dist / normed(dist);
    //Вычисляем скалярное значение силы в Н
    double F_scalar = TIME_SPEED * G * (a.getMass() * b.getMass()) / pow(normed(dist), 2);
    //Возвращаем вектор силы
    return F_vector * F_scalar;
}

//Вектор объектов
vector<Object> objects;

int main()
{   
    RenderWindow window(VideoMode(900, 900), "Simulation", sf::Style::Close);
    
    //Данные для Земли
    double earthMass = 5.9724e24; //kg
    double earthRadius = 6378.1; //km
    double GEO = 35786; //km
    
    
    //Спутник
    objects.push_back(Object("Moon", Vector2f(450, 200), 3e22, 0.005, Vector2f(10e2, 0)));
    //Спутник на геостационарной орбите. Скорость 3,07 км/c
    objects.push_back(Object("Probe", Vector2f(450, 450 + (earthRadius + GEO) / SCOPE), 1000, 0.005, Vector2f(3.07e3, 0)));
    //Земля
    objects.push_back(Object("Earth", Vector2f(450, 450), earthMass, earthRadius, Vector2f(0, 0)));
    //Время симуляции
    int time = 0;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        for (int i = 0; i < objects.size(); i++) {
            Vector2f F(0, 0);
            for (int j = 0; j < objects.size(); j++) {
                if (i != j) {
                    Vector2f d = distance(objects[i], objects[j]);
                    if (d.x != 0 || d.y != 0) {
                        Vector2f F_1 = gravitational_pull(objects[i], objects[j]);
                        F -= F_1;
                    }
                    else {
                        objects[i].setVelocity(Vector2f(0, 0));
                    }
                }
            }
            objects[i].impact(F);
            objects[i].updateTrajectory(Vertex(objects[i].getBody()->getPosition(), Color::White));
        }

        window.clear();
        for (int i = 0; i < objects.size(); i++) {
            vector<Vertex> trajectory = objects[i].getTrajectory();
            for (int i = 0; i < trajectory.size(); i++) {
                window.draw(&trajectory[i], 1, Points);
            }
            window.draw(*(objects[i].getBody()));
        }
        window.display();
        time++;
    }
    //cout << "Time of simulation: " << time * TIME_SPEED << " seconds";
    return 0;
}
