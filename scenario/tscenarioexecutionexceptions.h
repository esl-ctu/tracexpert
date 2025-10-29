#ifndef TSCENARIOEXECUTIONEXCEPTIONS_H
#define TSCENARIOEXECUTIONEXCEPTIONS_H

#include <QException>

class ScenarioHaltRequestedException : public QException
{
public:
    void raise() const override { throw *this; }
    ScenarioHaltRequestedException *clone() const override { return new ScenarioHaltRequestedException(*this); }
};

class ScenarioExecutionException : public QException
{
public:
    void raise() const override { throw *this; }
    ScenarioExecutionException *clone() const override { return new ScenarioExecutionException(*this); }
};


#endif // TSCENARIOEXECUTIONEXCEPTIONS_H
