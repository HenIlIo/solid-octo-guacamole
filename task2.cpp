#include "task2.h"
#include <QRandomGenerator>

// Диапазон рандомизации зависит от номера задания:
//   1-10:  числа от 1 до 20
//   11-20: числа от 1 до 50
//   21-30: числа от 1 до 100
//   31-40: числа от 1 до 200
Task2Problem task2_generate(int num)
{
    Task2Problem problem;
    auto* rng = QRandomGenerator::global();

    int minVal = 1, maxVal = 20;

    // Выбор диапазона в зависимости от номера задания
    if (num >= 1 && num <= 10)      { minVal = 1;  maxVal = 20; }
    else if (num >= 11 && num <= 20) { minVal = 1;  maxVal = 50; }
    else if (num >= 21 && num <= 30) { minVal = 1;  maxVal = 100; }
    else if (num >= 31 && num <= 40) { minVal = 1;  maxVal = 200; }
    else {
        // Некорректный номер
        problem.text = "error: invalid task number (1-40)\r\n";
        problem.correctAnswer = -1;
        return problem;
    }

    int a = rng->bounded(minVal, maxVal + 1);
    int b = rng->bounded(minVal, maxVal + 1);

    problem.text = QString(
        "=== Задание Task2 #%1 ===\r\n"
        "Чему равна сумма %2 + %3?\r\n"
    ).arg(num).arg(a).arg(b);

    problem.correctAnswer = a + b;
    return problem;
}
