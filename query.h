#ifndef DOX_QUERY_H
#define DOX_QUERY_H

#include <QWidget>
class QComboBox;
class QPushButton;
class QTextEdit;
class Resolvers;
class QLabel;

class Query : public QWidget {
  Q_OBJECT
public:
    Query(Resolvers *res, QWidget *parent=nullptr);
    void saveState();
protected slots:
    void run();
    void updateResolverAnno();
private:
    QString url();
    Resolvers *resolvers;
    QComboBox *resolver;
    QLabel *resolverAnno;
    QComboBox *type;
    QComboBox *name;
    QPushButton *go;
    QTextEdit *results;
};

#endif
