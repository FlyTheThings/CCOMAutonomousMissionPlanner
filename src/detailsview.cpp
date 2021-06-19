#include "detailsview.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include "autonomousvehicleproject.h"
#include "backgroundraster.h"
#include "backgrounddetails.h"
#include "waypoint.h"
#include "waypointdetails.h"
#include "trackline.h"
#include "tracklinedetails.h"
#include "surveypattern.h"
#include "surveypatterndetails.h"
#include "platform.h"
#include "platformdetails.h"
#include "behavior.h"
#include "behaviordetails.h"
#include <QInputDialog>

#include <QDebug>

DetailsView::DetailsView(QWidget *parent) : QWidget(parent), m_project(nullptr),currentWidget(nullptr)
{
    m_executePushButton = new QPushButton(this);
    m_executePushButton->setText("Execute");
    m_executePushButton->setDisabled(true);
    connect(m_executePushButton, &QPushButton::clicked, this, &DetailsView::onExecutePushButton_clicked );

    m_renamePushButton = new QPushButton(this);
    m_renamePushButton->setText("Rename");
    m_renamePushButton->setDisabled(true);
    connect(m_renamePushButton, &QPushButton::clicked, this, &DetailsView::onRenamedPushButton_clicked );

    m_clearTasksButton = new QPushButton(this);
    m_clearTasksButton->setText("Clear tasks");
    connect(m_clearTasksButton, &QPushButton::clicked, this, &DetailsView::onClearTasksPushButton_clicked);

    backgroundDetails = new BackgroundDetails(this);
    backgroundDetails->hide();
    waypointDetails = new WaypointDetails(this);
    waypointDetails->hide();
    trackLineDetails = new TrackLineDetails(this);
    trackLineDetails->hide();
    surveyPatternDetails = new SurveyPatternDetails(this);
    surveyPatternDetails->hide();
    platformDetails = new PlatformDetails(this);
    platformDetails->hide();
    behaviorDetails = new BehaviorDetails(this);
    behaviorDetails->hide();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_executePushButton);
    layout->addWidget(m_renamePushButton);
    layout->addWidget(m_clearTasksButton);
    layout->addWidget(backgroundDetails);
    layout->addWidget(waypointDetails);
    layout->addWidget(trackLineDetails);
    layout->addWidget(surveyPatternDetails);
    layout->addWidget(platformDetails);
    layout->addWidget(behaviorDetails);
    layout->addStretch();
    setLayout(layout);
}

QSize DetailsView::sizeHint() const
{
    if(currentWidget)
        return currentWidget->sizeHint();
    return QWidget::sizeHint();
}

void DetailsView::setProject(AutonomousVehicleProject *project)
{
    m_project = project;
}

void DetailsView::setCurrentWidget(QWidget *widget, bool canExecute)
{
    if(currentWidget != widget)
    {
        if(currentWidget)
            currentWidget->hide();
        currentWidget = widget;
        if(currentWidget)
        {
            currentWidget->show();
            updateGeometry();
            m_executePushButton->setEnabled(canExecute);
        }
        else
        {
            m_executePushButton->setEnabled(canExecute);
        }
    }
}

void DetailsView::onCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
    MissionItem* mi = m_project->itemFromIndex(current);
    if(mi)
    {
        m_renamePushButton->setEnabled(true);

        qDebug() << "metaobject class name: " << mi->metaObject()->className();
        QString itemType = mi->metaObject()->className();

        if (itemType == "BackgroundRaster")
        {
            BackgroundRaster *bg = qobject_cast<BackgroundRaster*>(mi);
            setCurrentWidget(backgroundDetails, bg->canBeSentToRobot());
            backgroundDetails->setBackgroundRaster(bg);
        }
        else if (itemType == "Waypoint")
        {
            Waypoint *wp = qobject_cast<Waypoint*>(mi);
            setCurrentWidget(waypointDetails, wp->canBeSentToRobot());
            waypointDetails->setWaypoint(wp);
        }
        else if (itemType == "TrackLine")
        {
            TrackLine *tl = qobject_cast<TrackLine*>(mi);
            setCurrentWidget(trackLineDetails, tl->canBeSentToRobot());
            trackLineDetails->setTrackLine(tl);
        }
        else if (itemType == "SurveyPattern")
        {
            SurveyPattern *sp = qobject_cast<SurveyPattern*>(mi);
            setCurrentWidget(surveyPatternDetails, sp->canBeSentToRobot());
            surveyPatternDetails->setSurveyPattern(sp);
        }
        else if (itemType == "Platform")
        {
            Platform *p = qobject_cast<Platform*>(mi);
            setCurrentWidget(platformDetails, p->canBeSentToRobot());
            platformDetails->setPlatform(p);
        }
        else if (itemType == "Behavior")
        {
            Behavior *b = qobject_cast<Behavior*>(mi);
            setCurrentWidget(behaviorDetails, b->canBeSentToRobot());
            behaviorDetails->setBehavior(b);
        }
        else
            setCurrentWidget(nullptr, mi->canBeSentToRobot());
    }
    else
    {
        m_renamePushButton->setEnabled(false);
        setCurrentWidget(nullptr, false);
    }

    m_project->setCurrent(current);
}

void DetailsView::onRenamedPushButton_clicked()
{
    MissionItem* mi = m_project->currentSelected();
    if(mi)
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Rename"),
                                         tr("New label:"), QLineEdit::Normal,
                                         mi->objectName(), &ok);
    if (ok && !text.isEmpty())
        mi->setObjectName(text);
    }
}

void DetailsView::onClearTasksPushButton_clicked()
{
    emit clearTasks();
}

void DetailsView::onExecutePushButton_clicked()
{
    MissionItem* mi = m_project->currentSelected();
    if(mi)
    {
        m_project->sendToROS(m_project->indexFromItem(mi));        
    }
}
