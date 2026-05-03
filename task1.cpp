#include "task1.h"
#include <QRandomGenerator>
#include <functional>
#include <cmath>
#include <QStringList>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// f — подынтегральная функция, a/b — пределы, n — число разбиений.
static double numericalTrapezoid(std::function<double(double)> f, double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = (f(a) + f(b)) / 2.0;
    for (int i = 1; i < n; i++) {
        sum += f(a + i * h);
    }
    return sum * h;
}

// Требует чётное n; если n нечётное, округляется вверх до ближайшего чётного.
static double numericalSimpson(std::function<double(double)> f, double a, double b, int n)
{
    if (n % 2 != 0) n++;
    double h = (b - a) / n;
    double sum = f(a) + f(b);
    for (int i = 1; i < n; i++) {
        double coeff = (i % 2 == 0) ? 2.0 : 4.0;
        sum += coeff * f(a + i * h);
    }
    return sum * h / 3.0;
}

// Используется для определения, какой из двух методов точнее.
static double referenceIntegral(std::function<double(double)> f, double a, double b)
{
    return numericalTrapezoid(f, a, b, 100000);
}

// ===============================
// ЗАДАНИЯ
// ===============================


// Для каждого задания вычисляются:
//   - результат методом трапеций (с малым n),
//   - результат методом Симпсона (с тем же n),
//   - эталонное значение (n=100000).
// Правильный ответ: 1 если |trap - ref| < |simp - ref|, иначе 2.
Task1Problem task1_generate(int num)
{
    Task1Problem problem;
    problem.correctAnswer = 2; // по умолчанию

    auto* rng = QRandomGenerator::global();

    std::function<double(double)> f;
    QString description;
    double a = 0, b = 1;
    int n = 2;

    // Вспомогная лямбда для округления double до 4 знаков
    auto fmt4 = [](double v) -> QString { return QString::number(v, 'f', 4); };

    switch (num) {
    // ---- Полиномы (1-10) ----
    case 1: {
        // ∫ c*x^2 dx
        double c = rng->bounded(1, 10);
        f = [c](double x) { return c * x * x; };
        description = QString("%1*x^2").arg(c);
        a = rng->bounded(0, 5); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 2: {
        // ∫ c*x^3 dx
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * x * x * x; };
        description = QString("%1*x^3").arg(c);
        a = rng->bounded(0, 4); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 3: {
        // ∫ c*x^4 dx
        double c = rng->bounded(1, 6);
        f = [c](double x) { return c * std::pow(x, 4); };
        description = QString("%1*x^4").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 4: {
        // ∫ c*x^5 dx
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c * std::pow(x, 5); };
        description = QString("%1*x^5").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 3);
        n = rng->bounded(2, 8);
        break;
    }
    case 5: {
        // ∫ (c1*x + c2) dx
        double c1 = rng->bounded(1, 8), c2 = rng->bounded(1, 10);
        f = [c1, c2](double x) { return c1 * x + c2; };
        description = QString("%1*x + %2").arg(c1).arg(c2);
        a = rng->bounded(0, 5); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 6: {
        // ∫ (c*x^2 + d) dx
        double c = rng->bounded(1, 7), d = rng->bounded(1, 10);
        f = [c, d](double x) { return c * x * x + d; };
        description = QString("%1*x^2 + %2").arg(c).arg(d);
        a = rng->bounded(0, 5); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 7: {
        // ∫ (c*x^3 + d*x) dx
        double c = rng->bounded(1, 5), d = rng->bounded(1, 8);
        f = [c, d](double x) { return c * x * x * x + d * x; };
        description = QString("%1*x^3 + %2*x").arg(c).arg(d);
        a = rng->bounded(0, 4); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 8: {
        // ∫ (c*x^4 + d*x^2) dx
        double c = rng->bounded(1, 4), d = rng->bounded(1, 6);
        f = [c, d](double x) { return c * std::pow(x, 4) + d * x * x; };
        description = QString("%1*x^4 + %2*x^2").arg(c).arg(d);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 3);
        n = rng->bounded(2, 8);
        break;
    }
    case 9: {
        // ∫ (c*x^2 + d*x + e) dx — полный квадратный трёхчлен
        double c = rng->bounded(1, 5), d = rng->bounded(-5, 6), e = rng->bounded(1, 10);
        f = [c, d, e](double x) { return c * x * x + d * x + e; };
        description = QString("%1*x^2 %2 %3*x + %4")
                      .arg(c).arg(d >= 0 ? "+" : "").arg(d).arg(e);
        a = rng->bounded(0, 5); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 10: {
        // ∫ (c*x^3 + d*x^2 + e) dx
        double c = rng->bounded(1, 4), d = rng->bounded(1, 5), e = rng->bounded(1, 8);
        f = [c, d, e](double x) { return c * x * x * x + d * x * x + e; };
        description = QString("%1*x^3 + %2*x^2 + %3").arg(c).arg(d).arg(e);
        a = rng->bounded(0, 4); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }

    // ---- Тригонометрия (11-20) ----
    case 11: {
        // ∫ c*sin(x) dx
        double c = rng->bounded(1, 10);
        f = [c](double x) { return c * std::sin(x); };
        description = QString("%1*sin(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 12: {
        // ∫ c*cos(x) dx
        double c = rng->bounded(1, 10);
        f = [c](double x) { return c * std::cos(x); };
        description = QString("%1*cos(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 13: {
        // ∫ c*sin(d*x) dx
        double c = rng->bounded(1, 8), d = rng->bounded(1, 5);
        f = [c, d](double x) { return c * std::sin(d * x); };
        description = QString("%1*sin(%2*x)").arg(c).arg(d);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 14: {
        // ∫ c*cos(d*x) dx
        double c = rng->bounded(1, 8), d = rng->bounded(1, 5);
        f = [c, d](double x) { return c * std::cos(d * x); };
        description = QString("%1*cos(%2*x)").arg(c).arg(d);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 15: {
        // ∫ c*sin^2(x) dx
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * std::sin(x) * std::sin(x); };
        description = QString("%1*sin^2(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 16: {
        // ∫ c*cos^2(x) dx
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * std::cos(x) * std::cos(x); };
        description = QString("%1*cos^2(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 17: {
        // ∫ c*|sin(x)| dx — негладкая функция (трапеции могут быть точнее!)
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * std::fabs(std::sin(x)); };
        description = QString("%1*|sin(x)|").arg(c);
        a = 0; b = M_PI * (rng->bounded(1, 4));
        n = rng->bounded(2, 8);
        break;
    }
    case 18: {
        // ∫ c*sin(x)*cos(x) dx
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * std::sin(x) * std::cos(x); };
        description = QString("%1*sin(x)*cos(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 19: {
        // ∫ c*sin(d*x)*cos(e*x) dx
        double c = rng->bounded(1, 5), d = rng->bounded(1, 4), e = rng->bounded(1, 4);
        f = [c, d, e](double x) { return c * std::sin(d * x) * std::cos(e * x); };
        description = QString("%1*sin(%2*x)*cos(%3*x)").arg(c).arg(d).arg(e);
        a = 0; b = rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 20: {
        // ∫ c*sin^2(d*x) dx
        double c = rng->bounded(1, 6), d = rng->bounded(1, 4);
        f = [c, d](double x) { double s = std::sin(d * x); return c * s * s; };
        description = QString("%1*sin^2(%2*x)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }

    // ---- Экспоненциальные (21-28) ----
    case 21: {
        // ∫ c*e^x dx
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c * std::exp(x); };
        description = QString("%1*e^x").arg(c);
        a = rng->bounded(0, 2); b = a + rng->bounded(1, 3);
        n = rng->bounded(2, 8);
        break;
    }
    case 22: {
        // ∫ c*e^(d*x) dx
        double c = rng->bounded(1, 5), d = rng->bounded(1, 4);
        f = [c, d](double x) { return c * std::exp(d * x); };
        description = QString("%1*e^(%2*x)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 3);
        n = rng->bounded(2, 8);
        break;
    }
    case 23: {
        // ∫ c*e^(-x) dx
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c * std::exp(-x); };
        description = QString("%1*e^(-x)").arg(c);
        a = 0; b = rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 24: {
        // ∫ c*e^(-d*x) dx
        double c = rng->bounded(1, 8), d = rng->bounded(1, 4);
        f = [c, d](double x) { return c * std::exp(-d * x); };
        description = QString("%1*e^(-%2*x)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 25: {
        // ∫ c*e^(x^2) dx — интеграл не берётся аналитически
        double c = rng->bounded(1, 3);
        f = [c](double x) { return c * std::exp(x * x); };
        description = QString("%1*e^(x^2)").arg(c);
        a = 0; b = rng->bounded(1, 2);
        n = rng->bounded(2, 8);
        break;
    }
    case 26: {
        // ∫ c*x*e^x dx
        double c = rng->bounded(1, 4);
        f = [c](double x) { return c * x * std::exp(x); };
        description = QString("%1*x*e^x").arg(c);
        a = 0; b = rng->bounded(1, 3);
        n = rng->bounded(2, 8);
        break;
    }
    case 27: {
        // ∫ c*x*e^(d*x) dx
        double c = rng->bounded(1, 3), d = rng->bounded(1, 3);
        f = [c, d](double x) { return c * x * std::exp(d * x); };
        description = QString("%1*x*e^(%2*x)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 2);
        n = rng->bounded(2, 8);
        break;
    }
    case 28: {
        // ∫ e^x*sin(x) dx — затухающие/растущие колебания
        f = [](double x) { return std::exp(x) * std::sin(x); };
        description = "e^x*sin(x)";
        a = 0; b = rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }

    // ---- Специальные функции (29-40) ----
    case 29: {
        // ∫ c/(1+x^2) dx — производная arctan
        double c = rng->bounded(1, 8);
        f = [c](double x) { return c / (1.0 + x * x); };
        description = QString("%1/(1+x^2)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 30: {
        // ∫ c*sqrt(x) dx (x >= 0)
        double c = rng->bounded(1, 6);
        f = [c](double x) { return c * std::sqrt(x); };
        description = QString("%1*sqrt(x)").arg(c);
        a = 0; b = rng->bounded(1, 10);
        n = rng->bounded(2, 8);
        break;
    }
    case 31: {
        // ∫ c/sqrt(1-x^2) dx — производная arcsin, интервал [-1, 1]
        double c = rng->bounded(1, 5);
        double lo = -0.9;
        f = [c](double x) { return c / std::sqrt(1.0 - x * x); };
        description = QString("%1/sqrt(1-x^2)").arg(c);
        a = lo; b = 0.9;
        n = rng->bounded(2, 8);
        break;
    }
    case 32: {
        // ∫ c*x/sqrt(1+x^2) dx
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c * x / std::sqrt(1.0 + x * x); };
        description = QString("%1*x/sqrt(1+x^2)").arg(c);
        a = 0; b = rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 33: {
        // ∫ c*ln(x) dx (x > 0)
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c * std::log(x); };
        description = QString("%1*ln(x)").arg(c);
        a = 1; b = rng->bounded(2, 10);
        n = rng->bounded(2, 8);
        break;
    }
    case 34: {
        // ∫ c*x*ln(x) dx (x > 0)
        double c = rng->bounded(1, 4);
        f = [c](double x) { return c * x * std::log(x); };
        description = QString("%1*x*ln(x)").arg(c);
        a = 1; b = rng->bounded(2, 8);
        n = rng->bounded(2, 8);
        break;
    }
    case 35: {
        // ∫ c*sin(x)/x dx — интегральный синус (sinc)
        double c = rng->bounded(1, 5);
        f = [c](double x) {
            if (std::fabs(x) < 1e-10) return c * 1.0; // предел sin(x)/x при x->0 равен 1
            return c * std::sin(x) / x;
        };
        description = QString("%1*sin(x)/x").arg(c);
        a = 0.1; b = rng->bounded(2, 10);
        n = rng->bounded(2, 8);
        break;
    }
    case 36: {
        // ∫ c/(x+d) dx — логарифмический интеграл
        double c = rng->bounded(1, 6), d = rng->bounded(1, 5);
        f = [c, d](double x) { return c / (x + d); };
        description = QString("%1/(x+%2)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }
    case 37: {
        // ∫ c*e^x*cos(x) dx
        double c = rng->bounded(1, 4);
        f = [c](double x) { return c * std::exp(x) * std::cos(x); };
        description = QString("%1*e^x*cos(x)").arg(c);
        a = 0; b = rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 38: {
        // ∫ c*e^(d*x)*sin(x) dx
        double c = rng->bounded(1, 4), d = rng->bounded(1, 3);
        f = [c, d](double x) { return c * std::exp(d * x) * std::sin(x); };
        description = QString("%1*e^(%2*x)*sin(x)").arg(c).arg(d);
        a = 0; b = rng->bounded(1, 4);
        n = rng->bounded(2, 8);
        break;
    }
    case 39: {
        // ∫ c/(1+e^(-x)) dx — сигмоида
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c / (1.0 + std::exp(-x)); };
        description = QString("%1/(1+e^(-x))").arg(c);
        a = rng->bounded(-5, 0); b = rng->bounded(1, 6);
        n = rng->bounded(2, 8);
        break;
    }
    case 40: {
        // ∫ c*arctan(x) dx
        double c = rng->bounded(1, 5);
        f = [c](double x) { return c * std::atan(x); };
        description = QString("%1*arctan(x)").arg(c);
        a = rng->bounded(0, 3); b = a + rng->bounded(1, 5);
        n = rng->bounded(2, 8);
        break;
    }

    default:
        // Если num вне диапазона 1-40 — возвращаем ошибку
        problem.text = "error: invalid task number (1-40)\r\n";
        problem.correctAnswer = -1;
        return problem;
    }

    // Вычисляем результаты трёх методов
    double trapResult = numericalTrapezoid(f, a, b, n);
    double simpResult = numericalSimpson(f, a, b, n);
    double refResult  = referenceIntegral(f, a, b);

    // Определяем, какой метод точнее (ближе к эталону)
    double trapError = std::fabs(trapResult - refResult);
    double simpError = std::fabs(simpResult - refResult);
    problem.correctAnswer = (trapError < simpError) ? 1 : 2;

    // Формируем текст задания
    problem.text = QString(
        "=== Задание Task1 #%1 ===\r\n"
        "f(x) = %2\r\n"
        "Интеграл от %3 до %4, разбиений n = %5\r\n"
        "Метод трапеций: %6\r\n"
        "Метод Симпсона:  %7\r\n"
        "Какой результат точнее? (1 - трапеции, 2 - Симпсон)\r\n"
    ).arg(num)
     .arg(description)
     .arg(fmt4(a)).arg(fmt4(b)).arg(n)
     .arg(fmt4(trapResult)).arg(fmt4(simpResult));

    return problem;
}
