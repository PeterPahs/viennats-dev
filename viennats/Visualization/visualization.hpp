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
        thread = new QThread();
        worker = new Worker(inputFile);

        initGraph();

        worker->moveToThread(thread);
        connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
        connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
        worker->requestWork();



    }

    ~Visualization(){
      /**
      * The program is stuck at "thread->wait()" since "_abort" in Worker is never used.
      * To solve this we need shared memory, etc.
      * The window is closed, but ViennaTS still runs and sends data to visualization as deconstructor
      * is done only after ViennaTS is done.
      */
        worker->abort();
        thread->quit();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;
        delete graph;
        delete container;

    }

  void initGraph(){

    graph = new Q3DScatter();
    container = QWidget::createWindowContainer(graph);

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width()/2, screenSize.height()/2));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);
    //container->setWindowTitle(QStringLiteral("ViennaTS"));


    graph->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
    graph->setAspectRatio(1.0);
    graph->setHorizontalAspectRatio(0.0);
    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRight);

    QScatterDataProxy *pProxy = new QScatterDataProxy; //proxy handles adding, removing, changing data items
    QScatterDataProxy *nProxy = new QScatterDataProxy;
    QScatterDataProxy *zProxy = new QScatterDataProxy;
    graph->addSeries(new QScatter3DSeries(pProxy)); //series holds the data/points for the graph.
    graph->addSeries(new QScatter3DSeries(nProxy));
    graph->addSeries(new QScatter3DSeries(zProxy));


    // Index 0: positive - green, 1: negative - red, 2: zero - black
    for(int i = 0; i<3; i++){
      graph->addSeries(new QScatter3DSeries);
      graph->seriesList().at(i)->setMesh(QAbstract3DSeries::MeshPoint);
      //graph->seriesList().at(i)->userDefinedMesh(QString("myMesh"));
      graph->seriesList().at(i)->setMeshSmooth(false);
      graph->seriesList().at(i)->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
      //graph->seriesList().at(i)->setItemLabelVisible(false);
    }
    graph->seriesList().at(0)->setBaseColor(Qt::green);
    graph->seriesList().at(1)->setBaseColor(Qt::red);
    graph->seriesList().at(2)->setBaseColor(Qt::black);


    graph->axisX()->setTitle("X");
    graph->axisY()->setTitle("Y");
    graph->axisZ()->setTitle("Z");

    //isAxisSet = false;
    xMin = 0;
    xMax = 0;
    yMin = 0;
    yMax = 0;
    zMin = 0;
    zMax = 0;

    setCentralWidget(container);
    //container->show();
  }

  void addData(){

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


#ifdef VERBOSE
      std::cout << "Count of pos: " << graph->seriesList().at(0)->dataProxy()->itemCount() << std::endl;
      std::cout << "Count of neg: " << graph->seriesList().at(1)->dataProxy()->itemCount() << std::endl;
      std::cout << "Count of zer: " << graph->seriesList().at(2)->dataProxy()->itemCount() << std::endl;
      //std::cout << "Current FPS: " << graph->currentFps() << std::endl;
#endif

    }
  }

	void addPoint(qreal x, qreal y, qreal z, qreal dist, int open_boundary_direction, bool is_open_boundary_negative){
		if(fabs(dist) <= 1){
      //set Orientation
      if(open_boundary_direction == 0){
        if(!is_open_boundary_negative){
          PointVector.append(QVector4D(z,x,y, dist)); // x up
          if(!isAxisSet){
            getMinMax(z,x,y);
          }
        }
        else{
          PointVector.append(QVector4D(z, -x, -y, dist)); // x down
          if(!isAxisSet){
            getMinMax(z,-x,-y);
          }
        }
      }

      if(open_boundary_direction == 1){
        if(!is_open_boundary_negative){
          PointVector.append(QVector4D(x,y,z, dist)); // y up,
          if(!isAxisSet){
            getMinMax(x,y,z);
          }
        }
        else{
          PointVector.append(QVector4D(x, -y, -z, dist)); // y down
          if(!isAxisSet){
            getMinMax(x,-y,-z);
          }
        }
      }

      else{
        if(!is_open_boundary_negative){
          PointVector.append(QVector4D(y,z,x, dist)); // z up
          if(!isAxisSet){
            getMinMax(y,z,x);
          }
        }
        else{
          PointVector.append(QVector4D(y, -z, -x, dist)); // z down
          if(!isAxisSet){
            getMinMax(y,-z,-x);
          }
        }
      }


      if(dist < 0.0){
        nCount++;
      }
      else if(dist > 0.0){
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

  void setAxisFixed(){
    if(!isAxisSet){
      isAxisSet = true;
      //set fixed range for graph
      graph->axisX()->setAutoAdjustRange(false);
      graph->axisX()->setRange(xMin-3, xMax+3);
      graph->axisY()->setAutoAdjustRange(false);
      graph->axisY()->setRange(yMin-3, yMax+3);
      graph->axisZ()->setAutoAdjustRange(false);
      graph->axisZ()->setRange(zMin-3, zMax+3);
    }
  }


private:
  void getMinMax(qreal x, qreal y, qreal z){
    if(x < xMin){
      xMin = x;
    }
    else if(x > xMax){
      xMax = x;
    }

    if(y < yMin){
      yMin = y;
    }
    else if(y > yMax){
      yMax = y;
    }

    if(z < zMin){
      zMin = z;
    }
    else if(z > zMax){
      zMax = z;
    }
  }



private:
  QVector<QVector4D> PointVector;
  Q3DScatter *graph;
  QWidget *container;
  int pCount;
  int nCount;
  int zCount;
  QScatterDataArray *pDat = new QScatterDataArray;
  QScatterDataArray *nDat = new QScatterDataArray;
  QScatterDataArray *zDat = new QScatterDataArray;
  QScatterDataItem *pPtr;
  QScatterDataItem *nPtr;
  QScatterDataItem *zPtr;
  bool isAxisSet;
  qreal xMin, xMax, yMin, yMax, zMin, zMax;

};

namespace lvlset{
    //Pass points to Qt in visualization.hpp

    template <class LevelSetsType>
    void create_visual(const LevelSetsType& LevelSets, Visualization& window, int open_boundary_direction, bool is_open_boundary_negative){
      const int D=LevelSetsType::value_type::dimensions;

      window.resetData();

      //Iterate over all LevelSets
      typename LevelSetsType::const_iterator it=LevelSets.begin();
      for (unsigned int i=0;i<LevelSets.size();i++) {
        //If last LevelSet is added, draw the graph
        if (i!=LevelSets.size()-1){
          add_to_visual(*it, open_boundary_direction, is_open_boundary_negative, D, window);
        }
        else {
          add_to_visual(*it, open_boundary_direction, is_open_boundary_negative, D, window);
          window.addData();
        }
        it++;
      }
      window.setAxisFixed();
    }




    template <class LevelSetType>
    void add_to_visual(const LevelSetType& ls, int open_boundary_direction, bool is_open_boundary_negative, const int D, Visualization& window){
        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
              if(D>2){
                window.addPoint(it.start_indices(0), it.start_indices(1), it.start_indices(2), it.value2(), open_boundary_direction, is_open_boundary_negative);
              }
              else {
                window.addPoint(it.start_indices(0), it.start_indices(1), 0.0, it.value2(), open_boundary_direction, is_open_boundary_negative);
              }
            }
        }
    }

}


#endif // VISUALIZATION_H
