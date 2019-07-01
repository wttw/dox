#ifndef DOX_SURVEY_H
#define DOX_SURVEY_H

#include <QWidget>
#include <QString>
#include <QMap>

#include "survey_data.h"

class QLabel;
class QPushButton;
class QTextEdit;
class QProgressBar;
class QNetworkAccessManager;

class SurveyData;

class Survey : public QWidget {
  Q_OBJECT
public:
    Survey(QWidget *parent=0);
    QString loadFile(const QString& filename);
public slots:
    void load();
    void save();
protected slots:
    void run();
    void surveyOne();
private:
    QString richText() const;
    QLabel *overview;
    QLabel *stats;
    QTextEdit *report;
    QPushButton *go;
    QProgressBar *progress;
    SurveyData data;
    QList<SurveyResult> queue;
    int queuePoint;
    QNetworkAccessManager *manager;
};

#endif
