#ifndef SURVEYPATTERNDETAILS_H
#define SURVEYPATTERNDETAILS_H

#include <QWidget>

namespace Ui {
class SurveyPatternDetails;
}

class SurveyPattern;

class SurveyPatternDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SurveyPatternDetails(QWidget *parent = 0);
    ~SurveyPatternDetails();

    void setSurveyPattern(SurveyPattern *surveyPattern);

public slots:
    void onSurveyPatternUpdated();


private slots:
    void on_headingDoubleSpinBox_valueChanged();

    void on_lineSpacingEdit_editingFinished();

    void on_lineLengthLineEdit_editingFinished();

    void on_totalWidthLineEdit_editingFinished();
    
    void on_maxSegmentLengthLineEdit_editingFinished();
    
    void on_alignmentComboBox_activated();

private:
    Ui::SurveyPatternDetails *ui;

    SurveyPattern * m_surveyPattern;
    QMetaObject::Connection m_connection;
    bool updating;
    bool m_updating_ui = false;

    void updateSurveyPattern();
};

#endif // SURVEYPATTERNDETAILS_H
