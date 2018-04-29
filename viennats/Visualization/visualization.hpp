#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "worker.h"

#include <QThread>
#include <QVector4D>
#include <QVector>
#include <QWidget>
#include <QtGui>
#include <QMainWindow>
#include <QScreen>
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

        initGraph();
    }

    ~Visualization(){
      delete graph;
        worker->abort();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;

    }

  void initGraph(){
    graph = new Q3DScatter();
    QWidget *container = QWidget::createWindowContainer(graph);

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width()/2, screenSize.height()/2));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);
    container->setWindowTitle(QStringLiteral("ViennaTS"));

    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

    QScatterDataProxy *proxy = new QScatterDataProxy;
    QScatter3DSeries *seriesPos = new QScatter3DSeries(proxy);
    seriesPos->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    seriesPos->setBaseColor(Qt::green);
    seriesPos->setMesh(QAbstract3DSeries::MeshSphere);
    seriesPos->setMeshSmooth(false);
/*
    QScatter3DSeries *seriesNeg = new QScatter3DSeries(proxy);
    seriesNeg->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    seriesNeg->setBaseColor(Qt::red);
    seriesNeg->setMesh(QAbstract3DSeries::MeshSphere);
    seriesNeg->setMeshSmooth(false);

    QScatter3DSeries *seriesZero = new QScatter3DSeries(proxy);
    seriesZero->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    seriesZero->setBaseColor(Qt::black);
    seriesZero->setMesh(QAbstract3DSeries::MeshSphere);
    seriesZero->setMeshSmooth(false);
*/
    graph->addSeries(seriesPos);
/*    graph->addSeries(seriesNeg);
    graph->addSeries(seriesZero);
*/
    //addData();

    container->show();
  }

  void addData(){
    graph->axisX()->setTitle("X");
    graph->axisY()->setTitle("Y");
    graph->axisZ()->setTitle("Z");


    //Add the points to graph
    if(!PointVector.isEmpty()){
      QScatterDataArray *dat = new QScatterDataArray;
      dat->resize(PointVector.size());
      QScatterDataItem *ptr = &dat->first();
      for(int i=0; i<PointVector.size(); i++){
        ptr->setPosition(PointVector.at(i));
        ptr++;
      }
      graph->seriesList().at(0)->dataProxy()->resetArray(dat);
    }

  }

	void addPoint(float x, float y, float z, float dist){
		if(fabsf(dist) <= 1){
      PointVector.append(QVector3D(x,y,z));
    }
	}

private:
  QVector<QVector3D> PointVector;
  Q3DScatter *graph;
  QAbstract3DSeries::Mesh m_style;

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
        window.addData();
    }

}


#endif // VISUALIZATION_H
