#include <QtCharts>
#include <QChartView>
#include "QtCharts/qchartview.h"

class ChartView : public QChartView
{
    Q_OBJECT

public:
    explicit ChartView(QWidget *parent);
    ~ChartView() override;
    // 保存坐标区域，用于复位



    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;




private:
    QPoint m_lastPoint;
    bool m_isPress;
    bool m_ctrlPress;
    bool m_alreadySaveRange;
    double m_xMin, m_xMax, m_yMin, m_yMax;
    QGraphicsSimpleTextItem* m_coordItem;
};
