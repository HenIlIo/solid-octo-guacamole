#ifndef TASK2_H
#define TASK2_H

#include <QString>

// text: текст условия (сумма двух чисел).
// correctAnswer: правильный ответ (сумма a + b).
struct Task2Problem {
    QString text;
    int correctAnswer;
};

// Задание: "Чему равна сумма a + b?" с рандомными числами.
Task2Problem task2_generate(int num);

#endif // TASK2_H
