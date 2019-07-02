#ifndef DOX_QUERY_H
#define DOX_QUERY_H

#include <QWidget>
class QComboBox;
class QPushButton;
class QTextEdit;
class Resolvers;

class Query : public QWidget {
  Q_OBJECT
public:
    Query(Resolvers *res, QWidget *parent=0);
    void saveState();
protected slots:
    void run();
private:
    Resolvers *resolvers;
    QComboBox *resolver;
    QComboBox *type;
    QComboBox *name;
    QPushButton *go;
    QTextEdit *results;
};

#endif
