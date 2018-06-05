/*
  Qt class for the visualization of LevelSets in ViennaTS.

  Usage of QtDataVisualization module under GNU General Public License v3 and
  Qt under LGPL v3 License.
  The Qt Company - www.qt.io
  License - www.gnu.org/licenses/
*/

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
#include <QCloseEvent>
#include <QMessageBox>
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
    //Constructor
    Visualization(char* inputFile)
    {
      //Spawn GUI thread and worker thread
        thread = new QThread();
        worker = new Worker(inputFile);
        //initialize the graph
        initGraph();
        //connect the signals and slots
        worker->moveToThread(thread);
        connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
        connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

        worker->requestWork();

    }

    //Destructor
    ~Visualization(){

        worker->abort();
        thread->quit();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;
        delete graph;
        delete container;

    }

    /**
    *@brief Set up the graph
    *
    * Create a Q3DScatter graph and a container that holds it. Configure display and render properties and initialize
    * variables.
    * This method is called only once from the constructor.
    */
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
      graph->seriesList().at(i)->setMeshSmooth(false);
      //graph->seriesList().at(i)->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
      //graph->seriesList().at(i)->setItemLabelVisible(false);
    }
    graph->seriesList().at(0)->setBaseColor(Qt::green);
    graph->seriesList().at(1)->setBaseColor(Qt::red);
    graph->seriesList().at(2)->setBaseColor(Qt::black);


    graph->axisX()->setTitle("X");
    graph->axisY()->setTitle("Y");
    graph->axisZ()->setTitle("Z");

    isAxisSet = false;
    xMin = 0;
    xMax = 0;
    yMin = 0;
    yMax = 0;
    zMin = 0;
    zMax = 0;

    setCentralWidget(container);
    winOpen = true;
  }

  /**
  * @brief Add the LevelSet points to the graph and display them.
  *
  */
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

#endif

    }
  }

  /**
  * @brief Take LevelSet values and insert them to the internal list (PointVector).
  *
  * Take the x, y and z coordinates and rearange them according to open_boundary_direction and its sign.
  * Add coordinates to PointVector only if its absolute distance is less or equal to 1. Also the number of positive,
  * negative and 'zero' distance points is updated.
  * If isAxisSet is false, that is, the method is called the first time, getMinMax is called to determine a boundary for
  * the graph dispay area. Afterwards the axis scale can't be changed.
  */
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

  /**
  * @brief Clear the PointVector and reset the count values.
  */
  void resetData(){
    if(!PointVector.isEmpty()){
      PointVector.clear();
    }
    pCount = 0;
    zCount = 0;
    nCount = 0;
  }

  /**
  * @brief Set the axis range and mark them fixed.
  *
  * After the first LevelSets are rendered, this method fixes the axis range depending on the
  * minimum and maximum positions of each coordinate inkl. a small offset. To ensure that the axis are only
  * set once, the boolean variable isAxisSet is set to true at the first call.
  */
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

  /**
  * @brief Check if window is open or closed. Used only for process interruption.
  */
  bool windowOpen(){
    return winOpen;
  }


private:

  /**
  *@brief Calculate the minimum and maximum values of each coordinate.
  */
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

  /**
  *@brief Handels the close window event by setting the variable winOpen to false.
  */
  void closeEvent(QCloseEvent *event){
    qDebug() << "close event triggerd in "<<this->QObject::thread()->currentThreadId();
    winOpen = false;
    //worker->abort();
  }

private:
  QVector<QVector4D> PointVector; //holds the LevelSet Points
  Q3DScatter *graph; //the displayed graph
  QWidget *container; //container holding the graph
  int pCount; //counter for number of Points in PointVector with positive distance
  int nCount; //counter negative distance
  int zCount; //counter no distance
  QScatterDataArray *pDat = new QScatterDataArray; //required structures for Q3DScatter graph
  QScatterDataArray *nDat = new QScatterDataArray;
  QScatterDataArray *zDat = new QScatterDataArray;
  QScatterDataItem *pPtr;
  QScatterDataItem *nPtr;
  QScatterDataItem *zPtr;
  bool isAxisSet; //used to ensure that axis are only set once
  qreal xMin, xMax, yMin, yMax, zMin, zMax; //used get range for axis
  bool winOpen; //true if window is open, else false

};

namespace lvlset{
    /**
    *@brief Pass LevelSets to Qt in visualization.hpp
    */
    template <class LevelSetsType>
    void create_visual(const LevelSetsType& LevelSets, Visualization& window, int open_boundary_direction, bool is_open_boundary_negative){
      const int D=LevelSetsType::value_type::dimensions;

      //clear the graph data from the previous call
      window.resetData();

      //Iterate over all LevelSets
      //If last LevelSet is added, draw the graph
      typename LevelSetsType::const_iterator it=LevelSets.begin();
      for (unsigned int i=0;i<LevelSets.size();i++) {
        if (i!=LevelSets.size()-1){
          add_to_visual(*it, open_boundary_direction, is_open_boundary_negative, D, window);
        }
        else {
          add_to_visual(*it, open_boundary_direction, is_open_boundary_negative, D, window);
          window.addData();
        }
        it++;
      }
      //fix the axis scale. Effect only on first call.
      window.setAxisFixed();
    }


    /**
    *@brief Add points of a LevelSet to Qt in visualization.hpp.
    */
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
