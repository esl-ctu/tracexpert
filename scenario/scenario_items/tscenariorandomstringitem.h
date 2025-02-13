#ifndef TSCENARIORANDOMSTRINGITEM_H
#define TSCENARIORANDOMSTRINGITEM_H

#include <QString>
#include <QList>
#include <QDataStream>
#include <random>

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioRandomStringItem class represents a block that generates a random 4 character [a,b,c,d] string.
 *
 * It is currently only left here for evaluation/testing purposes.
 * TODO: remove this class.
 *
 */
class TScenarioRandomStringItem : public TScenarioItem {

public:
    enum { TItemClass = 20 };
    int itemClass() const override { return TItemClass; }

    TScenarioRandomStringItem() : TScenarioItem("Random strgen", "This block generates a random string.") {
        addFlowInputPort("flowIn");
        addDataOutputPort("dataOut");
        addFlowOutputPort("flowOut");
    }

    TScenarioItem * copy() const override {
        return new TScenarioRandomStringItem(*this);
    }

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        QStringList list{"a", "b", "c", "d"};
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(list.begin(), list.end(), g);
        QString randomString = list.join("");

        log("Random string generated! (" + randomString.toUtf8() + ")");

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), randomString.toUtf8());

        return outputData;
    }

};

#endif // TSCENARIORANDOMSTRINGITEM_H
