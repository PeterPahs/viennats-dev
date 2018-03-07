#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QWidget>
#include <QtGui>
#include <QVector>
#include <QVector4D>


class Visualization : public QWidget
{
    Q_OBJECT

public:
    //explicit Visualization();
    //~Visualization();
    
    //Add point to plot
    //TODO use qreal = double instead of float
    void addPoint(float x, float y, float z, float dist){
		if(abs(dist) <= 1){
			PointVector.append(QVector4D(x,y,z,dist));
		}
    }

    void clearPoints(){
        PointVector.clear();
    }

protected:
    void paintEvent(QPaintEvent *){
        //create QPainter
        QPainter painter(this);
        QPen pen(Qt::black);

        //center coord.
        QMatrix mat;
        mat.translate(width()/2, height()/2);
        mat.scale(1, -1); //Depending on range of points, negative y axis for cartesian
        painter.setMatrix(mat);


        //Draw coord.
        pen.setWidthF(0.1f);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(0,0,10,0);
        painter.drawLine(0,0,0,10);
        
        //Draw points
        //If positive -> color green, else red
        //If dist = 0 -> color black, size 0.5f
        //size of point = dist = tPoint.w()
        QVector4D tPoint;
        foreach (tPoint, PointVector) {
            if(tPoint.w() > 0){
                pen.setColor(Qt::green);
                pen.setWidthF(tPoint.w());
            }
            else if(tPoint.w() < 0){
                pen.setColor(Qt::red);
                pen.setWidthF(-tPoint.w());
            }
            else{
				pen.setColor(Qt::black);
				pen.setWidthF(0.5f);
			}
            painter.setPen(pen);
            painter.drawPoint(tPoint.toPointF()); //Only takes x,y as coordinates
        }


    }

public:
    QVector<QVector4D> PointVector; //Only stores float, need struct with qreal TODO
};

/*
 * #include "visualization.hpp"
#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Visualization window;

    window.addPoint(QVector4D(4,5,0,0.3f));
    window.addPoint(QVector4D(-2,-1,2,-1));

    window.setWindowTitle("viennats");
    window.show();

    return app.exec();
}
* */


#endif // VISUALIZATION_H
