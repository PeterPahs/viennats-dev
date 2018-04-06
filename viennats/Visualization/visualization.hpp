#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "worker.h"

#include <QWidget>
#include <QtGui>
#include <QVector>
#include <QVector4D>
#include <QThread>
#include <QMatrix4x4>
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
        pen.setCapStyle(Qt::RoundCap);
        
        //create proj. matrix
        projMat = new QMatrix4x4(1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1);
        modelMat = new QMatrix4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
        
        
		
        //center coord.
        QMatrix mat;
        mat.translate(width()/2, height()/2); 
        mat.scale(3, -3); //Depending on range of points, negative y axis for cartesian
        painter.setMatrix(mat);


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
            
            QVector3D temp = tPoint.toVector3D();
            temp.project(modelMat, projMat, QRect(QPoint(-50,50),QPoint(50,-50)));
            
            painter.drawPoint(temp.toPointF()); //Only takes x,y as coordinates
        }


    }

private:
    QVector<QVector4D> PointVector;
    QMatrix4x4* projMat;
    QMatrix4x4* modelMat;
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
