/*
   Copyright 2013 Fabien Pierre-Nicolas.
      - Primarily authored by Fabien Pierre-Nicolas

   This file is part of simple-qt-thread-example, a simple example to demonstrate how to use threads.
   This example is explained on http://fabienpn.wordpress.com/qt-thread-simple-and-stable-with-sources/

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This progra is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "worker.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

#include <iostream>

Worker::Worker(char* inputFile, QObject *parent) :
    inputFile(inputFile), QObject(parent)
{
    _working =false;
    _abort = false;
    std::cout << "initialized worker: " << inputFile << std::endl;
}

void Worker::requestWork()
{
    mutex.lock();
    _working = true;
    _abort = false;
    std::cout <<"Request worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();

    emit workRequested();
}
/**
* Sets _abort=true, but never used since no loop in doWork
*/
void Worker::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        std::cout <<"Request worker aborting in Thread "<<thread()->currentThreadId() << std::endl;
    }
    mutex.unlock();
}


void Worker::doWork()
{
    std::cout <<"Starting worker process in Thread "<<thread()->currentThreadId();

    start_vts(inputFile);
    /**
    * Program executes even if _abort=true.
    * See Visualzation.hpp for more details
    */

    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();

    qDebug()<<"Worker process finished in Thread "<<thread()->currentThreadId();
    //std::cout << std::endl << "Close Graph Window to continue." << std::endl;

    //Once 60 sec passed, the finished signal is sent
    emit finished();
}
