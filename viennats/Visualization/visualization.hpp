#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "worker.h"

#include <QThread>
#include <QVector4D>
#include <QVector>
#include <QWidget>
#include <QtGui>
#include <QMainWindow>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>


#include <iostream>
#include <math.h>



using namespace QtDataVisualization;

class Visualization : public QMainWindow
{
    Q_OBJECT

    QThread* thread;
    Worker* worker;


public:

    Visualization(char* inputFile)
    {
        std::cout << "Constructing Visualization" << std::endl;
        // The thread and the worker are created in the constructor so it is always safe to delete them.
        thread = new QThread();
        worker = new Worker(inputFile);

        worker->moveToThread(thread);
        connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
        connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
        worker->requestWork();

        graph = new Q3DScatter();
        QWidget *container = QWidget::createWindowContainer(graph);

        QScatter3DSeries *series = new QScatter3DSeries;
        QScatterDataArray data;
        data << QVector3D(0.5f, 0.5f, 0.5f) << QVector3D(2.0f, 1.0f, 0.4f);
        series->dataProxy()->addItems(data);
        graph->addSeries(series);
        container->show();

    }

    ~Visualization(){
      delete graph;
        worker->abort();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;

    }

	void addPoint(float x, float y, float z, float dist){
		if(fabsf(dist) <= 1){
			PointVector.append(QVector4D(x, y, z, dist));
		}
	}

private:
	QVector<QVector4D> PointVector;
  Q3DScatter *graph;

};

namespace lvlset{
    //Pass points to Qt in visualization.hpp
    template <class LevelSetType>
    void create_visual(const LevelSetType& ls, Visualization& window){
        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
				//std::cout << it.value2() << std::endl;
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), (float)it.start_indices(2), (float)it.value2());
            }
        }

    }

}


#endif // VISUALIZATION_H
