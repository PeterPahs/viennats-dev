#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "worker.h"

#include <QOpenGLWindow>
#include <QSurfaceFormat>
#include <QOpenGLFunctions>
#include <QThread>
#include <QVector4D>
#include <QVector>
#include <QWidget>
#include <QtGui>

#include <GL/glu.h>

#include <iostream>
#include <math.h>


//#include "kernel.hpp"


class Visualization : public QOpenGLWindow
{
    Q_OBJECT

    QThread* thread;
    Worker* worker;
    

public:

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
        
		QSurfaceFormat format;
		format.setProfile(QSurfaceFormat::CompatibilityProfile); //to be on the save side, core profile with version 3.2 is faster
		format.setVersion(2,1);
		setFormat(format);
		
		QOpenGLContext *context = new QOpenGLContext;
		context->setFormat(format);
		context->create();
		context->makeCurrent(this);
		context->functions();
        
    }

    ~Visualization(){
		//OpenGL Clean Up
        //makeCurrent();
        //doneCurrent();
        
        worker->abort();
        thread->wait();
        qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
        delete thread;
        delete worker;

    }
    
protected:
	virtual void initializeGL(){
		
		glEnable(GL_DEPTH_TEST);
		resizeGL(this->width(), this->height());
	}
	
	virtual void resizeGL(int w, int h){
		//set viewport
		glViewport(0,0,w,h);
		//get window aspect ratio
		qreal ratio = qreal(w)/qreal(h);
		
		//initialize projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluPerspective(75, ratio, 0.1, 100000); //perspective projection plane - usees glu library
		
		//initialize modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	virtual void paintGL(){
		//set background color
		glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//reset modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		//'camera' position
		gluLookAt(80, 80, 80, 0, 0, 0, 0, 0, 1); //3x eye position, 3x look at, 3x up vector
		
		QVector4D temp;
		for(int i=0; i<PointVector.size(); ++i){
			temp = PointVector.at(i);
			glPushMatrix();
			glTranslatef(temp.x(), temp.y(), temp.z());
			solidSphere(1.0, 10, 10, temp.w());
			glPopMatrix();
		}
		
	}
	
	void paintEvent(QPaintEvent *event){
		paintGL();
		this->update();
	}
	
	void resizeEvent(QResizeEvent *event){
		resizeGL(this->width(), this->height());
		this->update();
	}
	
	
public:
	void addPoint(float x, float y, float z, float dist){
		if(fabsf(dist) <= 1){
			PointVector.append(QVector4D(x, y, z, dist));
		}
	}
	
private:
	void solidSphere(GLdouble radius, GLint slices, GLint stacks, float dist){
		if(dist < 0.f){
			glColor3f(1.0f, 0.f, 0.f); //red if dist is negative
		}
		else if(dist > 0.f){
			glColor3f(0.f, 1.0f, 0.f); //green if dist is positive
		}
		else {
			glColor3f(0.f, 0.f, 0.f); //black if dist is zero
		}
		
		//glColor3f(1.0f, 0.f, 0.f);
		glBegin(GL_LINE_LOOP);
		GLUquadricObj* quadric = gluNewQuadric();
		gluQuadricDrawStyle(quadric, GLU_FILL);
		gluSphere(quadric, radius, slices, stacks);
		gluDeleteQuadric(quadric);
		glEnd();
	}
	
	QVector<QVector4D> PointVector;
	
};


namespace lvlset{
    //Pass points to Qt in visualization.hpp
    template <class LevelSetType>
    void create_visual(const LevelSetType& ls, Visualization& window){

        for(typename LevelSetType::const_iterator_runs it(ls); !it.is_finished(); it.next()){
            if(it.is_active()){
                window.addPoint((float)it.start_indices(0), (float)it.start_indices(1), (float)it.start_indices(2), (float)it.value2());
            }
        }

    }

}


#endif // VISUALIZATION_H
