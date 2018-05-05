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


    graph->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
    graph->setAspectRatio(1.0);
    graph->setHorizontalAspectRatio(0.0);
    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

    QScatterDataProxy *pProxy = new QScatterDataProxy; //proxy handles adding, removing, changing data items
    QScatterDataProxy *nProxy = new QScatterDataProxy;
    QScatterDataProxy *zProxy = new QScatterDataProxy;
    graph->addSeries(new QScatter3DSeries(pProxy)); //series holds the data/points for the graph.
    graph->addSeries(new QScatter3DSeries(nProxy));
    graph->addSeries(new QScatter3DSeries(zProxy));


    // Index 0: positive - green, 1: negative - red, 2: zero - black
    for(int i = 0; i<3; i++){
      graph->addSeries(new QScatter3DSeries);
      graph->seriesList().at(i)->setMesh(QAbstract3DSeries::MeshSphere);
      graph->seriesList().at(i)->setMeshSmooth(false);
      graph->seriesList().at(i)->setItemLabelFormat(QStringLiteral("@zTitle: @zLabel @xTitle: @xLabel @yTitle: @yLabel"));
      //graph->seriesList().at(i)->setItemLabelVisible(false);
    }
    graph->seriesList().at(0)->setBaseColor(Qt::green);
    graph->seriesList().at(1)->setBaseColor(Qt::red);
    graph->seriesList().at(2)->setBaseColor(Qt::black);
/*
    QScatterDataProxy *proxy = new QScatterDataProxy;
    QScatter3DSeries *seriesPos = new QScatter3DSeries(proxy);
    seriesPos->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    seriesPos->setBaseColor(Qt::green);
    seriesPos->setMesh(QAbstract3DSeries::MeshSphere);
    seriesPos->setMeshSmooth(false);

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

    graph->addSeries(seriesPos);
    graph->addSeries(seriesNeg);
    graph->addSeries(seriesZero);
*/
    //addData();

    container->show();
  }

  void addData(){
    //z now showing up, x towards eys
    graph->axisX()->setTitle("Y");
    graph->axisY()->setTitle("Z");
    graph->axisZ()->setTitle("X");

    if(pCount > 0){
      pDat->resize(pCount);
      pPtr = &pDat->first();
    }
    if(nCount > 0){
      nDat->resize(nCount);
      nPtr = &nDat->first();
    }
    if(zCount > 0){
      zDat->resize(zCount);
      zPtr = &zDat->first();
    }

    if(!PointVector.isEmpty()){
      QVector4D temp;
      for(int i=0; i<PointVector.size(); i++){
        temp = PointVector.at(i);
        if(temp.w() > 0){
          pPtr->setPosition(temp.toVector3D());
          pPtr++;
        }
        else if(temp.w() < 0){
          nPtr->setPosition(temp.toVector3D());
          nPtr++;
        }
        else{
          zPtr->setPosition(temp.toVector3D());
          zPtr++;
        }
      }
      graph->seriesList().at(0)->dataProxy()->resetArray(pDat);
      graph->seriesList().at(1)->dataProxy()->resetArray(nDat);
      graph->seriesList().at(2)->dataProxy()->resetArray(zDat);
    }
  /*
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
*/
  }

	void addPoint(float x, float y, float z, float dist){
		if(fabsf(dist) <= 1){
      PointVector.append(QVector4D(y,z,x, dist)); //x pointing towards eye, z up
      if(dist < 0){
        nCount++;
      }
      else if(dist > 0){
        pCount++;
      }
      else {
        zCount++;
      }
    }
	}

  void resetData(){
    if(!PointVector.isEmpty()){
      PointVector.clear();
    }
    pCount = 0;
    zCount = 0;
    nCount = 0;
  }


private:
  QVector<QVector4D> PointVector;
  Q3DScatter *graph;
  int pCount;
  int nCount;
  int zCount;
  QScatterDataArray *pDat = new QScatterDataArray;
  QScatterDataArray *nDat = new QScatterDataArray;
  QScatterDataArray *zDat = new QScatterDataArray;
  QScatterDataItem *pPtr;
  QScatterDataItem *nPtr;
  QScatterDataItem *zPtr;

};

namespace lvlset{
    //Pass points to Qt in visualization.hpp
    template <class LevelSetType>
    void create_visual(const LevelSetType& ls, const int D, Visualization& window){
        window.resetData();
        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
              if(D>2){
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), (float)it.start_indices(2), (float)it.value2());
              }
              else {
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), 0.f, (float)it.value2());
              }
            }
        }
        window.addData();
    }

}


#endif // VISUALIZATION_H
