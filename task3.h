#ifndef TASK3_H
#define TASK3_H

#include <QString>

// text: текст условия (сумма двух чисел).
// correctAnswer: правильный ответ (сумма a + b).
struct Task3Problem {
    QString text;
    int correctAnswer;
};

Task3Problem task3_generate(int num);

#endif // TASK3_H
