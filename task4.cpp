#include "task4.h"
#include <QRandomGenerator>

// Диапазон рандомизации зависит от номера задания (большие числа):
//   1-10:  числа от 100 до 500
//   11-20: числа от 100 до 1000
//   21-30: числа от 500 до 2000
//   31-40: числа от 1000 до 5000
Task4Problem task4_generate(int num)
{
    Task4Problem problem;
    auto* rng = QRandomGenerator::global();

    int minVal = 100, maxVal = 500;

    if (num >= 1 && num <= 10)      { minVal = 100;  maxVal = 500; }
    else if (num >= 11 && num <= 20) { minVal = 100;  maxVal = 1000; }
    else if (num >= 21 && num <= 30) { minVal = 500;  maxVal = 2000; }
    else if (num >= 31 && num <= 40) { minVal = 1000; maxVal = 5000; }
    else {
        problem.text = "error: invalid task number (1-40)\r\n";
        problem.correctAnswer = -1;
        return problem;
    }

    int a = rng->bounded(minVal, maxVal + 1);
    int b = rng->bounded(minVal, maxVal + 1);

    problem.text = QString(
        "=== Задание Task4 #%1 ===\r\n"
        "Чему равна сумма %2 + %3?\r\n"
    ).arg(num).arg(a).arg(b);

    problem.correctAnswer = a + b;
    return problem;
}
