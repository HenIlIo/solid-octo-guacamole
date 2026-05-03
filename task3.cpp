#include "task3.h"
#include <QRandomGenerator>

// Диапазон рандомизации зависит от номера задания (средние числа):
//   1-10:  числа от 10 до 100
//   11-20: числа от 10 до 200
//   21-30: числа от 50 до 500
//   31-40: числа от 100 до 1000
Task3Problem task3_generate(int num)
{
    Task3Problem problem;
    auto* rng = QRandomGenerator::global();

    int minVal = 10, maxVal = 100;

    if (num >= 1 && num <= 10)      { minVal = 10;  maxVal = 100; }
    else if (num >= 11 && num <= 20) { minVal = 10;  maxVal = 200; }
    else if (num >= 21 && num <= 30) { minVal = 50;  maxVal = 500; }
    else if (num >= 31 && num <= 40) { minVal = 100; maxVal = 1000; }
    else {
        problem.text = "error: invalid task number (1-40)\r\n";
        problem.correctAnswer = -1;
        return problem;
    }

    int a = rng->bounded(minVal, maxVal + 1);
    int b = rng->bounded(minVal, maxVal + 1);

    problem.text = QString(
        "=== Задание Task3 #%1 ===\r\n"
        "Чему равна сумма %2 + %3?\r\n"
    ).arg(num).arg(a).arg(b);

    problem.correctAnswer = a + b;
    return problem;
}
