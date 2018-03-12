#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "worker.h"

#include <QWidget>
#include <QtGui>
#include <QVector>
#include <QVector4D>
#include <QThread>
#include <iostream>


//#include "kernel.hpp"


class Visualization : public QWidget
{
    Q_OBJECT

    QThread* thread;
    Worker* worker;

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

    Visualization(char* inputFile){
        std::cout << "Constructing Visualization" << std::endl;
        // The thread and the worker are created in the constructor so it is always safe to delete them.
        thread = new QThread();
        worker = new Worker(inputFile);

        worker->moveToThread(thread);
        connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
        connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
        worker->requestWork();
    }

    ~Visualization(){
        worker->abort();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;
    }

protected:
    void paintEvent(QPaintEvent *){
        std::cout << "Repaint" << std::endl;
        //create QPainter
        QPainter painter(this);
        QPen pen(Qt::black);

        //center coord.
        QMatrix mat;
        mat.translate(width()/2, height()/2);
        mat.scale(5, -5); //Depending on range of points, negative y axis for cartesian
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
                pen.setWidthF(1);
            }
            else if(tPoint.w() < 0){
                pen.setColor(Qt::red);
                pen.setWidthF(1);
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


namespace lvlset{
    //Pass points to QtWidget in visualization.hpp
    template <class LevelSetType>
    void create_visual(const LevelSetType& ls, Visualization& window){

        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
                //z-axis ignored, set to 0.0f
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), (float)it.start_indices(2), (float)it.value2());
            }
        }

    }
}

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
