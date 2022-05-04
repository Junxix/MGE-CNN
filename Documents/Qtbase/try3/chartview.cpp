#include "chartview.h"


ChartView::ChartView(QWidget *parent):QChartView(parent)
{

}

ChartView::~ChartView()
{

}





void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_lastPoint = event->pos();
        m_isPress = true;
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{

}



void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPress = false;
    if (event->button() == Qt::RightButton)
    {
        if (m_alreadySaveRange)
        {
            this->chart()->axisX()->setRange(m_xMin, m_xMax);
            this->chart()->axisY()->setRange(m_yMin, m_yMax);
        }
    }
}

void ChartView::wheelEvent(QWheelEvent *event)
{
   const QPointF curPos = event->position();
   QPointF curVal = this->chart()->mapToValue(QPointF(curPos));

   if (!m_alreadySaveRange)
   {
    //this->saveAxisRange();
    //m_alreadySaveRange = true;
   }
   const double factor = 1.5;//缩放比例
   if (m_ctrlPress)
   {//Y轴
    QValueAxis *axisY = dynamic_cast<QValueAxis*>(this->chart()->axisY());
    const double yMin = axisY->min();
    const double yMax = axisY->max();
    const double yCentral = curVal.y();

    double bottomOffset;
    double topOffset;
    if (event->inverted())
    {//放大
        bottomOffset = 1.0 / factor * (yCentral - yMin);
        topOffset = 1.0 / factor * (yMax - yCentral);
    }
    else
    {//缩小
        bottomOffset = 1.0 * factor * (yCentral - yMin);
        topOffset = 1.0 * factor * (yMax - yCentral);
    }

    this->chart()->axisY()->setRange(yCentral - bottomOffset, yCentral + topOffset);
   }
   else
   {//X轴
    QValueAxis *axisX = dynamic_cast<QValueAxis*>(this->chart()->axisX());
    const double xMin = axisX->min();
    const double xMax = axisX->max();
    const double xCentral = curVal.x();

    double leftOffset;
    double rightOffset;
    if (event->inverted())
    {//放大
        leftOffset = 1.0 / factor * (xCentral - xMin);
        rightOffset = 1.0 / factor * (xMax - xCentral);
    }
    else
    {//缩小
        leftOffset = 1.0 * factor * (xCentral - xMin);
        rightOffset = 1.0 * factor * (xMax - xCentral);
    }
    this->chart()->axisX()->setRange(xCentral - leftOffset, xCentral + rightOffset);
   }
}

