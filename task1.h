#ifndef TASK1_H
#define TASK1_H

#include <QString>

// text: текст условия задачи (интеграл, результаты двух методов).
// correctAnswer: 1 если точнее метод трапеций, 2 если точнее метод Симпсона.
struct Task1Problem {
    QString text;          // Описание задания
    int correctAnswer;     // 1 = трапеции точнее, 2 = Симпсон точнее
};


// Каждый номер  отдельный шаблон интеграла со своими параметрами.
// Параметры рандомизируются при каждом вызове.
// Возвращает структуру Task1Problem с текстом задания и правильным ответом.
Task1Problem task1_generate(int num);

#endif // TASK1_H
