#include "platform.h"
#include "ui_platform.h"
#include <QPainter>
#include "nav_source.h"
#include "backgroundraster.h"

#include <QDebug>

Platform::Platform(QWidget* parent, QGraphicsItem *parentItem):
  QWidget(parent),
  ShipTrack(parentItem),
  m_ui(new Ui::Platform)
{
  m_ui->setupUi(this);
  setAcceptHoverEvents(true);
  m_ui->geovizDisplay->setParentItem(this);
  setZValue(6.0);
}

Platform::~Platform()
{

}

QRectF Platform::boundingRect() const
{
  return (shape().boundingRect()).marginsAdded(QMargins(3,3,3,3));
}

void Platform::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  painter->save();
  QPen p;
  p.setCosmetic(true);
  p.setColor(m_color);
  p.setWidth(4);
  painter->setPen(p);
  painter->drawPath(shape());
  painter->restore();

}

QPainterPath Platform::shape() const
{
  QPainterPath ret;
  if(!m_nav_sources.empty())
  {
    BackgroundRaster* bg = findParentBackgroundRaster();
    if(bg)
      {
        bool forceTriangle = false;
        if (m_length == 0 || m_width == 0)
          forceTriangle = true;
        float max_size = std::max(m_length, m_width);
        qreal pixel_size = bg->scaledPixelSize();

        LocationPositionHeadingTime location, heading;
        for(const auto& ns: m_nav_sources)
        {
          auto possible_location = ns.second->location();
          if(possible_location.location.isValid() && possible_location.time > location.time)
            location = possible_location;
          auto possible_heading = ns.second->heading();
          if(!isnan(possible_heading.heading) && possible_heading.time > heading.time)
            heading = possible_heading;
        }

        if(pixel_size > max_size/10.0 || forceTriangle)
          drawTriangle(ret, bg, location.location, heading.heading, pixel_size);
        else
        {
          double half_width = m_width/2.0;
          double half_length = m_length/2.0;
          drawShipOutline(ret, bg, location.location, heading.heading, half_length - m_reference_x, half_width - m_reference_y, half_width + m_reference_y, half_length + m_reference_x);
        }
      }
  }
  return ret;
}

void Platform::update(project11_msgs::Platform& platform)
{
  if(objectName().toStdString() != platform.name)
  {
    setObjectName(platform.name.c_str());
  }
  std::string platformNamespace = platform.name;
  if(!platform.platform_namespace.empty())
    platformNamespace = platform.platform_namespace;
  m_ui->helmManager->updateRobotNamespace(platformNamespace.c_str());
  m_ui->missionManager->updateRobotNamespace(platformNamespace.c_str());
  m_ui->geovizDisplay->updateRobotNamespace(platformNamespace.c_str());

  m_width = platform.width;
  m_length = platform.length;
  m_reference_x = platform.reference_x;
  m_reference_y = platform.reference_y;

  if (platform.color.a > 0.0)
  {
    QColor color;
    color.setRedF(platform.color.r);
    color.setGreenF(platform.color.g);
    color.setBlueF(platform.color.b);
    color.setAlphaF(platform.color.a);
    setColor(color);
  }

  for(auto ns: platform.nav_sources)
    if(m_nav_sources.find(ns.name) == m_nav_sources.end())
    {
      m_nav_sources[ns.name] = new NavSource(ns, this, this);
      m_nav_sources[ns.name]->setHistoryDuration(7200);
      connect(m_nav_sources[ns.name], &NavSource::beforeNavUpdate, this, &Platform::aboutToUpdateNav);
      connect(m_nav_sources[ns.name], &NavSource::positionUpdate, this, &Platform::updatePosition);
      if(m_nav_sources.size() == 1)
        connect(m_nav_sources[ns.name], &NavSource::sog, this, &Platform::updateSog);
      m_nav_sources[ns.name]->setColor(m_color);
    }

}

void Platform::update(std::pair<const std::string, XmlRpc::XmlRpcValue> &platform)
{
  if(objectName().toStdString() != platform.first)
  {
    setObjectName(platform.first.c_str());
  }
  std::string platformNamespace = platform.first;
  if(platform.second.hasMember("namespace"))
    platformNamespace = std::string(platform.second["namespace"]);
  m_ui->helmManager->updateRobotNamespace(platformNamespace.c_str());
  m_ui->missionManager->updateRobotNamespace(platformNamespace.c_str());
  m_ui->geovizDisplay->updateRobotNamespace(platformNamespace.c_str());

  if(platform.second.hasMember("width"))
    m_width = double(platform.second["width"]);
  if(platform.second.hasMember("length"))
    m_length = double(platform.second["length"]);
  if(platform.second.hasMember("reference_x"))
    m_reference_x = double(platform.second["reference_x"]);
  if(platform.second.hasMember("reference_y"))
    m_reference_y = double(platform.second["reference_y"]);
  if(platform.second.hasMember("nav_sources"))
    for(auto nav: platform.second["nav_sources"])
    {
      m_nav_sources[nav.first] = new NavSource(nav, this, this);
      m_nav_sources[nav.first]->setHistoryDuration(7200);
      connect(m_nav_sources[nav.first], &NavSource::beforeNavUpdate, this, &Platform::aboutToUpdateNav);
      connect(m_nav_sources[nav.first], &NavSource::positionUpdate, this, &Platform::updatePosition);
      if(m_nav_sources.size() == 1)
        connect(m_nav_sources[nav.first], &NavSource::sog, this, &Platform::updateSog);
    }
  if(platform.second.hasMember("color"))
  {
    auto color = platform.second["color"];

    QColor c(255,255,255);
    if(color.hasMember("red"))
      c.setRedF(color["red"]);
    if(color.hasMember("green"))
      c.setGreenF(color["green"]);
    if(color.hasMember("blue"))
      c.setBlueF(color["blue"]);
    if(color.hasMember("alpha"))
      c.setAlphaF(color["alpha"]);
    setColor(c);
  }
}


void Platform::updateProjectedPoints()
{
  prepareGeometryChange();
  setPos(0,0);
  for(auto ns: m_nav_sources)
    ns.second->updateProjectedPoints();
  m_ui->geovizDisplay->updateProjectedPoints();

}

void Platform::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  updateLabel();
  setShowLabelFlag(true);
}

void Platform::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  setShowLabelFlag(false);
}

void Platform::updateLabel()
{
  setLabel(objectName());
  if(!m_nav_sources.empty())
  {
    setLabelPosition(m_nav_sources.begin()->second->location().pos);
  }
}

void Platform::aboutToUpdateNav()
{
  prepareGeometryChange();
}

void Platform::updateSog(double sog)
{
  // 1852m per NM
  m_sog = sog*1.9438;
  m_sog_history.append(m_sog);
  if(m_sog_history.length() > 200)
      m_sog_history.pop_front();
  qreal sog_sum = 0;
  for(auto s: m_sog_history)
      sog_sum += s;
  m_sog_avg = sog_sum/m_sog_history.length();
  QString sogLabel = "SOG: " + QString::number(m_sog,'f',1) + ", avg: " + QString::number(m_sog_avg,'f',1) + " (200 samples)";
  m_ui->sogLineEdit->setText(sogLabel);
}

MissionManager* Platform::missionManager() const
{
  return m_ui->missionManager;
}

HelmManager* Platform::helmManager() const
{
  return m_ui->helmManager;
}

void Platform::updatePosition(QGeoCoordinate position)
{
  emit platformPosition(this, position);
  }

void Platform::setColor(QColor color)
{
  m_color = color;
  QColor dim;
  dim.setRedF(color.redF()*.8);
  dim.setGreenF(color.greenF()*.8);
  dim.setBlueF(color.blueF()*.8);
  dim.setAlphaF(color.alphaF()*.8);
  for(auto nav: m_nav_sources)
    nav.second->setColor(dim);
}
