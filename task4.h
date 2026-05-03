#ifndef TASK4_H
#define TASK4_H

#include <QString>

// text: текст условия (сумма двух чисел).
// correctAnswer: правильный ответ (сумма a + b).
struct Task4Problem {
    QString text;
    int correctAnswer;
};

Task4Problem task4_generate(int num);

#endif // TASK4_H
