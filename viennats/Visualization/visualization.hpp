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
    //using float and not double due to defined Qt methods
    void addPoint(float x, float y, float z, float dist){
		if(abs(dist) <= 1){
			if(z != 0.0){
				x += 0.1*z;
				y += 0.5*z;
			}
			PointVector.append(QVector4D(x,y,z,dist));
			if(x < minScale){
				minScale = x;
			}
			if(y < minScale){
				minScale = y;
			}
			if(x > maxScale){
				maxScale = x;
			}
			if(y > maxScale){
				maxScale = y;
			}
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
        minScale = 0.0;
        maxScale = 0.0;
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
        pen.setCapStyle(Qt::RoundCap);
		
        //center coord.
        QMatrix mat;
        mat.translate(width()/2, height()/2);
        float scaleTemp = maxScale - minScale;
        scaleTemp = (width() / scaleTemp) / 2;
        mat.scale(scaleTemp, -scaleTemp); //Depending on range of points, negative y axis for cartesian
        painter.setMatrix(mat);


        /*Draw coord.
        pen.setWidthF(0.1f);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(0,0,10,0);
        painter.drawLine(0,0,0,10);
        * */

        //Draw points
        //If positive -> color green, else red
        //If dist = 0 -> color black, size 0.5f
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
    QVector<QVector4D> PointVector;
    float minScale, maxScale;
};


namespace lvlset{
    //Pass points to QtWidget in visualization.hpp
    template <class LevelSetType>
    void create_visual(const LevelSetType& ls, Visualization& window){

        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
                //cast values to float
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), (float)it.start_indices(2), (float)it.value2());
            }
        }

    }
}


#endif // VISUALIZATION_H
