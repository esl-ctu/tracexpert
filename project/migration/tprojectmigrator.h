#ifndef TPROJECTMIGRATOR_H
#define TPROJECTMIGRATOR_H

#include <QDomDocument>

class TProjectMigrator
{
public:
    static bool migrate(QDomDocument & doc, QString * errorMessage);

    static int schemaVersion();

private:
    using MigrationFunction = std::function<bool (QDomDocument &)>;

    static const QVector<MigrationFunction> & migrations();
};

#endif // TPROJECTMIGRATOR_H
